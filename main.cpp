#include "Session.hpp"
#include <boost/asio.hpp>
#include <fstream>
#include <iostream>

using boost::asio::ip::tcp;

class Server
{
private:
  tcp::acceptor acceptor_;
  tcp::socket socket_;
  const std::string folder_path_ = "backupsvr";

  void do_accept()
  {
    acceptor_.async_accept(socket_, [this](boost::system::error_code ec)
                           {
            if (!ec) {
                std::make_shared<Session>(std::move(socket_), folder_path_)->start();
            } else {
                std::cerr << "Error accepting connection: " << ec.message() << std::endl;
            }
            // Always try to accept new connections
            do_accept(); });
  }

public:
  Server(boost::asio::io_context &io_context, const tcp::endpoint &endpoint)
      : acceptor_(io_context, endpoint), socket_(io_context)
  {
    do_accept();
  }
};

bool parse_server_info(const std::string &filename, std::string &ip_address_str, int &port_number)
{
  std::ifstream infile(filename);
  std::string ip_port_str;

  if (std::getline(infile, ip_port_str))
  {
    size_t colon_pos = ip_port_str.find(':');
    if (colon_pos != std::string::npos)
    {
      ip_address_str = ip_port_str.substr(0, colon_pos);
      try
      {
        port_number = std::stoi(ip_port_str.substr(colon_pos + 1));
        return true;
      }
      catch (const std::exception &e)
      {
        std::cerr << "Error parsing port number: " << e.what() << std::endl;
        return false;
      }
    }
  }
  return false;
}

int main()
{
  std::string ip_address_str;
  int port_number = 0;

  if (parse_server_info("server.info", ip_address_str, port_number))
  {
    std::cout << "IP address: " << ip_address_str << std::endl;
    std::cout << "Port number: " << port_number << std::endl;

    try
    {
      boost::asio::ip::address ip_address = boost::asio::ip::address::from_string(ip_address_str);

      tcp::endpoint endpoint(ip_address, port_number);
      boost::asio::io_context io_context;

      std::cout << "Starting server... press Ctrl+C to quit" << std::endl;
      Server server(io_context, endpoint);
      io_context.run();
    }
    catch (const std::exception &e)
    {
      std::cerr << "Error: " << e.what() << std::endl;
    }
  }
  else
  {
    std::cerr << "Error: Failed to parse server.info file or invalid format." << std::endl;
  }

  return 0;
}
