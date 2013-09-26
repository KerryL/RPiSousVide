// File:  sousVideConfig.cpp
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Configuration options (to be read from file) for sous vide service.

// Standard C++ headers
#include <fstream>
#include <algorithm>
#include <sstream>

// *nix headers
#include <sys/stat.h>

// Local headers
#include "sousVideConfig.h"

//==========================================================================
// Class:			SousVideConfig
// Function:		Constant definitions
//
// Description:		Constant definitions for SousVideConfig class.
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
const std::string SousVideConfig::commentCharacter	= "#";

//==========================================================================
// Class:			SousVideConfig
// Function:		SousVideConfig
//
// Description:		Constructor for SousVideConfig class.
//
// Input Arguments:
//		outStream	= std::ostream&, to which messages will be streamed
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
SousVideConfig::SousVideConfig(std::ostream &outStream) : outStream(outStream)
{
	BuildConfigItems();
	AssignDefaults();
}

//==========================================================================
// Class:			SousVideConfig
// Function:		BuildConfigItems
//
// Description:		Builds the map of key-data pairs that relate string identifiers
//					with each config data field.
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
void SousVideConfig::BuildConfigItems(void)
{
	AddConfigItem("port", network.port);

	AddConfigItem("pumpPin", io.pumpRelayPin);
	AddConfigItem("heaterPin", io.heaterRelayPin);
	AddConfigItem("sensorID", io.sensorID);

	AddConfigItem("kp", controller.kp);
	AddConfigItem("ti", controller.ti);
	AddConfigItem("kd", controller.kd);
	AddConfigItem("kf", controller.kf);
	AddConfigItem("td", controller.td);
	AddConfigItem("tf", controller.tf);
	AddConfigItem("plateauTolerance", controller.plateauTolerance);
	AddConfigItem("pwmFrequency", controller.pwmFrequency);

	AddConfigItem("maxSaturationTime", system.interlock.maxSaturationTime);
	AddConfigItem("maxTemperature", system.interlock.maxTemperature);
	AddConfigItem("temperatureTolerance", system.interlock.temperatureTolerance);
	AddConfigItem("minErrorTime", system.interlock.minErrorTime);

	AddConfigItem("idleFrequency", system.idleFrequency);
	AddConfigItem("activeFrequency", system.activeFrequency);
	AddConfigItem("statisticsTime", system.statisticsTime);
	AddConfigItem("maxHeatingRate", system.maxHeatingRate);
	AddConfigItem("maxAutoTuneTime", system.maxAutoTuneTime);
	AddConfigItem("maxAutoTuneTemperatureRise", system.maxAutoTuneTemperatureRise);
	AddConfigItem("temperaturePlotPath", system.temperaturePlotPath);
}

//==========================================================================
// Class:			SousVideConfig
// Function:		ReadConfiguration
//
// Description:		Reads the configuration from file.
//
// Input Arguments:
//		fileName	= std::string
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool SousVideConfig::ReadConfiguration(std::string fileName)
{
	outStream << "Reading configuration from '" << fileName << "'" << std::endl;

	std::ifstream file(fileName.c_str(), std::ios::in);
	if (!file.is_open() || !file.good())
	{
		outStream << "Unable to open file '" << fileName << "' for input" << std::endl;
		return false;
	}

	std::string line;
	std::string field;
	std::string data;
	size_t inLineComment;

	while (std::getline(file, line))
	{
		if (line.empty())
			continue;

		if (commentCharacter.compare(line.substr(0,1)) != 0
			&& line.find(" ") != std::string::npos)
		{
			inLineComment = line.find(commentCharacter);
			if (inLineComment != std::string::npos)
				line = line.substr(0, inLineComment);

			SplitFieldFromData(line, field, data);
			ProcessConfigItem(field, data);
		}
	}

	file.close();

	return ConfigIsOK();
}

//==========================================================================
// Class:			SousVideConfig
// Function:		AssignDefaults
//
// Description:		Assigns default values to configuration.  Values are set
//					for ALL memebers, but items that are required to be set
//					by the user are intentionally set to invalid values.
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
void SousVideConfig::AssignDefaults(void)
{
	network.port = 2770;

	io.pumpRelayPin = 0;
	io.heaterRelayPin = 1;
	io.sensorID = "";// invalid -> must be specified by user

	controller.kp = -1.0;// invalid -> must be specified by user
	controller.ti = 0.0;
	controller.kd = 0.0;
	controller.kf = 0.0;
	controller.td = 1.0;
	controller.tf = 1.0;
	controller.plateauTolerance = 1.0;// [deg F]
	controller.pwmFrequency = 2.0;// [Hz]

	system.interlock.maxSaturationTime = 10.0;// [sec]
	system.interlock.maxTemperature = 200.0;// [deg F]
	system.interlock.temperatureTolerance = 2.0;// [deg F]
	system.interlock.minErrorTime = 5.0;// [sec]

	system.idleFrequency = 0.2;// [Hz]
	system.activeFrequency = 1.0;// [Hz]
	system.statisticsTime = 10.0;// [sec]
	system.maxHeatingRate = -1.0;// [deg F/sec] invalid -> must be specified by user
	system.maxAutoTuneTime = 30.0 * 60.0;// [sec]
	system.maxAutoTuneTemperatureRise = 15.0;// [deg F]
	system.temperaturePlotPath = ".";
}

//==========================================================================
// Class:			SousVideConfig
// Function:		ConfigIsOK
//
// Description:		Validates configuration.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for OK, false otherwise
//
//==========================================================================
bool SousVideConfig::ConfigIsOK(void) const
{
	bool configOK = NetworkConfigIsOK();
	configOK = IOConfigIsOK() && configOK;
	configOK = ControllerConfigIsOK() && configOK;
	configOK = InterlockConfigIsOK() && configOK;
	configOK = SystemConfigIsOK() && configOK;

	return configOK;
}

//==========================================================================
// Class:			SousVideConfig
// Function:		NetworkConfigIsOK
//
// Description:		Validates network configuration.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for OK, false otherwise
//
//==========================================================================
bool SousVideConfig::NetworkConfigIsOK(void) const
{
	bool ok(true);

	// Don't allow ports that will require root access
	if (network.port < 1024)
	{
		outStream << "Network:  " << GetKey(network.port) << " must be 1024 or greater" << std::endl;
		ok = false;
	}

	return ok;
}

//==========================================================================
// Class:			SousVideConfig
// Function:		IOConfigIsOK
//
// Description:		Validates I/O configuration.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for OK, false otherwise
//
//==========================================================================
bool SousVideConfig::IOConfigIsOK(void) const
{
	bool ok(true);

	if (io.pumpRelayPin < 0)
	{
		outStream << "IO:  " << GetKey(io.pumpRelayPin) << " must be positive" << std::endl;
		ok = false;
	}
	else if (io.pumpRelayPin > 20)
	{
		outStream << "IO:  " << GetKey(io.pumpRelayPin) << " must be less than or equal to 20" << std::endl;
		ok = false;
	}

	if (io.heaterRelayPin < 0)
	{
		outStream << "IO:  " << GetKey(io.heaterRelayPin) << " must be positive" << std::endl;
		ok = false;
	}
	else if (io.heaterRelayPin > 20)
	{
		outStream << "IO:  " << GetKey(io.heaterRelayPin) << " must be less than or equal to 20" << std::endl;
		ok = false;
	}

	if (io.heaterRelayPin == io.pumpRelayPin)
	{
		outStream << "IO:  " << GetKey(io.heaterRelayPin) << " and "
			<< GetKey(io.pumpRelayPin) << " must be unique" << std::endl;
		ok = false;
	}

	if (io.sensorID.length() != 15)
	{
		outStream << "IO:  " <<GetKey(io.sensorID) << " must contain 15 characters" << std::endl;
		ok = false;
	}

	return ok;
}

//==========================================================================
// Class:			SousVideConfig
// Function:		ControllerConfigIsOK
//
// Description:		Validates controller configuration.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for OK, false otherwise
//
//==========================================================================
bool SousVideConfig::ControllerConfigIsOK(void) const
{
	bool ok(true);

	if (controller.kp < 0.0)
	{
		outStream << "Controller:  " << GetKey(controller.kp) << " must be positive ("
			<< GetKey(controller.kp) << " must be specified)" << std::endl;
		ok = false;
	}

	if (controller.ti < 0.0)
	{
		outStream << "Controller:  " << GetKey(controller.ti) << " must be positive" << std::endl;
		ok = false;
	}

	if (controller.kd < 0.0)
	{
		outStream << "Controller:  " << GetKey(controller.kd) << " must be positive" << std::endl;
		ok = false;
	}

	if (controller.kf < 0.0)
	{
		outStream << "Controller:  " << GetKey(controller.kf) << " must be positive" << std::endl;
		ok = false;
	}

	if (controller.td <= 0.0)
	{
		outStream << "Controller:  " << GetKey(controller.td) << " must be strictly positive" << std::endl;
		ok = false;
	}

	if (controller.tf <= 0.0)
	{
		outStream << "Controller:  " << GetKey(controller.tf) << " must be strictly positive" << std::endl;
		ok = false;
	}

	if (controller.plateauTolerance <= 0.0)
	{
		outStream << "Controller:  " << GetKey(controller.plateauTolerance) << " must be strictly positive" << std::endl;
		ok = false;
	}

	// The hard-coded limits here can be explained by looking at the PWMOutput class.
	// The limitations are inherent to Raspberry PI hardware PWM.
	const double pwmMinFrequency(1.14);// [Hz]
	const double pwmMaxFrequency(96000.0);// [Hz]
	if (controller.pwmFrequency < pwmMinFrequency)
	{
		outStream << "Controller:  " << GetKey(controller.pwmFrequency) << " must be greater than " << pwmMinFrequency << " Hz" << std::endl;
		ok = false;
	}
	else if (controller.pwmFrequency > pwmMaxFrequency)
	{
		outStream << "Controller:  " << GetKey(controller.pwmFrequency) << " must be less than " << pwmMaxFrequency << " Hz" << std::endl;
		ok = false;
	}

	return ok;
}

//==========================================================================
// Class:			SousVideConfig
// Function:		InterlockConfigIsOK
//
// Description:		Validates interlock configuration.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for OK, false otherwise
//
//==========================================================================
bool SousVideConfig::InterlockConfigIsOK(void) const
{
	bool ok(true);

	if (system.interlock.maxSaturationTime <= 0.0)
	{
		outStream << "Interlock:  " << GetKey(system.interlock.maxSaturationTime) << " must be strictly positive" << std::endl;
		ok = false;
	}

	if (system.interlock.maxTemperature < 100.0)
	{
		outStream << "Interlock:  " << GetKey(system.interlock.maxTemperature) << " must be greater than 100.0 deg F" << std::endl;
		ok = false;
	}
	else if  (system.interlock.maxTemperature > 212.0)
	{
		outStream << "Interlock:  " << GetKey(system.interlock.maxTemperature) << " must be less than 212.0 deg F" << std::endl;
		ok = false;
	}

	if (system.interlock.temperatureTolerance <= 0.0)
	{
		outStream << "Interlock:  " << GetKey(system.interlock.temperatureTolerance) << " must be strictly positive" << std::endl;
		ok = false;
	}
	else if (system.interlock.temperatureTolerance > 30.0)
	{
		outStream << "Interlock:  " << GetKey(system.interlock.temperatureTolerance) << " must be less than 30 deg F" << std::endl;
		ok = false;
	}

	if (system.interlock.minErrorTime < 0.0)
	{
		outStream << "Interlock:  " << GetKey(system.interlock.minErrorTime) << " must be positive" << std::endl;
		ok = false;
	}

	return ok;
}

//==========================================================================
// Class:			SousVideConfig
// Function:		SystemConfigIsOK
//
// Description:		Validates system configuration.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for OK, false otherwise
//
//==========================================================================
bool SousVideConfig::SystemConfigIsOK(void) const
{
	bool ok(true);

	if (system.idleFrequency <= 0.0)
	{
		outStream << "System:  " << GetKey(system.idleFrequency) << " must be strictly positive" << std::endl;
		ok = false;
	}

	if (system.activeFrequency <= 0.0)
	{
		outStream << "System:  " << GetKey(system.activeFrequency) << " must be strictly positive" << std::endl;
		ok = false;
	}

	if (system.statisticsTime < 0.0)
	{
		outStream << "System:  " << GetKey(system.statisticsTime) << " must be positive" << std::endl;
		ok = false;
	}

	if (system.maxHeatingRate <= 0.0)
	{
		outStream << "System:  " << GetKey(system.maxHeatingRate) << " must be strictly positive ("
			<< GetKey(system.maxHeatingRate) << " must be specified)" << std::endl;
		ok = false;
	}

	if (system.maxAutoTuneTime <= 0.0)
	{
		outStream << "System:  " << GetKey(system.maxAutoTuneTime) << " must be strictly positive" << std::endl;
		ok = false;
	}

	if (system.maxAutoTuneTemperatureRise <= 0.0)
	{
		outStream << "System:  " << GetKey(system.maxAutoTuneTemperatureRise) << " must be strictly positive" << std::endl;
		ok = false;
	}

	struct stat info;
	if (stat(system.temperaturePlotPath.c_str(), &info) != 0)
	{
		outStream << "System:  " << "Path indicated by " << GetKey(system.temperaturePlotPath) << " does not exist" << std::endl;
		ok = false;
	}
#ifndef WIN32
	else if (!S_ISDIR(info.st_mode))
	{
		outStream << "System:  " << "Path indicated by " << GetKey(system.temperaturePlotPath) << " is not a directory" << std::endl;
		ok = false;
	}
#endif

	return ok;
}

//==========================================================================
// Class:			SousVideConfig
// Function:		SplitFieldFromData
//
// Description:		Splits the current line into a field portion and a data
//					portion.  Split occurs at first space or first equal sign
//					(whichever comes first - keys may not contain equal signs
//					or spaces).
//
// Input Arguments:
//		line	= const std::string&
//
// Output Arguments:
//		field	= std::string&
//		data	= std::string&
//
// Return Value:
//		None
//
//==========================================================================
void SousVideConfig::SplitFieldFromData(const std::string &line,
	std::string &field, std::string &data)
{
	// Break out the data into a Field and the data (data may
	// contain spaces or equal sign!)
	size_t spaceLoc = line.find_first_of(" ");
	size_t equalLoc = line.find_first_of("=");
	field = line.substr(0, std::min(spaceLoc, equalLoc));

	if (spaceLoc == std::string::npos)
		spaceLoc = equalLoc;
	else if (equalLoc == std::string::npos)
		equalLoc = spaceLoc;

	size_t startData = std::max(spaceLoc, equalLoc) + 1;
	spaceLoc = line.find_first_not_of(" ", startData);
	equalLoc = line.find_first_not_of("=", startData);

	if (spaceLoc == std::string::npos)
		spaceLoc = equalLoc;
	else if (equalLoc == std::string::npos)
		equalLoc = spaceLoc;

	startData = std::max(spaceLoc, equalLoc);
	data = line.substr(startData, line.length() - startData);
}

//==========================================================================
// Class:			SousVideConfig
// Function:		ProcessConfigItem
//
// Description:		Processes the specified configuration item.
//
// Input Arguments:
//		field	= const std::string&
//		data	= const std::string&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void SousVideConfig::ProcessConfigItem(const std::string &field, const std::string &data)
{
	if (configItems.count(field) > 0)
		(*configItems.find(field)).second.AssignValue(data);
	else
		outStream << "Unknown config field: " << field << std::endl;
}

//==========================================================================
// Class:			SousVideConfig
// Function:		ReadBooleanValue
//
// Description:		Reads the specified data into boolean form.  Interprets true
//					values when data is "1" or empty (boolean fields should be
//					named such that it is apparent that no data value will be
//					interpreted as true).
//
// Input Arguments:
//		data	= const std::string&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, value of boolean configuration item
//
//==========================================================================
bool SousVideConfig::ReadBooleanValue(const std::string &data) const
{
	return atoi(data.c_str()) == 1 || data.empty();
}

//==========================================================================
// Class:			ConfigItem
// Function:		AssignValue
//
// Description:		Assigns the value of the data string to the appropriate
//					dereferenced pointed, based on the this item's type.
//
// Input Arguments:
//		dataString	= const std::string&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void SousVideConfig::ConfigItem::AssignValue(const std::string &dataString)
{
	std::stringstream ss(dataString);
	if (type == TypeBool)
		ss >> *data.b;
	else if (type == TypeUnsignedChar)
		ss >> *data.uc;
	else if (type ==TypeChar)
		ss >> *data.c;
	else if (type ==TypeUnsignedShort)
		ss >> *data.us;
	else if (type ==TypeShort)
		ss >> *data.s;
	else if (type ==TypeUnsignedInt)
		ss >> *data.ui;
	else if (type ==TypeInt)
		ss >> *data.i;
	else if (type ==TypeUnsignedLong)
		ss >> *data.ul;
	else if (type ==TypeLong)
		ss >> *data.l;
	else if (type ==TypeFloat)
		ss >> *data.f;
	else if (type ==TypeDouble)
		ss >> *data.d;
	else if (type ==TypeString)
		ss >> *st;
	else
		assert(false);
}