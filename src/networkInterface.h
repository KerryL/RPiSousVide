// File:  networkInterface.h
// Date:  8/30/2013
// Auth:  K. Loux
// Desc:  Interface for Ethernet communication with front end.

#ifndef NETWORK_INTERFACE_H_
#define NETWORK_INTERFACE_H_

// Standard C++ headers
#include <string>

// Local headers
// TODO:  Socket class!

// Local forward declarations
struct NetworkConfiguration;
struct FrontToBackMessage;
struct BackToFrontMessage;

class NetworkInterface
{
public:
	NetworkInterface(NetworkConfiguration configuration);

	bool ReceiveData(FrontToBackMessage &message);
	bool SendData(BackToFrontMessage &message);

private:
	static const std::string loopback;
	const unsigned int listenPort;
	const unsigned int talkPort;

	// TODO:  Sockets
};

#endif// NETWORK_MESSAGE_DEFS_H_