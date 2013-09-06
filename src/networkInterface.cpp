// File:  networkInterface.cpp
// Date:  9/3/2013
// Auth:  K. Loux
// Desc:  Interface for Ethernet communication with front end.

// Local headers
#include "networkInterface.h"
#include "networkMessageDefs.h"
#include "linuxSocket.h"

//==========================================================================
// Class:			NetworkInterface
// Function:		Constant definitions
//
// Description:		Constant definitions for the NetworkInterface class.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================


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
	socket = new LinuxSocket(LinuxSocket::SocketTCPServer, CombinedLogger::GetLogger());
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

bool NetworkInterface::ReceiveData(FrontToBackMessage &message)
{
	// TODO:  It would be nice to have some type of mutex locker object so we don't need to explicitly release on all control paths
	socket->GetLock();
	int bytesReceived = socket->Receive();
	if (bytesReceived < sizeof(message))
	{
		socket->ReleaseLock();
		return false;
	}
	else if (bytesReceived == sizeof(message))
	{
		memcpy(&message, socket->GetLastMessage(), sizeof(message));
		socket->ReleaseLock();
		return true;
	}
	else
	{
		// TODO:  Do some extra work in case multiple messages arrive at the same time?
	}

	socket->ReleaseLock();
	return false;
}

bool NetworkInterface::SendData(BackToFrontMessage &message)
{
	return socket->TCPSend(&message, sizeof(message));
}