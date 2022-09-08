/*
	Author: Derek Ackworth
	Date: December 9th, 2019
	File: LANScan.hpp
	Purpose: LANScan class declaration
*/

#pragma once

#include <regex>
#include <mutex>
#include "TCPServer.hpp"
#include "FileMatch.hpp"

class LANScan
{
public:
	LANScan(TCPServer& socket_, bool isVerboseMode_, string entryPath_, string expression_, vector<string> extensions_);

	void sendScan();
	void sendReport();

private:
	bool isVerboseMode_;
	string entryPath_;
	regex expression_;
	vector<string> extensions_;
	vector<FileMatch> fileMatches_;
	unsigned int totalMatches_;
	LARGE_INTEGER frequency_;
	LARGE_INTEGER scanStart_;
	LARGE_INTEGER scanStop_;
	mutex fileMatchesMtx_;
	mutex totalMatchesMtx_;
	mutex sendMtx_;
	TCPServer& socket_;
};
