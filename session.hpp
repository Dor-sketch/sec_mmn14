#ifndef SESSION_HPP
#define SESSION_HPP

#include "message.hpp"
#include "file_handler.hpp"
#include <boost/asio.hpp>
#include <boost/algorithm/string/join.hpp>
#include <iostream>
#include <iomanip>
#include <string>

// The session class handles the asynchronous communication with the client
// it is created by the server class, it uses the message class to parse the
// request and the file_handler class to handle the request
class Session : public std::enable_shared_from_this<Session>
{
private:
    boost::asio::basic_stream_socket<boost::asio::ip::tcp> socket_;
    FileHandler file_handler_;
    std::vector<char> requestBuffer_;
    Message message_;
    char header_buffer_[8];

    // Private member functions helps to define the flow of the session
    // each request requires different flow. 
    // the session class uses the message class to determine the op code
    // and then keep reading the request according to the op code
    void do_read_header();

    // used by all except OP_GET_FILE_LIST
    void do_read_filename ();

    // ================================
    // used only for the OP_SAVE_FILE
    void do_read_fileSize();
    // called by do_read_fileSize
    void do_read_payload();
    // ================================

    void send_response(const std::string &response);
    // called by send_response
    void graceful_close();

public :
    Session(boost::asio::basic_stream_socket<boost::asio::ip::tcp> socket,
             const std::string &folder_path);

    void start();
};

#endif // SESSION_HPP
