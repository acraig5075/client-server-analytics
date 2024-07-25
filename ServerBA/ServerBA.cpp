#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <filesystem>
#include <cxxopts.hpp>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/prepared_statement.h>
#include "ProducerConsumer.h"
#include "../Utilities/SqlUtils.h"


const size_t BUF_SIZE = 100;

class ServerConnection : public std::enable_shared_from_this<ServerConnection>
{
public:
	ServerConnection(const boost::asio::any_io_executor &ex, ProducerConsumer &pc)
		: socket_(ex)
		, m_pc(pc)
	{
		message_ += std::to_string(++counter_);
		std::fill(data_.begin(), data_.end(), '\0');
	}

	static std::shared_ptr<ServerConnection> Create(const boost::asio::any_io_executor &ex, ProducerConsumer &pc)
	{
		return std::shared_ptr<ServerConnection>(new ServerConnection(ex, pc));
	}

	boost::asio::ip::tcp::socket &GetSocket()
	{
		return socket_;
	}

	void Start()
	{
		socket_.async_read_some(boost::asio::buffer(data_),
														boost::bind(&ServerConnection::HandleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}


private:
	// Handle the read operation completion.
	void HandleRead(const boost::system::error_code &error, size_t bytes_transferred)
	{
		if (!error)
			{
			recv_ += std::string{ &data_[0], bytes_transferred };

			if (bytes_transferred == BUF_SIZE)
				{
				// read more
				socket_.async_read_some(boost::asio::buffer(data_),
					boost::bind(&ServerConnection::HandleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
				}
			else
				{
				std::cout 
					<< "Rx: OK\n";

				m_pc.Produce(recv_);

				// acknowledge the client
				boost::asio::async_write(socket_, boost::asio::buffer(message_),
					std::bind(&ServerConnection::HandleWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
				}
			}
		else
			{
			std::cout << "Error: " << error.message() << std::endl;
			}
	}

	// Handle the write operation completion.
	void HandleWrite(const boost::system::error_code & /*error*/, size_t /*bytes_transferred*/)
	{
		std::cout << "Tx: Ack " << counter_ << "\n";
	}

private:
	boost::asio::ip::tcp::socket socket_;
	std::string message_{ "Ack " };
	std::string terminal_{ "<END>" };
	std::array<char, BUF_SIZE> data_;
	std::string recv_;
	static int counter_;
	ProducerConsumer &m_pc;
};

int ServerConnection::counter_ = 0;


class Server
{
public:
	Server(boost::asio::io_service &io, boost::asio::ip::port_type port, ProducerConsumer &pc)
		: acceptor_(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
		, m_pc(pc)
	{
		StartAccept();
	}

	void StartAccept()
	{
		const boost::asio::any_io_executor &ex = acceptor_.get_executor();
		std::shared_ptr<ServerConnection> connection = ServerConnection::Create(ex, m_pc);
		boost::asio::ip::tcp::socket &socket = connection->GetSocket();
		acceptor_.async_accept(socket, std::bind(&Server::HandleAccept, this, connection, std::placeholders::_1));
	}

	void HandleAccept(std::shared_ptr<ServerConnection> connection, const boost::system::error_code &ec)
	{
		if (!ec)
			{
			connection->Start();
			}
		StartAccept();
	}

private:
	boost::asio::ip::tcp::acceptor acceptor_;
	ProducerConsumer &m_pc;
};


int main(int argc, char *argv[])
{
	cxxopts::Options options("serverba.exe", "Analytics server, by Alasdair Craig");
	options.add_options()
		("l,listen", "Server port to listen on", cxxopts::value<unsigned short>()->default_value("54321"))
		("h,host", "MySQL hostname [host]", cxxopts::value<std::string>()->default_value("localhost"))
		("u,user", "MySQL username [user], alternatively env var ANALYTICS_MYSQL_USER", cxxopts::value<std::string>()->default_value(""))
		("p,pass", "MySQL password [pass], alternatively env var ANALYTICS_MYSQL_PASS", cxxopts::value<std::string>()->default_value(""))
		("help", "Print usage");

	auto params = options.parse(argc, argv);

	if (params.count("help"))
		{
		std::cout << options.help() << std::endl;
		exit(0);
		}

	unsigned short port = params["listen"].as<unsigned short>();
	std::string dbhost = params["host"].as<std::string>();
	std::string dbuser = params["user"].as<std::string>();
	std::string dbpass = params["pass"].as<std::string>();

	std::shared_ptr<sql::Connection> mysql;

#if defined(_WIN32_WINNT)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
	// Revert to environment variables if not set on command-line
	if (dbuser.empty())
		{
		const char *env = std::getenv("ANALYTICS_MYSQL_USER");
		if (env)
			dbuser = env;
		}
	if (dbpass.empty())
		{
		const char *env = std::getenv("ANALYTICS_MYSQL_PASS");
		if (env)
			dbpass = env;
		}
#if defined(_WIN32_WINNT)
#pragma warning(pop)
#endif

	std::string error;
	try
		{
		sql::Driver *driver = get_driver_instance();
		if (driver)
			{
			sql::ConnectOptionsMap options;
			options["hostName"] = dbhost;
			options["userName"] = dbuser;
			options["password"] = dbpass;
			options["schema"] = "Analytics";
			options["OPT_CONNECT_TIMEOUT"] = 5;

			std::cout << "Connecting to MySQL " << dbuser << "@" << dbhost<< " ...\n";
			mysql = std::shared_ptr<sql::Connection>(driver->connect(options));
			}
		}
	catch (sql::SQLException &e)
		{
		error = e.what();
		}

	if (mysql && mysql->isValid() && error.empty())
		{
		std::cout << "MySQL database OK ... yes\n";
		}
	else
		{
		std::cout 
			<< "MySQL database OK ... no\n"
			<< "Error: "
			<< error
			<< "\nAborting.\n";
		return EXIT_FAILURE;
		}

	ProducerConsumer pc(mysql);

	std::cout << "Listening on port " << port << " ...\n";

	try
		{
		boost::asio::io_service io;
		Server server(io, port, pc);
		io.run();
		}
	catch (std::exception &ex)
		{
		std::cout << ex.what() << "\n";
		}

	std::cout << "Ending.\n";

	return 0;
}

