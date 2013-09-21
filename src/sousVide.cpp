// File:  sousVide.cpp
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Main object for sous vide machine.

// Standard C++ headers
#include <cstdlib>
#include <cassert>
#include <iomanip>
#include <ctime>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <cstdio>

// *nix standard headers
#include <unistd.h>

// Local headers
#include "sousVide.h"
#include "networkInterface.h"
#include "gpio.h"
#include "temperatureSensor.h"
#include "pwmOutput.h"
#include "temperatureController.h"
#include "combinedLogger.h"
#include "timeHistoryLog.h"
#include "networkMessageDefs.h"
#include "autoTuner.h"

//==========================================================================
// Class:			None
// Function:		main
//
// Description:		Application entry point.
//
// Input Arguments:
//		int (unused)
//		char *[] (unused)
//
// Output Arguments:
//		None
//
// Return Value:
//		int, 0 for success, 1 otherwise
//
//==========================================================================
int main(int argc, char *argv[])
{
	bool autoTune(false);
	if (argc == 2)
	{
		std::string argument(argv[1]);
		if (argument.compare("--autoTune") == 0)
			autoTune = true;
		else
		{
			SousVide::PrintUsageInfo(argv[0]);
			return 1;
		}
	}
	else if (argc > 2)
	{
		SousVide::PrintUsageInfo(argv[0]);
		return 1;
	}

	SousVide *sousVide = new SousVide(autoTune);
	sousVide->Run();
	delete sousVide;
	CombinedLogger::GetLogger().Destroy();

	return 0;
}

//==========================================================================
// Class:			SousVide
// Function:		Constant definitions
//
// Description:		Constant definitions for SousVide class.
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
const std::string SousVide::configFileName = "sousVide.rc";
const std::string SousVide::autoTuneLogName = "autoTune.log";

//==========================================================================
// Class:			SousVide
// Function:		SousVide
//
// Description:		Constructor for SousVide class.
//
// Input Arguments:
//		autoTune	= bool
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
SousVide::SousVide(bool autoTune) : configuration(CombinedLogger::GetLogger())
{
	state = StateOff;
	nextState = state;

	if (!ReadConfiguration())
	{
		CombinedLogger::GetLogger() << "Failed to read configuration file.  Exiting..." << std::endl;
		exit(1);
	}

	if (autoTune)
		nextState = StateAutoTune;

	sendClientMessage = false;

	ni = new NetworkInterface(configuration.network);
	controller = new TemperatureController(1.0 / configuration.system.activeFrequency,
		configuration.controller,
		new TemperatureSensor(configuration.io.sensorID, CombinedLogger::GetLogger()),
		new PWMOutput(configuration.io.heaterRelayPin));
	controller->SetRateLimit(configuration.system.maxHeatingRate);
	pumpRelay = new GPIO(configuration.io.pumpRelayPin, GPIO::DirectionOutput);

	thLog = NULL;
	thLogFile = NULL;

	maxElements = configuration.system.activeFrequency * configuration.system.statisticsTime;
	keyElement = 0;
}

//==========================================================================
// Class:			SousVide
// Function:		~SousVide
//
// Description:		Destructor for SousVide class.
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
SousVide::~SousVide()
{
	delete ni;
	delete controller;
	delete pumpRelay;

	CleanUpTimeHistoryLog();
}

//==========================================================================
// Class:			SousVide
// Function:		PrintUsageInfo
//
// Description:		Prints usage information to stdout.
//
// Input Arguments:
//		name	= std::string
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void SousVide::PrintUsageInfo(std::string name)
{
	std::cout << "Usage:  " << name << " [--autoTune]" << std::endl;
}

//==========================================================================
// Class:			SousVide
// Function:		Run
//
// Description:		Main run loop for sous vide application.
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
void SousVide::Run()
{
	clock_t start, stop;
	double elapsed(timeStep);// [sec]
	FrontToBackMessage receivedMessage;

	while (true)
	{
		start = clock();

		if (maxElements > 0)
			UpdateTimingStatistics(elapsed);

		// Do the core work for the application
		if (ni->ReceiveData(receivedMessage))
		{
			ProcessMessage(receivedMessage);
			sendClientMessage = true;
			// TODO:  Determine when to send client messages -> could be any
			// time we get a message from the client, or on a timer, etc.
		}
		UpdateState();
		if (sendClientMessage)
		{
			if (!ni->SendData(AssembleMessage()))
				CombinedLogger::GetLogger() << "Failed to send message to front end" << std::endl;
			sendClientMessage = false;
		}

		stop = clock();

		elapsed = double(stop - start) / (double)CLOCKS_PER_SEC;

		// Handle overflows
		if (stop < start || elapsed > timeStep)
			continue;

		usleep(1000000 * (timeStep - elapsed));
	}
}

//==========================================================================
// Class:			SousVide
// Function:		InterlocksOK
//
// Description:		Checks status of all interlocks.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if interlocks are OK, false if any interlocks are tripped
//
//==========================================================================
bool SousVide::InterlocksOK(void)
{
	bool interlocksOK(true);

	if (state == StateHeating || state == StateSoaking)
	{
		if (SaturationTimeExceeded())
			interlocksOK = false;

		if (TemperatureTrackingToleranceExceeded())
			interlocksOK = false;

		if (MaximumTemperatureExceeded())
			interlocksOK = false;

		if (TemperatureSensorFailed())
			interlocksOK = false;
	}
	else if (state == StateAutoTune)
	{
		if (MaximumTemperatureExceeded())
			interlocksOK = false;

		if (TemperatureSensorFailed())
			interlocksOK = false;
	}
	else if (state != StateError)
	{
		if (MaximumTemperatureExceeded())
			interlocksOK = false;
	}

	return interlocksOK;
}

//==========================================================================
// Class:			SousVide
// Function:		TemperatureTrackingToleranceExceeded
//
// Description:		Checks to see if the difference between the desired
//					and actual temperature has exceeded the tolerance.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if the interlock has been tripped, false otherwise
//
//==========================================================================
bool SousVide::TemperatureTrackingToleranceExceeded(void) const
{
	double actualTemperature(controller->GetActualTemperature());
	double cmdTemperature(controller->GetCommandedTemperature());

	if (fabs(cmdTemperature - actualTemperature) > configuration.system.interlock.temperatureTolerance)
	{
		CombinedLogger::GetLogger()
			<< "INTERLOCK:  Temperature tolerance exceeded (cmd = "
			<< cmdTemperature << " deg F, act = "
			<< actualTemperature << " deg F)" << std::endl;
		return true;
	}

	return false;
}

//==========================================================================
// Class:			SousVide
// Function:		SaturationTimeExceeded
//
// Description:		Checks to see if the PWM output has been at its
//					maximum value for more than the permissible time.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if the interlock has been tripped, false otherwise
//
//==========================================================================
bool SousVide::SaturationTimeExceeded(void)
{
	if (!controller->OutputIsSaturated())
	{
		lastOutputSaturated = false;
		return false;
	}
	else if (!lastOutputSaturated)
	{
		saturationStartTime = time(NULL);
		return false;
	}

	time_t now(time(NULL));
	if (difftime(now, saturationStartTime) > configuration.system.interlock.maxSaturationTime)
	{
		CombinedLogger::GetLogger()
			<< "PWM output max. saturation time exceeded" << std::endl;
		return true;
	}

	return false;
}

//==========================================================================
// Class:			SousVide
// Function:		MaximumTemperatureExceeded
//
// Description:		Checks to see if the temperature has exceeded the
//					maximum permissible value.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if the interlock has been tripped, false otherwise
//
//==========================================================================
bool SousVide::MaximumTemperatureExceeded(void) const
{
	double actualTemperature(controller->GetActualTemperature());

	if (actualTemperature > configuration.system.interlock.maxTemperature)
	{
		CombinedLogger::GetLogger()
			<< "INTERLOCK:  Temperature limit exceeded (act = "
			<< actualTemperature << " deg F)" << std::endl;
		return true;
	}

	return false;
}

//==========================================================================
// Class:			SousVide
// Function:		TemperatureSensorFailed
//
// Description:		Checks to see if the communication with the
//					temperature sensor has failed.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if the interlock has been tripped, false otherwise
//
//==========================================================================
bool SousVide::TemperatureSensorFailed(void) const
{
	if (!controller->TemperatureSensorOK())
	{
		CombinedLogger::GetLogger() << "INTERLOCK:  Bad result from temperature sensor" << std::endl;
		return true;
	}

	return false;
}

//==========================================================================
// Class:			SousVide
// Function:		ReadConfiguration
//
// Description:		Reads the configuration from file.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if the configuration is read and values are valid, false otherwise
//
//==========================================================================
bool SousVide::ReadConfiguration(void)
{
	return configuration.ReadConfiguration(configFileName);
}

//==========================================================================
// Class:			SousVide
// Function:		UpdateState
//
// Description:		Updates the state machine and exectues state processing methods.
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

//==========================================================================
// Class:			SousVide
// Function:		EnterState
//
// Description:		Called by UpdateState() prior to entering a new state.
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
void SousVide::EnterState(void)
{
	assert(state >= 0 && state < StateCount);
	stateStartTime = time(NULL);

	CombinedLogger::GetLogger() << "Entering State " << GetStateName() << std::endl;

	if (state == StateOff)
	{
	}
	else if (state == StateInitializing)
	{
		if (!ReadConfiguration())
		{
			CombinedLogger::GetLogger() << "Failed to re-load configuration" << std::endl;
			nextState = StateError;
		}
			
		// NOTE:  Network or GPIO-related config file changes will require restart
		timeStep = 1.0 / configuration.system.idleFrequency;
	}
	else if (state == StateReady)
	{
	}
	else if (state == StateHeating)
	{
		controller->Reset();
		controller->SetPlateauTemperature(plateauTemperature);

		lastOutputSaturated = false;

		EnterActiveState();
		SetUpTimeHistoryLog();
	}
	else if (state == StateSoaking)
		EnterActiveState();
	else if (state == StateCooling)
	{
	}
	else if (state == StateError)
	{
		// TODO:  alert user?
	}
	else if (state == StateAutoTune)
	{
		pumpRelay->SetOutput(true);
		controller->DirectlySetPWMDuty(1.0);
		SetUpAutoTuneLog();
		startTemperature = controller->GetActualTemperature();
	}
	else
		assert(false);
}

//==========================================================================
// Class:			SousVide
// Function:		ProcessState
//
// Description:		Called by UpdateState().  Performs core actions appropriate
//					for the current state.
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
void SousVide::ProcessState(void)
{
	assert(state >= 0 && state < StateCount);

	controller->Update();

	if (!InterlocksOK() && state != StateError)
	{
		nextState = StateError;
		return;
	}

	if (state == StateOff)
	{
		// Do nothing here - this state exists only to provide an entry method for StateInitializing
		// This state cannot be entered except by restarting the application
		nextState = StateInitializing;
	}
	else if (state == StateInitializing)
	{
		// Reset to update the status of the temperature sensor
		controller->Reset();
		if (controller->TemperatureSensorOK() && ni->ClientConnected())
			nextState = StateReady;
	}
	else if (state == StateReady)
	{
		if (!ni->ClientConnected())
			nextState = StateInitializing;
		else if (command == CmdStart)
			nextState = StateHeating;
		else if (command == CmdAutoTune)
			nextState = StateAutoTune;
	}
	else if (state == StateHeating)
	{
		*thLog << controller->GetCommandedTemperature()
			<< controller->GetActualTemperature()
			<< controller->GetPWMDuty() << std::endl;

		if (fabs(controller->GetActualTemperature() - plateauTemperature)
			< configuration.controller.plateauTolerance)
			nextState = StateSoaking;

		if (command == CmdStop)
			nextState = StateCooling;
	}
	else if (state == StateSoaking)
	{
		*thLog << controller->GetCommandedTemperature()
			<< controller->GetActualTemperature()
			<< controller->GetPWMDuty() << std::endl;

		time_t now = time(NULL);
		if (difftime(now, stateStartTime) > soakTime)
			nextState = StateCooling;

		if (command == CmdStop)
			nextState = StateCooling;
	}
	else if (state == StateCooling)
	{
		// Not sure this state is necessary, but could provide interesting
		// information on heat transfer of unit to environment...

		*thLog << controller->GetActualTemperature()
			<< controller->GetActualTemperature()
			<< controller->GetPWMDuty() << std::endl;

		// Just wait here until the user asks us to do something different
		if (command == CmdReset)
			nextState = StateInitializing;
	}
	else if (state == StateError)
	{
		time_t now = time(NULL);
		if (difftime(now, stateStartTime) > configuration.system.interlock.minErrorTime &&
			command == CmdReset)
			nextState = StateInitializing;
	}
	else if (state == StateAutoTune)
	{
		*thLog << controller->GetActualTemperature() << std::endl;

		time_t now = time(NULL);
		if (difftime(now, stateStartTime) > configuration.system.maxAutoTuneTime ||
			controller->GetActualTemperature() - startTemperature > configuration.system.maxAutoTuneTemperatureRise)
			nextState = StateInitializing;

		if (command == CmdStop)
			nextState = StateCooling;
	}
	else
		assert(false);

	// Reset the command - if we don't get any new messages, we won't have any command
	command = CmdNone;
}

//==========================================================================
// Class:			SousVide
// Function:		ExitState
//
// Description:		Called by UpdateState() prior to a state change.
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
void SousVide::ExitState(void)
{
	assert(state >= 0 && state < StateCount);

	CombinedLogger::GetLogger() << "Exiting State " << GetStateName() << std::endl;

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
		ExitActiveState();
	else if (state == StateSoaking)
		ExitActiveState();
	else if (state == StateCooling)
	{
	}
	else if (state == StateError)
	{
	}
	else if (state == StateAutoTune)
	{
		controller->DirectlySetPWMDuty(0.0);
		pumpRelay->SetOutput(false);

		std::vector<double> time, temp;
		if (!CleanUpAutoTuneLog(time, temp))
			return;

		AutoTuner tuner(CombinedLogger::GetLogger());

		if (tuner.ProcessAutoTuneData(time, temp))
		{
			CombinedLogger::GetLogger() << "Model parameters:" << std::endl;
			CombinedLogger::GetLogger() << "  c1 = " << tuner.GetC1() << " 1/sec" << std::endl;
			CombinedLogger::GetLogger() << "  c2 = " << tuner.GetC2() << " deg F/BTU" << std::endl;

			CombinedLogger::GetLogger() << "Recommended Gains:" << std::endl;
			CombinedLogger::GetLogger() << "  Kp = " << tuner.GetKp() << " %/deg F" << std::endl;
			CombinedLogger::GetLogger() << "  Ti = " << tuner.GetTi() << " sec" << std::endl;
			CombinedLogger::GetLogger() << "  Kf = " << tuner.GetKf() << " deg F/BTU" << std::endl;

			CombinedLogger::GetLogger() << "Other parameters:" << std::endl;
			CombinedLogger::GetLogger() << "  Max. Heat Rate = " << tuner.GetMaxHeatRate() << " deg F/sec" << std::endl;
			CombinedLogger::GetLogger() << "  Ambient Temp. = " << tuner.GetAmbientTemperature() << " deg F" << std::endl;

			// TODO:  Automatically adopt these gains? Save to config file?
			
			std::vector<double> control(time.size(), 1.0), simTemp;
			if (!tuner.GetSimulatedOpenLoopResponse(time, control, simTemp, temp[0]))
				CombinedLogger::GetLogger() << "Simulation failed" << std::endl;

			// TODO:  Clean this up - this is a little sloppy
			std::ofstream file("autoTuneSimulation.log", std::ios::out);
			if (!file.is_open() || !file.good())
			{
				CombinedLogger::GetLogger() << "Failed to write simulation data" << std::endl;
				return;
			}

			file << "Time,Actual Temperature,SimulatedTemperature" << std::endl;
			file << "[sec],[deg F],[deg F]" << std::endl;

			unsigned int i;
			for (i = 0; i < time.size(); i++)
				file << time[i] << "," << temp[i] << "," << simTemp[i] << std::endl;

			file.close();

			// TODO:  Better way to qualify results?  Use R^2?
		}
		else
			CombinedLogger::GetLogger() << "Auto-tune failed" << std::endl;
	}
	else
		assert(false);
}

//==========================================================================
// Class:			SousVide
// Function:		GetStateName
//
// Description:		Returns a string representing the current state.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		std::string
//
//==========================================================================
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
	else if (state == StateAutoTune)
		return "Auto-Tuning";
	else
		assert(false);
}

//==========================================================================
// Class:			SousVide
// Function:		EnterActiveState
//
// Description:		Performs actions necessary to enter an active (i.e. pump
//					and heater ON) state.
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
void SousVide::EnterActiveState(void)
{
	timeStep = 1.0 / configuration.system.activeFrequency;
	pumpRelay->SetOutput(true);
	controller->SetOutputEnable(true);
}

//==========================================================================
// Class:			SousVide
// Function:		ExitActiveState
//
// Description:		Performs actions necessary when leaving an active (i.e. pump
//					and heater ON) state.
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
void SousVide::ExitActiveState(void)
{
	timeStep = 1.0 / configuration.system.idleFrequency;
	pumpRelay->SetOutput(false);
	controller->SetOutputEnable(false);
}

//==========================================================================
// Class:			SousVide
// Function:		ProcessMessage
//
// Description:		Processes the received messages from the front end.
//
// Input Arguments:
//		receivedMessage	= const FrontToBackMessage&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void SousVide::ProcessMessage(const FrontToBackMessage &receivedMessage)
{
	if (receivedMessage.command == CmdStart)
	{
		if (state == StateReady)
		{
			plateauTemperature = receivedMessage.plateauTemperature;
			soakTime = receivedMessage.soakTime;

			CombinedLogger::GetLogger() << "Received START command (" << soakTime
				<< " sec at " << plateauTemperature << " deg F)" << std::endl;
		}
		else
			CombinedLogger::GetLogger()
			<< "Received START command, but system is not in Ready state (state = "
			<< GetStateName() << ")" << std::endl;
	}
	else if (receivedMessage.command == CmdStop)
	{
		if (state == StateHeating || state == StateCooling)
			CombinedLogger::GetLogger() << "Received STOP command" << std::endl;
		else
			CombinedLogger::GetLogger()
			<< "Received STOP command, but system is not in an active state (state = "
			<< GetStateName() << ")" << std::endl;
	}
	else if (receivedMessage.command == CmdReset)
	{
		if (state == StateCooling || state == StateError)
			CombinedLogger::GetLogger() << "Received RESET command" << std::endl;
		else
			CombinedLogger::GetLogger()
			<< "Received RESET command, but system is not in resettable state (state = "
			<< GetStateName() << ")" << std::endl;
	}
	else if (receivedMessage.command == CmdAutoTune)
	{
		if (state == StateReady)
			CombinedLogger::GetLogger() << "Received AUTOTUNE command" << std::endl;
		else
			CombinedLogger::GetLogger()
			<< "Received AUTOTUNE command, but system is not in ready state (state = "
			<< GetStateName() << ")" << std::endl;
	}
	else
	{
		CombinedLogger::GetLogger() << "Received unknown command from front end:  "
			<< receivedMessage.command << std::endl;
		return;
	}
}

//==========================================================================
// Class:			SousVide
// Function:		AssembleMessage
//
// Description:		Assembles a message to send to the front end.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		BackToFrontMessage
//
//==========================================================================
BackToFrontMessage SousVide::AssembleMessage(void) const
{
	BackToFrontMessage message;
	message.state = state;
	message.commandedTemperature = controller->GetCommandedTemperature();
	message.actualTemperature = controller->GetActualTemperature();

	return message;
}

//==========================================================================
// Class:			SousVide
// Function:		GetLogFileName
//
// Description:		Generates a unique name for a log file containing the
//					current date and time.
//
// Input Arguments:
//		activity	= const std::string& appended to file name
//
// Output Arguments:
//		None
//
// Return Value:
//		std::string containing the file name
//
//==========================================================================
std::string SousVide::GetLogFileName(const std::string &activity) const
{
	time_t now(time(NULL));
	struct tm* timeInfo = localtime(&now);

	std::stringstream timeStamp;
	timeStamp.fill('0');
	timeStamp << timeInfo->tm_year + 1900 << "-"
		<< std::setw(2) << timeInfo->tm_mon + 1 << "-"
		<< std::setw(2) << timeInfo->tm_mday << " "
		<< std::setw(2) << timeInfo->tm_hour << ":"
		<< std::setw(2) << timeInfo->tm_min << ":"
		<< std::setw(2) << timeInfo->tm_sec
		<< " " << activity << ".log";

	return timeStamp.str();
}

//==========================================================================
// Class:			SousVide
// Function:		SetUpTimeHistoryLog
//
// Description:		Closes any previously opened logs, and creates and configures
//					the objects required for new logs.
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
void SousVide::SetUpTimeHistoryLog(void)
{
	CleanUpTimeHistoryLog();

	thLogFile = new std::ofstream(GetLogFileName().c_str(), std::ios::out);
	thLog = new TimeHistoryLog(*thLogFile);

	thLog->AddColumn("Commanded Temperature", "deg F");
	thLog->AddColumn("Actual Temperature", "deg F");
	thLog->AddColumn("PWM Duty", "%");
}

//==========================================================================
// Class:			SousVide
// Function:		CleanUpTimeHistoryLog
//
// Description:		Closes and delete objects associated with the temperature
//					time history log.
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
void SousVide::CleanUpTimeHistoryLog(void)
{
	if (thLog)
		delete thLog;

	if (thLogFile)
	{
		thLogFile->close();
		delete thLogFile;
	}
}

//==========================================================================
// Class:			SousVide
// Function:		SetUpAutoTuneLog
//
// Description:		Configures the time history log for use as an auto-tuning
//					log.
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
void SousVide::SetUpAutoTuneLog(void)
{
	assert(!thLogFile);

	thLogFile = new std::ofstream(autoTuneLogName.c_str(), std::ios::out);
	thLog = new TimeHistoryLog(*thLogFile);

	thLog->AddColumn("Actual Temperature", "deg F");
}

//==========================================================================
// Class:			SousVide
// Function:		CleanUpAutoTuneLog
//
// Description:		Closes and delete objects associated with the temperature
//					time history log (after it was set up for autotuning).
//
// Input Arguments:
//		None
//
// Output Arguments:
//		time		= std::vector<double>& [sec]
//		temperature	= std::vector<double>& [deg F]
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool SousVide::CleanUpAutoTuneLog(std::vector<double> &time,
	std::vector<double> &temperature)
{
	assert(thLog);

	thLogFile->close();
	delete thLogFile;

	std::ifstream file(autoTuneLogName.c_str(), std::ios::in);
	if (!file.is_open() || !file.good())
	{
		CombinedLogger::GetLogger() << "Failed to open '"
			<< autoTuneLogName << "' for input" << std::endl;
		return false;
	}

	std::string line;

	// Disregard first two lines (column title and units)
	std::getline(file, line);
	std::getline(file, line);

	double value;
	std::stringstream ss;
	while (std::getline(file, line))
	{
		ss.str(line);
		ss >> value;
		time.push_back(value);
		ss.ignore();
		ss >> value;
		temperature.push_back(value);
	}

	file.close();

	std::string newName(GetLogFileName("auto-tune"));
	if (rename(autoTuneLogName.c_str(), newName.c_str()) != 0)
		CombinedLogger::GetLogger() << "Failed to move auto-tune log from '"
		<< autoTuneLogName << "' to '" << newName << "'" << std::endl;

	return true;
}

//==========================================================================
// Class:			SousVide
// Function:		UpdateTimingStatistics
//
// Description:		Updates timing statistics for the main run loop, and
//					periodically logs the results.
//
// Input Arguments:
//		elapsed	= double, elapsed busy time of last frame [sec]
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void SousVide::UpdateTimingStatistics(double elapsed)
{
	// TODO:  This can be improved by breaking active and idle times apart and
	// reporting statistics independently for each category
	clock_t now(clock());
	double totalElapsed = double(now - lastUpdate) / (double)CLOCKS_PER_SEC;
	lastUpdate = now;

	if (frameTimes.size() < maxElements)
	{
		busyTimes.push_back(elapsed);
		frameTimes.push_back(totalElapsed);
	}
	else
	{
		busyTimes[keyElement] = elapsed;
		frameTimes[keyElement] = totalElapsed;
	}
	keyElement = (keyElement + 1) % maxElements;

	if (keyElement == 0 && frameTimes.size() > 1)
	{
		double totalAverage(AverageVector(frameTimes));
		double busyAverage(AverageVector(busyTimes));

		CombinedLogger::GetLogger() << "Timing statistics for last "
			<< maxElements << " frames:" << std::endl;
		CombinedLogger::GetLogger() << "    Min total period: "
			<< *std::min_element(frameTimes.begin(), frameTimes.end()) << " sec" << std::endl;
		CombinedLogger::GetLogger() << "    Max total period: "
			<< *std::max_element(frameTimes.begin(), frameTimes.end()) << " sec" << std::endl;
		CombinedLogger::GetLogger() << "    Avg total period: "
			<< totalAverage << " sec" << std::endl;
		CombinedLogger::GetLogger() << "    Min busy period: "
			<< *std::min_element(busyTimes.begin(), busyTimes.end()) << " sec" << std::endl;
		CombinedLogger::GetLogger() << "    Max busy period: "
			<< *std::max_element(busyTimes.begin(), busyTimes.end()) << " sec" << std::endl;
		CombinedLogger::GetLogger() << "    Avg busy period: "
			<< busyAverage << " sec (" << busyAverage / totalAverage * 100.0 << "%)" << std::endl;
	}

	assert(frameTimes.size() == busyTimes.size());
}

//==========================================================================
// Class:			SousVide
// Function:		AverageVector
//
// Description:		Computes the average value of the vector values.
//
// Input Arguments:
//		values	= std::vector<double>
//
// Output Arguments:
//		None
//
// Return Value:
//		double
//
//==========================================================================
double SousVide::AverageVector(const std::vector<double> &values)
{
	unsigned int i;
	double sum(0.0);
	for (i = 0; i < values.size(); i++)
		sum += values[i];

	return sum / (double)values.size();
}
