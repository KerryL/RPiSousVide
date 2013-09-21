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

// Entry point
int main(int argc, char *argv[])
{
	clock_t start, stop;
	double elapsed;
	double timeStep(0.01);// [sec]

	double t(0.0);
	double f(2 * M_PI * 0.5);// [Hz]

	GPIO inPin(0, GPIO::DirectionInput);
	GPIO outPin(2, GPIO::DirectionOutput);
	PWMOutput pwmPin(1);
	inPin.SetPullUpDown(GPIO::PullUp);
	
	while (true)
	{
		start = clock();

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

		t += timeStep;

		stop = clock();

		// Handle overflows
		elapsed = double(stop - start) / (double)(CLOCKS_PER_SEC);
		if (stop < start || elapsed > timeStep)
			continue;

		usleep(1000000 * (timeStep - elapsed));
	}

	return 0;
}
