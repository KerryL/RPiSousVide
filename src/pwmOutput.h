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
	void SetRange(unsigned int range);
	bool SetFrequency(double frequency, unsigned int minResolution = 100);

	double GetDutyCycle(void) const { return duty; };
	double GetFrequency(void) const { return frequency; };

private:
	static const double pwmClockFrequency;// [Hz]
	static const unsigned int minClockDivisor, maxClockDivisor, maxRange;
	double frequency;// [Hz]
	double duty;// [%]
	unsigned int range;
	PWMMode mode;

	unsigned int GetMinimumAcceptableFactor(unsigned int i) const;
};

#endif// PWM_OUTPUT_H_
