// File:  sousVideConfig.h
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Configuration options (to be read from file) for sous vide service.

#ifndef SOUS_VIDE_CONFIG_H_
#define SOUS_VIDE_CONFIG_H_

// Local headers
#include "configFile.h"

struct NetworkConfiguration
{
	unsigned short port;
};

struct IOConfiguration
{
	int pumpRelayPin;
	int heaterRelayPin;

	std::string sensorID;
};

struct ControllerConfiguration
{
	// Gains
	double kp;// [%/deg F]
	double ti;// [sec]
	double kd;// [sec]
	double kf;// [%-sec/deg F]

	// Derivative time constants
	double td;// [sec]
	double tf;// [sec]

	double plateauTolerance;// [deg F]
	double pwmFrequency;// [Hz]
};

struct InterlockConfiguration
{
	double maxSaturationTime;// [sec]
	double maxTemperature;// [deg F]
	double temperatureTolerance;// [deg F]

	double minErrorTime;// [sec]
};

struct SystemConfiguration
{
	InterlockConfiguration interlock;

	double idleFrequency;// [Hz]
	double activeFrequency;// [Hz]

	double maxHeatingRate;// [deg F/sec]

	double maxAutoTuneTime;// [sec]
	double maxAutoTuneTemperatureRise;// [deg F]

	std::string temperaturePlotPath;
};

struct SousVideConfig : public ConfigFile
{
public:
	SousVideConfig(std::ostream &outStream = std::cout)
		: ConfigFile(outStream) {};
	virtual ~SousVideConfig() {};

	// Configuration options to be read from file
	NetworkConfiguration network;
	IOConfiguration io;
	ControllerConfiguration controller;
	SystemConfiguration system;

private:
	virtual void BuildConfigItems(void);
	virtual void AssignDefaults(void);

	virtual bool ConfigIsOK(void) const;
	bool NetworkConfigIsOK(void) const;
	bool IOConfigIsOK(void) const;
	bool ControllerConfigIsOK(void) const;
	bool InterlockConfigIsOK(void) const;
	bool SystemConfigIsOK(void) const;
};

#endif// SOUS_VIDE_CONFIG_H_
