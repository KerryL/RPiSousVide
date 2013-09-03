// File:  sousVide.h
// Date:  8/30/2013
// Auth:  K. Loux
// Desc:  Main object for sous vide machine.

#ifndef SOUS_VIDE_H_
#define SOUS_VIDE_H_

// Standard C++ headers
#include <string>
#include <time.h>

// Local headers
#include "sousVideConfig.h"

// Local forward declarations
class NetworkInterface;
class TemperatureController;

class SousVide
{
public:
	SousVide();
	~SousVide();

	void Run();

private:
	SousVideConfig configuration;
	bool ReadConfiguration(void);

	double timeStep;

	enum State
	{
		StateOff,
		StateInitializing,
		StateReady,
		StateHeating,
		StateSoaking,
		StateCooling,
		StateError,

		StateCount
	};

	NetworkInterface *ni;
	TemperatureController *controller;

	// Methods for doing the cooking
	//bool Cook(double temperature, double soakTime);
	/*void SetPumpRelay(bool on);
	void SetHeaterRelay(bool on);*/
	// Spawn a thread for bit-banging PWM?
	
	// TODO:
	// Provide means for scheduling start/stop (duration based on meat + thickness + tables?)
	// Provide means to display current temp + soak time + temp history?
	// If we control start/stop with this thing, do we have relay for pump, too?
	// Any feedback to ensure pump is running?  Assume increasing temperature feedback is good enough?

	// TODO:  Logging

	// State machine
	State state, nextState;
	time_t stateStartTime;

	void UpdateState(void);
	void EnterState(void);
	void ProcessState(void);
	void ExitState(void);

	std::string GetStateName(void);

	bool InterlocksOK(void);
};

#endif// SOUS_VIDE_H_