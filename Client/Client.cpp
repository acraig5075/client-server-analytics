// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <boost/asio.hpp>
#include <cxxopts.hpp>
#include "../Utilities/Analytics.h"
#include "../Utilities/Commands.h"

#if defined(_WIN32_WINNT)
#include <Windows.h>
#endif

using boost::asio::ip::tcp;

void SendAnalytics(const std::string &server, const std::string &port, const Analytics &analytics)
{
	std::string str = analytics.ToJson();

	std::cout << "Tx: " << str << "\n";

	boost::asio::io_context io_context;

	tcp::resolver resolver(io_context);
	auto endpoints = resolver.resolve(server, port);

	tcp::socket socket(io_context);
	boost::asio::connect(socket, endpoints);

	if (!socket.is_open())
		{
		std::cout << "Failed to connect.\n";
		return;
		}

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
	cxxopts::Options options("client.exe", "Analytics client simulator, by Alasdair Craig");
	options.add_options()
		("s,server", "Server address", cxxopts::value<std::string>()->default_value("127.0.0.1"))
		("p,port", "Server port", cxxopts::value<std::string>()->default_value("54321"))
		("c,commands", "Number of commands per module to simulate", cxxopts::value<int>()->default_value("5"))
		("e,sends", "Number of simultaneous sends to simulate", cxxopts::value<int>()->default_value("4"))
		("w,wait", "Wait interval (in seconds) between successive sends", cxxopts::value<long>()->default_value("1"))
		("h,help", "Print usage");

	auto params = options.parse(argc, argv);

	if (params.count("help"))
		{
		std::cout << options.help() << std::endl;
		exit(0);
		}

	std::string server = params["server"].as<std::string>();
	std::string port = params["port"].as<std::string>();
	int count = params["commands"].as<int>();
	int sends = params["sends"].as<int>();
	long wait = params["wait"].as<long>();

	int total = 0;

	std::string startTime = FormattedCurrentTime();

	try
		{
		bool exit = false;

		while (exit == false)
			{
			auto SendThreadFunc = [count, server, port]()
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

				// Send the stream to the server
				SendAnalytics(server, port, analytics);
				};

			for (int tid = 0; tid < sends; ++tid)
				{
				std::thread(SendThreadFunc).detach();
				total++;
				}

			std::this_thread::sleep_for(std::chrono::seconds(wait));

			//exit = true;

			// Continue until <Esc> or <Ctrl-C> press
#if defined(_WIN32_WINNT)
			if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
				exit = true;
#endif
			}

		std::string endTime = FormattedCurrentTime();

		std::cout 
			<< "Start time = " << startTime << "\n"
			<< "End time = " << endTime << "\n"
			<< "Total sent = " << total << "\n";
		}
	catch (std::exception &e)
		{
		std::cerr << e.what() << std::endl;
		}
	return 0;
}