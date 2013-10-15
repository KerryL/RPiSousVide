// File:  timingUtility.h
// Date:  10/14/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Timing utilities including elapsed time methods and loop timer.

#ifndef TIMING_UTILITY_H_
#define TIMING_UTILITY_H_

// Standard C/C++ headers
#include <string>
#include <iostream>
#include <time.h>

class TimingUtility
{
public:
	TimingUtility(double timeStep, std::ostream &outStream = std::cout);

	void SetLoopTime(double timeStep);
	bool TimeLoop(void);
	double GetLastLoopTime(void) const { return elapsed; };// [sec]
	double GetTimeStep(void) const { return timeStep; };// [sec]
	void Reset(void);

	static clockid_t clockID;
	static bool GetCurrentTime(struct timespec &ts);
	static bool GetResolution(struct timespec &ts);
	static struct timespec GetDeltaTime(const struct timespec &newTime, const struct timespec &oldTime);
	static double TimespecToSeconds(const struct timespec &ts);

private:
	std::ostream &outStream;

	double timeStep, elapsed;// [sec]
	struct timespec loopTime;

	bool loopStarted;
};

#endif// TIMING_UTILITY_H_
