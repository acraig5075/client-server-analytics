#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <sqlite3.h>
#include <iostream>
#include <filesystem>
#include "ProducerConsumer.h"
#include "..\Utilities\SqlUtils.h"


const size_t BUF_SIZE = 100;

class Connection : public std::enable_shared_from_this<Connection>
{
public:
	Connection(const boost::asio::any_io_executor &ex, ProducerConsumer &pc)
		: socket_(ex)
		, m_pc(pc)
	{
		message_ += std::to_string(++counter_);
		std::fill(data_.begin(), data_.end(), '\0');
	}

	static std::shared_ptr<Connection> create(const boost::asio::any_io_executor &ex, ProducerConsumer &pc)
	{
		return std::shared_ptr<Connection>(new Connection(ex, pc));
	}

	boost::asio::ip::tcp::socket &socket()
	{
		return socket_;
	}

	void start()
	{
		socket_.async_read_some(boost::asio::buffer(data_),
														boost::bind(&Connection::handle_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}


private:
	// Handle the read operation completion.
	void handle_read(const boost::system::error_code &error, size_t bytes_transferred)
	{
		if (!error)
			{
			recv_ += std::string{ &data_[0], bytes_transferred };

			if (bytes_transferred == BUF_SIZE)
				{
				// read more
				socket_.async_read_some(boost::asio::buffer(data_),
					boost::bind(&Connection::handle_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
				}
			else
				{
				std::cout 
					<< "Rx: OK\n";

				m_pc.produce(recv_);

				// acknowledge the client
				boost::asio::async_write(socket_, boost::asio::buffer(message_),
					std::bind(&Connection::handle_write, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
				}
			}
		else
			{
			std::cout << "Error: " << error.message() << std::endl;
			}
	}

	// Handle the write operation completion.
	void handle_write(const boost::system::error_code & /*error*/, size_t /*bytes_transferred*/)
	{
		std::cout << "Tx: Ack" << "\n";
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

int Connection::counter_ = 0;


class Server
{
public:
	Server(boost::asio::io_service &io, ProducerConsumer &pc)
		: acceptor_(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 13))
		, m_pc(pc)
	{
		start_accept();
	}

	void start_accept()
	{
		const boost::asio::any_io_executor &ex = acceptor_.get_executor();
		std::shared_ptr<Connection> connection = Connection::create(ex, m_pc);
		boost::asio::ip::tcp::socket &socket = connection->socket();
		acceptor_.async_accept(socket, std::bind(&Server::handle_accept, this, connection, std::placeholders::_1));
	}

	void handle_accept(std::shared_ptr<Connection> connection, const boost::system::error_code &ec)
	{
		if (!ec)
			{
			connection->start();
			}
		start_accept();
	}

private:
	boost::asio::ip::tcp::acceptor acceptor_;
	ProducerConsumer &m_pc;
};


int main()
{
	std::filesystem::path dbPath = R"(analytics.db)";

	if (std::filesystem::exists(dbPath))
		{
		std::error_code ec;
		if (!std::filesystem::remove(dbPath, ec))
			{
			std::cerr
				<< "Unable to remove existing database\n"
				<< ec.message()
				<< "\n";
			}
		}

	sqlite3 *db = nullptr;
	int error = sqlite3_open(dbPath.string().c_str(), &db);

	if (error)
		{
		std::cerr << "Error open DB " << sqlite3_errmsg(db) << "\n";
		return (-1);
		}

	::sqlite3_exec(db, CreateInstanceTable().c_str(), nullptr, nullptr, nullptr);
	::sqlite3_exec(db, CreateModuleTable("SurveyTbl").c_str(), nullptr, nullptr, nullptr);
	::sqlite3_exec(db, CreateModuleTable("TerrainTbl").c_str(), nullptr, nullptr, nullptr);
	::sqlite3_exec(db, CreateModuleTable("RoadTbl").c_str(), nullptr, nullptr, nullptr);
	::sqlite3_exec(db, CreateModuleTable("SewerTbl").c_str(), nullptr, nullptr, nullptr);
	::sqlite3_exec(db, CreateModuleTable("StormTbl").c_str(), nullptr, nullptr, nullptr);
	::sqlite3_exec(db, CreateModuleTable("WaterTbl").c_str(), nullptr, nullptr, nullptr);
	::sqlite3_exec(db, CreateModuleTable("SignageTbl").c_str(), nullptr, nullptr, nullptr);

	ProducerConsumer pc(db);

	boost::asio::io_service io;
	Server server(io, pc);
	io.run();

	return 0;
}

