// File:  loggerTest.cpp
// Date:  9/15/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Application for testing two logging classes:
//        TimeHistoryLog and CombinedLogger.

// Standard C++ headers
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>

// *nix Standard headers
#include <unistd.h>

// Local headers
#include "combinedLogger.h"
#include "logger.h"
#include "timeHistoryLog.h"

using namespace std;

void LoggingFunctionTakingOStreamArg(ostream& s);

// Application entry point
int main(int, char *[])
{
	cout << "Beginning TimeHistoryLog Test" << endl;
	cout << "Output will be written to stdout:" << endl << endl;
	TimeHistoryLog timeHistoryLog(cout);
	timeHistoryLog.AddColumn("First Field", "Unit 1");
	timeHistoryLog.AddColumn("Second Field", "Unit 2");

	// First data passed to the log will be time stamped 0.0 (or close to it)
	// Must ALWAYS send data equal to number of columns, then endl
	timeHistoryLog << 1.0 << 2.0 << endl;
	sleep(1);
	timeHistoryLog << 3.0 << 4.0 << endl;
	sleep(3);
	timeHistoryLog << 5.0 << 6.0 << endl;

	cout << "Ending test of TimeHistoryLog" << endl << endl;
	cout << "Beginning test of CombinedLogger (passed as ostream argument)" << endl;
	const string logFileName("loggerTest.log");
	cout << "Output will be written to stdout and '" << logFileName << "':" << endl << endl;

	ofstream logFile(logFileName.c_str(), ios::out);
	if (!logFile.is_open() || !logFile.good())
	{
		cout << "Failed to open '" << logFileName << "' for output" << endl;
		return 1;
	}

	CombinedLogger::GetLogger().Add(new Logger(cout));
	CombinedLogger::GetLogger().Add(new Logger(logFile));

	// We use a separate function here, because we want to be able to pass our logger
	// to funcitons taking an ostream argument, and we demonstrate that here
	LoggingFunctionTakingOStreamArg(CombinedLogger::GetLogger());

	CombinedLogger::Destroy();
	logFile.close();

	cout << "Ending test of CombinedLogger (passed as ostream argument)" << endl;
	cout << "Beginning test of CombinedLogger (used directly for special formatting)" << endl;
	cout << "Output will be written to stdout only:" << endl << endl;

	CombinedLogger::GetLogger().Add(new Logger(cout));

	CombinedLogger::GetLogger() << "Here's a number with trailing zeros:  " << std::setprecision(10) << std::fixed << 1.0 << std::endl;
	CombinedLogger::GetLogger() << "Here's a hex number:  0x" << std::hex << 256 << std::endl;
	CombinedLogger::GetLogger() << "Here's a number in scientific notation:  " << std::scientific << 6548454.8486868 << std::endl;

	CombinedLogger::Destroy();

	cout << "Ending test of CombinedLogger (used directly for special formatting)" << endl;

	return 0;
}

void LoggingFunctionTakingOStreamArg(ostream& s)
{
	s << "Here's the first entry" << endl;
	sleep(1);
	s << "Here's the second" << endl;
	sleep(3);
	s << "And now the third (and last)" << endl;
}
