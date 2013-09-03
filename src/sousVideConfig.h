// File:  sousVideConfig.h
// Date:  8/30/2013
// Auth:  K. Loux
// Desc:  Configuration options (to be read from file) for sous vide service.

#ifndef SOUS_VIDE_CONFIG_H_
#define SOUS_VIDE_CONFIG_H_

// Standard C++ headers
#include <string>

struct NetworkConfiguration
{
	unsigned int listenPort;
	unsigned int talkPort;
};

struct IOConfiguration
{
	unsigned char pumpRelayPin;
	unsigned char heaterRelayPin;

	// TODO:  Need to identify temp sensor communication (via serial number if SPI?)
};

struct ControllerConfiguration
{
	double kp;// [%/deg F]
	double ti;// [sec]
};

struct SystemConfiguration
{
	double idleFrequency;// [Hz]
	double activeFrequency;// [Hz]
	double heaterPWMFrequency;// [Hz]

	double maxHeatingRate;// [deg F/sec]
};

struct SousVideConfig
{
	bool ReadConfiguration(std::string fileName);

	// Configuration options to be read from file
	NetworkConfiguration network;
	IOConfiguration io;
	ControllerConfiguration control;
	SystemConfiguration system;
};

#endif// SOUS_VIDE_CONFIG_H_