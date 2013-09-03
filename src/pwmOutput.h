// File:  pwmOutput.h
// Date:  8/31/2013
// Auth:  K. Loux
// Desc:  C++ wrapper for Wiring Pi PWM methods.

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

	void SetPWMFrequency(double frequency);
	void SetDutyCycle(double duty);
	void SetMode(PWMMode mode);
	void SetRange(unsigned int range);

	double GetDutyCycle(void) const { return duty; }

private:
	double frequency;// [Hz]
	double duty;// [%]
	unsigned int range;
};

#endif// PWM_OUTPUT_H_
