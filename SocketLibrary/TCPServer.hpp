/*
	Author: Derek Ackworth
	Date: December 9th, 2019
	File: TCPServer.hpp
	Purpose: TCPServer class declaration
*/

#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <string>
#include <WinSock2.h>

using namespace std;

class TCPServer
{
public:
	TCPServer() : wsaData(), hSocket(), hAccepted(), serverAddress(), isConnectedToClient(false) {}
	~TCPServer();

	bool bindSocket(string ip, int port);
	bool disconnectSocket();
	bool disconnectClient();
	bool waitForClient();
	bool sendData(string data);
	string receiveData();

	WSAData wsaData;
	SOCKET hSocket;
	SOCKET hAccepted;
	sockaddr_in serverAddress;
	bool isConnectedToClient;
};
