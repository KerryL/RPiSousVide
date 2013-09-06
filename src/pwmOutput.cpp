// File:  pwmOutput.cpp
// Date:  9/1/2013
// Auth:  K. Loux
// Desc:  C++ wrapper for Wiring Pi PWM methods.  Currently supports only hardware PWM.

// Standard C++ headers
#include <cassert>

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
PWMOutput::PWMOutput(int pin, PWMMode mode) : GPIO(pin, DirectionPWMOutput), range(1024)
{
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
}