// File:  networkInterface.cpp
// Date:  9/3/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Interface for Ethernet communication with front end.

// Standard C++ headers
#include <string.h>

// cJSON headers
#include "cJSON.h"

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
	socket = new LinuxSocket(LinuxSocket::SocketTCPServer, CombinedLogger::GetLogger());
	socket->Create(configuration.port);
	socket->SetBlocking(false);

	buffer = new char[LinuxSocket::maxMessageSize];
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
	delete [] buffer;
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
		memcpy(buffer, socket->GetLastMessage(), LinuxSocket::maxMessageSize);
		std::string stringBuffer(buffer);
		if (!DecodeMessage(stringBuffer, message))
			return false;

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
	std::string stringBuffer;
	if (!EncodeMessage(message, stringBuffer))
		return false;
	return socket->TCPSend(stringBuffer.c_str(), stringBuffer.length());
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

//==========================================================================
// Class:			NetworkInterface
// Function:		DecodeMessage
//
// Description:		Decodes the JSON-encoded message.
//
// Input Arguments:
//		buffer	= const std::string&
//
// Output Arguments:
//		message	= FrontToBackMessage&
//
// Return Value:
//		bool, true if decode is successful, false otherwise
//
//==========================================================================
bool NetworkInterface::DecodeMessage(const std::string &buffer,
	FrontToBackMessage &message)
{
	cJSON *root = cJSON_Parse(buffer.c_str());
	if (!root)
		return false;

	int command;
	if (!ReadJSON(root, JSONKeys::CommandKey, command))
		return false;
	message.command = (SousVide::Command)command;

	if (!ReadJSON(root, JSONKeys::PlateauTemperatureKey, message.plateauTemperature))
		return false;

	if (!ReadJSON(root, JSONKeys::SoakTimeKey, message.soakTime))
		return false;

	cJSON_Delete(root);

	return true;
}

//==========================================================================
// Class:			NetworkInterface
// Function:		EncodeMessage
//
// Description:		Encodes the message into the JSON format expected by
//					the clients.
//
// Input Arguments:
//		message	= const BackToFrontMessage&
//
// Output Arguments:
//		buffer	= std::string&
//
// Return Value:
//		bool, true if encode is successful, false otherwise
//
//==========================================================================
bool NetworkInterface::EncodeMessage(const BackToFrontMessage &message,
	std::string &buffer)
{
	cJSON *root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, JSONKeys::StateKey.c_str(), (int)message.state);// TODO:  Use the string describing the state instead?
	cJSON_AddNumberToObject(root, JSONKeys::CommandedTemperatureKey.c_str(), message.commandedTemperature);
	cJSON_AddNumberToObject(root, JSONKeys::ActualTemperatureKey.c_str(), message.actualTemperature);

	buffer.assign(cJSON_Print(root));
	cJSON_Delete(root);

	return true;
}

//==========================================================================
// Class:			NetworkInterface
// Function:		ReadJSON
//
// Description:		Reads the value associated with the specified key.
//
// Input Arguments:
//		parent	= cJSON* containing the key-value pair
//		key		= std::string to read
//
// Output Arguments:
//		value	= double&
//
// Return Value:
//		bool, true if read is successful, false otherwise
//
//==========================================================================
bool NetworkInterface::ReadJSON(cJSON *parent, std::string key,
	double &value)
{
	cJSON *item;
	item = cJSON_GetObjectItem(parent, key.c_str());
	if (!item)
		return false;

	value = item->valuedouble;

	return true;
}

//==========================================================================
// Class:			NetworkInterface
// Function:		ReadJSON
//
// Description:		Reads the value associated with the specified key.
//
// Input Arguments:
//		parent	= cJSON* containing the key-value pair
//		key		= std::string to read
//
// Output Arguments:
//		value	= int&
//
// Return Value:
//		bool, true if read is successful, false otherwise
//
//==========================================================================
bool NetworkInterface::ReadJSON(cJSON *parent, std::string key,
	int &value)
{
	cJSON *item;
	item = cJSON_GetObjectItem(parent, key.c_str());
	if (!item)
		return false;

	value = item->valueint;

	return true;
}

//==========================================================================
// Class:			NetworkInterface
// Function:		ReadJSON
//
// Description:		Reads the value associated with the specified key.
//
// Input Arguments:
//		parent	= cJSON* containing the key-value pair
//		key		= std::string to read
//
// Output Arguments:
//		value	= std::string&
//
// Return Value:
//		bool, true if read is successful, false otherwise
//
//==========================================================================
bool NetworkInterface::ReadJSON(cJSON *parent, std::string key,
	std::string &value)
{
	cJSON *item;
	item = cJSON_GetObjectItem(parent, key.c_str());
	if (!item)
		return false;

	value = item->valuestring;

	return true;
}
