// File:  loggerTest.cpp
// Date:  9/15/2013
// Auth:  K. Loux
// Desc:  Application for testing two logging classes:
//        TimeHistoryLog and CombinedLogger.

// Standard C++ headers
#include <cstdlib>
#include <iostream>

// *nix Standard headers
#include <unistd.h>

// Local headers
#include "combinedLogger.h"
#include "timeHistoryLog.h"

using namespace std;

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
	cout << "Beginning test of CombinedLogger" << endl;
	cout << "Output will be written to stdout and sousVide.log:" << endl << endl;

	CombinedLogger::GetLogger() << "Here's the first entry" << endl;
	sleep(1);
	CombinedLogger::GetLogger() << "Here's the second" << endl;
	sleep(3);
	CombinedLogger::GetLogger() << "And now the third (and last)" << endl;

	CombinedLogger::Destroy();

	cout << "Ending test of CombinedLogger" << endl;

	return 0;
}
