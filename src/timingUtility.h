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
#include <vector>
#include <map>
#include <time.h>

class TimingUtility
{
public:
	TimingUtility(double timeStep, std::ostream &outStream = std::cout);

	void SetLoopTime(double timeStep);
	bool TimeLoop(void);
	double GetTimeStep(void) const { return timeStep; };// [sec]

	std::string GetTimingStatistics(void) const;

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

	// For timing statistics
	void UpdateTimingStatistics(void);
	double AverageVector(const std::vector<double> &v) const;
	std::string MakeColumn(double value, unsigned int columnWidth) const;
	std::string MakeColumn(std::string s, unsigned int columnWidth, char pad = ' ') const;

	unsigned int currentIndex;
	std::map<double, unsigned int> stepIndices;
	std::vector<unsigned long> counts;
	std::vector<double> minFrameTimes, maxFrameTimes, averageFrameTimes;// [sec]
	std::vector<double> minBusyTimes, maxBusyTimes, averageBusyTimes;// [sec]
	struct timespec lastUpdate;
};

#endif// TIMING_UTILITY_H_
