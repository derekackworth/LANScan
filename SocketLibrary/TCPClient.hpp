/*
	Author: Derek Ackworth
	Date: December 9th, 2019
	File: TCPClient.hpp
	Purpose: TCPClient class declaration
*/

#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <string>
#include <WinSock2.h>

using namespace std;

class TCPClient
{
public:
	TCPClient() : wsaData(), hSocket(), serverAddress(), isConnectedToServer(false) {}
	~TCPClient();

	bool connectSocket(string ip, int port);
	bool disconnectSocket();
	bool sendData(string data);
	string receiveData();

	WSAData wsaData;
	SOCKET hSocket;
	sockaddr_in serverAddress;
	bool isConnectedToServer;
};
