// File:  pwmOutput.cpp
// Date:  9/1/2013
// Auth:  K. Loux
// Desc:  C++ wrapper for Wiring Pi PWM methods.

// Standard C++ headers
#include <cassert>

// Wiring pi headers
#include <wiringPi.h>

// Local headers
#include "pwmOutput.h"

PWMOutput::PWMOutput(int pin, PWMMode mode) : GPIO(pin, DirectionPWMOutput)
{
	range = 1024;// TODO:  Is this right?
	SetMode(mode);
}

void PWMOutput::SetPWMFrequency(double frequency)
{
	// TODO:  Implement
	// I think it will have to use pwmSetClock(int divisor)
}

void PWMOutput::SetDutyCycle(double duty)
{
	assert(duty >= 0.0 && duty <= 1.0);

	this->duty = duty;
	pwmWrite(pin, duty * range);// TODO:  Not sure if this is right - maybe 1024?
}

void PWMOutput::SetMode(PWMMode mode)
{
	if (mode == ModeBalanced)
		pwmSetMode(PWM_MODE_BAL);
	else if (mode == ModeMarkSpace)
		pwmSetMode(PWM_MODE_MS);
	else
		assert(false);
}

void PWMOutput::SetRange(unsigned int range)
{
	// TODO:  Learn what this does, validate range value, etc.
	this->range = range;
	pwmSetRange(range);
}
