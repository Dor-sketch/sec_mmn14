#include "Session.hpp"

namespace asio = boost::asio; // for cleaner code

Session::Session(asio::basic_stream_socket<asio::ip::tcp> socket,
                 const std::string &folder_path)
    : socket_(std::move(socket)),
      file_handler_(folder_path), requestBuffer_{0}, header_buffer_{0}
// Initialize header_buffer_ with size 8
// TODO - finish init for examp msg
{}

void Session::start() {
  std::cout << "start processing request...";
  readHeader();
}

void Session::readHeader() {
  auto self(shared_from_this());
  asio::async_read(
      socket_, asio::buffer(header_buffer_, Message::HEADER_LENGTH),
      [this, self](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
          if (length != Message::HEADER_LENGTH) {
            std::cerr << "Unexpected number of bytes read!" << std::endl;
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
            std::cerr << "Error in parsing header." << std::endl;
          }
        } else {
          std::cerr << "Error in reading header from socket: " << ec.message()
                    << std::endl;
        }
      });
}

// the function tries to read dynamic size name based on name_len_ field
// it stores the name in message_.filename_,
// after processing the name in the msg buffer_
void Session::readFilename() {
  auto self(shared_from_this());
  message_.getBuffer().resize(message_.getNameLength());
  asio::async_read(
      socket_, asio::buffer(message_.getBuffer(), message_.getNameLength()),
      [this, self](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
          if (length != message_.getNameLength()) {
            std::cerr << "Unexpected number of bytes read!" << std::endl;
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
            std::cerr << "Error: invalid op code" << std::endl;
          }
        } else {
          std::cerr << "Error reading from socket: " << ec.message()
                    << std::endl;
        }
      });
}

void Session::readFileSize() {
  auto self(shared_from_this());
  asio::async_read(
      socket_,
      asio::buffer(message_.getFileSizeBuffer(),
                   Message::FILE_SIZE_BUFFER_LENGTH),
      [this, self](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
          if (length != Message::FILE_SIZE_BUFFER_LENGTH) {
            std::cerr << "Unexpected number of bytes read!" << std::endl;
          }
          message_.getFileSize();
          // ready to read file content
          readPayload();
        } else {
          std::cerr << "Error reading header: " << ec.message() << std::endl;
        }
      });
}

void Session::readPayload() {

  auto self(shared_from_this());

  // Resize the buffer to hold the name and the file content
  message_.getBuffer().resize(message_.getNameLength() +
                              message_.getFileSize());

  // Create an iterator to point to the start of where the file content should
  // be
  auto it = message_.getBuffer().begin() + message_.getNameLength();

  asio::async_read(
      socket_, asio::buffer(&(*it), message_.getFileSize()),
      [this, self](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
          if (length != message_.getFileSize()) {
            std::cerr << "Unexpected number of bytes read!" << std::endl;
          }

          // store file content in file_contents_
          message_.getFileContent();

          std::vector<char> response_buf = file_handler_.save_file(
              message_.getUserId(), message_.getFilename(),
              message_.getFileContent());

          // Send the response to the client
          send_response(std::string(response_buf.begin(), response_buf.end()));

          return;
        } else {
          std::cerr << "Error reading from socket: " << ec.message()
                    << std::endl;
        }
      });
}

void Session::send_response(const std::string &responseBuffer) {
  asio::write(socket_, asio::buffer(responseBuffer));
  std::cout << "response sent" << std::endl;
  graceful_close();
}

void Session::graceful_close() {
  sleep(1); // to make sure the client has time to read the response
  try {
    socket_.close();
  } catch (const std::exception &e) {
    std::cerr << "Close error: " << e.what() << std::endl;
  }
  std::cout << "done. socket was closed." << std::endl;
}