#pragma once
#include <vector>
#include <string>
#include <format>

namespace sql
	{
	class Connection;
	}

std::string CreateInstanceTable();

std::string CreateModuleTable(const std::string &moduleName);

int InsertDatabase(sql::Connection *db, const std::string &jstr);

