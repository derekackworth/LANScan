/*
	Author: Derek Ackworth
	Date: December 9th, 2019
	File: TCPServer.cpp
	Purpose: TCPServer class implementation
*/

#include <iostream>
#include <thread>
#include <WS2tcpip.h>
#include "TCPServer.hpp"

#pragma comment (lib, "ws2_32.lib")

// Destructor
TCPServer::~TCPServer()
{
	disconnectSocket();
}

/*
	Bind the socket
	Input: Ip address and port number
	Output: Whether it was successful
*/
bool TCPServer::bindSocket(string ip, int port)
{
	// Initialize WSA
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (result != 0)
	{
		return false;
	}

	// Create the TCP socket
	hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Create the server address
	serverAddress = { 0 };
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &(serverAddress.sin_addr));

	// Bind the socket
	if (bind(hSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		disconnectSocket();
		return false;
	}

	return true;
}

/*
	Close the server socket
	Output: Whether it was successful
*/
bool TCPServer::disconnectSocket()
{
	if (isConnectedToClient)
	{
		sendData("Disconnected from and stopped the server.\n");
		this_thread::sleep_for(chrono::milliseconds(500));
		sendData("done");
	}

	int closeCode = closesocket(hSocket);
	WSACleanup();

	if (closeCode == 0)
	{
		isConnectedToClient = false;
		cout << "Server disconnected.\n";
		return true;
	}
	else
	{
		cout << "Server failed to disconnect.\n";
		return false;
	}
}

/*
	Close the client socket
	Output: Whether it was successful
*/
bool TCPServer::disconnectClient()
{
	if (isConnectedToClient)
	{
		sendData("Disconnected from the server.\n");
		this_thread::sleep_for(chrono::milliseconds(500));
		sendData("done");

		int closeCode = closesocket(hAccepted);

		if (closeCode == 0)
		{
			cout << "Client disconnected.\n";
			isConnectedToClient = false;
			waitForClient();
			return true;
		}
		else
		{
			cout << "Client failed to disconnect.\n";
			return false;
		}
	}
	else
	{
		return false;
	}
}

/*
	Wait for the client to connect
	Output: Whether it was successful
*/
bool TCPServer::waitForClient()
{
	if (!isConnectedToClient)
	{
		// Listen on the socket
		if (listen(hSocket, 1) == SOCKET_ERROR)
		{
			disconnectSocket();
			return false;
		}

		cout << "Waiting for connection...\n";
		hAccepted = SOCKET_ERROR;

		// Wait for the client
		while (hAccepted == SOCKET_ERROR)
			hAccepted = accept(hSocket, NULL, NULL);

		cout << "Client connected.\n";
		this_thread::sleep_for(chrono::milliseconds(500));
		sendData("done");
		isConnectedToClient = true;
		return true;
	}

	return false;
}

/*
	Send data to the socket
	Input: Data being sent
	Output: Whether it was successful
*/
bool TCPServer::sendData(string data)
{
	if (isConnectedToClient)
	{
		if (send(hAccepted, data.c_str(), (int)strlen(data.c_str()), 0) < 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		return false;
	}
}

/*
	Receive data from the socket
	Output: Data received
*/
string TCPServer::receiveData()
{
	if (isConnectedToClient)
	{
		char data[2048] = "";
		recv(hAccepted, data, 2048, 0);
		return data;
	}
	else
	{
		return "";
	}
}
