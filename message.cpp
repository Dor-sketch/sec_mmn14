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
      buffer_{},
      filename_{""}
{}


bool Message::parse_fixed_header()
{
    // for (int i = 0; i < HEADER_LENGTH; ++i)
    // {
    //     std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(header_buffer_[i]) << " ";
    // }
    // std::cout << std::endl;

    user_id_ = *reinterpret_cast<uint32_t *>(&header_buffer_[0]);
    version_ = header_buffer_[4];
    op_ = header_buffer_[5];
    name_len_ = *reinterpret_cast<uint16_t *>(&header_buffer_[6]);

    // std::cout << "request header fields after parsing: " << std::endl;
    // std::cout << "user id string is: " << std::to_string(user_id_) << std::endl;
    // std::cout << "version_ cast to int: " << static_cast<int>(version_) << std::endl;
    // std::cout << "std::to_string(op_): " << std::to_string(op_) << std::endl;
    // std::cout << "name_len_ in decimal: " << std::dec << name_len_ << std::endl;

    // std::cout << "name_len_: " << name_len_ << std::endl;
    // name_len_ = le16toh(*reinterpret_cast<uint16_t *>(&header_buffer_[6]));
    // std::cout << "name_len_ after le16toh: " << name_len_ << std::endl;

    if (op_ != OP_SAVE_FILE && op_ != OP_RESTORE_FILE && op_ != OP_DELETE_FILE && op_ != OP_GET_FILE_LIST)
    {
        std::cerr << "Error: Invalid OP code received." << std::endl;
        throw std::runtime_error ("Invalid OP code received.");
    }

    return true;
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

const std::string &Message::get_file_content() const {
    return file_contents_;
}


// setters
void Message::set_file_content() {
    // std::cout << "inside set_file_content" << std::endl;
    // for (const auto &byte : file_contents_)
    // {
    //     std::cout << std::hex << static_cast<int>(byte) << " ";
    // }
    // std::cout << std::endl;

    if (op_ != OP_SAVE_FILE)
    {
        std::cerr << "Error: invalid op for file content" << std::endl;
        return;
    }
    else if (buffer_.empty())
    {
        std::cerr << "Error: Cannot set filename. Buffer is empty." << std::endl;
    }

    // file_contents_ = std::string(buffer_.data()+name_len_, file_size_);
    std::copy(buffer_.begin() + name_len_, buffer_.begin() + name_len_ + file_size_, std::back_inserter(file_contents_));
    // std::cout << "file_contents_ set: " << file_contents_ << std::endl;
    // for (const auto &byte : file_contents_)
    // {
    //     std::cout << static_cast<int>(byte) << " ";
    // }

    std::string str(reinterpret_cast<const char *>(file_contents_.data()), file_contents_.size());
    // std::cout << "\n\nfile_contents_ set to string: " << file_contents_ << std::endl;
    // for (const auto &byte : file_contents_)
    // {
    //     std::cout << static_cast<int>(byte) << " ";
    // }
    // std::cout << std::endl;

    buffer_.clear();
}




void Message::set_header_buffer(const char *buffer) {
    // std::cout << "inside set_header_buffer" << std::endl;
    // std::cout << "buffer: " << buffer << std::endl;
    // for (int i = 0; i < 8; i++)
    // {
    //     std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[i] << " ";
    // }
    std::copy(buffer, buffer + 8, header_buffer_);
    // std::cout << "after copy" << std::endl;
    // std::cout << "header_buffer_ " << header_buffer_ << std::endl;
    header_buffer_[HEADER_LENGTH] = {0}; // avoid buffer overflow
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
    // std::cout << "filename_ was set: " << filename_ << std::endl;
    // buffer_.clear();
    // std::cout << "buffer_ was cleared" << std::endl;

}

 std::vector<char> &Message::get_buffer() {
    return buffer_;
}

char *Message::get_file_size_buffer(){
    return file_size_buffer_;
}

uint32_t Message::get_user_id() const
{
    return user_id_;
}

void Message::set_file_size() {
    //convert the buffer from little endian bytes to integer
    file_size_ = (*reinterpret_cast<uint32_t *>(&file_size_buffer_[0]));
    // printf("file_size_ was set: %d\n", file_size_);
}

// std::string Message::get_file_size_buffer() const {
//     return std::string(file_size_buffer_, FIELD_FILE_SIZE_LENGTH);
// }