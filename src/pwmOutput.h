// File:  pwmOutput.h
// Date:  8/31/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  C++ wrapper for Wiring Pi PWM methods.  Currently supports only hardware PWM.

#ifndef PWM_OUTPUT_H_
#define PWM_OUTPUT_H_

// Local headers
#include "gpio.h"

class PWMOutput : public GPIO
{
public:
	enum PWMMode
	{
		ModeBalanced,
		ModeMarkSpace
	};

	PWMOutput(int pin = 1, PWMMode mode = ModeBalanced);

	void SetDutyCycle(double duty);
	void SetMode(PWMMode mode);

	double GetDutyCycle(void) const { return duty; }

private:
	double frequency;// [Hz]
	double duty;// [%]
	const unsigned int range;
};

#endif// PWM_OUTPUT_H_
