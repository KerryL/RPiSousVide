// File:  tempSensorTest.cpp
// Date:  9/24/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Application for testing TemperatureSensor object.

// Standard C++ headers
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <cassert>

// *nix standard headers
#include <unistd.h>

// Local headers
#include "ds18b20UART.h"

using namespace std;

string GetReadingString(DS18B20UART *ts);
string MakeColumn(string c, unsigned int width);

// Structure for storing information about connected sensors
struct eeParams
{
	double alarmTemp;
	DS18B20UART::TemperatureResolution resolution;
};

// Application entry point
int main(int, char*[])
{
	vector<string> roms;
	if (!DS18B20UART::SearchROMs(roms))
	{
		cout << "Failed to detect devices" << endl;
		return 1;
	}

	if (roms.size() == 0)
	{
		cout << "No DS18B20 devices detected" << endl;
		return 1;
	}

	cout << "Found " << roms.size() << " connected DS18B20 devices:" << endl;

	unsigned int i;
	for (i = 0; i < roms.size(); i++)
	{
		cout << roms[i] << endl;
	}

	DS18B20UART **tsArray = new DS18B20UART*[roms.size()];

	vector<eeParams> originalParameters;

	for (i = 0; i < roms.size(); i++)
	{
		tsArray[i] = new DS18B20UART(roms[i]);

		cout << endl << endl;
		cout << "Sensor " << i << " --> " << roms[i] << ":" << endl;
		cout << "  reset temperature:  " << tsArray[i]->GetTemperature() << " deg C";
		cout << "  alarm temperature:  " << tsArray[i]->GetAlarmTemperature() << " deg C";
		cout << "  resolution:  " << 9 + (int)tsArray[i]->GetResolution() << "-bit";

		eeParams ee;
		ee.alarmTemp = tsArray[i]->GetAlarmTemperature();
		ee.resolution = tsArray[i]->GetResolution();

		originalParameters.push_back(ee);

		if (ee.resolution != DS18B20UART::Resolution12Bit)
		{
			if (!tsArray[i]->WriteScratchPad(DS18B20UART::Resolution12Bit))
				cout << "Failed to set resolution to 12-bit" << endl;
		}
	}

	double timeStep(1.0);// [sec]
	cout << endl << endl;
	cout << "Reading from sensors every " << timeStep << " seconds, with resolution = 12-bit" << endl << endl;
	
	string heading1, heading2, heading3;
	stringstream s;
	const unsigned int columnWidth(12);
	for (i = 1; i < roms.size(); i++)
	{
		tsArray[i - 1] = new DS18B20UART(roms[i]);

		s.str("");
		s << "Sensor " << i;
		heading1.append(MakeColumn(s.str(), columnWidth));
		heading2.append(MakeColumn("[deg C]", columnWidth));
		heading3.append(string(columnWidth,'-'));
	}

	cout << heading1 << endl;
	cout << heading2 << endl;
	cout << heading3 << endl;

	clock_t start, stop;
	double elapsed;
	
	int reading;
	for (reading = 0; reading < 10; reading++)
	{
		start = clock();

		for (i = 1; i < roms.size(); i++)
			cout << MakeColumn(GetReadingString(tsArray[i - 1]), columnWidth);
		cout << endl;

		stop = clock();

		// Handle overflows
		elapsed = double(stop - start) / (double)(CLOCKS_PER_SEC);
		if (stop < start || elapsed > timeStep)
			continue;

		usleep(1000000 * (timeStep - elapsed));
	}

	for (i = 0; i < roms.size(); i++)
	{
		if (!tsArray[i]->WriteScratchPad(DS18B20UART::Resolution9Bit))
			cout << "Failed to set resolution to 9-bit" << endl;
	}

	timeStep = 0.1;// [sec]
	cout << endl << endl;
	cout << "Reading from sensors every " << timeStep << " seconds, with resolution = 9-bit" << endl << endl;

	cout << heading1 << endl;
	cout << heading2 << endl;
	cout << heading3 << endl;
	
	for (reading = 0; reading < 10; reading++)
	{
		start = clock();

		for (i = 1; i < roms.size(); i++)
			cout << MakeColumn(GetReadingString(tsArray[i - 1]), columnWidth);
		cout << endl;

		stop = clock();

		// Handle overflows
		elapsed = double(stop - start) / (double)(CLOCKS_PER_SEC);
		if (stop < start || elapsed > timeStep)
			continue;

		usleep(1000000 * (timeStep - elapsed));
	}

	// Additional Things to check:
	// Read temperature (for 10 seconds? 10 readings?)
	// Change resolution and alarm
	// Read scratch pad to ensure settings took effect
	// Restore from EEPROM
	// Display new scratch pad
	// Write resolution and alarm temp again
	// Save to EEPROM
	// Load from EEPROM
	// Display results
	// Restore original settings
	// Save to EEPROM
	// Read power supply mode
	// Check that alrms work?  Set temperature artificially low, then check alarmed devices?

	// TODO:  Restore original settings (after we start changing/saving EEPROM)

	for (i = 1; i < roms.size(); i++)
		delete tsArray[i - 1];
	delete [] tsArray;

	return 0;
}

string GetReadingString(DS18B20UART *ts)
{
	assert(ts);

	stringstream s;
	if (ts->ConvertTemperature() && ts->ReadScratchPad())
	{
		s << setprecision(4) << ts->GetTemperature();
	}
	else
		s << "Error";

	return s.str();
}

string MakeColumn(string c, unsigned int width)
{
	assert(c.length() <= width);

	if (c.length() == width)
		return c;

	c.append(string(width - c.length(), ' '));

	return c;
}
