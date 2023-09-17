#include "message.hpp"
#include <boost/asio.hpp>
#include <boost/algorithm/string/join.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <boost/algorithm/string.hpp>
#include <string>

Message::Message()
    : header_buffer_{},
      version_{0},
      op_{0},
      name_len_{0},
      user_id_{0},
      file_size_{0},
      file_contents_{""},
      file_list_{},
      buffer_{},
      filename_{""},
      io_context_(),
      socket_(io_context_)
{
}

Message::Message(std::vector<char> &requestBuffer,
                boost::asio::ip::tcp::socket &socket)
    : header_buffer_{},
      version_{0},
      op_{0},
      name_len_{0},
      user_id_{0},
      file_size_{0},
      file_contents_{""},
      file_list_{},
      buffer_(std::move(requestBuffer)),
      filename_{""},
      socket_(std::move(socket))
{
}

void Message::startReadFilename()
    {
    //     std::cout << "inside startReadFilename" << std::endl;
    //     try
    //     {
    //         if (socket_.is_open())
    //         {
    //             std::cout << "inside startReadFilename socket is open" << std::endl;
    //             socket_.async_read_some(
    //                 boost::asio::buffer(buffer_, get_file_size()),
    //                 [this](boost::system::error_code ec, std::size_t bytes_transferred)
    //                 {
    //                     if (!ec)
    //                     {
    //                         set_filename(std::string(buffer_.begin(), buffer_.begin() + bytes_transferred));
    //                     }
    //                     else
    //                     {

    //                     }
    //                 });
    //          }
    //     }
    // catch (std::exception &e)
    // {
    //     std::cerr << "Exception: " << e.what() << "\n";
    // }
}

// void Message::startReadFilename()
// {
//     auto temp_buffer = std::make_shared<std::vector<char>>(get_name_length());

//     std::cout << "inside startReadFilename" << std::endl;

//     socket_.async_read_some(boost::asio::buffer(temp_buffer->data(), temp_buffer->size()),
//                             [this, temp_buffer](boost::system::error_code ec, std::size_t bytes_transferred)
//                             {
//                                 if (!ec)
//                                 {
//                                     filename_ = std::string(temp_buffer->begin(), temp_buffer->begin() + bytes_transferred);
//                                     // Rest of your code...
//                                 }
//                                 // The else block is unchanged. Since temp_buffer is a shared pointer,
//                                 // it will be destroyed automatically when it goes out of scope.
//                             });
// }

// void Message::do_read_dynamicsize(uint16_t size, std::string *out_string)
// {
//     std::cout << "inside do_read_dynamicsize" << std::endl;
//     std::cout << "size: " << size << std::endl;
//     std::vector<char> buffer(get_name_length());
//     std::cout << "after alloc" << std::endl;
//     socket_.async_read_some(boost::asio::buffer(buffer, size),
//                             [this, buffer, out_string, size](boost::system::error_code ec, std::size_t length)
//                             {
//                                 std::cout << "inside do_read_dynamicsize async_read_some" << std::endl;
//                                 if (!ec)
//                                 {
//                                     std::cout << "inside do_read_dynamicsize async_read_some" << std::endl;
//                                     *out_string = std::string(buffer, length);
//                                 }
//                                 else
//                                 {
//                                 }
//                             });
// }


bool Message::parse_fixed_header()
{
    for (int i = 0; i < HEADER_LENGTH; ++i)
    {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(header_buffer_[i]) << " ";
    }
    std::cout << std::endl;

    std::cout << "inside parse_header" << std::endl;
    user_id_ = *reinterpret_cast<uint32_t *>(&header_buffer_[0]);
    std::cout << "user_id_ (bytes): " << user_id_ << std::endl;
    std::cout << "user id string is: " << std::to_string(user_id_) << std::endl;
    
    
    version_ = header_buffer_[4];
    std::cout << "version_: " << version_ << std::endl;
    std::cout << "version_ cast to int: " << static_cast<int>(version_) << std::endl;
    
    op_ = header_buffer_[5];
    std::cout << "op_: " << op_ << std::endl;
    std::cout << "static_cast<int>(op_): " << static_cast<int>(op_) << std::endl;
    std::cout << "std::to_string(op_): " << std::to_string(op_) << std::endl;
    std::cout << "op_ Memory value: " << std::hex << +op_ << std::endl;

    uint8_t testOp = 100;
    std::cout << "Direct testOp: " << testOp << std::endl;
    std::cout << "Direct static_cast<int>(testOp): " << static_cast<int>(testOp) << std::endl;
    std::cout << "Direct std::to_string(testOp): " << std::to_string(testOp) << std::endl;



    name_len_ = *reinterpret_cast<uint16_t *>(&header_buffer_[6]);
    std::cout << "name_len_: " << name_len_ << std::endl;
    name_len_ = le16toh(*reinterpret_cast<uint16_t *>(&header_buffer_[6]));
    std::cout << "name_len_ after le16toh: " << name_len_ << std::endl;


  

    std::cout << "version_: " << static_cast<int>(version_) << std::endl;
    std::cout << "op_: " << static_cast<int>(op_) << std::endl;
    std::cout << "name_len_: " << name_len_ << std::endl;

    // // If op is not get file list copy filename from header and add null terminator
    // if (op_ != OP_GET_FILE_LIST)
    // {
    //     return false;
    // }


    if (op_ != OP_SAVE_FILE && op_ != OP_RESTORE_FILE && op_ != OP_DELETE_FILE && op_ != OP_GET_FILE_LIST)
    {
        throw std::runtime_error ("Invalid OP code received.");
    }

    return true;
}

void Message::pack_response(Status status, std::vector<char> &responseBuffer)
{
    std::cout << "inside pack_response" << std::endl;
    uint8_t version = 1;
    uint16_t status_code = static_cast<uint16_t>(status);
    uint16_t name_len = 0;
    uint32_t payload_size = 0;
    std::string payload;

    switch (op_) {
    case OP_SAVE_FILE:
        if (status == Status::SUCCESS_SAVE)
        {
            // Set response fields
            name_len = filename_.length();
            payload_size = file_size_;

            // Pack response
            payload.append(reinterpret_cast<const char *>(&payload_size), sizeof(payload_size));
            payload.append(file_contents_);
        }
        break;

    case OP_RESTORE_FILE:
        if (status == Status::SUCCESS_SAVE)
        {
            // Set response fields
            name_len = filename_.length();
            payload_size = file_size_;

            // Pack response
            payload.append(reinterpret_cast<const char *>(&payload_size), sizeof(payload_size));
            payload.append(file_contents_);
        }
        break;

    case OP_DELETE_FILE:
        if (status == Status::SUCCESS_FILE_LIST)
        {
            // Set response fields
            payload = boost::algorithm::join(file_list_, "\n");
            payload_size = payload.length();

            // Pack response
            payload.insert(0, reinterpret_cast<const char *>(&payload_size), sizeof(payload_size));
        }
            break;

        case OP_GET_FILE_LIST:
            //... Handle get file list response logic
            break;

        default:
            payload = "Error: Invalid operation.";
            payload_size = payload.size();
            break;
    }
    // Packing the header. (This is just a simple example, adapt as needed.)
    responseBuffer.push_back(version);
    responseBuffer.push_back(static_cast<char>(status_code));
    responseBuffer.push_back(name_len & 0xFF);            // lower byte
    responseBuffer.push_back((name_len >> 8) & 0xFF);     // upper byte
    responseBuffer.push_back(payload_size & 0xFF);        // lower byte
    responseBuffer.push_back((payload_size >> 8) & 0xFF); // upper byte
    // ... pack other header fields as requi
    // Packing the payload
    responseBuffer.insert(responseBuffer.end(), payload.begin(), payload.end());
}

// getters - small type
uint8_t Message::get_op_code() const {
   return op_;
}

uint16_t Message::get_name_length() const {
    return name_len_;
}

uint32_t Message::get_file_size() const {
    return file_size_;
}

// getters - large type
const std::string &Message::get_filename() const {
    return filename_;
}

std::string Message::get_header_buffer() const
{
    return std::string(header_buffer_, HEADER_LENGTH);
}

const std::vector<std::string> &Message::get_file_list() const {
    return file_list_;
}

const std::string &Message::get_file_content() const {
    return file_contents_;
}


// setters
void Message::set_file_content(const std::string &content) {
    file_contents_ = content;
    file_size_ = file_contents_.size();
}


void Message::set_file_list(const std::vector<std::string> &file_list) {
    file_list_ = file_list;
}

void Message::set_header_buffer(const char *buffer) {
    std::cout << "inside set_header_buffer" << std::endl;
    std::cout << "buffer: " << buffer << std::endl;
    for (int i = 0; i < 8; i++)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[i] << " ";
    }
    std::copy(buffer, buffer + 8, header_buffer_);
    std::cout << "after copy" << std::endl;
    std::cout << "header_buffer_ " << header_buffer_ << std::endl;
    header_buffer_[HEADER_LENGTH] = {0}; // Initialize all elements to zero
}

void Message::set_filename() {
    if (op_ == OP_GET_FILE_LIST)
    {
        std::cerr << "Error: Cannot set filename for OP_GET_FILE_LIST" << std::endl;
        return;
    }
    else if (buffer_.empty())
    {
        std::cerr << "Error: Cannot set filename. Buffer is empty." << std::endl;
    }
    
    filename_ = std::string(buffer_.data(), name_len_);
}

 std::vector<char> &Message::get_buffer() {
    return buffer_;
}
