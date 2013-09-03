// File:  temperatureController.h
// Date:  8/30/2013
// Auth:  K. Loux
// Desc:  Temperature controller object.  Closes a loop around a temperature sensor.

// Standard C++ headers
#include <cmath>

// Local headers
#include "temperatureController.h"
#include "sousVideConfig.h"
#include "temperatureSensor.h"
#include "pwmOutput.h"

TemperatureController::TemperatureController(double timeStep,
	ControllerConfiguration configuration,
	TemperatureSensor *sensor, PWMOutput *pwmOut)
	: PIController(timeStep, configuration.kp, configuration.ti), sensor(sensor), pwmOut(pwmOut)
{
	SetOutputClamp(0.0, 1.0);
	pwmOut->SetDutyCycle(0.0);
}

TemperatureController::~TemperatureController()
{
	delete sensor;
	delete pwmOut;
}

void TemperatureController::Update(void)
{
	actualTemperature = sensor->GetTemperature();
	commandedTemperature += rate * timeStep;
	if (commandedTemperature > plateauTemperature)
		commandedTemperature = plateauTemperature;

	pwmOut->SetDutyCycle(PIController::Update(
		commandedTemperature - actualTemperature));
}

void TemperatureController::SetRateLimit(double rate)
{
	this->rate = fabs(rate);
}

void TemperatureController::SetPlateauTemperature(double temperature)
{
	plateauTemperature = temperature;
}

bool TemperatureController::OutputIsSaturated(void) const
{
	return pwmOut->GetDutyCycle() == 1.0;
}
