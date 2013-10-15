// File:  gpioTest.cpp
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Test application for GPIO class.

// Standard C++ headers
#include <cstdlib>
#include <cassert>
#include <ctime>
#include <iostream>
#include <cmath>

// *nix standard headers
#include <unistd.h>

// Local headers
#include "gpio.h"
#include "pwmOutput.h"
#include "timingUtility.h"

using namespace std;

// Entry point
int main(int, char *[])
{
	TimingUtility loopTimer(0.1);

	double t(0.0);
	double f(2 * M_PI * 0.5);// [Hz]

	GPIO inPin(0, GPIO::DirectionInput);
	GPIO outPin(2, GPIO::DirectionOutput);
	PWMOutput pwmPin(1);
	inPin.SetPullUpDown(GPIO::PullUp);

	pwmPin.SetMode(PWMOutput::ModeMarkSpace);
	if (!pwmPin.SetFrequency(5000.0))
	{
		cout << "Failed to set frequency" << endl;
		return 1;
	}
	
	while (true)
	{
		loopTimer.TimeLoop();

		// This test application waits for the input to go low (by
		// default, it's pulled high - wiring must account for this)
		// and then generates a sinusoudal PWM output to pin 1, and
		// makes pin 2 high.
		if (inPin.GetInput())
		{
			pwmPin.SetDutyCycle(0.5 + 0.5 * sin(t*f));
			outPin.SetOutput(true);
		}
		else
		{
			pwmPin.SetDutyCycle(0.0);
			outPin.SetOutput(false);
		}

		t += loopTimer.GetTimeStep();
	}

	return 0;
}
