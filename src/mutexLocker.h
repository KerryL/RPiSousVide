// File:  mutexLocker.h
// Date:  9/16/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Helper function for pThread mutexes.  Locks a mutex when created,
//        releases when destroyed.  This can be used to ease handling of
//        mutexes in a function with multiple control paths.

#ifndef MUTEX_HELPER_H_
#define MUTEX_HELPER_H_

// pThread headers (must be first!)
#include <pthread.h>

class MutexLocker
{
public:
	MutexLocker(pthread_mutex_t &mutex);
	~MutexLocker();

private:
	pthread_mutex_t &mutex;
};

#endif// MUTEX_HELPER_H_
