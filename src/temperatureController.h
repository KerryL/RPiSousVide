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
class DS18B20UART;
class PWMOutput;

class TemperatureController : private PIDController
{
public:
	TemperatureController(double timeStep, ControllerConfiguration configuration,
		DS18B20UART *sensor, PWMOutput *pwmOut);
	~TemperatureController();

	void Reset(void);
	void Update(void);
	void SetOutputEnable(bool enabled = true);

	void UpdateConfiguration(ControllerConfiguration configuration);

	void SetRateLimit(double rate);
	void SetPlateauTemperature(double temperature);
	void DirectlySetPWMDuty(double duty);

	double GetActualTemperature(void) const { return actualTemperature; };
	double GetCommandedTemperature(void) const { return commandedTemperature; };
	bool TemperatureSensorOK(void) const { return sensorOK; };
	bool PWMOutputOK(void) const { return pwmOK; };

	double GetPWMDuty(void) const;
	bool OutputIsSaturated(void) const;

private:
	DS18B20UART* const sensor;
	PWMOutput* const pwmOut;

	bool enabled;
	bool sensorOK, pwmOK;

	double rate;// [deg F/sec]
	double plateauTemperature;// [deg F]
	double commandedTemperature;// [deg F]
	double actualTemperature;// [deg F]

	bool ReadTemperature(void);
};

#endif// TEMPERATURE_CONTROLLER_H_
