// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <sstream>
#include <boost/asio.hpp>
#include <format>

using boost::asio::ip::tcp;


int main()
{
	try
		{
		boost::asio::io_context io_context;

		tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 13));

		for (;;)
			{
			tcp::socket socket(io_context);
			acceptor.accept(socket);

			std::array<char, 100> buf;
			std::string str;
			boost::system::error_code error;

			size_t n = socket.read_some(boost::asio::buffer(buf), error);
			while (n > 0)
				{
				str += std::string{ &buf[0], n };
				n = socket.read_some(boost::asio::buffer(buf), error);
				}

			if (error == boost::asio::error::eof)
				break; // Connection closed cleanly by peer.
			else if (error)
				throw boost::system::system_error(error); // Some other error.

			std::cout << str << "\n";
			}
		}
	catch (const std::exception &e)
		{
		std::cerr << e.what() << std::endl;
		}
}

