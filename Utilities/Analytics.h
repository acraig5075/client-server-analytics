#pragma once

#include "ModuleID.h"
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <sstream>
#include <json11.hpp>


std::string FormattedCurrentTime();

static std::unordered_map<ModuleID, std::string> moduleNames =
{
	{ SURVEY_ID,  "Survey" },
	{ TERRAIN_ID, "Terrain" },
	{ ROAD_ID,    "Road" },
	{ SEWER_ID,   "Sewer" },
	{ STORM_ID,   "Storm" },
	{ WATER_ID,   "Water" },
	{ SIGNAGE_ID, "Signage" },
};

class ModuleAnalytics
{
public:
	ModuleAnalytics(ModuleID id);
	ModuleID GetID() const;

	void AddCommands(const std::vector<std::string> &commands);
	void GatherUsage();

	std::string ToString() const;
	json11::Json::object ToJson() const;

private:
	ModuleID m_id = ModuleID::MAX;
	std::vector<std::string> m_commands;
	std::unordered_map<std::string, unsigned int> m_usage;
};


class Analytics
{
public:
	void AddCommands(ModuleID moduleID, const std::vector<std::string> &commands);
	void GatherUsage();

	std::string ToString() const;
	std::string ToJson() const;

private:
	std::vector<ModuleAnalytics>::iterator Find(ModuleID id);

private:
	std::vector<ModuleAnalytics> m_data;
};


/*
  {
	"code": "1234567890987654321",
	"pcname": "DEV5-PCC",
	"datetime": "16:36:32",
	"analytics": [
		{
			"module": "survey",
			"usage": "EDITFIELDDATA,2 SURVEYIMPORTYXZ,4 SURVEYIMPORTMULTIYXZ,7 SURVEYIMPORTSERIAL,10"
		},
		{
			"module": "terrain",
			"usage": "SELECTDTMPOINTS,12 SELECTDTMPOINTSCLEAR,12"
		},
	]
  }

	DECLARE lastInstanceID INT DEFAULT 0;
	START TRANSACTION;
	INSERT INTO "InstanceTbl" ('DateTime', 'License', 'PCName');
	SET lastInstanceID = LAST_INSERT_ID();
	IF lastInsertID > 0 THEN
		INSERT INTO "SurveyTbl" ('InstanceID', 'Command', 'Count') VALUES (lastInstanceID, 'EDITFIELDDATA', 2), (lastInstanceID, 'SURVEYIMPORTYXZ', 4), (lastInstanceID, 'SURVEYIMPORTMULTIYXZ', 4), (lastInstanceID, 'SURVEYIMPORTSERIAL', 4);
		INSERT INTO "TerrainTbl" ('InstanceID', 'Command', 'Count') VALUES (lastInstanceID, 'SELECTDTMPOINTS', 12), (lastInstanceID, 'SELECTDTMPOINTSCLEAR', 4);
		COMMIT;
	END IF;

*/