#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/ip/tcp.hpp>

class ConnectionHandler : public boost::enable_shared_from_this<ConnectionHandler> {
public:
    typedef boost::shared_ptr<ConnectionHandler> pointer;

    static pointer create(boost::asio::io_service& io_service) {
        return pointer(new ConnectionHandler(io_service));
    }

    boost::asio::ip::tcp::socket& socket() {
        return socket_;
    }

    void start() {
        // Implement the logic to handle the connection.
        // For example, read data from the socket.
    }

private:
    ConnectionHandler(boost::asio::io_service& io_service) : socket_(io_service) {}

    boost::asio::ip::tcp::socket socket_;
};

class Server {
private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;

    void start_accept() {
        ConnectionHandler::pointer new_connection = ConnectionHandler::create(io_service_);
        acceptor_.async_accept(new_connection->socket(),
            boost::bind(&Server::handle_accept, this, new_connection,
                boost::asio::placeholders::error));
    }

public:
    Server(boost::asio::io_service& io_service)
        : io_service_(io_service),
          acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 1234)) {
        start_accept();
    }

    void handle_accept(ConnectionHandler::pointer new_connection, const boost::system::error_code& error) {
        if (!error) {
            new_connection->start();
        }
        start_accept();
    }
};

int main() {
    boost::asio::io_service io_service;
    Server server(io_service);
    io_service.run();
    return 0;
}
