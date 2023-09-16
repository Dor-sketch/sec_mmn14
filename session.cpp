#include "session.hpp"

Session::Session(boost::asio::basic_stream_socket<boost::asio::ip::tcp> socket, const std::string &folder_path)
    : socket_(std::move(socket)), file_handler_(folder_path)
{
    // Constructor implementation...
}

void Session::start()
{
    do_read_header();
}

void Session::do_read_header()
{
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(message_.get_header_buffer(), Message::HEADER_LENGTH),
                            [this, self](boost::system::error_code ec, std::size_t /*length*/)
                            {
                                if (!ec && message_.parse_fixed_header())
                                {
                                    handle_request();
                                }
                                else
                                {
                                    std::cerr << "Error reading header: " << ec.message() << std::endl;
                                }
                            });
}

void Session::handle_request()
{
    switch (message_.get_op_code())
    {
    case Message::OP_SAVE_FILE:
    {
        // Read file name
        std::string filename;
        do_read_dynamicsize(message_.get_name_length(), &filename);
        message_.set_filename(filename);

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
    int payload_size = message_.get_file_size();
    char *payload_buffer = new char[payload_size];

    auto self(shared_from_this());
    boost::asio::async_read(socket_,
                            boost::asio::buffer(payload_buffer, payload_size),
                            [this, self, payload_buffer, payload_size](boost::system::error_code ec, std::size_t /*length*/)
                            {
                                if (!ec)
                                {
                                    message_.set_file_content(std::string(payload_buffer, payload_size));
                                    // Now save it using FileHandler
                                    file_handler_.save_file(message_.get_filename(), message_.get_file_content());
                                    delete[] payload_buffer;
                                }
                                else
                                {
                                    std::cerr << "Error reading payload: " << ec.message() << std::endl;
                                    delete[] payload_buffer;
                                }
                            });
}
