#include "Session.hpp"
#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include "LoggerModule.hpp"
#include <boost/filesystem.hpp>

using boost::asio::ip::tcp;

class Server {
private:
  tcp::acceptor acceptor_;
  tcp::socket socket_;
  const std::string folder_path_ = "backupsvr";

  void do_accept() {
    check_folder_path();
    acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
      if (!ec) {
        std::make_shared<Session>(std::move(socket_), folder_path_)->start();
      } else {
        ERROR_LOG("Error accepting connection: ", ec.message());
      }
      // Always try to accept new connections
      do_accept();
    });
  }

  void check_folder_path() {
    try
    {
      if (!boost::filesystem::exists(folder_path_))
      {
        WARN_LOG("Folder {} does not exist. Creating it now...", folder_path_);
        boost::filesystem::create_directory(folder_path_);
      }
      else
      {
        if (!boost::filesystem::is_directory(folder_path_))
        {
          CRITICAL_LOG("Path {} is not a directory. Exiting...", folder_path_);
          exit(1);
        }
      }
    }
    catch (const boost::filesystem::filesystem_error &e)
    {
      CRITICAL_LOG("Error in checking folder path: {}", e.what());
      exit(1);
    }
  }

public:
  Server(boost::asio::io_context &io_context, const tcp::endpoint &endpoint)
      : acceptor_(io_context, endpoint), socket_(io_context) {
    do_accept();
  }
};

bool parse_server_info(const std::string &filename, std::string &ip_address_str,
                       int &port_number) {
  std::ifstream infile(filename);
  std::string ip_port_str;

  if (std::getline(infile, ip_port_str)) {
    size_t colon_pos = ip_port_str.find(':');
    if (colon_pos != std::string::npos) {
      ip_address_str = ip_port_str.substr(0, colon_pos);
      try {
        port_number = std::stoi(ip_port_str.substr(colon_pos + 1));
        return true;
      } catch (const std::exception &e) {
        ERROR_LOG("Error parsing port number: ", e.what());
        return false;
      }
    }
  }
  return false;
}



int main() {
  LoggerModule::init();
  std::string ip_address_str;
  int port_number = 0;

  if (parse_server_info("server.info", ip_address_str, port_number)) {
    DEBUG_LOG("Initialized server: [IP address: {}] [port number: {}]", ip_address_str, port_number);
    try {
      boost::asio::ip::address ip_address =
          boost::asio::ip::address::from_string(ip_address_str);

      tcp::endpoint endpoint(ip_address, port_number);
      boost::asio::io_context io_context;

      LOG("Starting server... press Ctrl+C to quit");
      Server server(io_context, endpoint);
      io_context.run();
    } catch (const std::exception &e) {
      ERROR_LOG("Error in starting server: {}", e.what());
    }
  } else {
    ERROR_LOG("Error: Failed to parse server.info file or invalid format. Please check the file and try again.");
  }

  return 0;
}
