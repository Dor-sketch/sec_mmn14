#include "session.hpp"

Session::Session(boost::asio::basic_stream_socket<boost::asio::ip::tcp> socket, const std::string &folder_path)
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
                                std::cout << "checking if socket_ is open" << std::endl;
                                std::cout << "socket_.is_open() is: " << socket_.is_open() << std::endl;
                                if (!ec)
                                {
                                    std::cout << "inside do_read_header" << std::endl;
                                    std::cout << "header_buffer_ ";
                                    for (int i = 0; i < 8; i++)
                                    {
                                        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)header_buffer_[i] << " ";
                                    }
                                    std::cout << std::endl;
                                    message_.set_header_buffer(header_buffer_);
                                    std::cout << "header_buffer_ was set" << std::endl; 
                                    
                                    std::cout << "head_buffer_ is: " << message_.get_header_buffer() << std::endl;
                                    if(message_.parse_fixed_header())
                                    {
                                        // copy file name if not OP_GET_FILE_LIST
                                        if (message_.get_op_code() != Message::OP_GET_FILE_LIST)
                                        {
                                            do_read_filename();
                                        }
                                        else
                                        {
                                            std::cout << "inside do_read_header else" << std::endl;
                                            // handle_request();
                                        }
                                        // handle_request();
                                    }
                                }
                                else
                                {
                                    std::cerr << "Error reading header: " << ec.message() << std::endl;
                                }
                            });
}

void Session::do_read_filename()
{
    auto self(shared_from_this());
    message_.get_buffer().resize(message_.get_name_length());
    std::cout << "inside do_read_filename" << std::endl;
    boost::asio::async_read(socket_,
                            boost::asio::buffer(message_.get_buffer(), message_.get_name_length()),
                            [this, self](boost::system::error_code ec, std::size_t length)
                            {
                                if (!ec)
                                {
                                    std::cout << "Bytes read: " << length << std::endl;

                                    // If the length read is not what you expect, it might be indicative of a problem.
                                    if (length != message_.get_name_length())
                                    {
                                        std::cerr << "Unexpected number of bytes read!" << std::endl;
                                    }

                                    for (int i = 0; i < length; i++)
                                    {
                                        printf("%02X ", static_cast<unsigned char>(message_.get_buffer()[i]));
                                    }
                                    printf("\n");
                                    message_.set_filename();
                                    // Continue processing the filename or the next piece of data.
                                }
                                else
                                {
                                    std::cerr << "Error reading from socket: " << ec.message() << std::endl;
                                }
                            });
}

void Session::handle_request()
{
    switch (message_.get_op_code())
    {
    case Message::OP_SAVE_FILE:
    {

        // Save file
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

void Session::do_read_dynamicsize(int field_size, std::string *my_copy)
{
    auto self(shared_from_this());
    char *buffer = new char[field_size];
    boost::asio::async_read(socket_,
                            boost::asio::buffer(buffer, field_size),
                            [this, self, buffer, field_size, my_copy](boost::system::error_code ec, std::size_t /*length*/)
                            {
                                if (!ec)
                                {
                                    my_copy->assign(buffer, field_size);
                                    delete[] buffer;
                                }
                                else
                                {
                                    std::cerr << "Error reading dynamic field: " << ec.message() << std::endl;
                                    delete[] buffer;
                                }
                            });
}

void Session::do_read_payload()
{
    // int payload_size = message_.get_file_size();
    // char *payload_buffer = new char[payload_size];

    // auto self(shared_from_this());
    // boost::asio::async_read(socket_,
    //                         boost::asio::buffer(payload_buffer, payload_size),
    //                         [this, self, payload_buffer, payload_size](boost::system::error_code ec, std::size_t /*length*/)
    //                         {
    //                             if (!ec)
    //                             {
    //                                 message_.set_file_content(std::string(payload_buffer, payload_size));
    //                                 // Now save it using FileHandler
    //                                 file_handler_.save_file(message_.get_filename(), message_.get_file_content());
    //                                 delete[] payload_buffer;
    //                             }
    //                             else
    //                             {
    //                                 std::cerr << "Error reading payload: " << ec.message() << std::endl;
    //                                 delete[] payload_buffer;
    //                             }
    //                         });
}
