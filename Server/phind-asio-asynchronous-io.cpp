#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/ip/tcp.hpp>

class Connection : public std::enable_shared_from_this<Connection> {
private:
    boost::asio::ip::tcp::socket socket_;
    std::string message_{ "Async hello from ASIO " };
    static int counter_;

public:
    Connection(boost::asio::io_context& io) 
    	: socket_(io) {
        message_ += std::to_string(counter_++);
    }

    static std::shared_ptr<Connection> create(boost::asio::io_context& io) {
        return std::shared_ptr<Connection>(new Connection(io));
    }

    boost::asio::ip::tcp::socket& socket() {
        return socket_;
    }

    void start() {
        boost::asio::async_write(socket_, boost::asio::buffer(message_),
            std::bind(&Connection::write, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

private:
    void write(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/) {
        // Handle the write operation completion.
    }
};

class Server {
private:
    boost::asio::ip::tcp::acceptor acceptor_;

public:
    Server(boost::asio::io_context& io) 
    	: acceptor_(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 50013)) {
        start();
    }

    void start() {
        boost::asio::io_context& io = acceptor_.get_executor().context();
        std::shared_ptr<Connection> connection = Connection::create(io);
        boost::asio::ip::tcp::socket& socket = connection->socket();
        acceptor_.async_accept(socket, std::bind(&Server::handle, this, connection, std::placeholders::_1));
    }

    void handle(std::shared_ptr<Connection> connection, const boost::system::error_code& ec) {
        if (!ec) {
            connection->start();
        }
        start();
    }
};

int main() {
    boost::asio::io_context io;
    Server server(io);
    io.run();
    return 0;
}

