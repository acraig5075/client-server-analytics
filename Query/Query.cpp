// Query.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <mysql\jdbc.h>
#include <cxxopts.hpp>

static const std::vector<std::string> tableNames =
{
	"SewerTbl",
	"SurveyTbl",
	"TerrainTbl",
	"RoadTbl",
	"StormTbl",
	"WaterTbl",
	"SignageTbl",
};


void ConnectToDatabase(const std::string &hostName, const std::string &userName, const std::string &password, std::shared_ptr<sql::Connection> &connection)
{
	// Attempt to establish a connection
	sql::Driver *driver = get_driver_instance();
	if (driver)
		{
		sql::ConnectOptionsMap options;
		options["hostName"] = hostName;
		options["userName"] = userName;
		options["password"] = password;
		options["schema"] = "Analytics";
		options["OPT_CONNECT_TIMEOUT"] = 5;

		connection = std::shared_ptr<sql::Connection>(driver->connect(options));
		}
}

void PrintRecordsSince(std::shared_ptr<sql::Connection> &connection, const std::string &timestamp)
{
	if (timestamp.empty())
		return;

	sql::PreparedStatement *stmt = connection->prepareStatement("SELECT COUNT(1) FROM InstanceTbl WHERE DateTime > ?");
	stmt->setDateTime(1, timestamp);
	sql::ResultSet *res = stmt->executeQuery();
	res->first();
	std::cout << res->getInt(1) << " InstanceTbl records.\n";

	delete stmt;
	delete res;

	for (const std::string &table : tableNames)
		{
		std::string str = std::format("SELECT COUNT(1) FROM {} JOIN InstanceTbl ON {}.InstanceID = InstanceTbl.id WHERE InstanceTbl.DateTime > ?", table, table);
		sql::PreparedStatement *stmt = connection->prepareStatement(str);
		stmt->setDateTime(1, timestamp);
		sql::ResultSet *res = stmt->executeQuery();
		res->first();
		std::cout << res->getInt(1) << " " << table << " records.\n";
		delete stmt;
		delete res;
		}

	std::cout << "\n";
}

void PrintInUse(const std::string &table, const std::vector<std::pair<std::string, int>> &inUse)
{
	size_t width = 0;
	for (const auto &pr : inUse)
		{
		if (pr.first.length() > width)
			width = pr.first.length();
		}

	std::string heading = table;
	std::string underline(heading.length(), '-');
	std::cout
			<< heading << "\n"
			<< underline << "\n";

	for (const auto &pr : inUse)
		{
		std::cout
				<< std::left
				<< std::setw(width)
				<< pr.first
				<< " "
				<< pr.second
				<< "\n";
		}

	std::cout << "\n";
}

void PrintInUseStats(std::shared_ptr<sql::Connection> &connection, int limit)
{
	for (const std::string &table : tableNames)
		{
		std::vector<std::pair<std::string, int>> inUse;

		std::string str = std::format("SELECT Command, SUM(Count) AS Sum FROM {} GROUP BY Command ORDER BY Sum DESC LIMIT {}", table, limit);
		sql::PreparedStatement *stmt = connection->prepareStatement(str);
		sql::ResultSet *res = stmt->executeQuery();
		while (res->next())
			inUse.push_back(std::make_pair(res->getString(1), res->getInt(2)));

		delete stmt;
		delete res;

		PrintInUse(table, inUse);
		}
}

// Front and back trimming
std::string Trim(const std::string &value, const std::string &chars)
{
	auto right_trim = [chars](const std::string & str)
		{
		size_t endpos = str.find_last_not_of(chars);
		if (std::string::npos != endpos)
			{
			return str.substr(0, endpos + 1);
			}
		return std::string{};
		};

	auto left_trim = [chars](const std::string & str)
		{
		size_t startpos = str.find_first_not_of(chars);
		if (std::string::npos != startpos)
			{
			return str.substr(startpos);
			}
		return std::string{};
		};

	return left_trim(right_trim(value));
}

int main(int argc, char *argv[])
{
	cxxopts::Options options("query.exe [stats|since]", "Analytics server queries, by Alasdair Craig");
	options.add_options()
		("m,mysql", "MySQL target [server]", cxxopts::value<std::string>()->default_value("localhost"))
		("u,user", "MySQL user [user]", cxxopts::value<std::string>()->default_value(""))
		("p,pass", "MySQL password [pass]", cxxopts::value<std::string>()->default_value(""))
		("s,stats", "Print in-use usage stats")
		("l,limit", "Limit in-use usage stats to top n records", cxxopts::value<int>()->default_value("3"))
		("r,since", "Print new record counts since [timestamp (\"YYYY-MM-DD HH:MM:SS\")]", cxxopts::value<std::string>())
		("h,help", "Print usage");

	auto params = options.parse(argc, argv);

	if (params.count("help") || (params.count("stats") == 0 && params.count("since") == 0))
		{
		std::cout << options.help() << std::endl;
		exit(EXIT_SUCCESS);
		}

	std::string dbserver = params["mysql"].as<std::string>();
	std::string dbuser = params["user"].as<std::string>();
	std::string dbpass = params["pass"].as<std::string>();
	bool usedStats = params.count("stats") > 0;
	int limit = params["limit"].as<int>();
	std::string timestamp = params.count("since") > 0 ? params["since"].as<std::string>() : "";

	timestamp = Trim(timestamp, "\'\"");

	std::shared_ptr<sql::Connection> connection;

	try
		{
		ConnectToDatabase(dbserver, dbuser, dbpass, connection);
		std::cout << "MySQL connection ... yes\n";
		}
	catch (sql::SQLException &ex)
		{
		std::cout << "MySQL connection ... no\n";
		std::cout << ex.what() << "\n";
		return EXIT_FAILURE;
		}

	if (!timestamp.empty())
		PrintRecordsSince(connection, timestamp);

	if (usedStats)
		PrintInUseStats(connection, limit);

	std::cout << "Done\n";

	return EXIT_SUCCESS;
}
