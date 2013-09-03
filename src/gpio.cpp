// File:  gpio.cpp
// Date:  8/30/2013
// Auth:  K. Loux
// Desc:  C++ wrapper for Wiring Pi general-purpose I/O function calls.

// Standard C++ headers
#include <cassert>

// Wiring pi headers
#include <wiringPi.h>

// Local headers
#include "gpio.h"

// Pin numbers refer to wiring pi pin numbers
// See:  https://projects.drogon.net/raspberry-pi/wiringpi/pins/
GPIO::GPIO(int pin, DataDirection direction) : pin(pin)
{
	assert(pin >= 0 && pin <= 20);
	SetDataDirection(direction);
}

void GPIO::SetDataDirection(DataDirection direction)
{
	assert(direction != DirectionPWMOutput || pin == 1);

	// TODO:  Is this bit necessary?
	if (direction == DirectionOutput)
		SetPullUpDown(PullOff);

	this->direction = direction;

	if (direction == DirectionInput)
		pinMode(pin, INPUT);
	else if (direction == DirectionOutput)
		pinMode(pin, OUTPUT);
	else if (direction == DirectionPWMOutput)
		pinMode(pin, PWM_OUTPUT);
	else
		assert(false);
}

void GPIO::SetPullUpDown(PullResistance state)
{
	// TODO:  Is this assertion necessary?
	assert(state == PullOff || direction == DirectionInput);

	if (state == PullOff)
		pullUpDnControl(pin, PUD_OFF);
	else if (state == PullUp)
		pullUpDnControl(pin, PUD_UP);
	else if (state == PullDown)
		pullUpDnControl(pin, PUD_DOWN);
	else
		assert(false);
}

void GPIO::SetOutput(bool high)
{
	assert(direction == DirectionOutput);

	digitalWrite(pin, high ? 1 : 0);
}

bool GPIO::GetInput(void)
{
	assert(direction == DirectionInput);
	return digitalRead(pin) == 1;
}
