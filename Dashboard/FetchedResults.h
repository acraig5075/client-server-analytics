#pragma once

struct CommandCountPair
{
	std::string command;
	int count = 0;
};

using FetchedResults = std::vector<CommandCountPair>;