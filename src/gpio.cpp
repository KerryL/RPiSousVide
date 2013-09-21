// File:  gpio.cpp
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  C++ wrapper for Wiring Pi general-purpose I/O function calls.

// Standard C++ headers
#include <cassert>

// Wiring pi headers
#include <wiringPi.h>

// Local headers
#include "gpio.h"

//==========================================================================
// Class:			GPIO
// Function:		None
//
// Description:		Static member initializaiton.
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
bool GPIO::initialized = false;

//==========================================================================
// Class:			GPIO
// Function:		GPIO
//
// Description:		Constructor for GPIO class.
//
// Input Arguments:
//		pin			= int, pin number using Wiring Pi numbering scheme.
//					  See:  http://wiringpi.com/pins/
//		direction	= DataDirection
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
GPIO::GPIO(int pin, DataDirection direction) : pin(pin)
{
	if (!initialized)
	{
		wiringPiSetup();
		initialized = true;
	}

	assert(pin >= 0 && pin <= 20);
	SetDataDirection(direction);
}

//==========================================================================
// Class:			GPIO
// Function:		~GPIO
//
// Description:		Destructor for GPIO class.
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
GPIO::~GPIO()
{
	// Turn everything off
	digitalWrite(pin, 0);
	pullUpDnControl(pin, PUD_OFF);
}

//==========================================================================
// Class:			GPIO
// Function:		SetDataDirection
//
// Description:		Sets the data direction for the pin.
//
// Input Arguments:
//		direction	= DataDirection
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
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

//==========================================================================
// Class:			GPIO
// Function:		SetPullUpDown
//
// Description:		Sets the state of the optional internal pull-up/pull-down resistors.
//
// Input Arguments:
//		state	= PullResistance
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
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

//==========================================================================
// Class:			GPIO
// Function:		SetOutput
//
// Description:		Sets the state of the output pin.
//
// Input Arguments:
//		high	= bool, true for high, false for low
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void GPIO::SetOutput(bool high)
{
	assert(direction == DirectionOutput);

	digitalWrite(pin, high ? 1 : 0);
}

//==========================================================================
// Class:			GPIO
// Function:		GetInput
//
// Description:		Reads the status of the input pin.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for high, false otherwise
//
//==========================================================================
bool GPIO::GetInput(void)
{
	assert(direction == DirectionInput);
	return digitalRead(pin) == 1;
}
