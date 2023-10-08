#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include "Status.hpp"

// The message class is used to parse the request and to pack and to store
// the requests fields. It used by the session class.
class Message : public std::enable_shared_from_this<Message>
{
public:
    Message();

    enum { HEADER_LENGTH = 8 }; // without filename
    enum { FILE_SIZE_BUFFER_LENGTH = 4 };

private:
    char header_buffer_[HEADER_LENGTH]; 
    char file_size_buffer_[FILE_SIZE_BUFFER_LENGTH];
    uint8_t version_;
    uint8_t op_;
    uint16_t name_len_;
    uint32_t user_id_;
    uint32_t file_size_;
    std::string file_contents_;
    std::vector<char> buffer_; // to store big messages and inteact with asio
    std::string filename_;


public:
    // getters
    // used by Session for the asio socket - it enables to read dynamic size data
    uint8_t get_op_code() const;
    uint16_t get_name_length() const;
    uint32_t get_file_size() const;
    uint32_t get_user_id() const;
    const std::string &get_file_content() const;
    std::string get_header_buffer() const;
    std::string get_file_size_buffer() const;
    const std::string &get_filename() const;
    std::vector<char> &get_buffer() ;
    char *get_file_size_buffer();

    // setters
    void set_file_content();
    void set_filename();
    void set_header_buffer(const char *buffer);
    void set_file_size();

    // this utility function is called after the session class has read the
    // request header and determined the op code. It helps determine the
    // flow of control in the session class based on the op code
    bool parse_fixed_header();

};

#endif // MESSAGE_HPP