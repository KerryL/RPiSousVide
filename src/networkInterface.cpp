// File:  networkInterface.cpp
// Date:  9/3/2013
// Auth:  K. Loux
// Desc:  Interface for Ethernet communication with front end.

// Standard C++ headers
#include <string.h>

// Local headers
#include "networkInterface.h"
#include "networkMessageDefs.h"
#include "linuxSocket.h"
#include "mutexLocker.h"
#include "combinedLogger.h"

//==========================================================================
// Class:			NetworkInterface
// Function:		NetworkInterface
//
// Description:		Constructor for NetworkInterface class.
//
// Input Arguments:
//		configuration	= NetworkConfiguration
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
NetworkInterface::NetworkInterface(NetworkConfiguration configuration)
{
	socket = new LinuxSocket(LinuxSocket::SocketTCPServer, /*CombinedLogger::GetLogger()*/std::cout);
	socket->Create(configuration.port);
	socket->SetBlocking(false);
}

//==========================================================================
// Class:			NetworkInterface
// Function:		~NetworkInterface
//
// Description:		Destructor for NetworkInterface class.
//
// Input Arguments:
//		configuration	= NetworkConfiguration
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
NetworkInterface::~NetworkInterface()
{
	delete socket;
}

//==========================================================================
// Class:			NetworkInterface
// Function:		ReceiveData
//
// Description:		Gets data from socket, if available.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		message	= FrontToBackMessage&
//
// Return Value:
//		bool, true for data available and successfully read, false otherwise
//
//==========================================================================
bool NetworkInterface::ReceiveData(FrontToBackMessage &message)
{
	MutexLocker locker(socket->GetMutex());

	int bytesReceived = socket->Receive();
	if (bytesReceived < (int)sizeof(message))
		return false;
	else if (bytesReceived == sizeof(message))
	{
		memcpy(&message, socket->GetLastMessage(), sizeof(message));
		return true;
	}
	else
	{
		// TODO:  Do some extra work in case multiple messages arrive at the same time?
		CombinedLogger::GetLogger() << "Message size mismatch in NetworkInterface::ReceiveData" << std::endl;
	}

	return false;
}

//==========================================================================
// Class:			NetworkInterface
// Function:		SendData
//
// Description:		Sends data to all connected clients.
//
// Input Arguments:
//		message	= const BackToFrontMessage&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool NetworkInterface::SendData(const BackToFrontMessage &message)
{
	return socket->TCPSend(&message, sizeof(message));
}

//==========================================================================
// Class:			NetworkInterface
// Function:		ClientConnected
//
// Description:		Checks to see if we have any active clients.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for at least one client connection, false otherwise
//
//==========================================================================
bool NetworkInterface::ClientConnected(void) const
{
	return socket->GetClientCount() > 0;
}
