#include "ServerConnection.h"
#include "ProducerConsumer.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>

int ServerConnection::m_counter = 0;

ServerConnection::ServerConnection(const boost::asio::any_io_executor &ex, ProducerConsumer &pc)
	: m_socket(ex)
	, m_pc(pc)
{
	m_message += std::to_string(++m_counter);
	std::fill(m_data.begin(), m_data.end(), '\0');
}

std::shared_ptr<ServerConnection> ServerConnection::Create(const boost::asio::any_io_executor &ex, ProducerConsumer &pc)
{
	return std::shared_ptr<ServerConnection>(new ServerConnection(ex, pc));
}

boost::asio::ip::tcp::socket &ServerConnection::GetSocket()
{
	return m_socket;
}

void ServerConnection::Start()
{
	m_socket.async_read_some(boost::asio::buffer(m_data),
													 boost::bind(&ServerConnection::HandleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}


// Handle the read operation completion.
void ServerConnection::HandleRead(const boost::system::error_code &error, size_t bytes_transferred)
{
	if (!error)
		{
		m_recv += std::string{ &m_data[0], bytes_transferred };

		if (bytes_transferred == BUF_SIZE)
			{
			// read more
			m_socket.async_read_some(boost::asio::buffer(m_data),
															 boost::bind(&ServerConnection::HandleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
			}
		else
			{
			std::cout
					<< "Rx: OK\n";

			m_pc.Produce(m_recv);

			// acknowledge the client
			boost::asio::async_write(m_socket, boost::asio::buffer(m_message),
															 std::bind(&ServerConnection::HandleWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
			}
		}
	else
		{
		std::cout << "Error: " << error.message() << std::endl;
		}
}

// Handle the write operation completion.
void ServerConnection::HandleWrite(const boost::system::error_code & /*error*/, size_t /*bytes_transferred*/)
{
	std::cout << "Tx: Ack " << m_counter << "\n";
}











Server::Server(boost::asio::io_service &io, boost::asio::ip::port_type port, ProducerConsumer &pc)
	: m_acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
	, m_pc(pc)
{
	std::cout << "Server running ... yes\n";
	std::cout << "Listening on port " << port << " ...\n";

	StartAccept();
}

void Server::StartAccept()
{
	const boost::asio::any_io_executor &ex = m_acceptor.get_executor();
	std::shared_ptr<ServerConnection> connection = ServerConnection::Create(ex, m_pc);
	boost::asio::ip::tcp::socket &socket = connection->GetSocket();
	m_acceptor.async_accept(socket, std::bind(&Server::HandleAccept, this, connection, std::placeholders::_1));
}

void Server::HandleAccept(std::shared_ptr<ServerConnection> connection, const boost::system::error_code &ec)
{
	if (!ec)
		{
		connection->Start();
		}
	StartAccept();
}
