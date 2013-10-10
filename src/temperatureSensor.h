// File:  temperatureSensor.h
// Date:  8/31/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Temperature sensor object.  For use with a DS18B20 "1-wire"
//        temperature sensor.  This is implemented using system calls, as the
//        1-wire interface is built into the Raspian kernel.  Note that
//        for this to work, the sensor must be connected to GPIO7.

#ifndef TEMPERATURE_SENSOR_H_
#define TEMPERATURE_SENSOR_H_

// Standard C++ headers
#include <string>
#include <ostream>
#include <iostream>

class TemperatureSensor
{
public:
	TemperatureSensor(std::string deviceID, std::ostream& outStream = std::cout,
		std::string baseDirectory = "/sys/bus/w1/devices/");

	bool GetTemperature(double &temperature) const;

private:
	static bool initialized;
	static const std::string deviceFile;

	const std::string device;
	std::ostream &outStream;
};

#endif// TEMPERATURE_SENSOR_H_
