/*
	Author: Derek Ackworth
	Date: December 9th, 2019
	File: LANScanServer.cpp
	Purpose: Run commands based on the client's input
*/

#include <iostream>
#include <sstream>
#include "TCPServer.hpp"
#include "ThreadPool.hpp"
#include "LANScan.hpp"

int main(int argc, char* argv[])
{
	cout << "LANScanServer\n\n";
	TCPServer socket;

	// Bind to the socket
	if (argc == 1)
	{
		if (socket.bindSocket("127.0.0.1", 27016))
		{
			cout << "Successfully started the server on 127.0.0.1:27016.\n";
		}
		else
		{
			cout << "Error: Failed to start the server on 127.0.0.1:27016.\n";
			return EXIT_FAILURE;
		}
	}
	else if (argc == 2)
	{
		if (!socket.bindSocket(argv[1], 27016))
		{
			cout << "Successfully started the server on " << argv[1] << ":27016.\n";
		}
		else
		{
			cout << "Error: Failed to start the server on " << argv[1] << ":27016.\n";
			return EXIT_FAILURE;
		}
	}
	else
	{
		cout << "Usage: LANScanServer [ip]\n";
		return EXIT_FAILURE;
	}

	// Wait for a client to connect
	if (!socket.waitForClient())
		return EXIT_FAILURE;

	for (;;)
	{
		// Receive data from the client
		string input = socket.receiveData();

		if (!input.empty())
		{
			// Receive scan commmand
			if (input.rfind("scan", 0) == 0)
			{
				// Parse input into argVector and argCount
				vector<string> argVector;
				int argCount = 0;
				istringstream iss(input);
				string token;

				while (iss >> quoted(token, '\"', '\0'))
				{
					argVector.push_back(token);
					argCount++;
				}

				bool isVerboseMode = false;
				string entryPath;
				string expression;
				vector<string> extensions;
				bool error = false;

				// Set scan variables
				if (argCount == 3)
				{
					entryPath = argVector[1];
					expression = argVector[2];
					extensions.push_back("txt");
				}
				else if (argCount == 4)
				{
					if (argVector[1] == "-v")
					{
						isVerboseMode = true;
						entryPath = argVector[2];
						expression = argVector[3];
						extensions.push_back("txt");
					}
					else
					{
						entryPath = argVector[1];
						expression = argVector[2];
						string sExtensions = argVector[3];
						string sExtension;

						for (unsigned int i = 0; i < sExtensions.size(); i++)
						{
							if (i == 0 && sExtensions.at(i) != '.')
							{
								socket.sendData("Error: Extensions must start with a .\n");
								this_thread::sleep_for(chrono::milliseconds(500));
								socket.sendData("done");
								error = true;
								break;
							}
							else if (i == sExtensions.size() - 1 && sExtensions.at(i) == '.')
							{
								socket.sendData("Error: Extensions must not end with a .\n");
								this_thread::sleep_for(chrono::milliseconds(500));
								socket.sendData("done");
								error = true;
								break;
							}
							else if (i == sExtensions.size() - 1)
							{
								sExtension += sExtensions.at(i);
								extensions.push_back(sExtension);
							}
							else if (sExtensions.at(i) == '.')
							{
								if (i != 0)
								{
									extensions.push_back(sExtension);
								}

								sExtension = "";
							}
							else
							{
								sExtension += sExtensions.at(i);
							}
						}
					}
				}
				else if (argCount == 5)
				{
					isVerboseMode = true;
					entryPath = argVector[2];
					expression = argVector[3];
					string sExtensions = argVector[4];
					string sExtension;

					for (unsigned int i = 0; i < sExtensions.size(); i++)
					{
						if (i == 0 && sExtensions.at(i) != '.')
						{
							socket.sendData("Error: Extensions must start with a .\n");
							this_thread::sleep_for(chrono::milliseconds(500));
							socket.sendData("done");
							error = true;
							break;
						}
						else if (i == sExtensions.size() - 1 && sExtensions.at(i) == '.')
						{
							socket.sendData("Error: Extensions must not end with a .\n");
							this_thread::sleep_for(chrono::milliseconds(500));
							socket.sendData("done");
							error = true;
							break;
						}
						else if (i == sExtensions.size() - 1)
						{
							sExtension += sExtensions.at(i);
							extensions.push_back(sExtension);
						}
						else if (sExtensions.at(i) == '.')
						{
							if (i != 0)
							{
								extensions.push_back(sExtension);
							}

							sExtension = "";
						}
						else
						{
							sExtension += sExtensions.at(i);
						}
					}
				}
				else
				{
					socket.sendData("Usage: scan [-v] path expression [extension-list]*\n");
					this_thread::sleep_for(chrono::milliseconds(500));
					socket.sendData("done");
					error = true;
				}

				if (error == true)
				{
					continue;
				}

				// Check for directory found or empty
				if (!fs::exists(entryPath))
				{
					socket.sendData("Error: Folder <" + entryPath + "> doesn't exist.\n");
					this_thread::sleep_for(chrono::milliseconds(500));
					socket.sendData("done");
					error = true;
					continue;
				}
				else if (!fs::is_directory(entryPath))
				{
					socket.sendData("Error: Path <" + entryPath + "> is not a folder.\n");
					this_thread::sleep_for(chrono::milliseconds(500));
					socket.sendData("done");
					error = true;
					continue;
				}
				else if (fs::is_empty(entryPath))
				{
					socket.sendData("Error: Folder <" + entryPath + "> is empty.\n");
					this_thread::sleep_for(chrono::milliseconds(500));
					socket.sendData("done");
					error = true;
					continue;
				}

				// Sends scan and report to client
				cout << "Scan in progress...\n";
				LANScan lanScan(socket, isVerboseMode, entryPath, expression, extensions);
				lanScan.sendScan();
				lanScan.sendReport();
				this_thread::sleep_for(chrono::milliseconds(500));
				socket.sendData("done");
				cout << "Scan finished.\n";
			}
			// Receive drop commmand
			else if (input == "drop")
			{
				socket.disconnectClient();
			}
			// Receive stopserver commmand
			else if (input == "stopserver")
			{
				return EXIT_SUCCESS;
			}
		}
	}

	return EXIT_SUCCESS;
}
