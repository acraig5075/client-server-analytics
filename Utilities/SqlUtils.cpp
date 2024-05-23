#include "pch.h"
#include "SqlUtils.h"
#include <iostream>
#include <sstream>
#include <format>
#include <json11.hpp>
#include <sqlite3.h>
#include <mysql\jdbc.h>


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

int InsertDatabase(sqlite3 *db, const std::string &jstr)
{
	if (!db)
		return 0;

	std::string error;
	json11::Json json = json11::Json::parse(jstr, error);
	if (!error.empty())
		std::cerr << error << "\n";

	std::string sql;

	::sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

	sql = std::format(R"(INSERT INTO InstanceTbl (DateTime, License, PCName, ProgramCode, ProgramVersion, ProgramRelease) VALUES ('{}', '{}', '{}', '{}', {}, {});)",
		json["datetime"].string_value(),
		json["code"].string_value(),
		json["pcname"].string_value(),
		json["program"].string_value(),
		json["version"].int_value(),
		json["release"].int_value()
		);

	sqlite3_int64 instanceID = -1;
	if (SQLITE_OK == ::sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr))
		instanceID = ::sqlite3_last_insert_rowid(db);

	if (instanceID == -1)
		{
		::sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
		return 0;
		}

	int numInserts = 0;

	auto vec = json["analytics"].array_items();

	for (auto &v : vec)
		{
		std::string moduleName = v["module"].string_value();
		std::string usage = v["usage"].string_value();

		std::string tableName = FindTableName(moduleName);
		if (tableName.empty())
			continue;

		sql = std::format(R"(INSERT INTO {} (InstanceID, Command, Count) VALUES )", tableName);

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

			sql += std::format(R"(({}, '{}', {}), )", instanceID, command, count);
			values++;
			}

		sql = TrimRight(sql, ", ");
		sql.append(";");

		if (values > 0)
			{
			if (SQLITE_OK == ::sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr))
				numInserts++;
			}
		}

	if (numInserts > 0)
		::sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
	else
		::sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);

	return numInserts;
}

int InsertDatabase(sql::Connection *db, const std::string &jstr)
{
	if (!db)
		return 0;

	std::string error;
	json11::Json json = json11::Json::parse(jstr, error);
	if (!error.empty())
		std::cerr << error << "\n";

	sql::Statement *stmt = nullptr;
	sql::PreparedStatement *pstmt = nullptr;
	sql::ResultSet *res = nullptr;
	bool ok = false;

	db->setAutoCommit(false); // Starts transaction

	pstmt = db->prepareStatement("INSERT INTO InstanceTbl (DateTime, License, PCName, ProgramCode, ProgramVersion, ProgramRelease) VALUES (?, ?, ?, ?, ?, ?)");
	pstmt->setString(1, json["datetime"].string_value());
	pstmt->setString(2, json["code"].string_value());
	pstmt->setString(3, json["pcname"].string_value());
	pstmt->setString(4, json["program"].string_value());
	pstmt->setInt(5, json["version"].int_value());
	pstmt->setInt(6, json["release"].int_value());
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

	auto vec = json["analytics"].array_items();

	for (auto &v : vec)
		{
		std::string moduleName = v["module"].string_value();
		std::string usage = v["usage"].string_value();

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
