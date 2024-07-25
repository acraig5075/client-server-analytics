#include "pch.h"
#include "SqlUtils.h"
#include <iostream>
#include <sstream>
#include <format>
#include <nlohmann/json.hpp>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>


static std::vector<std::pair<std::string, std::string>> tableNames =
{
	{ "Survey",  "SurveyTbl" },
	{ "Terrain", "TerrainTbl" },
	{ "Road",    "RoadTbl" },
	{ "Sewer",   "SewerTbl" },
	{ "Storm",   "StormTbl" },
	{ "Water",   "WaterTbl" },
	{ "Signage", "SignageTbl" },
};


std::string CreateInstanceTable()
	{
	return std::string{ "CREATE TABLE InstanceTbl ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		"DateTime TEXT NOT NULL, "
		"License TEXT NOT NULL, "
		"PCName TEXT NOT NULL, "
		"ProgramCode TEXT NOT NULL, "
		"ProgramVersion INT NOT NULL, "
		"ProgramRelease INT NOT NULL);" 
		};
	}

std::string CreateModuleTable(const std::string &moduleName)
{
	return std::format("CREATE TABLE {} ("
										 "id INTEGER PRIMARY KEY AUTOINCREMENT, "
										 "InstanceID INT NOT NULL, "
										 "Command TEXT NOT NULL, "
										 "Count INT NOT NULL);",
										 moduleName);
}

static std::string FindTableName(const std::string &moduleName)
{
	auto itr = std::find_if(tableNames.begin(), tableNames.end(), [moduleName](const auto & pair)
		{
		return pair.first == moduleName;
		});

	if (itr == tableNames.end())
		return {};

	return itr->second;
}

std::string TrimRight(const std::string &str, const std::string &chars)
{
	size_t endpos = str.find_last_not_of(chars);
	if (std::string::npos != endpos)
		{
		return str.substr(0, endpos + 1);
		}
	return{};
}

int InsertDatabase(const std::shared_ptr<sql::Connection> &db, const std::string &jstr)
{
	if (!db)
		return 0;

	nlohmann::json json;

	try
		{
		json = nlohmann::json::parse(jstr);
		}
	catch (nlohmann::json::parse_error &ex)
		{
		std::cerr << ex.what() << "\nInput:\n" << jstr << "\n";
		return 0;
		}

	sql::Statement *stmt = nullptr;
	sql::PreparedStatement *pstmt = nullptr;
	sql::ResultSet *res = nullptr;
	bool ok = false;

	db->setAutoCommit(false); // Starts transaction

	pstmt = db->prepareStatement("INSERT INTO InstanceTbl (DateTime, License, PCName, ProgramCode, ProgramVersion, ProgramRelease) VALUES (?, ?, ?, ?, ?, ?)");
	pstmt->setString(1, json.at("datetime").template get<std::string>());
	pstmt->setString(2, json.at("code").template get<std::string>());
	pstmt->setString(3, json.at("pcname").template get<std::string>());
	pstmt->setString(4, json.at("program").template get<std::string>());
	pstmt->setInt(5, json.at("version").template get<int>());
	pstmt->setInt(6, json.at("release").template get<int>());
	ok = pstmt->executeUpdate();
	delete pstmt;

	uint32_t instanceID = 0;
	if (ok)
		{
		stmt = db->createStatement();
		res = stmt->executeQuery("SELECT LAST_INSERT_ID() AS id");
		if (res->first())
			instanceID = res->getUInt("id");
		delete stmt;
		}

	if (instanceID == 0)
		{
		db->rollback();
		return 0;
		}

	int numInserts = 0;

	nlohmann::json subjson = json["analytics"];

	for (auto &v : subjson)
		{
		std::string moduleName = v.at("module").template get<std::string>();
		std::string usage = v.at("usage").template get<std::string>();

		std::string tableName = FindTableName(moduleName);
		if (tableName.empty())
			continue;

		std::string bulk = std::format("INSERT INTO {} (InstanceID, Command, Count) VALUES ", tableName);

		std::stringstream ss(usage);
		std::string word;
		int values = 0;

		while (ss >> word)
			{
			size_t found = word.find(',');
			std::string command = word.substr(0, found);
			std::string second = word.substr(found + 1);
			int count = std::atoi(second.c_str());

			if (command.empty() || second.empty() || count == 0)
				continue;

			bulk += std::format(R"(({}, '{}', {}), )", instanceID, command, count);
			values++;
			}

		bulk = TrimRight(bulk, ", ");

		if (values > 0)
			{
			stmt = db->createStatement();
			ok = stmt->executeUpdate(bulk.c_str());
			delete stmt;
			if (ok)
				numInserts++;
			}
		}

	if (numInserts > 0)
		db->commit();
	else
		db->rollback();

	return numInserts;
}
