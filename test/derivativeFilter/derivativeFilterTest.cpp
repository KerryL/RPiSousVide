// File:  derivativeFilterTest.cpp
// Date:  9/18/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Test application for derivative filter class.

// Standard C++ headers
#include <cstdlib>
#include <iostream>
#include <fstream>

// Local headers
#include "derivativeFilter.h"

using namespace std;

// Application entry point
int main(int, char *[])
{
	const std::string testOutput("filterTest.csv");
	cout << "Writing test data to " << testOutput << endl;
	cout << "File will contain columns for time, input, fast filter output and slow filter output" << endl;
	cout << "Fast filter has time constant of 0.1 sec, slow filter has time constant of 1.0 sec" << endl;

	fstream file(testOutput.c_str(), ios::out);
	if (!file.is_open() || !file.good())
	{
		cout << "Failed to open file for output" << endl;
		return 1;
	}

	const double timeStep(0.01);
	const double endTime(20.0);
	DerivativeFilter fastFilter(timeStep, 0.1);
	DerivativeFilter slowFilter(timeStep, 1.0);

	file << "Time,Input,Fast Filter,SlowFilter" << endl;
	file << "[sec],[-],[1/sec],[1/sec]" << endl;

	unsigned int i;
	double time(0.0), rate(1.0), input(10.0);
	fastFilter.Reset(input);
	slowFilter.Reset(input);
	for (i = 0; i < endTime / timeStep + 1; i++)
	{
		file << time << "," << input << "," << fastFilter.Apply(input) << "," << slowFilter.Apply(input) << endl;
		if (i == endTime / timeStep / 2)
			rate *= 2.0;

		input += rate;
		time += timeStep;
	}

	file.close();

	return 0;
}
