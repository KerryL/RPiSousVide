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
	if (argc < 2)
	{
		cout << "Usage:  " << argv[0] << " sensorID1 [sensorID2] ..." << endl;
		return 1;
	}

	TemperatureSensor **tsArray = new TemperatureSensor*[argc - 1];

	string heading1, heading2, heading3;
	stringstream s;
	const unsigned int columnWidth(12);
	int i;
	for (i = 1; i < argc; i++)
	{
		tsArray[i - 1] = new TemperatureSensor(argv[i]);

		s.str("");
		s << "Sensor " << i;
		heading1.append(MakeColumn(s.str(), columnWidth));
		heading2.append(MakeColumn("[deg F]", columnWidth));
		heading3.append(string(columnWidth,'-'));
	}

	cout << heading1 << endl;
	cout << heading2 << endl;
	cout << heading3 << endl;

	clock_t start, stop;
	double elapsed;
	double timeStep(1.0);// [sec]
	// TODO:  Demonstrate changing sensor resolution and using faster sample time
	
	while (true)
	{
		start = clock();

		for (i = 1; i < argc; i++)
			cout << MakeColumn(GetReadingString(tsArray[i - 1]), columnWidth);
		cout << endl;

		stop = clock();

		// Handle overflows
		elapsed = double(stop - start) / (double)(CLOCKS_PER_SEC);
		if (stop < start || elapsed > timeStep)
			continue;

		usleep(1000000 * (timeStep - elapsed));
	}

	for (i = 1; i < argc; i++)
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
