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

    // Op codes
    static const uint8_t OP_SAVE_FILE = 100;
    static const uint8_t OP_RESTORE_FILE = 200;
    static const uint8_t OP_DELETE_FILE = 201;
    static const uint8_t OP_GET_FILE_LIST = 202;

    enum { HEADER_LENGTH = 8 }; // without filename

private:
    // class variables
    char header_buffer_[HEADER_LENGTH];
    uint8_t version_;
    uint8_t op_;
    uint16_t name_len_;
    uint32_t user_id_;
    uint32_t file_size_;
    std::string file_contents_;
    std::vector<std::string> file_list_;
    std::vector<char> buffer_;
    std::string filename_;
    boost::asio::io_context io_context_; 
    boost::asio::ip::tcp::socket socket_;

public:
    // getters
    uint8_t get_op_code() const;
    uint16_t get_name_length() const;
    uint32_t get_file_size() const;
    const std::string &get_file_content() const;
    std::string get_header_buffer() const;
    const std::string &get_filename() const;
    const std::vector<std::string> &get_file_list() const;

    // setters
    void set_file_content(const std::string &content);
    void set_filename(const std::string &filename);
    void set_header_buffer(const char *buffer);
    void set_file_list(const std::vector<std::string> &file_list);

    // ... other utility methods ...
    bool parse_fixed_header();
    void pack_response(Status status, std::vector<char> &responseBuffer);
    void do_read_dynamicsize(uint16_t size, std::string *dest);
    void do_read_payload(uint32_t size, std::string *dest);
    void startReadFilename();

    // ... other utility methods ...
};

#endif // MESSAGE_HPP