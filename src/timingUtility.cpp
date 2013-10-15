// File:  timingUtility.cpp
// Date:  10/14/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Timing utilities including elapsed time methods and loop timer.

// Standard C++ headers
#include <errno.h>
#include <cstring>
#include <cassert>

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
	Reset();
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
}

//==========================================================================
// Class:			TimingUtility
// Function:		TimeLoop
//
// Description:		Performs timing and sleeping to maintain specified time
//					step.  Must be called exactly once per loop and AT
//					THE TOP OF THE LOOP!.
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
	if (loopStarted)
	{
		struct timespec now;
		if (!GetCurrentTime(now))
		{
			outStream << "Failed to get current time:  " << std::strerror(errno) << std::endl;
			return false;
		}

		elapsed = TimespecToSeconds(GetDeltaTime(now, loopTime));
		if (elapsed > timeStep)
			outStream << "Warning:  Elapsed time is greater than time step ("
				<< elapsed << " > " << timeStep << ")" << std::endl;
		else
			usleep(1000000 * (timeStep - elapsed));
	}
	else
	{
		elapsed = timeStep;
		loopStarted = true;
	}

	if (!GetCurrentTime(loopTime))
	{
		outStream << "Failed to get current time:  " << std::strerror(errno) << std::endl;
		return false;
	}

	return true;
}

//==========================================================================
// Class:			TimingUtility
// Function:		Reset
//
// Description:		Resets the timer - useful for preventing large sleeps
//					and/or warning messages when the timer is intentionally
//					not updated.
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
void TimingUtility::Reset(void)
{
	loopStarted = false;
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
