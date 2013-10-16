// File:  timingUtility.cpp
// Date:  10/14/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Timing utilities including elapsed time methods and loop timer.

// Standard C++ headers
#include <errno.h>
#include <cstring>
#include <cassert>
#include <iomanip>
#include <sstream>

// *nix standard headers
#include <unistd.h>

// Local headers
#include "timingUtility.h"

//==========================================================================
// Class:			TimingUtility
// Function:		Constant Definitions
//
// Description:		Constant definitions for TimingUtility class.
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
clockid_t TimingUtility::clockID = CLOCK_MONOTONIC;

//==========================================================================
// Class:			TimingUtility
// Function:		TimingUtility
//
// Description:		Constructor for TimingUtility class.
//
// Input Arguments:
//		timeStep	= double [sec]
//		outStream	= std::ostream&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
TimingUtility::TimingUtility(double timeStep, std::ostream &outStream) : outStream(outStream)
{
	SetLoopTime(timeStep);
}

//==========================================================================
// Class:			TimingUtility
// Function:		SetLoopTime
//
// Description:		Sets the time step for the loop timer.
//
// Input Arguments:
//		timeStep	= double [sec]
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void TimingUtility::SetLoopTime(double timeStep)
{
	assert(timeStep > 0.0);
	this->timeStep = timeStep;

	if (stepIndices.find(timeStep) == stepIndices.end())
	{
		currentIndex = counts.size();
		stepIndices[timeStep] = currentIndex;

		counts.push_back(0);
		minFrameTimes.push_back(timeStep * 10.0);// Something much larger than timeStep
		maxFrameTimes.push_back(0.0);
		averageFrameTimes.push_back(0.0);
		minBusyTimes.push_back(1.0);
		maxBusyTimes.push_back(0.0);
		averageBusyTimes.push_back(0.0);
	}
	else
		currentIndex = stepIndices[timeStep];

	assert(currentIndex < counts.size());
	assert(currentIndex < minFrameTimes.size());
	assert(currentIndex < maxFrameTimes.size());
	assert(currentIndex < averageFrameTimes.size());
	assert(currentIndex < minBusyTimes.size());
	assert(currentIndex < maxBusyTimes.size());
	assert(currentIndex < averageBusyTimes.size());
}

//==========================================================================
// Class:			TimingUtility
// Function:		TimeLoop
//
// Description:		Performs timing and sleeping to maintain specified time
//					step.  Must be called exactly once per loop and AT
//					THE TOP OF THE LOOP!.  If not placed at the top of the loop,
//					the first and second passes through the loop both occur
//					prior to the first sleep.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool TimingUtility::TimeLoop(void)
{
	if (counts[0] > 0)
	{
		struct timespec now;
		if (!GetCurrentTime(now))
		{
			outStream << "Failed to get current time:  " << std::strerror(errno) << std::endl;
			return false;
		}

		const double warningThreshold(1.01);// 1% threshold
		elapsed = TimespecToSeconds(GetDeltaTime(now, loopTime));
		if (elapsed > timeStep * warningThreshold)
			outStream << "Warning:  Elapsed time is greater than time step ("
				<< elapsed << " > " << timeStep << ")" << std::endl;
		else
			usleep(1000000 * (timeStep - elapsed));
	}
	else
	{
		elapsed = 0.0;
		assert(currentIndex == 0);
	}

	if (!GetCurrentTime(loopTime))
	{
		outStream << "Failed to get current time:  " << std::strerror(errno) << std::endl;
		return false;
	}

	UpdateTimingStatistics();
	counts[currentIndex]++;

	return true;
}

//==========================================================================
// Class:			TimingUtility
// Function:		GetCurrentTime (static)
//
// Description:		Returns timespec struct representing current time.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		ts	= struct timespec&
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool TimingUtility::GetCurrentTime(struct timespec &ts)
{
	if (clock_gettime(clockID, &ts) == -1)
		return false;

	return true;
}

//==========================================================================
// Class:			TimingUtility
// Function:		GetResolution (static)
//
// Description:		Returns timespec struct representing timing resolution on
//					this system.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		ts	= struct timespec&
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool TimingUtility::GetResolution(struct timespec &ts)
{
	if (clock_getres(clockID, &ts) == -1)
		return false;

	return true;
}

//==========================================================================
// Class:			TimingUtility
// Function:		GetDeltaTime (static)
//
// Description:		Returns timespec struct representing difference between
//					the arguments.
//
// Input Arguments:
//		newTime	= const struct timespec&
//		oldTime	= const struct timespec&
//
// Output Arguments:
//		None
//
// Return Value:
//		struct timespec representing the time different between the arguments
//
//==========================================================================
struct timespec TimingUtility::GetDeltaTime(const struct timespec &newTime,
	const struct timespec &oldTime)
{
	struct timespec ts;

	ts.tv_sec = newTime.tv_sec - oldTime.tv_sec;
	ts.tv_nsec = newTime.tv_nsec - oldTime.tv_nsec;

	return ts;
}

//==========================================================================
// Class:			TimingUtility
// Function:		TimespecToSeconds (static)
//
// Description:		Converts the argument to a single value representing the
//					time in seconds.
//
// Input Arguments:
//		ts		= const struct timespec&
//
// Output Arguments:
//		None
//
// Return Value:
//		double, representing time in seconds
//
//==========================================================================
double TimingUtility::TimespecToSeconds(const struct timespec &ts)
{
	return ts.tv_sec + ts.tv_nsec * 1.0e-9;
}

//==========================================================================
// Class:			TimingUtility
// Function:		UpdateTimingStatistics
//
// Description:		Updates timing statistics for the loop.
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
void TimingUtility::UpdateTimingStatistics(void)
{
	struct timespec now;
	if (!TimingUtility::GetCurrentTime(now))
	{
		outStream << "Warning:  Failed to update time while calculating timing statistics" << std::endl;
		return;
	}

	double totalElapsed = TimingUtility::TimespecToSeconds(TimingUtility::GetDeltaTime(now, lastUpdate));
	lastUpdate = now;

	if (counts[0] == 0)
		return;

	if (totalElapsed < minFrameTimes[currentIndex])
		minFrameTimes[currentIndex] = totalElapsed;
	if (totalElapsed > maxFrameTimes[currentIndex])
		maxFrameTimes[currentIndex] = totalElapsed;

	if (elapsed < minBusyTimes[currentIndex])
		minBusyTimes[currentIndex] = elapsed;
	if (elapsed > maxBusyTimes[currentIndex])
		maxBusyTimes[currentIndex] = elapsed;

	// To compute averages, we need at least one good data point
	if (counts[currentIndex] == 0)
		return;

	averageFrameTimes[currentIndex] = (averageFrameTimes[currentIndex] * (counts[currentIndex] - 1)
		+ totalElapsed) / (double)counts[currentIndex];
	averageBusyTimes[currentIndex] = (averageBusyTimes[currentIndex] * (counts[currentIndex] - 1)
		+ elapsed) / (double)counts[currentIndex];
}

//==========================================================================
// Class:			TimingUtility
// Function:		GetTimingStatistics
//
// Description:		Returns a string containing the timing statistics for the
//					loop.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		std::string
//
//==========================================================================
std::string TimingUtility::GetTimingStatistics(void) const
{
	unsigned int i;
	std::vector<double> timeAtStep;
	double totalTime(0.0);// [sec]
	std::map<double, unsigned int>::const_iterator it(stepIndices.begin());
	for (i = 0; i < counts.size(); i++)
	{
		timeAtStep.push_back(it->first * counts[it->second]);
		totalTime += timeAtStep[i];
		it++;
	}

	const unsigned int titleColumnWidth(24), dataColumnWidth(12);

	std::stringstream ss;
	it = stepIndices.begin();
	for (i = 0; i < counts.size(); i++)
	{
		ss << "Time step = " << it->first <<
			" sec (" << timeAtStep[i] / totalTime * 100.0 << "% of total loop time)" << std::endl;
		ss << MakeColumn("", titleColumnWidth)
			<< MakeColumn("Min", dataColumnWidth)
			<< MakeColumn("Max", dataColumnWidth)
			<< MakeColumn("Avg", dataColumnWidth) << std::endl;
		ss << MakeColumn("", titleColumnWidth + 3 * dataColumnWidth, '-') << std::endl;
		ss << MakeColumn("Frame Duration (sec)", titleColumnWidth)
			<< MakeColumn(minFrameTimes[it->second], dataColumnWidth)
			<< MakeColumn(maxFrameTimes[it->second], dataColumnWidth)
			<< MakeColumn(averageFrameTimes[it->second], dataColumnWidth)
			<< std::endl;
		ss << MakeColumn("Busy Period    (sec)", titleColumnWidth)
			<< MakeColumn(minBusyTimes[it->second], dataColumnWidth)
			<< MakeColumn(maxBusyTimes[it->second], dataColumnWidth)
			<< MakeColumn(averageBusyTimes[it->second], dataColumnWidth)
			<< std::endl;
		ss << std::endl;
		it++;
	}

	return ss.str();
}

//==========================================================================
// Class:			TimingUtility
// Function:		MakeColumn
//
// Description:		Formats the argument into a fixed-width right-padded string.
//
// Input Arguments:
//		value		= double
//		columnWidth	= unsigned int
//
// Output Arguments:
//		None
//
// Return Value:
//		std::string
//
//==========================================================================
std::string TimingUtility::MakeColumn(double value, unsigned int columnWidth) const
{
	std::stringstream ss;
	ss.precision(6);
	ss << std::fixed << value;
	return MakeColumn(ss.str(), columnWidth);
}

//==========================================================================
// Class:			TimingUtility
// Function:		MakeColumn
//
// Description:		Formats the argument into a fixed-width right-padded string.
//
// Input Arguments:
//		s			= std::string
//		columnWidth	= unsigned int
//		pad			= char
//
// Output Arguments:
//		None
//
// Return Value:
//		std::string
//
//==========================================================================
std::string TimingUtility::MakeColumn(std::string s, unsigned int columnWidth, char pad) const
{
	if (s.length() < columnWidth)
		s.append(std::string(columnWidth - s.length(), pad));

	return s;
}