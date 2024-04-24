// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <boost\asio.hpp>
#include <Windows.h>
#include "..\Utilities\Analytics.h"
#include "..\Utilities\Commands.h"


using boost::asio::ip::tcp;

std::string MakeDaytimeString()
{
	char buf[100];
	SYSTEMTIME st;
	GetLocalTime(&st); // Local time
	sprintf(buf, "%.2u:%.2u:%.2u", st.wHour, st.wMinute, st.wSecond);
	return std::string(buf);
}

void send(const Analytics &analytics)
{
	std::string str = analytics.ToJson();

	std::cout << "Tx: " << str << "\n";

	boost::asio::io_context io_context;

	tcp::resolver resolver(io_context);
	auto endpoints = resolver.resolve("127.0.0.1", "13");

	tcp::socket socket(io_context);
	boost::asio::connect(socket, endpoints);

	boost::system::error_code error;
	socket.write_some(boost::asio::buffer(&str.c_str()[0], str.length()), error);

	if (error == boost::asio::error::eof)
		return; // Connection closed cleanly by peer.
	else if (error)
		throw boost::system::system_error(error); // Some other error.
	else
		{
		std::array<char, 1024> ack;
		size_t n = socket.read_some(boost::asio::buffer(ack), error);
		if (n > 0 && !error)
			{
			std::cout << "Rx: " << std::string(&ack[0], n) << "\n";
			}
		}

	return;
}


int main(int argc, char *argv[])
{
	size_t count = 5; // Number of commands per module to simulate

	if (argc > 1)
		{
		int param = std::atoi(argv[1]);
		if (param > 0)
			count = param;
		}

	int total = 0;

	try
		{
		bool exit = false;

		while (exit == false)
			{
			auto SendThreadFunc = [count]()
				{
				// Collection of commands as analytics
				Analytics analytics;
				analytics.AddCommands(SURVEY_ID,  GetCommandSelection(SURVEY_ID,  count));
				analytics.AddCommands(TERRAIN_ID, GetCommandSelection(TERRAIN_ID, count));
				analytics.AddCommands(ROAD_ID,    GetCommandSelection(ROAD_ID,    count));
				analytics.AddCommands(SEWER_ID,   GetCommandSelection(SEWER_ID,   count));
				analytics.AddCommands(STORM_ID,   GetCommandSelection(STORM_ID,   count));
				analytics.AddCommands(WATER_ID,   GetCommandSelection(WATER_ID,   count));
				analytics.AddCommands(SIGNAGE_ID, GetCommandSelection(SIGNAGE_ID, count));
				analytics.GatherUsage();

				//std::string data = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
				//std::string data = "short";

				// Send the stream to the server
				send(analytics);
				};

			int numSends = 4;
			for (int tid = 0; tid < numSends; ++tid)
				{
				std::thread(SendThreadFunc).detach();
				total++;
				}

			std::this_thread::sleep_for(std::chrono::seconds(1));

			//exit = true;

			// Continue until Esc press
			if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
				exit = true;
			//std::cout << "Press <Esc> to exit\n";
			}

		std::cout << "Total sent = " << total << "\n";
		}
	catch (std::exception &e)
		{
		std::cerr << e.what() << std::endl;
		}
	return 0;
}