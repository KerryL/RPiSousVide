// File:  autoTunerTest.cpp
// Date:  9/19/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Test application for heated tank auto tuner.

// Standard C++ headers
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

// Local headers
#include "autoTuner.h"

using namespace std;

// Application entry point
int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "Usage:  " << argv[0] << " pathToFile" << endl;
		return 1;
	}

	ifstream dataFile(argv[1], ios::in);
	if (!dataFile.is_open() || !dataFile.good())
	{
		cout << "Failed to open '" << argv[1] << "' for input" << endl;
		return 1;
	}

	string line;
	vector<double> time, temp;
	size_t split;
	while (getline(dataFile, line))
	{
		split = line.find_first_not_of(' ');
		split = line.find(' ', split + 1);
		time.push_back(atof(line.substr(0, split).c_str()));
		temp.push_back(atof(line.substr(split).c_str()));
	}

	dataFile.close();

	AutoTuner tuner;
	if (!tuner.ProcessAutoTuneData(time, temp))
	{
		cout << "Auto-tune failed" << endl;
		return 1;
	}

	cout << "Model parameters:" << endl;
	cout << "  c1 = " << tuner.GetC1() << " 1/sec" << endl;
	cout << "  c2 = " << tuner.GetC2() << " deg F/BTU" << endl;

	cout << "Recommended Gains:" << endl;
	cout << "  Kp = " << tuner.GetKp() << " %/deg F" << endl;
	cout << "  Ti = " << tuner.GetTi() << " sec" << endl;
	cout << "  Kf = " << tuner.GetKf() << " deg F/BTU" << endl;

	cout << "Other parameters:" << endl;
	cout << "  Max. Heat Rate = " << tuner.GetMaxHeatRate() << " deg F/sec" << endl;
	cout << "  Ambient Temp. = " << tuner.GetAmbientTemperature() << " deg F" << endl;

	std::vector<double> simulatedTemp;
	std::vector<double> controlInput(time.size(), 1.0);
	cout << endl << "Simulating time response..." << endl;
	if (!tuner.GetSimulatedOpenLoopResponse(time, controlInput, simulatedTemp))
	{
		cout << "Simulation failed" << endl;
		return 1;
	}
	else
		cout << "Simulation complete" << endl;

	const std::string resultsFileName("simulationComparison.log");
	cout << "Writing results to '" << resultsFileName << "'" << endl;

	ofstream simResults(resultsFileName.c_str(), ios::out);
	if (!simResults.is_open() || !simResults.good())
	{
		cout << "Unable to open '" << resultsFileName << "' for output" << endl;
		return 1;
	}

	simResults << "Time,Actual Temperature,SimulatedTemperature" << endl;
	simResults << "[sec],[deg F],[deg F]" << endl;

	unsigned int i;
	for (i = 0; i < time.size(); i++)
		simResults << time[i] << "," << temp[i] << "," << simulatedTemp[i] << endl;

	simResults.close();

	return 0;
}
