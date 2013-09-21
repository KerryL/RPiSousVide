// File:  temperatureController.h
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Temperature controller object.  Closes a loop around a temperature sensor.

#ifndef TEMPERATURE_CONTROLLER_H_
#define TEMPERATURE_CONTROLLER_H_

// Local headers
#include "pidController.h"

// Local forward declarations
class ControllerConfiguration;
class TemperatureSensor;
class PWMOutput;

class TemperatureController : private PIDController
{
public:
	TemperatureController(double timeStep, ControllerConfiguration configuration,
		TemperatureSensor *sensor, PWMOutput *pwmOut);
	~TemperatureController();

	void Reset(void);
	void Update(void);
	void SetOutputEnable(bool enabled = true);

	void SetRateLimit(double rate);
	void SetPlateauTemperature(double temperature);
	void DirectlySetPWMDuty(double duty);

	double GetActualTemperature(void) const { return actualTemperature; };
	double GetCommandedTemperature(void) const { return commandedTemperature; };
	bool TemperatureSensorOK(void) const { return sensorOK; };

	double GetPWMDuty(void) const;
	bool OutputIsSaturated(void) const;

private:
	TemperatureSensor* const sensor;
	PWMOutput* const pwmOut;

	bool enabled;
	bool sensorOK;

	double rate;// [deg F/sec]
	double plateauTemperature;// [deg F]
	double commandedTemperature;// [deg F]
	double actualTemperature;// [deg F]
};

#endif// TEMPERATURE_CONTROLLER_H_
