// Client-one.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cxxopts.hpp>
#include "../Utilities/Analytics.h"
#include "../Utilities/Commands.h"


#define EST_TIMEOUT 2500 // Timeout in milliseconds for establishing the connection
#define ACK_TIMEOUT 5000 // Timeout in milliseconds for acknowledgement

#pragma comment(lib, "ws2_32.lib")


bool SendAnalytics(const std::string &server, unsigned short port, const Analytics &analytics)
{
	// Initialise Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
		std::cerr << "WSAStartup failed\n";
		return false;
		}

	// Create a socket for connecting with the server
	SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket == INVALID_SOCKET)
		{
		std::cerr << "Failed to create socket with error: " << WSAGetLastError() << "\n";
		WSACleanup();
		return false;
		}

	// Set the socket to non-blocking mode
	unsigned long mode = 1; // 1 means non-blocking
	int result = ::ioctlsocket(socket, FIONBIO, &mode);
	if (result != NO_ERROR)
		{
		std::cerr << "ioctlsocket failed with error: " << WSAGetLastError() << "\n";
		closesocket(socket);
		WSACleanup();
		return false;
		}

	sockaddr_in service;
	service.sin_family = AF_INET;
	inet_pton(AF_INET, server.c_str(), &service.sin_addr.s_addr);
	service.sin_port = htons(port);

	// Attempt to connect to the server, (non-blocking returns immediately)
	result = ::connect(socket, (SOCKADDR *)&service, sizeof(service));
	if (result == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
		{
		std::cerr << "connect failed with error: " << WSAGetLastError() << "\n";
		closesocket(socket);
		WSACleanup();
		return false;
		}

	// Set timeout for select
	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(socket, &writefds);
	timeval connTimeout;
	connTimeout.tv_sec = EST_TIMEOUT / 1000; // full seconds
	connTimeout.tv_usec = EST_TIMEOUT % 1000; // remaining microseconds

	// Wait for a writeable connection to be established or fail within the timeout period
	result = ::select(static_cast<int>(socket) + 1, NULL, &writefds, NULL, &connTimeout);

	if (result == SOCKET_ERROR)
		{
		std::cerr << "select (for connection) failed with error: " << WSAGetLastError() << "\n";
		closesocket(socket);
		WSACleanup();
		return false;
		}
	else if (result == 0)
		{
		std::cerr << "Establishing the connection timed out.\n";
		closesocket(socket);
		WSACleanup();
		return false;
		}
	else
		{
		if (!FD_ISSET(socket, &writefds)) // not ready for writing
			{
			std::cerr << "Connection failed.\n";
			closesocket(socket);
			WSACleanup();
			return false;
			}

		std::cout << "Connection established.\n";
		}

	//// Switch the socket to blocking mode before sending data
	//mode = 0; // 0 means blocking mode
	//result = ioctlsocket(socket, FIONBIO, &mode);
	//if (result != NO_ERROR)
	//	{
	//	std::cerr << "ioctlsocket failed with error: " << WSAGetLastError() << "\n";
	//	closesocket(socket);
	//	WSACleanup();
	//	return 1;
	//	}

	// Send message in chunks
	std::string message = analytics.ToJson();
	int bytesSent = 0;
	const size_t BUFFERSIZE = 1024;
	std::string txError;
	for (size_t offset = 0; offset < message.size(); offset += bytesSent)
		{
		size_t bytesToSend = message.size() - offset;
		if (bytesToSend > BUFFERSIZE)
			bytesToSend = BUFFERSIZE;

		bytesSent = ::send(socket, &message[offset], static_cast<int>(bytesToSend), 0);
		if (bytesSent == SOCKET_ERROR)
			{
			txError = "Tx: Failed (Socket error)";
			break;
			}
		else if (bytesSent != bytesToSend)
			{
			txError = "Tx: Failed (Sent bytes mismatch)";
			break;
			}
		}

	if (txError.empty())
		std::cout << "Tx: OK\n";
	else
		std::cout << txError << "\n";

	// Explicitly flush the socket to ensure all data is sent
	result = ::shutdown(socket, SD_SEND);
	if (result == SOCKET_ERROR)
		{
		std::cerr << "shutdown failed with error: " << WSAGetLastError() << "\n";
		closesocket(socket);
		WSACleanup();
		return false;
		}

	// Wait for acknowledgment
	char buffer[256] = { 0 };
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(socket, &readfds);
	timeval ackTimeout;
	ackTimeout.tv_sec = ACK_TIMEOUT / 1000;
	ackTimeout.tv_usec = (ACK_TIMEOUT % 1000) * 1000;

	result = ::select(static_cast<int>(socket) + 1, &readfds, NULL, NULL, &ackTimeout);
	if (result == SOCKET_ERROR)
		{
		std::cerr << "select (for acknowledgement) failed with error: " << WSAGetLastError() << "\n";
		closesocket(socket);
		WSACleanup();
		return false;
		}
	else
		{
		if (FD_ISSET(socket, &readfds)) // ready for reading
			{
			int bytesRead = ::recv(socket, buffer, sizeof(buffer), 0);

			if (bytesRead < 0)
				std::cerr << "Connection closed or error while waiting for acknowledgement\n";
			else if (bytesRead == 0)
				std::cerr << "No acknowledgement was sent, safe to close connection\n";
			else
				std::cout << "Rx: " << buffer << "\n";
			}
		else
			{
			std::cerr << "Timeout waiting for acknowledgment\n";
			}
		}

	closesocket(socket);
	WSACleanup();
	return true;
}


int main(int argc, char *argv[])
{
	cxxopts::Options options("client-one.exe", "Analytics client simulator, by Alasdair Craig");
	options.add_options()
	("s,server", "Server address", cxxopts::value<std::string>()->default_value("127.0.0.1"))
	("p,port", "Server port", cxxopts::value<std::string>()->default_value("54321"))
	("c,commands", "Number of commands per module to simulate", cxxopts::value<int>()->default_value("10"))
	("h,help", "Print usage");

	auto params = options.parse(argc, argv);

	if (argc == 1 || params.count("help"))
		{
		std::cout << options.help() << std::endl;
		exit(0);
		}

	std::string server = params["server"].as<std::string>();
	std::string port = params["port"].as<std::string>();
	size_t count = params["commands"].as<int>();

	if (server == "localhost")
		server = "127.0.0.1";

	// Collection of commands as analytics
	Analytics analytics;
	analytics.AddCommands(SURVEY_ID, GetCommandSelection(SURVEY_ID, count));
	analytics.AddCommands(TERRAIN_ID, GetCommandSelection(TERRAIN_ID, count));
	analytics.AddCommands(ROAD_ID, GetCommandSelection(ROAD_ID, count));
	analytics.AddCommands(SEWER_ID, GetCommandSelection(SEWER_ID, count));
	analytics.AddCommands(STORM_ID, GetCommandSelection(STORM_ID, count));
	analytics.AddCommands(WATER_ID, GetCommandSelection(WATER_ID, count));
	analytics.AddCommands(SIGNAGE_ID, GetCommandSelection(SIGNAGE_ID, count));
	analytics.GatherUsage();

	unsigned short portno = std::stoi(port);

	// Do the send (and receive)
	int ret = SendAnalytics(server, portno, analytics);

	return 0;
}
