// File:  temperatureController.h
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Temperature controller object.  Closes a loop around a temperature sensor.

// Standard C++ headers
#include <cmath>

// Local headers
#include "temperatureController.h"
#include "sousVideConfig.h"
#include "temperatureSensor.h"
#include "pwmOutput.h"

//==========================================================================
// Class:			TemperatureController
// Function:		TemperatureController
//
// Description:		Constructor for TemperatureController class.
//
// Input Arguments:
//		timeStep		= double [sec]
//		configuration	= ControllerConfiguration
//		sensor			= TemperatureSensor*
//		pwmOut			= PWMOutput*
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
TemperatureController::TemperatureController(double timeStep,
	ControllerConfiguration configuration,
	TemperatureSensor *sensor, PWMOutput *pwmOut)
	: PIController(timeStep, configuration.kp, configuration.ti), sensor(sensor), pwmOut(pwmOut)
{
	SetOutputClamp(0.0, 1.0);
	SetOutputEnable(false);
	sensorOK = false;
}

//==========================================================================
// Class:			TemperatureController
// Function:		~TemperatureController
//
// Description:		Destructor for TemperatureController class.
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
TemperatureController::~TemperatureController()
{
	delete sensor;
	delete pwmOut;
}

//==========================================================================
// Class:			TemperatureController
// Function:		Reset
//
// Description:		Resets the integral of the error signal and initializes
//					the commanded temperature to the current temperature.
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
void TemperatureController::Reset(void)
{
	PIController::Reset();
	if (sensor->GetTemperature(commandedTemperature))
		sensorOK = true;
	else
		sensorOK = false;
}

//==========================================================================
// Class:			TemperatureController
// Function:		Update
//
// Description:		Updates the temperature controller calcuations.  Must be
//					called once every timeStep seconds.
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
void TemperatureController::Update(void)
{
	if (!enabled)
		return;

	if (!sensor->GetTemperature(actualTemperature))
		sensorOK = true;
	else
	{
		sensorOK = false;
		return;// TODO:  Generate error?
	}

	commandedTemperature += rate * timeStep;
	if (commandedTemperature > plateauTemperature)
		commandedTemperature = plateauTemperature;

	pwmOut->SetDutyCycle(PIController::Update(
		commandedTemperature - actualTemperature));
}

//==========================================================================
// Class:			TemperatureController
// Function:		TemperatureSensorOK
//
// Description:		Checks the status of the temperature controller.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for OK, false otherwise
//
//==========================================================================
bool TemperatureController::TemperatureSensorOK(void) const
{
	return false;
}

//==========================================================================
// Class:			TemperatureController
// Function:		SetOutputEnable
//
// Description:		Enable/disable the PWM output.
//
// Input Arguments:
//		enabled	= bool
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void TemperatureController::SetOutputEnable(bool enabled)
{
	this->enabled = enabled;

	if (!enabled)
		pwmOut->SetDutyCycle(0.0);
}

//==========================================================================
// Class:			TemperatureController
// Function:		SetRateLimit
//
// Description:		Sets the rate at which the temperature reference increases
//					during the heating state.
//
// Input Arguments:
//		rate	= double [deg F/sec]
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void TemperatureController::SetRateLimit(double rate)
{
	this->rate = fabs(rate);
}

//==========================================================================
// Class:			TemperatureController
// Function:		SetPlateauTemperature
//
// Description:		Sets the level of the temperature plateau.
//
// Input Arguments:
//		temperature	=double [deg F]
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void TemperatureController::SetPlateauTemperature(double temperature)
{
	plateauTemperature = temperature;
}

//==========================================================================
// Class:			TemperatureController
// Function:		GetPWMDuty
//
// Description:		Returns the current duty cycle of the PWM output.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		double
//
//==========================================================================
double TemperatureController::GetPWMDuty(void) const
{
	return pwmOut->GetDutyCycle();
}

//==========================================================================
// Class:			TemperatureController
// Function:		OutputIsSaturated
//
// Description:		Returns true if the PWM output is at its maximum value of 1.0.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if output is saturated
//
//==========================================================================
bool TemperatureController::OutputIsSaturated(void) const
{
	return pwmOut->GetDutyCycle() == 1.0;
}
