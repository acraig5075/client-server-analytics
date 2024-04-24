#pragma once
#include <vector>
#include <string>
#include <format>

struct sqlite3;

std::string CreateInstanceTable();

std::string CreateModuleTable(const std::string &moduleName);

int InsertDatabase(sqlite3 *db, const std::string &jstr);

