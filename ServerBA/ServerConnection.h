#pragma once
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

class ProducerConsumer;

const size_t BUF_SIZE = 100;

class ServerConnection : public std::enable_shared_from_this<ServerConnection>
{
public:
	ServerConnection(const boost::asio::any_io_executor &ex, ProducerConsumer &pc);

	static std::shared_ptr<ServerConnection> Create(const boost::asio::any_io_executor &ex, ProducerConsumer &pc);

	boost::asio::ip::tcp::socket &GetSocket();

	void Start();

private:
	// Handle the read operation completion.
	void HandleRead(const boost::system::error_code &error, size_t bytes_transferred);

	// Handle the write operation completion.
	void HandleWrite(const boost::system::error_code & /*error*/, size_t /*bytes_transferred*/);

private:
	boost::asio::ip::tcp::socket m_socket;
	std::string m_message{ "Ack " };
	std::string m_terminal{ "<END>" };
	std::array<char, BUF_SIZE> m_data;
	std::string m_recv;
	static int m_counter;
	ProducerConsumer &m_pc;
};


class Server
{
public:
	Server(boost::asio::io_service &io, boost::asio::ip::port_type port, ProducerConsumer &pc);

	void StartAccept();

	void HandleAccept(std::shared_ptr<ServerConnection> connection, const boost::system::error_code &ec);

private:
	boost::asio::ip::tcp::acceptor m_acceptor;
	ProducerConsumer &m_pc;
};

