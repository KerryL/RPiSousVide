// File:  sousVideConfig.h
// Date:  8/30/2013
// Auth:  K. Loux
// Desc:  Configuration options (to be read from file) for sous vide service.

#ifndef SOUS_VIDE_CONFIG_H_
#define SOUS_VIDE_CONFIG_H_

// Standard C++ headers
#include <string>
#include <iostream>

struct NetworkConfiguration
{
	unsigned short port;
};

struct IOConfiguration
{
	int pumpRelayPin;
	int heaterRelayPin;

	// TODO:  Need to identify temp sensor communication (via serial number if SPI?)
};

struct ControllerConfiguration
{
	double kp;// [%/deg F]
	double ti;// [sec]
};

struct InterlockConfiguration
{
	double maxSaturationTime;// [sec]
	double maxTemperature;// [deg F]
	double temperatureTolerance;// [deg F]

	// TODO:  Bypasses?
};

struct SystemConfiguration
{
	InterlockConfiguration interlock;

	double idleFrequency;// [Hz]
	double activeFrequency;// [Hz]
	double statisticsTime;// [sec]

	double maxHeatingRate;// [deg F/sec]
};

struct SousVideConfig
{
public:
	SousVideConfig(std::ostream &outStream = std::cout);

	bool ReadConfiguration(std::string fileName);

	// Configuration options to be read from file
	NetworkConfiguration network;
	IOConfiguration io;
	ControllerConfiguration controller;
	SystemConfiguration system;

private:
	std::ostream& outStream;

	void AssignDefaults(void);

	bool ConfigIsOK(void) const;
	bool NetworkConfigIsOK(void) const;
	bool IOConfigIsOK(void) const;
	bool ControllerConfigIsOK(void) const;
	bool InterlockConfigIsOK(void) const;
	bool SystemConfigIsOK(void) const;

	struct ConfigFields
	{
		static const std::string CommentCharacter;

		static const std::string NetworkPortKey;

		static const std::string IOPumpPinKey;
		static const std::string IOHeaterPinKey;

		static const std::string ControllerKpKey;
		static const std::string ControllerTiKey;

		static const std::string InterlockMaxSaturationTimeKey;
		static const std::string InterlockMaxTemperatureKey;
		static const std::string InterlockTemperatureToleranceKey;

		static const std::string SystemIdleFrequencyKey;
		static const std::string SystemActiveFrequencyKey;
		static const std::string SystemStatisticsTimeKey;
		static const std::string SystemMaxHeatingRateKey;
	};

	void SplitFieldFromData(const std::string &line, std::string &field, std::string &data);
	void ProcessConfigItem(const std::string &field, const std::string &data);
	bool ReadBooleanValue(const std::string &data) const;
};

#endif// SOUS_VIDE_CONFIG_H_
