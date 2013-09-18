// File:  serverTest.cpp
// Date:  9/10/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Server socket test application.

// Comment out the next line for UDP sockets
#define TEST_TCP

// Standard C++ headers
#include <iostream>
#include <cstdlib>
#include <string>
#include <string.h>

// Local headers
#include "linuxSocket.h"

using namespace std;

void DoStuff(void);// Separate function to ensure socket destructor is called

// Application entry point
int main(int, char*[])
{
	DoStuff();
	return 0;
}

void DoStuff(void)
{
#ifdef TEST_TCP
	LinuxSocket socket(LinuxSocket::SocketTCPServer);
#else
	LinuxSocket socket(LinuxSocket::SocketUDPServer);
#endif
	if (socket.IsTCP())
		cout << "Starting server test application in TCP mode" << endl;
	else
		cout << "Starting server test application in UDP mode" << endl;

	if (!socket.Create(2770))
		exit(1);
	socket.SetBlocking(true);

	string buffer;
	while (true)
	{
		// Receive messages until two clients are connected, then send to both clients
#ifdef TCP_TEST
		do
		{
#endif
			int rcvSize;
			while (rcvSize = socket.Receive(), rcvSize == 0) {}
			cout << "Received " << rcvSize << " bytes" << endl;

			if (socket.IsTCP())
			{
				if (socket.GetLock())
					cout << "Aquired mutex lock" << endl;
				else
				{
					cout << "Failed to aquire mutex lock" << endl;
					break;
				}
			}

			const int bufSize(15);
			char cBuffer[bufSize];
			memcpy(cBuffer, socket.GetLastMessage(), bufSize);
			buffer.assign(cBuffer);
			cout << "Received message '" << buffer << "'" << endl;
		
			if (socket.IsTCP())
			{		
				if (socket.ReleaseLock())
						cout << "Released mutex lock" << endl;
				else
				{
					cout << "Failed to release mutex lock" << endl;
					break;
				}
			}
#ifdef TCP_TEST
		} while (socket.GetClientCount() < 2);
#endif

		buffer = "from server";
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
			// NOTE:  A real application might want to choose the outgoing address based on the last received message (or similar) - we currently don't support this with LinuxSocket!
			if (socket.UDPSend("127.0.0.1", 2771, buffer.c_str(), buffer.length()))
				cout << "Send succeeded" << endl;
			else
				cout << "Send failed" << endl;
		}

		// A real application might continue here...
		break;
	}
}
