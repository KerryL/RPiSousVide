// File:  combinedLogger.h
// Date:  9/3/2013
// Auth:  K. Loux
// Copy:  (c) Kerry Loux 2013
// Desc:  Logging object that permits writing to multiple logs simultaneously
//        and from multiple threads.

// Local headers
#include "combinedLogger.h"

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
pthread_mutex_t CombinedLogger::mutex = PTHREAD_MUTEX_INITIALIZER;

//==========================================================================
// Class:			CombinedLogger
// Function:		~CombinedLogger
//
// Description:		Destructor for CombinedLogger class.
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
	// No lock necessary, since this can only be reached through Destroy() (which includes a lock)
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
//		None
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
	// Check, lock, then check again to ensure we don't encounter a race condition
	if (!logger)
	{
		MutexLocker lock(mutex);

		if (!logger)
			logger = new CombinedLogger();
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
	MutexLocker lock(mutex);

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
//		log	= std::ostream*
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void CombinedLogger::Add(std::ostream* log)
{
	assert(log);
	MutexLocker lock(mutex);
	logs.push_back(log);
}
