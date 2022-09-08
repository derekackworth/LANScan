/*
	Author: Derek Ackworth
	Date: December 9th, 2019
	File: LANScan.cpp
	Purpose: LANScan class implementation
*/

#include <iostream>
#include <fstream>
#include "LANScan.hpp"
#include "ThreadPool.hpp"

/*
	Construct LANScan
	Input: If it's verbose mode, entry path, expression and extensions
*/
LANScan::LANScan(TCPServer& socket_, bool isVerboseMode_, string entryPath_, string expression_, vector<string> extensions_)
	: socket_(socket_)
{
	this->isVerboseMode_ = isVerboseMode_;
	this->entryPath_ = entryPath_;
	this->expression_.assign(expression_);
	this->extensions_ = extensions_;
	totalMatches_ = 0;
	QueryPerformanceFrequency(&frequency_);
}

// Scan for matches and if it's in verbose mode send verbose to client
void LANScan::sendScan()
{
	QueryPerformanceCounter(&scanStart_);
	ThreadPool threadpool;

	// Scans every recursive file/directory
	for (fs::directory_entry de : fs::recursive_directory_iterator(entryPath_, std::filesystem::directory_options::skip_permission_denied))
	{
		threadpool.performTask([=]
		{
			fs::path path = de.path().string();

			if (fs::is_directory(path) && isVerboseMode_)
			{
				unique_lock<mutex> lk(sendMtx_);
				socket_.sendData("Scanning: " + path.string() + "\n");
			}
			else
			{
				// Check if file extension matches an extension from scan
				for (string extension : extensions_)
				{
					if (path.string().substr(path.string().find_last_of(".") + 1) == extension)
					{
						if (isVerboseMode_)
						{
							unique_lock<mutex> lk(sendMtx_);
							socket_.sendData("Scanning: " + path.string() + "\n");
						}

						ifstream file;
						file.open(path);

						// Send error if the file can't be opened
						if (!file.is_open())
						{
							unique_lock<mutex> lk(sendMtx_);
							socket_.sendData("Error: couldn't open " + path.string() + "\n");
							continue;
						}

						vector<LineMatch> lineMatches;
						string line;
						unsigned int lineNumber = 0;

						// Add line matches
						while (getline(file, line))
						{
							lineNumber++;
							unsigned int matches = (unsigned int)distance(sregex_iterator(line.begin(), line.end(), expression_),
								sregex_iterator());

							if (matches > 0)
							{

								if (isVerboseMode_)
								{
									unique_lock<mutex> lk(sendMtx_);
									socket_.sendData("Matched " + to_string(matches) + ": " + path.string() +
										"\n[" + to_string(lineNumber) + "] " + line + "\n");
								}

								lineMatches.push_back(LineMatch(lineNumber, matches, line));
							}
						}

						// Add file matches
						if (lineMatches.size() > 0)
						{
							unique_lock<mutex> lk(fileMatchesMtx_);
							fileMatches_.push_back(FileMatch(path, lineMatches));
						}

						break;
					}
				}
			}
		});
	}
}

// Sort file matches based on path, then line matches based on line number and send report to client
void LANScan::sendReport()
{
	QueryPerformanceCounter(&scanStop_);
	double scanElapsed = (scanStop_.QuadPart - scanStart_.QuadPart) / double(frequency_.QuadPart);
	sort(fileMatches_.begin(), fileMatches_.end());
	socket_.sendData("\nScan Report:\n\n");

	// Send file match info
	for (FileMatch fileMatch : fileMatches_)
	{
		sort(fileMatch.lineMatches.begin(), fileMatch.lineMatches.end());
		socket_.sendData(fileMatch.path.string());

		for (LineMatch lineMatch : fileMatch.lineMatches)
		{
			socket_.sendData("[" + to_string(lineMatch.lineNumber) + ":" + to_string(lineMatch.matches) + "] " + lineMatch.content + "\n");
			totalMatches_ += lineMatch.matches;
		}

		socket_.sendData("\n");
	}

	// Send footer
	socket_.sendData("Files with matches: " + to_string(fileMatches_.size()) + "\n");
	socket_.sendData("Total number of matches: " + to_string(totalMatches_) + "\n");
	socket_.sendData("Scan completed in " + to_string(scanElapsed) + " seconds.\n");
}
