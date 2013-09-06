// File:  combinedLogger.h
// Date:  9/3/2013
// Auth:  K. Loux
// Desc:  Logging object.  Handles two logs, one for application status and
//        one for easy temperature vs. time plotting.  Note that this class
//		  is NOT thread-safe!

// Standard C++ headers
#include <cassert>
#include <iostream>

// Local headers
#include "combinedLogger.h"
#include "logger.h"

//==========================================================================
// Class:			CombinedLogger
// Function:		None
//
// Description:		Static member variable initialization.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
CombinedLogger *CombinedLogger::logger = NULL;
const std::string CombinedLogger::logFileName = "sousVide.log";
std::ofstream CombinedLogger::file(logFileName.c_str(), std::ios::out);

//==========================================================================
// Class:			CombinedLogger
// Function:		CombinedLogger
//
// Description:		Constructor for CombinedLogger class.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
CombinedLogger::CombinedLogger()
{
}

//==========================================================================
// Class:			CombinedLogger
// Function:		CombinedLogger
//
// Description:		Constructor for CombinedLogger class.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
CombinedLogger::~CombinedLogger()
{
	file.close();

	unsigned int i;
	for (i = 0; i < logs.size(); i++)
		delete logs[i];
}

//==========================================================================
// Class:			CombinedLogger
// Function:		GetLogger
//
// Description:		Returns a pointer to an instance of the application's log object.
//
// Input Arguments:
//		appLogFile	= std::string, specifying path for the overwritten-on-each-instance
//					  application log file
//
// Output Arguments:
//		None
//
// Return Value:
//		CombinedLogger*, pointing to the static CombinedLogger object for the application
//
//==========================================================================
CombinedLogger& CombinedLogger::GetLogger(void)
{
	if (!logger)
	{
		logger = new CombinedLogger();

		logger->Add(new Logger(std::cout));
		logger->Add(new Logger(file));
	}

	return *logger;
}

//==========================================================================
// Class:			CombinedLogger
// Function:		Destroy
//
// Description:		Deletes the static logging object.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void CombinedLogger::Destroy(void)
{
	delete logger;
	logger = NULL;
}

//==========================================================================
// Class:			CombinedLogger
// Function:		Add
//
// Description:		Adds a log sink to the vector.
//
// Input Arguments:
//		log	= Logger*
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void CombinedLogger::Add(Logger* log)
{
	logs.push_back(log);
}

//==========================================================================
// Class:			(friend of) CombinedLogger
// Function:		operator<<
//
// Description:		Adds the argument to the log buffers.  This one is included
//					becuase the template method doesn't properly handle std::endl.
//
// Input Arguments:
//		log	= CombinedLogger&
//		pf	= std::ostream&* (std::ostream&) (<-- function call)
//
// Output Arguments:
//		None
//
// Return Value:
//		CombinedLogger& (returns log argument)
//
//==========================================================================
CombinedLogger& operator<<(CombinedLogger& log, std::ostream& (*pf) (std::ostream&))
{
	unsigned int i;
	for (i = 0; i < log.logs.size(); i++)
		*log.logs[i] << pf;
	return log;
};