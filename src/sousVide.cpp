// File:  sousVide.cpp
// Date:  8/30/2013
// Auth:  K. Loux
// Desc:  Main object for sous vide machine.

// Standard C++ headers
#include <cstdlib>
#include <cassert>
#include <ctime>

// *nix standard headers
#include <unistd.h>

// Wiring Pi headers
#include <wiringPi.h>

// Local headers
#include "sousVide.h"
#include "networkInterface.h"
#include "temperatureController.h"


// Entry point
int main(int argc, char *argv[])
{
	wiringPiSetup();

	SousVide *sousVide = new SousVide;
	sousVide->Run();
	delete sousVide;

	return 0;
}

SousVide::SousVide()
{
	state = StateOff;
	nextState = state;

	if (!ReadConfiguration())
	{
		// TODO:  Warn the user?
	}

	ni = new NetworkInterface(configuration.network);
	controller = new TemperatureController(1.0 / configuration.system.activeFrequency,
		configuration.control,
		new TemperatureSensor(),
		new PWMOutput());
}

SousVide::~SousVide()
{
	delete ni;
	delete controller;
}

void SousVide::Run()
{
	clock_t start, stop;
	double elapsed;
	unsigned int sleepTime;
	// TODO:  get actual max, min, average frame duration to verify timing is working properly
	while (true)
	{
		start = clock();
		// TODO:  Using the wiring pi method millis would make this easier, but it would be much better if we could be more accurate than milliseconds

		if (!InterlocksOK())
		{
			// TODO:  Alert user?  Shut down?
		}

		ni->ReceiveData();
		UpdateState();
		ni->SendData();

		stop = clock();

		// Handle overflows
		elapsed = double(stop - start) / (double)(CLOCKS_PER_SEC);
		if (stop < start || elapsed > timeStep)
			continue;

		usleep(1000000 * (timeStep - elapsed));
	}
}

bool InterlocksOK(void)
{
	// TODO:  Implement
	// Only thing coming to mind at the moment is to trip if the output is saturated over some period of time (shut down to prevent resulting overshoot)
	// Shutdown on overtemp condition (use a fixed maximum + some constant (or percentage) over setpoint)
	// Under temp condition (temperature not hitting deisred value)

	return true;
}

bool SousVide::ReadConfiguration(void)
{
	return configuration.ReadConfiguration("sousVide.rc");
}

void SousVide::UpdateState(void)
{
	if (state != nextState)
	{
		ExitState();
		state = nextState;
		EnterState();
	}

	ProcessState();
}

void SousVide::EnterState(void)
{
	assert(state >= 0 && state < StateCount);
	stateStartTime = time(NULL);

	if (state == StateOff)
	{
	}
	else if (state == StateInitializing)
	{
		timeStep = 1.0 / configuration.system.idleFrequency;
	}
	else if (state == StateReady)
	{
	}
	else if (state == StateHeating)
	{
		timeStep = 1.0 / configuration.system.activeFrequency;
		controller->Reset();
		controller->SetPlateauTemperature();
	}
	else if (state == StateSoaking)
	{
	}
	else if (state == StateCooling)
	{
		timeStep = 1.0 / configuration.system.idleFrequency;
	}
	else if (state == StateError)
	{
		timeStep = 1.0 / configuration.system.idleFrequency;
	}
	else
		assert(false);
}

void SousVide::ProcessState(void)
{
	assert(state >= 0 && state < StateCount);

	if (state == StateOff)
	{
		nextState = StateInitializing;
	}
	else if (state == StateInitializing)
	{
	}
	else if (state == StateReady)
	{
	}
	else if (state == StateHeating)
	{
		// TODO:  Write to log file
	}
	else if (state == StateSoaking)
	{
		// TODO:  Write to log file

		// If soak time >= desired soak time, set next state
		//nextState = StateCooling;
	}
	else if (state == StateCooling)
	{
		// Wait for temperature to drop below certain threshold?
		if (controller->GetActualTemperature() < someValue)
			nextState = StateInitializing;
	}
	else if (state == StateError)
	{
		// Wait here for minimum of XX seconds, and then only reset if user acknowledges
	}
	else
		assert(false);
}

void SousVide::ExitState(void)
{
	assert(state >= 0 && state < StateCount);

	if (state == StateOff)
	{
	}
	else if (state == StateInitializing)
	{
	}
	else if (state == StateReady)
	{
	}
	else if (state == StateHeating)
	{
	}
	else if (state == StateSoaking)
	{
	}
	else if (state == StateCooling)
	{
	}
	else if (state == StateError)
	{
	}
	else
		assert(false);
}

std::string SousVide::GetStateName(void)
{
	assert(state >= 0 && state < StateCount);

	if (state == StateOff)
		return "Off";
	else if (state == StateInitializing)
		return "Initializing";
	else if (state == StateReady)
		return "Ready";
	else if (state == StateHeating)
		return "Heating";
	else if (state == StateSoaking)
		return "Soaking";
	else if (state == StateCooling)
		return "Cooling";
	else if (state == StateError)
		return "Error";
	else
		assert(false);
}
