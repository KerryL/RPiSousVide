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
		tsArray[0] = new TemperatureSensor(sensorList[i]);

		s.str("");
		s << "Sensor " << i;
		heading1.append(MakeColumn(s.str(), columnWidth));
		heading2.append(MakeColumn("[deg C]", columnWidth));
		heading3.append(string(columnWidth,'-'));
	}

	cout << endl;
	cout << heading1 << endl;
	cout << heading2 << endl;
	cout << heading3 << endl;

	clock_t start, stop;
	double elapsed;
	double timeStep(1.0);// [sec]
	
	while (true)
	{
		start = clock();

		for (i = 1; i < (unsigned int)argc; i++)
			cout << MakeColumn(GetReadingString(tsArray[i - 1]), columnWidth);
		cout << endl;

		stop = clock();

		// Handle overflows
		elapsed = double(stop - start) / (double)(CLOCKS_PER_SEC);
		if (stop < start || elapsed > timeStep)
			continue;

		usleep(1000000 * (timeStep - elapsed));
	}

	for (i = 1; i < (unsigned int)argc; i++)
		delete tsArray[i - 1];
	delete [] tsArray;

	return 0;
}

string GetReadingString(TemperatureSensor *ts)
{
	assert(ts);

	double temp;
	stringstream s;
	if (ts->GetTemperature(temp))
		s << setprecision(4) << temp;
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
