#include "message.hpp"
#include <boost/asio.hpp>
#include <boost/algorithm/string/join.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

Message::Message()
{
    // Constructor implementation...
}

// Not part of your original code, but considering you want to read dynamic size data:
void Message::do_read_dynamicsize(uint16_t size, std::string *out_string)
{
    // Note: This is just a placeholder function. You'll need to replace this with actual functionality
    // that reads `size` bytes from wherever you're getting your data, and appends it to `out_string`.
}


bool Message::parse_fixed_header()
{
    std::cout << "inside parse_header" << std::endl;

    // Extract fields from header
    user_id_ = *reinterpret_cast<uint32_t *>(header_buffer_); // Assuming little endian
    version_ = header_buffer_[4];
    op_ = header_buffer_[5];
    name_len_ = *reinterpret_cast<uint16_t *>(header_buffer_ + 6); // little endian

    // If op is not get file list copy filename from header and add null terminator
    if (op_ != OP_GET_FILE_LIST)
    {
        do_read_dynamicsize(name_len_, &filename_);
        filename_.push_back('\0');
    }

    // Debug logs
    for (int i = 0; i < HEADER_LENGTH; ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(header_buffer_[i]) << " ";
    }
    std::cout << "User id: " << user_id_ << std::endl;
    std::cout << "Version: " << static_cast<int>(version_) << std::endl;
    std::cout << "Op code: " << static_cast<int>(op_) << std::endl;
    std::cout << "Name length: " << name_len_ << std::endl;
    std::cout << "Filename: " << filename_ << std::endl;

    if (op_ != OP_SAVE_FILE && op_ != OP_RESTORE_FILE && op_ != OP_DELETE_FILE && op_ != OP_GET_FILE_LIST)
    {
        std::cerr << "Invalid OP code received." << std::endl;
        return false; // indicate failure to parse header
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

    switch (op_)
    {
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
        // ... pack other header fields as required

        // Packing the payload
        responseBuffer.insert(responseBuffer.end(), payload.begin(), payload.end());
    }

    uint8_t Message::get_op_code() const
    {
        return op_;
    }

    uint16_t Message::get_name_length() const
    {
        return name_len_;
    }

    const std::string &Message::get_filename() const
    {
        return filename_;
    }

    char *Message::get_header_buffer()
    {
        return header_buffer_;
    }

    uint32_t Message::get_file_size() const
    {
        return file_size_;
    }

    void Message::set_file_content(const std::string &content)
    {
        file_contents_ = content;
        file_size_ = file_contents_.size();
    }

    const std::string &Message::get_file_content() const
    {
        return file_contents_;
    }

    void Message::set_file_list(const std::vector<std::string> &file_list)
    {
        file_list_ = file_list;
    }

    void Message::set_header_buffer(const char *buffer)
    {
        std::copy(buffer, buffer + HEADER_LENGTH, header_buffer_);
    }

    void Message::set_filename(const std::string &filename)
    {
        filename_ = filename;
        name_len_ = filename_.length();
    }
// ... other method implementations ...
