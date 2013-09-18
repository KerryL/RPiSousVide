// File:  configTest.cpp
// Date:  9/15/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Test application for sous vide config file class.

// Standard C++ headers
#include <cstdlib>
#include <iostream>

// Local headers
#include "sousVideConfig.h"

using namespace std;

// Application entry point
int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "Usage:  configTest <fileName>" << endl;
		return 1;
	}

	SousVideConfig config;
	if (config.ReadConfiguration(argv[1]))
		cout << "Successfully read configuration from " << argv[0] << endl;
	else
	{
		cout << "Failed to read configuration from " << argv[0] << endl;
		return 1;
	}

	// Display configuration as stored in the config object
	cout << endl;
	cout << "Network configuration" << endl;
	cout << "  Server Port = " << config.network.port << endl;

	cout << endl;
	cout << "I/O Configuration" << endl;
	cout << "  Pump Relay Pin = " << config.io.pumpRelayPin << endl;
	cout << "  Heater Relay Pin = " << config.io.heaterRelayPin << endl;
	cout << "  Sensor ID = " << config.io.sensorID << endl;

	cout << endl;
	cout << "Controller Configuration" << endl;
	cout << "  Proportional Gain = " << config.controller.kp << " %/deg F" << endl;
	cout << "  Integral Time Constant = " << config.controller.ti << " sec" << endl;
	cout << "  Plateau Tolerance = " << config.controller.plateauTolerance << " deg F" << endl;

	cout << endl;
	cout << "System Configuration" << endl;
	cout << "  Interlock Configuration" << endl;
	cout << "    Max. Saturation Rate = " << config.system.interlock.maxSaturationTime << " sec" << endl;
	cout << "    Max. Temperature = " << config.system.interlock.maxTemperature << " deg F" << endl;
	cout << "    Temperature Tolerance = " << config.system.interlock.temperatureTolerance << " deg F" << endl;
	cout << "    Min. Error Time = " << config.system.interlock.minErrorTime << " sec" << endl;
	cout << "  Idle Frequency = " << config.system.idleFrequency << " Hz" << endl;
	cout << "  Active Frequency = " << config.system.activeFrequency << " Hz" << endl;
	cout << "  Statistics Time = " << config.system.statisticsTime << " sec" << endl;
	cout << "  Max. Heating Rate = " << config.system.maxHeatingRate << " deg F/sec" << endl;

	return 0;
}
