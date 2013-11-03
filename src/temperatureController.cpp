// File:  temperatureController.cpp
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Temperature controller object.  Closes a loop around a temperature sensor.

// Standard C++ headers
#include <cmath>
#include <cassert>

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
	: PIDController(timeStep, configuration.kp, configuration.ti, configuration.kd,
	configuration.kf, configuration.td, configuration.tf), sensor(sensor), pwmOut(pwmOut)
{
	pwmOut->SetMode(PWMOutput::ModeMarkSpace);
	UpdateConfiguration(configuration);

	SetOutputClamp(0.0, 1.0);
	SetOutputEnable(false);
	ReadTemperature();// Sets sensorOK value
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
// Function:		UpdateConfiguration
//
// Description:		Updates any configuration options that may have changed.
//
// Input Arguments:
//		configuration	= ControllerConfiguration
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void TemperatureController::UpdateConfiguration(ControllerConfiguration configuration)
{
	pwmOK = pwmOut->SetFrequency(configuration.pwmFrequency);
	SetKp(configuration.kp);
	SetTi(configuration.ti);
	SetKd(configuration.ti);
	SetKf(configuration.kf);
	SetTd(configuration.td);
	SetTf(configuration.tf);
}

//==========================================================================
// Class:			TemperatureController
// Function:		ReadTemperature
//
// Description:		Reads the actual temperature from the sensor.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool TemperatureController::ReadTemperature(void)
{
	if (sensor->GetTemperature(actualTemperature))
	{
		sensorOK = true;

		// Convert from deg C to deg F
		actualTemperature = actualTemperature * 1.8 + 32.0;
	}
	else
		sensorOK = false;

	return sensorOK;
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
	if (ReadTemperature())
	{
		commandedTemperature = actualTemperature;
		PIDController::Reset(commandedTemperature);
	}
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
	if (!ReadTemperature())
		return;

	if (!enabled)
		return;

	commandedTemperature += rate * timeStep;
	if (commandedTemperature > plateauTemperature)
		commandedTemperature = plateauTemperature;

	pwmOut->SetDutyCycle(PIDController::Update(
		commandedTemperature, actualTemperature));
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

//==========================================================================
// Class:			TemperatureController
// Function:		DirectlySetPWMDuty
//
// Description:		Bypasses the controller and sets the PWM duty cycle.
//
// Input Arguments:
//		duty	= double
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void TemperatureController::DirectlySetPWMDuty(double duty)
{
	assert(duty >= 0.0 && duty <= 1.0);

	pwmOut->SetDutyCycle(duty);
}
