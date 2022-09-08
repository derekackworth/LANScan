/*
	Author: Derek Ackworth
	Date: December 9th, 2019
	File: LineMatch.hpp
	Purpose: LineMatch class declaration
*/

#pragma once

#include <string>

using namespace std;

class LineMatch
{
public:
	LineMatch(unsigned int lineNumber, unsigned int matches, string content) : lineNumber(lineNumber), matches(matches), content(content) {}

	bool operator < (const LineMatch& lineMatch) const { return (lineNumber < lineMatch.lineNumber); }

	unsigned int lineNumber;
	unsigned int matches;
	string content;
};
