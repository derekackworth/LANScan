/*
	Author: Derek Ackworth
	Date: December 9th, 2019
	File: TCPClient.cpp
	Purpose: TCPClient class implementation
*/

#include <iostream>
#include <WS2tcpip.h>
#include "TCPClient.hpp"

#pragma comment (lib, "ws2_32.lib")

// Destructor
TCPClient::~TCPClient()
{
	if (isConnectedToServer)
	{
		sendData("drop");
	}
	
	disconnectSocket();
}

/*
	Connect the socket
	Input: Ip address and port number
	Output: Whether it was successful
*/
bool TCPClient::connectSocket(string ip, int port)
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

	// Connect the socket
	if (connect(hSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		disconnectSocket();
		return false;
	}

	isConnectedToServer = true;
	return true;
}

/*
	Close the socket
	Output: Whether it was successful
*/
bool TCPClient::disconnectSocket()
{
	int closeCode = closesocket(hSocket);
	WSACleanup();

	if (closeCode == 0)
	{
		isConnectedToServer = false;
		return true;
	}
	else
	{
		return false;
	}
}

/*
	Send data to the socket
	Input: Data being sent
	Output: Whether it was successful
*/
bool TCPClient::sendData(string data)
{
	if (isConnectedToServer)
	{
		if (send(hSocket, data.c_str(), (int)strlen(data.c_str()), 0) < 0)
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
string TCPClient::receiveData()
{
	if (isConnectedToServer)
	{
		char data[2048] = "";
		recv(hSocket, data, 2048, 0);
		return data;
	}
	else
	{
		return "";
	}
}
