/*
	Author: Derek Ackworth
	Date: December 9th, 2019
	File: FileMatch.hpp
	Purpose: FileMatch class declaration
*/

#pragma once

#include <filesystem>
#include <vector>
#include "LineMatch.hpp"

namespace fs = filesystem;

class FileMatch
{
public:
	FileMatch(fs::path path, vector<LineMatch> lineMatches) : path(path), lineMatches(lineMatches) {}

	bool operator < (const FileMatch& fileMatch) const { return (path < fileMatch.path); };

	fs::path path;
	vector<LineMatch> lineMatches;
};
