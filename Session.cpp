#include "Session.hpp"
#include "LoggerModule.hpp"

namespace asio = boost::asio; // for cleaner code

Session::Session(asio::basic_stream_socket<asio::ip::tcp> socket,
                 const std::string &folder_path)
    : socket_(std::move(socket)),
      file_handler_(folder_path), requestBuffer_{0}, header_buffer_{0}
// Initialize header_buffer_ with size 8
// TODO - finish init for examp msg
{}

void Session::start() {
  DEBUG_LOG("start processing request, socket: {}", socket_.native_handle());
  readHeader();
}

void Session::readHeader() {
  auto self(shared_from_this());
  asio::async_read(
      socket_, asio::buffer(header_buffer_, Message::HEADER_LENGTH),
      [this, self](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
          if (length != Message::HEADER_LENGTH) {
            ERROR_LOG("Unexpected number of bytes read: {}", length);
          }
          message_.setHeaderBuffer(header_buffer_);
          if (message_.parseFixedHeader()) {
            // chek if OP_GET_FILE_LIST - no need for more data
            if (message_.getOperationCode() == OP_GET_FILE_LIST) {
              // get file list from file_handler_
              std::vector<char> response_buf =
                  file_handler_.get_file_list(message_.getUserId());

              // Send the response to the client
              send_response(
                  std::string(response_buf.begin(), response_buf.end()));

              return;
            }

            // contine fllow to read dynamic size name
            readFilename();
          } else {
            ERROR_LOG("Error in parsing header. Is massage valid?", "");
          }
        } else {
          ERROR_LOG("Error in reading header from socket: ", ec.message());
        }
      });
}

// the function tries to read dynamic size name based on name_len_ field
// it stores the name in message_.filename_,
// after processing the name in the msg buffer_
void Session::readFilename() {
  auto self(shared_from_this());
  message_.getBuffer().resize(message_.getNameLength());
  try
  {
    asio::async_read(
        socket_, asio::buffer(message_.getBuffer(), message_.getNameLength()),
        [this, self](boost::system::error_code ec, std::size_t length) {
          if (!ec) {
            if (length != message_.getNameLength()) {
              ERROR_LOG("Unexpected number of bytes read: {}", length);
            }
            // file name stored successfully in msg buffer_
            message_.setFilename();

            // AFTER filename is read and set:
            if (message_.getOperationCode() == OP_SAVE_FILE) {
              // start by getting file size from payload
              readFileSize();
              return;
            } else if (message_.getOperationCode() == OP_RESTORE_FILE) {
              // get file content from file_handler_
              std::vector<char> response_buf = file_handler_.restore_file(
                  message_.getUserId(), message_.getFilename());

              // Send the response to the client
              send_response(
                  std::string(response_buf.begin(), response_buf.end()));
              return;
            } else if (message_.getOperationCode() == OP_DELETE_FILE) {
              // delete file from file_handler_
              std::vector<char> response_buf2 = file_handler_.delete_file(
                  message_.getUserId(), message_.getFilename());

              send_response(
                  std::string(response_buf2.begin(), response_buf2.end()));
              return;
            } else {
              ERROR_LOG("Error: invalid op code: {}", message_.getOperationCode());
            }
          } else {
            ERROR_LOG("Error in reading filename from socket: ", ec.message());
          }
        });
  }
  catch (const std::exception &e)
  {
    ERROR_LOG("Error in reading filename from socket: ", e.what());
  }
}

void Session::readFileSize()
{
  auto self(shared_from_this());

  asio::async_read(
      socket_,
      asio::buffer(message_.getFileSizeBuffer(), Message::FILE_SIZE_BUFFER_LENGTH),
      [this, self](boost::system::error_code ec, std::size_t length)
      {
        if (!ec)
        {
          DEBUG_LOG("\x1b[32mSuccessfully read {} bytes for file size.\x1b[0m", length); // Green color

          if (length != Message::FILE_SIZE_BUFFER_LENGTH)
          {
            WARN_LOG("\x1b[33mExpected {} bytes but got {} bytes for file size.\x1b[0m", Message::FILE_SIZE_BUFFER_LENGTH, length); // Yellow color
          }

          message_.setFileSize();

          // ready to read file content
          readPayload();
        }
        else
        {
          ERROR_LOG("Error reading file size: ", ec.message());
          ERROR_LOG("\x1b[31mError reading file size: {}\x1b[0m", ec.message()); // Red color
        }
      });
}
void Session::readPayload()
{
  auto self(shared_from_this());

  // Resize the buffer to hold the name and the file content
  message_.getBuffer().resize(message_.getNameLength() + message_.getFileSize());

  // Debug: Log the buffer size
  DEBUG_LOG("Resized buffer to {} bytes.", message_.getBuffer().size());

  // Create an iterator to point to the start of where the file content should be
  auto it = message_.getBuffer().begin() + message_.getNameLength();

  asio::async_read(
      socket_, asio::buffer(&(*it), message_.getFileSize()),
      [this, self](boost::system::error_code ec, std::size_t length)
      {
        if (!ec)
        {
          DEBUG_LOG("Successfully read {} bytes.", length);

          if (length != message_.getFileSize())
          {
            WARN_LOG("Expected {} bytes but got {} bytes.", message_.getFileSize(), length);
          }


          // Debugging logs
          DEBUG_LOG("Saving file: User ID: {}, Filename: {}, Expected Size: {} bytes.",
                        message_.getUserId(),
                        message_.getFilename(),
                        message_.getFileSize());

          const auto &content = message_.getFileContent();
          DEBUG_LOG("Actual content size being passed to file handler: {} bytes.", content.size());

          if (content.empty())
          {
            WARN_LOG("Content is empty!", "");
          }

          std::string firstFewBytesOfContent(content.begin(), content.begin() + std::min<std::size_t>(content.size(), 10));
          DEBUG_LOG("First few characters of file content: {}", firstFewBytesOfContent);

          // store file content in file_contents_
          message_.setFileContent();
          std::vector<char> response_buf = file_handler_.save_file(
              message_.getUserId(), message_.getFilename(),
              content);

          // Debug: Log the response buffer size
          DEBUG_LOG("Response buffer size after saving file: {} bytes.", response_buf.size());

          if (response_buf.empty())
          {
            WARN_LOG("Response buffer after saving file is empty!","");
          }
          else
          {
            // Maybe log the first few characters of the response, similar to earlier
          }

          // Send the response to the client
          send_response(std::string(response_buf.begin(), response_buf.end()));
        }
        else
        {
          ERROR_LOG("Error reading from socket: {}", ec.message());
        }
      });
}

void Session::send_response(const std::string &responseBuffer) {
  asio::write(socket_, asio::buffer(responseBuffer));
  LOG("response sent.", "");
  graceful_close();
}

void Session::graceful_close() {
  sleep(1); // to make sure the client has time to read the response
  try {
    socket_.close();
  } catch (const std::exception &e) {
    ERROR_LOG("Error closing socket: {}", e.what());
  }
}