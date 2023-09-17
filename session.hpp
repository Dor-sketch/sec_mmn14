#ifndef SESSION_HPP
#define SESSION_HPP

#include "message.hpp"
#include "file_handler.hpp"
#include <boost/asio.hpp>
#include <boost/algorithm/string/join.hpp>
#include <iostream>
#include <iomanip>
#include <string>

class Session : public std::enable_shared_from_this<Session>
{
private:
    boost::asio::basic_stream_socket<boost::asio::ip::tcp> socket_;
    FileHandler file_handler_;
    std::vector<char> requestBuffer_;
    Message message_;
    char header_buffer_[8];

    // Private member functions
    void do_read_header();
    void do_read_dynamicsize(int field_size, std::string *my_copy);
    void do_read_payload();
    void handle_request();
    void do_read_filename ();
    void do_read_fileSize();

public :
        // Constructor
        Session(boost::asio::basic_stream_socket<boost::asio::ip::tcp> socket,
                const std::string &folder_path);

    // Public member function
    void start();
};

#endif // SESSION_HPP
