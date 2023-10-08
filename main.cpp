// Description: Main file for the server. It creates the server and starts it.

#include "Session.hpp"
#include <fstream>
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Server
{
private:
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    static const std::string folder_path_;

    void do_accept()
    {
        acceptor_.async_accept(socket_,
                               [this](boost::system::error_code ec)
                               {
                                   if (!ec)
                                   {
                                       std::make_shared<Session>(
                                        std::move(socket_),
                                        folder_path_
                                       )->start();
                                   }
                                   do_accept();
                               });
    }

public:
    Server(boost::asio::io_context &io_context,
        boost::asio::ip::tcp::endpoint endpoint)
    : acceptor_(io_context, endpoint),
      socket_(io_context)
    {
        do_accept();
    }
};

const std::string Server::folder_path_ = "backupsvr";

int main()
{
    std::ifstream infile("server.info");
    std::string ip_port_str;

    if (std::getline(infile, ip_port_str))
    {
        size_t colon_pos = ip_port_str.find(':');
        if (colon_pos != std::string::npos)
        {
            std::string ip_address_str = ip_port_str.substr(0, colon_pos);
            int port_number = std::stoi(ip_port_str.substr(colon_pos + 1));
            std::cout << "IP address: " << ip_address_str << std::endl;
            std::cout << "Port number: " << port_number << std::endl;

            boost::asio::ip::address ip_address =
                boost::asio::ip::address::from_string(ip_address_str);

            // creating endpoint based on ip address and port number from file "server.info"
            boost::asio::ip::tcp::endpoint endpoint(ip_address, port_number);
            boost::asio::io_context io_context;

            std::cout << "Starting server... press Ctrl+C to quit" << std::endl;
            Server server(io_context, endpoint);
            io_context.run();
        }
        else
        {
            std::cerr << "Error: Invalid IP address and port number format." << std::endl;
        }
    }
    else
    {
        std::cerr << "Error: Failed to read server.info file." << std::endl;
    }

    return 0;
}