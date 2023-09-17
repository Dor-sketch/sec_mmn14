#include "session.hpp"
#include "message.hpp"
#include "file_handler.hpp"
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
                                       // Assuming Session class has a constructor that takes a tcp::socket and a folder path
                                       std::make_shared<Session>(
                                        std::move(socket_),
                                        folder_path_
                                       )->start();
                                   }
                                   do_accept();
                               });
    }

public:


    Server(boost::asio::io_context &io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
          socket_(io_context)
    {
        do_accept();
    }
};

const std::string Server::folder_path_ = "backupsvr";

int main()
{
    std::cout << "Starting server..." << std::endl;
    try
    {
        boost::asio::io_context io_context;

        // Create and start the server
        Server server(io_context, 1234); // Listening on port 12345

        // Run the io_context to start the server's I/O operations
        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}