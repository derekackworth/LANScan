/*
	Author: Derek Ackworth
	Date: December 9th, 2019
	File: LANScanClient.cpp
	Purpose: Run a scan on a remote directory and match files/lines based on the input
*/

#include <iostream>
#include <sstream>
#include <iomanip>
#include "TCPClient.hpp"
#include "ThreadPool.hpp"

mutex consoleMtx;

int main(int argc, char* argv[])
{
	cout << "LANScanClient\n\n";
	TCPClient socket;

	// Connect to the socket
	if (argc == 1)
	{
		if (socket.connectSocket("127.0.0.1", 27016))
		{
			cout << "Successfully connected to 127.0.0.1:27016.\n";
		}
		else
		{
			cout << "Error: Failed to connect to 127.0.0.1:27016.\n";
			return EXIT_FAILURE;
		}
	}
	else if (argc == 2)
	{
		if (socket.connectSocket(argv[1], 27016))
		{
			cout << "Successfully connected to " << argv[1] << ":27016.\n";
		}
		else
		{
			cout << "Error: Failed to connect to " << argv[1] << ":27016.\n";
			return EXIT_FAILURE;
		}
	}
	else
	{
		cout << "Usage: LANScanClient [ip]\n";
		return EXIT_FAILURE;
	}

	ThreadPool threadPool;
	bool isRunning = true;
	bool hasTask = false;

	while (isRunning)
	{
		if (!hasTask)
		{
			// Receive input from the user
			cout << "\nInput: ";
			string input;
			getline(cin, input);
			cout << "\n";

			// Send scan, drop or stopserver command to the server if it's connected
			if (input.rfind("scan", 0) == 0 || input == "drop" || input == "stopserver")
			{
				if (socket.isConnectedToServer)
				{
					socket.sendData(input);
					hasTask = true;
				}
				else
				{
					cout << "Error: You can't " << input.substr(0, input.find(" ")) << ", you aren't connected to a server.\n";
				}
			}
			// Connect to the server if it isn't connected
			else if (input.rfind("connect", 0) == 0)
			{
				if (!socket.isConnectedToServer)
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

					if (argCount == 1)
					{
						if (socket.connectSocket("127.0.0.1", 27016))
						{
							cout << "Successfully connected to 127.0.0.1:27016.\n";
						}
						else
						{
							cout << "Error: Failed to connect to 127.0.0.1:27016.\n";
						}
					}
					else if (argCount == 2)
					{
						if (socket.connectSocket(argVector[1], 27016))
						{
							cout << "Successfully connected to " << argVector[1] << ":27016.\n";
						}
						else
						{
							cout << "Error: Failed to connect to " << argVector[1] << ":27016.\n";
						}
					}
					else
					{
						cout << "Usage: connect [ip]\n";
					}
				}
				else
				{
					cout << "Error: You can't connect, you are already connected to a server.\n";
				}
			}
			// Print help
			else
			{
				cout << "Usage:\tConnect to the server:\t\t\tconnect [ip]\n";
				cout << "\tDisconnect from the server:\t\tdrop\n";
				cout << "\tDisconnect from and stop the server:\tstopserver\n";
				cout << "\tScan the server's files for expression:\tLANScanClient [-v] path expression [extension-list]*\n";
			}
		}
		else
		{
			threadPool.performTask([&socket, &hasTask]
			{
				// Receive data from the server
				string output = socket.receiveData();

				if (!output.empty())
				{
					// Print output if it isn't done
					if (output != "done")
					{
						{
							unique_lock<mutex> lk(consoleMtx);
							cout << output;
						}

						if (output == "Disconnected from the server.\n" || output == "Disconnected from and stopped the server.\n")
						{
							socket.isConnectedToServer = false;
						}
					}
					else
					{
						hasTask = false;
					}
				}
			});
		}
	}

	return EXIT_SUCCESS;
}
