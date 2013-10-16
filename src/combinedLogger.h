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
#include <sstream>
#include <cassert>

// Local headers
#include "mutexLocker.h"

class CombinedLogger : public std::ostream
{
public:
	static CombinedLogger& GetLogger(void);
	static void Destroy(void);

	void Add(std::ostream* log, bool manageMemory = true);

private:
	class CombinedStreamBuffer : public std::stringbuf
	{
	public:
		CombinedStreamBuffer(CombinedLogger &log) : log(log) {};
		virtual ~CombinedStreamBuffer()
		{
			std::map<pthread_t, std::stringstream*>::iterator it;
			for (it = threadBuffer.begin(); it != threadBuffer.end(); it++)
				delete it->second;

			int errorNumber;
			if ((errorNumber = pthread_mutex_destroy(&mutex)) != 0)
				std::cout << "Error destroying mutex (" << errorNumber << ")" << std::endl;
		};

	protected:
		virtual int overflow(int c)
		{
			CreateThreadBuffer();
			if (c != traits_type::eof())
				*threadBuffer[pthread_self()] << (char)c;

			return c;
		};

		virtual int sync(void)
		{
			assert(log.logs.size() > 0);// Make sure we didn't forget to add logs

			CreateThreadBuffer();// Before mutex locker, because this might lock the mutex, too
			MutexLocker lock(mutex);
			
			unsigned int i;
			for (i = 0; i < log.logs.size(); i++)
			{
				*log.logs[i].first << threadBuffer[pthread_self()]->str();
				log.logs[i].first->flush();
			}

			// Clear out the buffers
			threadBuffer[pthread_self()]->str("");
			str("");

			return 0;
		};

	private:
		CombinedLogger &log;
		std::map<pthread_t, std::stringstream*> threadBuffer;
		static pthread_mutex_t mutex;
		void CreateThreadBuffer(void)
		{
			if (threadBuffer.find(pthread_self()) == threadBuffer.end())
			{
				MutexLocker lock(mutex);
				if (threadBuffer.find(pthread_self()) == threadBuffer.end())
					threadBuffer[pthread_self()] = new std::stringstream;
			}
		};
	} buffer;

	CombinedLogger() : std::ostream(&buffer), buffer(*this) {};
	virtual ~CombinedLogger();

	static CombinedLogger *logger;
	static pthread_mutex_t mutex;

	std::vector<std::pair<std::ostream*, bool> > logs;
};

#endif// COMBINED_LOGGER_H_