// File:  clientTest.cpp
// Date:  9/10/2013
// Auth:  K. Loux
// Desc:  Client socket test application.

// Comment out the next line to test UDP sockets
#define TEST_TCP


// Standard C++ headers
#include <iostream>
#include <cstdlib>
#include <string>
#include <string.h>

// Local headers
#include "linuxSocket.h"

using namespace std;

// Application entry point
int main(int, char*[])
{
#ifdef TEST_TCP
	LinuxSocket socket(LinuxSocket::SocketTCPClient);
#else
	LinuxSocket socket(LinuxSocket::SocketUDPClient);
#endif

	unsigned short port;
	if (socket.IsTCP())
	{
		cout << "Starting client test application in TCP mode" << endl;
		port = 2770;// Server port we want to connect to
	}
	else
	{
		cout << "Starting client test application in UDP mode" << endl;
		port = 2771;// Port to bind client socket to (our port)
	}

	if (!socket.Create(port, "127.0.0.1"))
		return 1;
	socket.SetBlocking(true);
	
	string buffer("from client");

	while (true)
	{
		cout << "Sending '" << buffer << "'" << endl;
		if (socket.IsTCP())
		{
			if (socket.TCPSend(buffer.c_str(), buffer.length()))
				cout << "Send succeeded" << endl;
			else
				cout << "Send failed" << endl;
		}
		else
		{
			if (socket.UDPSend("127.0.0.1", 2770, buffer.c_str(), buffer.length()))
				cout << "Send succeeded" << endl;
			else
				cout << "Send failed" << endl;
		}

		int rcv = socket.Receive();

		cout << "Recieved " << rcv << " bytes" << endl;
		if (rcv > 0)
		{
			const int bufLen(15);
			char cBuffer[bufLen];
			memcpy(cBuffer, socket.GetLastMessage(), bufLen);
			buffer.assign(cBuffer);
			cout << "Received message '" << buffer << "'" << endl;
		}

		// A real application might continue here...
		break;
	}

	return 0;
}
