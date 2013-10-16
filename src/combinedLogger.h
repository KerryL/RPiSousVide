// File:  combinedLogger.h
// Date:  9/3/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Logging object that permits writing to multiple logs simultaneously
//        and from multiple threads.

#ifndef COMBINED_LOGGER_H_
#define COMBINED_LOGGER_H_

// pThread headers (must be first!)
#include <pthread.h>

// Standard C++ headers
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <cassert>

// Local headers
#include "mutexLocker.h"

class CombinedLogger : public std::ostream
{
public:
	static CombinedLogger& GetLogger(void);
	static void Destroy(void);

	void Add(std::ostream* log);

	template <typename T>
	friend CombinedLogger& operator<<(CombinedLogger &out, T t);

private:
	class CombinedStreamBuffer : public std::stringbuf
	{
	public:
		CombinedStreamBuffer(CombinedLogger &log) : log(log) {}

		virtual int sync(void)
		{
			assert(log.logs.size() > 0);// Make sure we didn't forget to add logs
			MutexLocker lock(log.mutex);
			
unsigned int i;
			for (i = 0; i < log.logs.size(); i++)
				*log.logs[i] << log.threadBuffer[pthread_self()] << std::endl;
/*{
				*log.logs[i] << str();
log.logs[i]->flush();
}*/

			// Clear out the buffers
			log.threadBuffer[pthread_self()].clear();
			str("");

			return 0;
		};

	private:
		CombinedLogger &log;
	} buffer;

	CombinedLogger() : std::ostream(&buffer), buffer(*this) {};
	~CombinedLogger();

	static CombinedLogger *logger;
	static pthread_mutex_t mutex;

	std::vector<std::ostream*> logs;
	std::map<pthread_t, std::string> threadBuffer;
};

template <typename T>
CombinedLogger& operator<<(CombinedLogger &out, T t)
{
	std::stringstream ss;
	ss << t;
	out.threadBuffer[pthread_self()].append(ss.str());
	std::cerr << "t contains: " << t << std::endl;
	std::cerr << "buffer should now contatin: " << ss.str() << std::endl;
	std::cerr << "it does contain: " << out.threadBuffer[pthread_self()] << std::endl;
	return out;
}

#endif// COMBINED_LOGGER_H_
