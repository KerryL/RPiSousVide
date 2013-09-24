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
// Function:		Constant definitions
//
// Description:		Constant definitions for PWMOutput class.
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
const double PWMOutput::pwmClockFrequency = 19.2e6;// [Hz]
const unsigned int PWMOutput::minClockDivisor = 2;
const unsigned int PWMOutput::maxClockDivisor = 4095;
const unsigned int PWMOutput::maxRange = 4096;

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
	assert(range <= maxRange);

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
//					This method seems to work pretty well, but I have no proof
//					suggesting that it will always generate the best pair or
//					range-clock divisor values.  That criteria is somewhat
//					dependent on your valuationg of resolution vs. frequency
//					accuracy anyway.
//
// Input Arguments:
//		frequency		= double [Hz]
//		minResolution	= unsigned int
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if successfully set, false otherwise (frequency out of
//		bounds or wrong PWM mode)
//
//==========================================================================
bool PWMOutput::SetFrequency(double frequency, unsigned int minResolution)
{
	unsigned int newRange, divisor;

	if (mode == ModeMarkSpace)
	{
		const unsigned int rangeDivisorProduct = floor(pwmClockFrequency / frequency + 0.5);
		divisor = 1;

		// Make sure the frequency is within the range we can attempt
		if (rangeDivisorProduct / minResolution < minClockDivisor ||// Frequency too high
			rangeDivisorProduct / maxClockDivisor > maxRange)// Frequency too low
			return false;

		unsigned int i(1);
		while (divisor < 2 || newRange > maxRange ||
			divisor > maxClockDivisor || newRange < minResolution)
		{
			// If we've tried all combinations, we're done
			if (i >= 2 * rangeDivisorProduct)
				return false;

			if (i % 2 == 0)
				divisor = GetMinimumAcceptableFactor(rangeDivisorProduct + floor(i / 2.0));
			else
				divisor = GetMinimumAcceptableFactor(rangeDivisorProduct - floor(i / 2.0));
			i++;

			newRange = floor(rangeDivisorProduct / divisor + 0.5);
		}
	}
	else
	{
		// See page 139 of the Broadcom ARM documentation.  The default PWM mode
		// (balanced) DOES in fact change the apparent frequency as a function of
		// duty cycle.  Their goal is to achieve as even a distribution of on/off
		// pulses as possible within any arbitrary block of time (i.e. when the
		// arbitrary time slice is not an integer multiple of the PWM period).
		// For this reason, we won't attempt to implement SetFrequency for balanced
		// PWM mode (although it does have an affect on the PWM output).
		// http://www.element14.com/community/servlet/JiveServlet/downloadBody/43016-102-1-231518/Broadcom.Datasheet.pdf
		//assert(false);
		return false;
	}

	assert(divisor >= minClockDivisor &&
		divisor <= maxClockDivisor &&
		newRange >= minResolution);

	pwmSetClock(divisor);
	SetRange(newRange);

	return true;
}

//==========================================================================
// Class:			PWMOutput
// Function:		GetMinimumAcceptableFactor
//
// Description:		Returns the smallest prime factor of the argument.
//
// Input Arguments:
//		i	= unsigned int
//
// Output Arguments:
//		None
//
// Return Value:
//		unsigned int
//
//==========================================================================
unsigned int PWMOutput::GetMinimumAcceptableFactor(unsigned int i) const
{
	// We're going to use a naieve approach with special stop condition
	// TODO:  Improve the speed here (may or may not be acceptable for large values of i)

	unsigned int f(1);
	while (f++, (i % f != 0 || i / f > maxRange) && f <= maxClockDivisor) {}

	return f;
}
