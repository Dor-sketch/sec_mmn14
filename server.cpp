#include <boost/asio.hpp>
#include <thread>
#include <iostream>

using boost::asio::ip::tcp;

class Server {
public:
    Server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        accept();
    }

private:
    void accept() {
        acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::make_shared<Session>(std::move(socket))->start();
            }
            accept();
        });
    }

    tcp::acceptor acceptor_;
};


class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket) : socket_(std::move(socket)) {}

    void start() {
        // Read data from the client
        // TODO: Implement reading the request header and payload
    }

private:
    tcp::socket socket_;
};




int main() {
    boost::asio::io_context io_context;
    Server server(io_context, 8080);
    io_context.run();
    return 0;
}
