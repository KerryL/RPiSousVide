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
pthread_mutex_t CombinedLogger::CombinedStreamBuffer::mutex = PTHREAD_MUTEX_INITIALIZER;

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
	// No lock here - users must ensure that Destroy is called by only one thread!
	unsigned int i;
	for (i = 0; i < logs.size(); i++)
	{
		if (logs[i].second)
			delete logs[i].first;
	}

	int errorNumber;
	if ((errorNumber = pthread_mutex_destroy(&mutex)) != 0)
		std::cout << "Error destroying mutex (" << errorNumber << ")" << std::endl;
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
//		log				= std::ostream*
//		manageMemory	= bool
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void CombinedLogger::Add(std::ostream* log, bool manageMemory)
{
	assert(log);
	MutexLocker lock(mutex);
	logs.push_back(std::make_pair(log, manageMemory));
}