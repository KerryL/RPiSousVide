// File:  temperatureController.h
// Date:  8/30/2013
// Auth:  K. Loux
// Desc:  Temperature controller object.  Closes a loop around a temperature sensor.

#ifndef TEMPERATURE_CONTROLLER_H_
#define TEMPERATURE_CONTROLLER_H_

// Local headers
#include "piController.h"

// Local forward declarations
class ControllerConfiguration;
class TemperatureSensor;
class PWMOutput;

class TemperatureController : private PIController
{
public:
	TemperatureController(double timeStep, ControllerConfiguration configuration,
		TemperatureSensor *sensor, PWMOutput *pwmOut);
	~TemperatureController();

	void Reset(void) { PIController::Reset(); };
	void Update(void);

	void SetRateLimit(double rate);
	void SetPlateauTemperature(double temperature);

	double GetActualTemperature(void) const { return actualTemperature; };
	double GetCommandedTemperature(void) const { return commandedTemperature; };

	bool OutputIsSaturated(void) const;

private:
	TemperatureSensor* const sensor;
	PWMOutput* const pwmOut;

	double rate;// [deg F/sec]
	double plateauTemperature;// [deg F]
	double commandedTemperature;// [deg F]
	double actualTemperature;// [deg F]
};

#endif// TEMPERATURE_CONTROLLER_H_
