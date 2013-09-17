// File:  combinedLogger.h
// Date:  9/3/2013
// Auth:  K. Loux
// Desc:  Logging object that permits writing to multiple logs simultaneously.

#ifndef COMBINED_LOGGER_H_
#define COMBINED_LOGGER_H_

// Standard C++ headers
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

// Local headers
#include "logger.h"

class CombinedLogger
{
public:
	static CombinedLogger& GetLogger(void);
	static void Destroy(void);

	void Add(Logger* log);

	friend CombinedLogger& operator<<(CombinedLogger& log, std::ostream& (*pf) (std::ostream&));

	template<typename T>
	friend CombinedLogger& operator<<(CombinedLogger& log, T const& value);

private:
	CombinedLogger();
	~CombinedLogger();

	static CombinedLogger *logger;
	static const std::string logFileName;

	static std::ofstream file;
	std::vector<Logger*> logs;
};

//==========================================================================
// Class:			(friend of) CombinedLogger
// Function:		operator<<
//
// Description:		Adds the argument to the log buffers.
//
// Input Arguments:
//		log		= CombinedLogger&
//		value	= T const& (template type)
//
// Output Arguments:
//		None
//
// Return Value:
//		CombinedLogger& (returns log argument)
//
//==========================================================================
template<typename T>
CombinedLogger& operator<<(CombinedLogger& log, T const& value)
{
	unsigned int i;
	for (i = 0; i < log.logs.size(); i++)
		*log.logs[i] << value;
	return log;
};

#endif// COMBINED_LOGGER_H_
