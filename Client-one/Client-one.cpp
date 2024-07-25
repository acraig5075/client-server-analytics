// Client-one.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "../Utilities/Analytics.h"
#include "../Utilities/Commands.h"


#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 54321
#define ACK_TIMEOUT 5000 // Timeout in milliseconds

#pragma comment(lib, "ws2_32.lib")


bool SendAnalytics(const std::string &server, unsigned short port, const Analytics &analytics)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
		std::cerr << "WSAStartup failed\n";
		return false;
		}

	// Make the connection
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET)
		{
		std::cerr << "Failed to create socket";
		WSACleanup();
		return false;
		}

	sockaddr_in service;
	service.sin_family = AF_INET;
	inet_pton(AF_INET, SERVER_IP, &service.sin_addr.s_addr);
	service.sin_port = htons(SERVER_PORT);

	if (connect(clientSocket, (SOCKADDR *)&service, sizeof(service)) == SOCKET_ERROR)
		{
		std::cerr << "connect failed\n";
		closesocket(clientSocket);
		WSACleanup();
		return false;
		}

	// Send message in chunks
	std::string message = analytics.ToJson();
	int bytesSent = 0;
	const size_t BUFFERSIZE = 1024;
	for (size_t offset = 0; offset < message.size(); offset += bytesSent)
		{
		size_t bytesToSend = message.size() - offset;
		if (bytesToSend > BUFFERSIZE)
			bytesToSend = BUFFERSIZE;

		bytesSent = send(clientSocket, &message[offset], static_cast<int>(bytesToSend), 0);
		if (bytesSent == SOCKET_ERROR)
			{
			std::cout << "Tx: Failed\n";
			break;
			}
		}
	std::cout << "Tx: OK\n";


	// Wait for acknowledgment
	char buffer[256] = { 0 };
	struct timeval tv;
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(clientSocket, &readfds);
	tv.tv_sec = ACK_TIMEOUT / 1000; // Convert to seconds
	tv.tv_usec = (ACK_TIMEOUT % 1000) * 1000; // Convert remaining milliseconds to microseconds

	int ret = select(static_cast<int>(clientSocket) + 1, &readfds, NULL, NULL, &tv);
	if (ret <= 0)
		{
		std::cerr << "select failed\n";
		closesocket(clientSocket);
		WSACleanup();
		return false;
		}

	if (FD_ISSET(clientSocket, &readfds))
		{
		recv(clientSocket, buffer, sizeof(buffer), 0);
		std::cout << "Rx: " << buffer << "\n";
		}
	else
		{
		std::cerr << "Timeout waiting for acknowledgment\n";
		}

	closesocket(clientSocket);
	WSACleanup();
	return true;
}


int main()
{
	// Collection of commands as analytics
	size_t count = 10;
	Analytics analytics;
	analytics.AddCommands(SURVEY_ID, GetCommandSelection(SURVEY_ID, count));
	analytics.AddCommands(TERRAIN_ID, GetCommandSelection(TERRAIN_ID, count));
	analytics.AddCommands(ROAD_ID, GetCommandSelection(ROAD_ID, count));
	analytics.AddCommands(SEWER_ID, GetCommandSelection(SEWER_ID, count));
	analytics.AddCommands(STORM_ID, GetCommandSelection(STORM_ID, count));
	analytics.AddCommands(WATER_ID, GetCommandSelection(WATER_ID, count));
	analytics.AddCommands(SIGNAGE_ID, GetCommandSelection(SIGNAGE_ID, count));
	analytics.GatherUsage();

	int ret = SendAnalytics(SERVER_IP, SERVER_PORT, analytics);

	return 0;
}
