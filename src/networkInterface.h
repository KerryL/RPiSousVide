// File:  networkInterface.h
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Interface for Ethernet communication with front end.

#ifndef NETWORK_INTERFACE_H_
#define NETWORK_INTERFACE_H_

// Standard C++ headers
#include <string>

// cJSON forward declarations
struct cJSON;

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
	char *buffer;

	static bool DecodeMessage(const std::string &buffer,
		FrontToBackMessage &message);
	static bool EncodeMessage(const BackToFrontMessage &message,
		std::string &buffer);

	static bool ReadJSON(cJSON *parent, std::string key, double &value);
	static bool ReadJSON(cJSON *parent, std::string key, int &value);
	static bool ReadJSON(cJSON *parent, std::string key, std::string &value);
};

#endif// NETWORK_INTERFACE_H_
