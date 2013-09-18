// File:  combinedLogger.h
// Date:  9/3/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
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

class CombinedLogger : public std::ostream
{
public:
	static CombinedLogger& GetLogger(void);
	static void Destroy(void);

	void Add(Logger* log);

private:
	class CombinedStreamBuffer : public std::stringbuf
	{
	public:
		CombinedStreamBuffer(CombinedLogger &log) : log(log) {}

		virtual int sync(void)
		{
			unsigned int i;
			for (i = 0; i < log.logs.size(); i++)
			{
				*log.logs[i] << str();
				log.logs[i]->flush();
			}
			str("");
			return 0;
		};

	private:
		CombinedLogger &log;
	} buffer;

	CombinedLogger() : std::ostream(&buffer), buffer(*this) {};
	~CombinedLogger();

	static CombinedLogger *logger;
	static const std::string logFileName;

	static std::ofstream file;
	std::vector<Logger*> logs;
};

#endif// COMBINED_LOGGER_H_
