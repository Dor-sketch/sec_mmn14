#ifndef MESSAGE_HPP
#define MESSAGE_HPP
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp> // <-- Add this line
#include "server_status.hpp"

class Message : public std::enable_shared_from_this<Message>
{
public:
    Message();
    Message(std::vector<char> & requestBuffer,
            boost::asio::ip::tcp::socket &socket);



    enum { HEADER_LENGTH = 8 }; // without filename
    enum { FILE_SIZE_BUFFER_LENGTH = 4 };

private:
    // class variables
    char header_buffer_[HEADER_LENGTH]; // without filename
    char file_size_buffer_[FILE_SIZE_BUFFER_LENGTH];
    uint8_t version_;
    uint8_t op_;
    uint16_t name_len_;
    uint32_t user_id_;
    uint32_t file_size_;
    std::string file_contents_;
    std::vector<std::string> file_list_;
    std::vector<char> buffer_; // to store bigget messages and inteact with asio
    std::string filename_;
    boost::asio::io_context io_context_; 
    boost::asio::ip::tcp::socket socket_;

public:
    // getters
    uint8_t get_op_code() const;
    uint16_t get_name_length() const;
    uint32_t get_file_size() const;
    uint32_t get_user_id() const;
    const std::string &get_file_content() const;
    std::string get_header_buffer() const;
    std::string get_file_size_buffer() const;
    const std::string &get_filename() const;
    const std::vector<std::string> &get_file_list() const;
    std::vector<char> &get_buffer() ;
    char *get_file_size_buffer();

    // setters
    void set_file_content();
    void set_filename();
    void set_header_buffer(const char *buffer);
    void set_file_list(const std::vector<std::string> &file_list);
    void set_file_size();

    // ... other utility methods ...
    bool parse_fixed_header();

    void do_read_dynamicsize(uint16_t size, std::string *dest);
    void startReadFilename();

    // ... other utility methods ...
};

#endif // MESSAGE_HPP