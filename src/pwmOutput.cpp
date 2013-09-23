// File:  pwmOutput.cpp
// Date:  9/1/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  C++ wrapper for Wiring Pi PWM methods.  Currently supports only hardware PWM.

// Standard C++ headers
#include <cassert>
#include <cmath>

// Wiring pi headers
#include <wiringPi.h>

// Local headers
#include "pwmOutput.h"

//==========================================================================
// Class:			PWMOutput
// Function:		PWMOutput
//
// Description:		Constructor for PWMOutput class.
//
// Input Arguments:
//		pin		= int, represents hardware pin number according to Wiring Pi
//		mode	= PWMMode, indicating the style of PWM phasing to use
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
PWMOutput::PWMOutput(int pin, PWMMode mode) : GPIO(pin, DirectionPWMOutput), frequency(285700.0), range(1024)// TODO:  Get better frequency estimate
{
	SetDutyCycle(0.0);
	SetMode(mode);
}

//==========================================================================
// Class:			PWMOutput
// Function:		SetDutyCycle
//
// Description:		Sets the duty cycle of the PWM output.
//
// Input Arguments:
//		duty	= double, must range from 0.0 to 1.0
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PWMOutput::SetDutyCycle(double duty)
{
	assert(duty >= 0.0 && duty <= 1.0);

	this->duty = duty;
	pwmWrite(pin, duty * range);
}

//==========================================================================
// Class:			PWMOutput
// Function:		SetMode
//
// Description:		Sets the PWM mode.
//
// Input Arguments:
//		mode	= PWMMode, use ModeBalanced for phase-correct PWM (better for driving motors),
//				  and ModeMarkSpace for classical on-then-off style PWM.
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PWMOutput::SetMode(PWMMode mode)
{
	if (mode == ModeBalanced)
		pwmSetMode(PWM_MODE_BAL);
	else if (mode == ModeMarkSpace)
		pwmSetMode(PWM_MODE_MS);
	else
		assert(false);

	this->mode = mode;
}

//==========================================================================
// Class:			PWMOutput
// Function:		SetRange
//
// Description:		Sets the PWM range (resolution).  Default is 1024.
//
// Input Arguments:
//		range	= unsigned int
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PWMOutput::SetRange(unsigned int range)
{
	pwmSetRange(range);
	this->range = range;
	SetDutyCycle(duty);
}

//==========================================================================
// Class:			PWMOutput
// Function:		SetFrequency
//
// Description:		Sets the PWM frequency.  Note that the frequency
//					may be rounded and thus not exactly as specified.
//					The achievable frequencies are dependent on the
//					range - higher ranges allow lower frequencies.
//					If the frequency cannot be achieved with the current
//					range, this function returns false.
//
// Input Arguments:
//		frequency		= unsigned int [Hz]
//		minResolution	= unsigned int
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if successfully set, false otherwise
//
//==========================================================================
bool PWMOutput::SetFrequency(unsigned int frequency, unsigned int minResolution)
{
	int clock;
	double period(1000.0 / frequency);// [msec]

	if (mode == ModeMarkSpace)
	{
		// This equation is based on empirical data taken with an old,
		// uncalibrated oscilloscope.  It's probably close, but I'd feel
		// better if we got some numbers from a datasheet.
		clock = (int)floor((period / range + 2.47264e-5) / 5.2946815e-5);

		// TODO:  Adjust the range to achieve as close a match as possible
		// Really, it's not "as close a match as possible" but some
		// function we want to minimize taking into account resolution
		// (more is better) and error (less is better)
	}
	else
		assert(false);// TODO:  Implement

	if (clock < 2 || clock > 4095)
		return false;

	pwmSetClock(clock);
	return true;
}
