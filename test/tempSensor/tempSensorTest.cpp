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
#include <cassert>

// *nix standard headers
#include <unistd.h>

// Local headers
#include "temperatureSensor.h"
#include "timingUtility.h"

using namespace std;

string GetReadingString(TemperatureSensor *ts);
string MakeColumn(string c, unsigned int width);

// Application entry point
int main(int argc, char *argv[])
{
	unsigned int i;
	std::vector<std::string> sensorList;
	if (argc > 1)
	{
		for (i = 1; i < (unsigned int)argc; i++)
			sensorList.push_back(argv[i]);

		cout << "Using " << sensorList.size() << " user-specified sensor ROMs" << endl;
	}
	else
	{
		sensorList = TemperatureSensor::GetConnectedSensors();
		cout << "Auto-detected " << sensorList.size() << " connected sensors" << endl;
	}

	if (sensorList.size() == 0)
	{
		cout << "No valid sensors detected" << endl;
		return 1;
	}

	TemperatureSensor **tsArray = new TemperatureSensor*[sensorList.size()];

	string heading1, heading2, heading3;
	stringstream s;
	const unsigned int columnWidth(12);

	cout << endl << "Sensor list:" << endl;

	for (i = 0; i < sensorList.size(); i++)
	{
		cout << "Sensor " << i << ":  " << sensorList[i] << endl;
		tsArray[i] = new TemperatureSensor(sensorList[i]);

		s.str("");
		s << "Sensor " << i;
		heading1.append(MakeColumn(s.str(), columnWidth));
		heading2.append(MakeColumn("[deg C]", columnWidth));
		heading3.append(string(columnWidth,'-'));
	}

	struct timespec start, stop, resolution;
	TimingUtility loopTimer(0.85);
	if (!loopTimer.GetResolution(resolution))
		cout << "Failed to read timer resolution" << endl;
	else
		cout << "Timer resoltuion on this machine is " << TimingUtility::TimespecToSeconds(resolution) * 1.0e9 << " nsec" << endl;

	cout << "Sensor readings will be taken every " << loopTimer.GetTimeStep() << " seconds." << endl;
	cout << "Reading frequency with the built-in system method is limited to one sensor every 750 msec (due to unconfigurable 12-bit resolution)" << endl;
	cout << "In practice, additional time is required for reading/writing from/to the sensor" << endl;
	cout << "Testing has shown that a minimum of about 0.83 seconds per sensor is required" << endl;
	cout << "Access to the 1-wire interface can improve timing in two ways:" << endl;
	cout << "  1) Configure the sensor to use less resolution" << endl;
	cout << "  2) Broadcast to all sensors to begin measurements at once, then read from each sensor"
		<< " (sensors update in parallel instead of series - applicable only for cases where multiple sensors are used)" << endl;

	cout << endl;
	cout << heading1 << endl;
	cout << heading2 << endl;
	cout << heading3 << endl;

	if (!TimingUtility::GetCurrentTime(start))
		cout << "Failed to start execution timer" << endl;

	const unsigned int readings(10);
	for (i = 0; i < readings * sensorList.size(); i++)
	{
		if (!loopTimer.TimeLoop())
			cout << "Loop timer failed" << endl;

		cout << MakeColumn(GetReadingString(tsArray[i % sensorList.size()]), columnWidth);
		cout.flush();

		if (i % sensorList.size() == sensorList.size() - 1)
			cout << endl;
	}

	if (!TimingUtility::GetCurrentTime(stop))
		cout << "Failed to stop execution timer" << endl;
	else
	{
		cout << "Average actual read time:  ";
		cout << TimingUtility::TimespecToSeconds(TimingUtility::GetDeltaTime(stop, start))
			/ double(readings * sensorList.size()) << " sec" << endl;
	}

	for (i = 0; i < sensorList.size(); i++)
		delete tsArray[i];
	delete [] tsArray;

	return 0;
}

string GetReadingString(TemperatureSensor *ts)
{
	assert(ts);

	double temp;
	stringstream s;
	s.precision(3);
	if (ts->GetTemperature(temp))
		s << fixed << temp;
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
