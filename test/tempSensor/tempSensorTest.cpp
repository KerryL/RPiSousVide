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
#include <time.h>

// *nix standard headers
#include <unistd.h>

// Local headers
#include "temperatureSensor.h"

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

	struct timespec start, stop, loopStart, loopStop;
	double elapsed;
	double timeStep(0.85);// [sec]

	cout << "Sensor readings will be taken every " << timeStep << " seconds." << endl;
	cout << "Reading frequency with the built-in system method is limited to one sensor every 750 msec (+overhead for reading from sensor)" << endl;

	cout << endl;
	cout << heading1 << endl;
	cout << heading2 << endl;
	cout << heading3 << endl;

	if (clock_gettime(CLOCK_MONOTONIC, &loopStart) == -1)
	{
		cout << "clock_gettime failed" << endl;
		return 1;
	}

	const unsigned int readings(10);
	for (i = 0; i < readings * sensorList.size(); i++)
	{
		if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
		{
			cout << "clock_gettime failed" << endl;
			return 1;
		}

		cout << MakeColumn(GetReadingString(tsArray[i % sensorList.size()]), columnWidth);
		cout.flush();

		if (i % sensorList.size() == sensorList.size() - 1)
			cout << endl;

		if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1)
		{
			cout << "clock_gettime failed" << endl;
			return 1;
		}

		// Handle overflows
		elapsed = stop.tv_sec - start.tv_sec + (stop.tv_nsec - start.tv_nsec) * 1.0e-9;
		if (elapsed > timeStep)
			continue;

		usleep(1000000 * (timeStep - elapsed));
	}

	if (clock_gettime(CLOCK_MONOTONIC, &loopStop) == -1)
	{
		cout << "clock_gettime failed" << endl;
		return 1;
	}

	elapsed = loopStop.tv_sec - loopStart.tv_sec + (loopStop.tv_nsec - loopStart.tv_nsec) * 1.0e-9;

	cout << "Average actual read time:  ";
	cout << elapsed / double(readings * sensorList.size()) << " sec" << endl;

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
