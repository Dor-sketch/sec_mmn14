#include "session.hpp"

Session::Session(boost::asio::basic_stream_socket<boost::asio::ip::tcp> socket,
                    const std::string &folder_path)
: socket_(std::move(socket)),
  file_handler_(folder_path),
  requestBuffer_{0},
//   message_(requestBuffer_, socket_),
  header_buffer_{0} // Initialize header_buffer_ with size 8
{}

void Session::start()
{
    do_read_header();
}


// a function to header fields from client request
void Session::do_read_header()
{
    auto self(shared_from_this());
    std::cout << "inside do_read_header" << std::endl;
    std::cout << "checking if socket_ is open" << std::endl;
    std::cout << "socket_.is_open() is: " << socket_.is_open() << std::endl;
    boost::asio::async_read(socket_,
                            boost::asio::buffer(header_buffer_, Message::HEADER_LENGTH),
                            [this, self](boost::system::error_code ec, std::size_t /*length*/)
                            {
                                if (!ec) {
                                    message_.set_header_buffer(header_buffer_);
                                    if(message_.parse_fixed_header()) {                 
                                        
                                        // copy file name if not OP_GET_FILE_LIST
                                        if (message_.get_op_code()!= Message::OP_GET_FILE_LIST)
                                            do_read_filename();
                                        
                                        // copy file content if OP_SAVE_FILE
                                        if (message_.get_op_code() == Message::OP_SAVE_FILE){
                                            do_read_fileSize();
                                            do_read_payload();
                                        }
                                        
                                        handle_request();
                                    }
                                } 
                                else
                                    std::cerr << "Error reading header: " 
                                        << ec.message() << std::endl;
                            });
}

// the function tries to read dynamic size name based on name_len_ field
// it stores the name in message_.filename_, after processing the name in the msg buffer_
void Session::do_read_filename()
{
    auto self(shared_from_this());
    message_.get_buffer().resize(message_.get_name_length());
    std::cout << "inside do_read_filename" << std::endl;
    boost::asio::async_read(socket_,
                            boost::asio::buffer(message_.get_buffer(), message_.get_name_length()),
                            [this, self](boost::system::error_code ec, std::size_t length)
                            {
                                if (!ec) {
                                    if (length != message_.get_name_length()) {
                                        std::cerr << "Unexpected number of bytes read!" << std::endl;
                                    }

                                    message_.set_filename();
                                }
                                else std::cerr << "Error reading from socket: " << ec.message() << std::endl;
                            });
}

void Session::do_read_fileSize()
{
    auto self(shared_from_this());
    std::cout << "inside do_read_fileSize" << std::endl;

    boost::asio::async_read(socket_,
                            boost::asio::buffer(message_.get_file_size_buffer(), Message::FILE_SIZE_BUFFER_LENGTH),
                            [this, self](boost::system::error_code ec, std::size_t length)
                            {
                                if (!ec)
                                {
                                    message_.set_file_size();
                                }
                                else
                                {
                                    std::cerr << "Error reading header: "
                                              << ec.message() << std::endl;
                                }
                            });
}

void Session::do_read_payload()
{
    auto self(shared_from_this());
    message_.get_buffer().resize(message_.get_file_size());
    std::cout << "inside do_read_payload" << std::endl;
    boost::asio::async_read(socket_,
                            boost::asio::buffer(message_.get_buffer(), message_.get_file_size()),
                            [this, self](boost::system::error_code ec, std::size_t length)
                            {
                                if (!ec) {
                                    if (length != message_.get_file_size()) {
                                        std::cerr << "Unexpected number of bytes read!" << std::endl;
                                    }

                                    message_.set_file_content();
                                }
                                else std::cerr << "Error reading from socket: " << ec.message() << std::endl;
                            });
}



void Session::handle_request()
{
    switch (message_.get_op_code())
    {
    case Message::OP_SAVE_FILE:{
        file_handler_.save_file(message_.get_filename(), message_.get_file_content());

        // Once file name is read, you can further continue reading the file payload.
        do_read_payload();
        break;
    }
    case Message::OP_RESTORE_FILE:
        //... similar approach
        break;
    case Message::OP_DELETE_FILE:
        //... similar approach
        break;
    case Message::OP_GET_FILE_LIST:
        //... Handle get file list logic here
        break;
    default:
        std::cerr << "Invalid operation code." << std::endl;
        break;
    }
}


