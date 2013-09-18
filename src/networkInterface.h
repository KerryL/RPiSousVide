// File:  networkInterface.h
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Interface for Ethernet communication with front end.

#ifndef NETWORK_INTERFACE_H_
#define NETWORK_INTERFACE_H_

// Local forward declarations
struct NetworkConfiguration;
struct FrontToBackMessage;
struct BackToFrontMessage;
class LinuxSocket;

class NetworkInterface
{
public:
	NetworkInterface(NetworkConfiguration configuration);
	~NetworkInterface();

	bool ReceiveData(FrontToBackMessage &message);
	bool SendData(const BackToFrontMessage &message);

	bool ClientConnected(void) const;

private:
	LinuxSocket *socket;
};

#endif// NETWORK_MESSAGE_DEFS_H_
