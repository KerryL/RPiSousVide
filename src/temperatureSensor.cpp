// File:  temperatureSensor.cpp
// Date:  9/17/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Temperature sensor object.  For use with a DS18B20 "1-wire"
//        temperature sensor.  This is implemented using system calls, as the
//        1-wire interface is built into the Raspian kernel.

// Standard C++ headers
#include <cstdlib>
#include <fstream>

// Local headers
#include "temperatureSensor.h"

//==========================================================================
// Class:			TemperatureSensor
// Function:		None
//
// Description:		Static and constant member initialization/definitions for
//					TemperatureSensor class.
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
bool TemperatureSensor::initialized = false;
const std::string TemperatureSensor::deviceFile = "/w1_slave";

//==========================================================================
// Class:			TemperatureSensor
// Function:		TemperatureSensor
//
// Description:		Constructor for TemperatureSensor class.
//
// Input Arguments:
//		deviceID		= std::string, unique idenfier for sensor
//		outStream		= std::ostream& (optional)
//		baseDirectory	= std::string (optional, default should be fine for Raspian OS)
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
TemperatureSensor::TemperatureSensor(std::string deviceID,
	std::ostream &outStream, std::string baseDirectory)
	: device(baseDirectory + deviceID + deviceFile), outStream(outStream)
{
	if (!initialized)
	{
		system("modprobe w1-gpio");
		system("modprobe w1-therm");
		initialized = true;
	}
}

//==========================================================================
// Class:			TemperatureSensor
// Function:		GetTemperature
//
// Description:		Reads current temperature from DS18B20 sensor.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		temperature	= double& [deg F]
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool TemperatureSensor::GetTemperature(double &temperature) const
{
	std::ifstream file(deviceFile.c_str(), std::ios::in);
	if (!file.is_open() || !file.good())
	{
		outStream << "Could not open file '" << deviceFile << "' for input" << std::endl;
		return false;
	}

	std::string data;
	if (!std::getline(file, data))
	{
		outStream << "Failed to read from file '" << deviceFile << "'" << std::endl;
		return false;
	}

	// Line must contain at least "YES" at the end...
	if (data.length() < 3)
	{
		outStream << "File contents too short" << std::endl;
		return false;
	}

	if (data.substr(data.length() - 3).compare("YES") != 0)
	{
		outStream << "Temperature reading does not end in 'YES'" << std::endl;
		return false;
	}

	size_t start(data.find("t="));
	if (start == std::string::npos)
	{
		outStream << "Temperature reading does not contain 't='" << std::endl;
		return false;
	}

	temperature = atof(data.substr(start + 2).c_str()) / 1000.0 * 9.0 / 5.0 + 32.0;
	// TODO:  Allow calibration?

	return true;
}
