// File:  networkInterface.h
// Date:  8/30/2013
// Auth:  K. Loux
// Desc:  Interface for Ethernet communication with front end.

#ifndef NETWORK_INTERFACE_H_
#define NETWORK_INTERFACE_H_

// Standard C++ headers
#include <string>

// Local forward declarations
struct NetworkConfiguration;
struct FrontToBackMessage;
struct BackToFrontMessage;
class Socket;

class NetworkInterface
{
public:
	NetworkInterface(NetworkConfiguration configuration);
	~NetworkInterface();

	bool ReceiveData(FrontToBackMessage &message);
	bool SendData(BackToFrontMessage &message);

private:
	const unsigned int port;

	/*unsigned char overflowBuffer[overflowMaxSize];
	unsigned long overflowBufferSize;*/

	Socket *socket;
};

#endif// NETWORK_MESSAGE_DEFS_H_