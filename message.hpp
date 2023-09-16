# include <cstdint>
# include <string>
# include <vector>
# include <algorithm>
# include <boost/filesystem.hpp>
# include "server_status.hpp"




class Message
{
public:
    // Op codes
    static const uint8_t OP_SAVE_FILE = 100;
    static const uint8_t OP_RESTORE_FILE = 200;
    static const uint8_t OP_DELETE_FILE = 201;
    static const uint8_t OP_GET_FILE_LIST = 202;

    enum { HEADER_LENGTH = 8 }; // without filename


private:
    char header_buffer_[HEADER_LENGTH];
    uint32_t user_id_;
    uint8_t version_;
    uint8_t op_;
    uint16_t name_len_;
    std::string filename_;
    uint32_t file_size_;
    std::string file_contents_;
    std::vector<std::string> file_list_;

public:
    Message();
    bool parse_fixed_header();
    void pack_response(Status status, std::vector<char> &responseBuffer);
    void do_read_dynamicsize(uint16_t size, std::string *dest);
    void do_read_payload(uint32_t size, std::string *dest);

    uint8_t get_op_code() const;
    uint16_t get_name_length() const;
    char *get_header_buffer();
    uint32_t get_file_size() const;
    void set_file_content(const std::string &content);
    const std::string &get_file_content() const;

    const std::string &get_filename() const;
    void set_filename(const std::string &filename);
    void set_header_buffer(const char *buffer);
    void get_header_buffer(char *buffer) const;
    void set_file_list(const std::vector<std::string> &file_list);




    // ... other utility methods ...
};


