#include "pch.h"
#include "Analytics.h"
#include <date\date.h>


static const std::string gLicenseCode = "1234567890987654321";
static const std::string gComputerName = "DEV5-PCC";
static const std::string gProgramCode = "CD";
static const int gVersion = 91;
static const int gRelease = 101;



ModuleAnalytics::ModuleAnalytics(ModuleID id)
	: m_id(id)
{}

ModuleID ModuleAnalytics::GetID() const
{
	return m_id;
}

void ModuleAnalytics::AddCommands(const std::vector<std::string> &commands)
{
	for (const auto &cmd : commands)
		m_commands.push_back(cmd);
}

void ModuleAnalytics::GatherUsage()
{
	for (const auto &command : m_commands)
		m_usage[command]++;
	m_commands.clear();
}

std::string ModuleAnalytics::ToString() const
{
	std::stringstream ss;
	for (const auto &pair : m_usage)
		{
		ss
				<< pair.first
				<< ","
				<< pair.second
				<< " ";
		}

	return ss.str();
}

json11::Json::object ModuleAnalytics::ToJson() const
{
	std::string name = moduleNames[m_id];
	std::string usage = ToString();

	return json11::Json::object
		{
			{ "module", name},
			{ "usage", usage },
		};
}






std::vector<ModuleAnalytics>::iterator Analytics::Find(ModuleID id)
{
	for (auto itr = m_data.begin(); itr != m_data.end(); ++itr)
		{
		if (itr->GetID() == id)
			return itr;
		}

	return m_data.end();
}

void Analytics::AddCommands(ModuleID moduleID, const std::vector<std::string> &commands)
{
	auto itr = Find(moduleID);
	if (itr == m_data.end())
		{
		m_data.push_back(ModuleID{ moduleID });
		itr = std::prev(m_data.end());
		}

	itr->AddCommands(commands);
}

void Analytics::GatherUsage()
{
	for (size_t i = 0; i < m_data.size(); ++i)
		{
		m_data[i].GatherUsage();
		}
}

std::string Analytics::ToString() const
{
	std::string str;

	for (size_t i = 0; i < m_data.size(); ++i)
		{
		str += m_data[i].ToString();
		}

	return str;
}

std::string FormattedCurrentTime()
{
	auto now = std::chrono::system_clock::now();

	using namespace date;
	std::stringstream ss;
	ss << now;

	return ss.str();
}

std::string Analytics::ToJson() const
{
	std::vector<json11::Json> arrModules;
	for (size_t i = 0; i < m_data.size(); ++i)
		{
		arrModules.push_back(m_data[i].ToJson());
		}

	std::string dateTime = FormattedCurrentTime();

	json11::Json::object obj =
		{
			{ "datetime", dateTime },
			{ "code", gLicenseCode },
			{ "pcname", gComputerName },
			{ "program", gProgramCode },
			{ "version", gVersion },
			{ "release", gRelease },
			{ "analytics", arrModules },
		};

	return json11::Json(obj).dump();
}
