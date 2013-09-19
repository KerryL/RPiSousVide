// File:  sousVideConfig.cpp
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Configuration options (to be read from file) for sous vide service.

// Standard C++ headers
#include <fstream>
#include <algorithm>

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
const std::string SousVideConfig::ConfigFields::CommentCharacter					= "#";

const std::string SousVideConfig::ConfigFields::NetworkPortKey						= "port";

const std::string SousVideConfig::ConfigFields::IOPumpPinKey						= "pumpPin";
const std::string SousVideConfig::ConfigFields::IOHeaterPinKey						= "heaterPin";
const std::string SousVideConfig::ConfigFields::IOSensorIDKey						= "sensorID";

const std::string SousVideConfig::ConfigFields::ControllerKpKey						= "kp";
const std::string SousVideConfig::ConfigFields::ControllerKdKey						= "kd";
const std::string SousVideConfig::ConfigFields::ControllerTiKey						= "ti";
const std::string SousVideConfig::ConfigFields::ControllerFfKey						= "ff";
const std::string SousVideConfig::ConfigFields::ControllerTdKey						= "td";
const std::string SousVideConfig::ConfigFields::ControllerTfKey						= "tf";
const std::string SousVideConfig::ConfigFields::ControllerPlateauToleranceKey		= "plateauTolerance";

const std::string SousVideConfig::ConfigFields::InterlockMaxSaturationTimeKey		= "maxSaturationTime";
const std::string SousVideConfig::ConfigFields::InterlockMaxTemperatureKey			= "maxTemperature";
const std::string SousVideConfig::ConfigFields::InterlockTemperatureToleranceKey	= "temperatureTolerance";
const std::string SousVideConfig::ConfigFields::InterlockMinErrorTimeKey			= "minErrorTime";

const std::string SousVideConfig::ConfigFields::SystemIdleFrequencyKey				= "idleFrequency";
const std::string SousVideConfig::ConfigFields::SystemActiveFrequencyKey			= "activeFrequency";
const std::string SousVideConfig::ConfigFields::SystemStatisticsTimeKey				= "statisticsTime";
const std::string SousVideConfig::ConfigFields::SystemMaxHeatingRateKey				= "maxHeatingRate";

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
	AssignDefaults();
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

		if (ConfigFields::CommentCharacter.compare(line.substr(0,1)) != 0
			&& line.find(" ") != std::string::npos)
		{
			inLineComment = line.find(ConfigFields::CommentCharacter);
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
	controller.ff = 0.0;
	controller.td = 1.0;
	controller.tf = 1.0;
	controller.plateauTolerance = 1.0;// [deg F]

	system.interlock.maxSaturationTime = 10.0;// [sec]
	system.interlock.maxTemperature = 200.0;// [deg F]
	system.interlock.temperatureTolerance = 2.0;// [deg F]
	system.interlock.minErrorTime = 5.0;// [sec]

	system.idleFrequency = 1.0;// [Hz]
	system.activeFrequency = 10.0;// [Hz]
	system.statisticsTime = 10.0;// [sec]
	system.maxHeatingRate = -1.0;// [deg F/sec] invalid -> must be specified by user
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
		outStream << "Network:  " << ConfigFields::NetworkPortKey << " must be 1024 or greater" << std::endl;
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
		outStream << "IO:  " << ConfigFields::IOPumpPinKey << " must be positive" << std::endl;
		ok = false;
	}
	else if (io.pumpRelayPin > 20)
	{
		outStream << "IO:  " << ConfigFields::IOPumpPinKey << " must be less than or equal to 20" << std::endl;
		ok = false;
	}

	if (io.heaterRelayPin < 0)
	{
		outStream << "IO:  " << ConfigFields::IOHeaterPinKey << " must be positive" << std::endl;
		ok = false;
	}
	else if (io.heaterRelayPin > 20)
	{
		outStream << "IO:  " << ConfigFields::IOHeaterPinKey << " must be less than or equal to 20" << std::endl;
		ok = false;
	}

	if (io.heaterRelayPin == io.pumpRelayPin)
	{
		outStream << "IO:  " << ConfigFields::IOPumpPinKey << " and "
			<< ConfigFields::IOHeaterPinKey << " must be unique" << std::endl;
		ok = false;
	}

	if (io.sensorID.length() != 15)
	{
		outStream << "IO:  " << ConfigFields::IOSensorIDKey << " must contain 15 characters" << std::endl;
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
		outStream << "Controller:  " << ConfigFields::ControllerKpKey << " must be positive ("
			<< ConfigFields::ControllerKpKey << " must be specified)" << std::endl;
		ok = false;
	}

	if (controller.ti < 0.0)
	{
		outStream << "Controller:  " << ConfigFields::ControllerTiKey << " must be positive" << std::endl;
		ok = false;
	}

	if (controller.kd < 0.0)
	{
		outStream << "Controller:  " << ConfigFields::ControllerKdKey << " must be positive" << std::endl;
		ok = false;
	}

	if (controller.ff < 0.0)
	{
		outStream << "Controller:  " << ConfigFields::ControllerFfKey << " must be positive" << std::endl;
		ok = false;
	}

	if (controller.td <= 0.0)
	{
		outStream << "Controller:  " << ConfigFields::ControllerTdKey << " must be strictly positive" << std::endl;
		ok = false;
	}

	if (controller.tf <= 0.0)
	{
		outStream << "Controller:  " << ConfigFields::ControllerTfKey << " must be strictly positive" << std::endl;
		ok = false;
	}

	if (controller.plateauTolerance <= 0.0)
	{
		outStream << "Controller:  " << ConfigFields::ControllerPlateauToleranceKey << " must be strictly positive" << std::endl;
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
		outStream << "Interlock:  " << ConfigFields::InterlockMaxSaturationTimeKey << " must be strictly positive" << std::endl;
		ok = false;
	}

	if (system.interlock.maxTemperature < 100.0)
	{
		outStream << "Interlock:  " << ConfigFields::InterlockMaxTemperatureKey << " must be greater than 100.0 deg F" << std::endl;
		ok = false;
	}
	else if  (system.interlock.maxTemperature > 212.0)
	{
		outStream << "Interlock:  " << ConfigFields::InterlockMaxTemperatureKey << " must be less than 212.0 deg F" << std::endl;
		ok = false;
	}

	if (system.interlock.temperatureTolerance <= 0.0)
	{
		outStream << "Interlock:  " << ConfigFields::InterlockTemperatureToleranceKey << " must be strictly positive" << std::endl;
		ok = false;
	}
	else if (system.interlock.temperatureTolerance > 30.0)
	{
		outStream << "Interlock:  " << ConfigFields::InterlockTemperatureToleranceKey << " must be less than 30 deg F" << std::endl;
		ok = false;
	}

	if (system.interlock.minErrorTime < 0.0)
	{
		outStream << "Interlock:  " << ConfigFields::InterlockMinErrorTimeKey << " must be positive" << std::endl;
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
		outStream << "System:  " << ConfigFields::SystemIdleFrequencyKey << " must be strictly positive" << std::endl;
		ok = false;
	}

	if (system.activeFrequency <= 0.0)
	{
		outStream << "System:  " << ConfigFields::SystemActiveFrequencyKey << " must be strictly positive" << std::endl;
		ok = false;
	}

	if (system.statisticsTime < 0.0)
	{
		outStream << "System:  " << ConfigFields::SystemStatisticsTimeKey << " must be strictly positive" << std::endl;
		ok = false;
	}

	if (system.maxHeatingRate <= 0.0)
	{
		outStream << "System:  " << ConfigFields::SystemMaxHeatingRateKey << " must be strictly positive ("
			<< ConfigFields::SystemMaxHeatingRateKey << " must be specified)" << std::endl;
		ok = false;
	}

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
	if (field.compare(ConfigFields::NetworkPortKey) == 0)
		network.port = (unsigned short)atoi(data.c_str());
	else if (field.compare(ConfigFields::IOPumpPinKey) == 0)
		io.pumpRelayPin = atoi(data.c_str());
	else if (field.compare(ConfigFields::IOHeaterPinKey) == 0)
		io.heaterRelayPin = atoi(data.c_str());
	else if (field.compare(ConfigFields::IOSensorIDKey) == 0)
		io.sensorID = data;
	else if (field.compare(ConfigFields::ControllerKpKey) == 0)
		controller.kp = atof(data.c_str());
	else if (field.compare(ConfigFields::ControllerTiKey) == 0)
		controller.ti = atof(data.c_str());
	else if (field.compare(ConfigFields::ControllerKdKey) == 0)
		controller.kd = atof(data.c_str());
	else if (field.compare(ConfigFields::ControllerFfKey) == 0)
		controller.ff = atof(data.c_str());
	else if (field.compare(ConfigFields::ControllerTdKey) == 0)
		controller.td = atof(data.c_str());
	else if (field.compare(ConfigFields::ControllerTfKey) == 0)
		controller.tf = atof(data.c_str());
	else if (field.compare(ConfigFields::ControllerPlateauToleranceKey) == 0)
		controller.plateauTolerance = atof(data.c_str());
	else if (field.compare(ConfigFields::InterlockMaxSaturationTimeKey) == 0)
		system.interlock.maxSaturationTime = atof(data.c_str());
	else if (field.compare(ConfigFields::InterlockMaxTemperatureKey) == 0)
		system.interlock.maxTemperature = atof(data.c_str());
	else if (field.compare(ConfigFields::InterlockTemperatureToleranceKey) == 0)
		system.interlock.temperatureTolerance = atof(data.c_str());
	else if (field.compare(ConfigFields::InterlockMinErrorTimeKey) == 0)
		system.interlock.minErrorTime = atof(data.c_str());
	else if (field.compare(ConfigFields::SystemIdleFrequencyKey) == 0)
		system.idleFrequency = atof(data.c_str());
	else if (field.compare(ConfigFields::SystemActiveFrequencyKey) == 0)
		system.activeFrequency = atof(data.c_str());
	else if (field.compare(ConfigFields::SystemStatisticsTimeKey) == 0)
		system.statisticsTime = atof(data.c_str());
	else if (field.compare(ConfigFields::SystemMaxHeatingRateKey) == 0)
		system.maxHeatingRate = atof(data.c_str());
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
