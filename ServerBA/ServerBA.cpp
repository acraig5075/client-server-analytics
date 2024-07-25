#include <iostream>
#include <cxxopts.hpp>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/prepared_statement.h>
#include "ServerConnection.h"
#include "ProducerConsumer.h"


// Get a system environment variable
std::string GetEnvironmentVar(const std::string &varName)
{
#if defined(_WIN32_WINNT)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
		const char *env = std::getenv(varName.c_str());
		if (env)
			return std::string{ env };
		return {};
#if defined(_WIN32_WINNT)
#pragma warning(pop)
#endif
}

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

	// Revert to environment variables if not set on command-line
	if (dbuser.empty())
		dbuser = GetEnvironmentVar("ANALYTICS_MYSQL_USER");
	if (dbpass.empty())
		dbpass = GetEnvironmentVar("ANALYTICS_MYSQL_PASS");

	std::shared_ptr<sql::Connection> mysql;
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
		std::cout << "MySQL database OK ... no\nError: " << error << "\nAborting.\n";
		return EXIT_FAILURE;
		}

	ProducerConsumer pc(mysql);

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

