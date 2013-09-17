// File:  sousVide.cpp
// Date:  8/30/2013
// Auth:  K. Loux
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

// *nix standard headers
#include <unistd.h>

// Wiring Pi headers
#include <wiringPi.h>

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
	wiringPiSetup();

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
SousVide::SousVide(bool autoTune) : configuration(/*CombinedLogger::GetLogger()*/std::cout)
{
	state = StateOff;
	nextState = state;

	if (!ReadConfiguration())
	{
		CombinedLogger::GetLogger() << "Failed to read configuration file.  Exiting..." << std::endl;
		exit(1);
	}

	if (autoTune)
	{
		// TODO:  Allow starting in an "auto-tune" mode?
		//        Compute max heating rate and suggest reasonable controller gains?
		//        This will take some testing, but shouldn't be too difficult
		CombinedLogger::GetLogger() << "Auto-tuning requested!  Unfortunately this has not yet been implement" << std::endl;
	}

	ni = new NetworkInterface(configuration.network);
	controller = new TemperatureController(1.0 / configuration.system.activeFrequency,
		configuration.controller,
		new TemperatureSensor(),
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
			ProcessMessage(receivedMessage);
		UpdateState();
		if (true)// FIXME:  How do we decide when to update the front end?  On a timer?  When we receive a message?
		{
			if (!ni->SendData(AssembleMessage()))
				CombinedLogger::GetLogger() << "Failed to send message to front end" << std::endl;
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
	// TODO:  Implement the following interlocks:
	// Lost communication with temperature sensor?

	bool interlocksOK(true);

	// TODO:  Implement this interlock!
	//configuration.system.interlock.maxSaturationTime;

	double actualTemperature(controller->GetActualTemperature());
	double cmdTemperature(controller->GetCommandedTemperature());

	if (actualTemperature > configuration.system.interlock.maxTemperature)
	{
		CombinedLogger::GetLogger() << "Temperature limit exceeded (act = " << actualTemperature << " deg F)" << std::endl;
		interlocksOK = false;
	}

	if (fabs(cmdTemperature - actualTemperature) > configuration.system.interlock.temperatureTolerance)
	{
		CombinedLogger::GetLogger() << "Temperature tolerance exceeded (cmd = " << cmdTemperature << " deg F, act = " << actualTemperature << " deg F)" << std::endl;
		interlocksOK = false;
	}

	return interlocksOK;
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
		timeStep = 1.0 / configuration.system.idleFrequency;
	}
	else if (state == StateReady)
	{
	}
	else if (state == StateHeating)
	{
		controller->Reset();
		controller->SetPlateauTemperature(plateauTemperature);

		EnterActiveState();
//		CombinedLogger::GetLogger()->CreateNewTemperatureLog();// TODO:  this is wrong...
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
		if (controller->TemperatureSensorOK() && ni->ClientConnected())
			nextState = StateReady;
	}
	else if (state == StateReady)
	{
		if (!ni->ClientConnected())
			nextState = StateInitializing;
		else if (command == CmdStart)
			nextState = StateHeating;
	}
	else if (state == StateHeating)
	{
		*thLog << controller->GetCommandedTemperature()
			<< controller->GetActualTemperature()
			<< controller->GetPWMDuty() << std::endl;

		if (fabs(controller->GetActualTemperature() - plateauTemperature) < configuration.controller.plateauTolerance)
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
		// Not sure this state is necessary, but could provide interesting information on heat transfer of unit to environment...

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
		if (difftime(now, stateStartTime) > configuration.system.interlock.minErrorTime && command == CmdReset)
			nextState = StateInitializing;
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

			CombinedLogger::GetLogger() << "Received START command (" << soakTime << " sec at " << plateauTemperature << " deg F)" << std::endl;
		}
		else
			CombinedLogger::GetLogger() << "Received START command, but system is not in Ready state (state = " << GetStateName() << ")" << std::endl;
	}
	else if (receivedMessage.command == CmdStop)
	{
		if (state == StateHeating || state == StateCooling)
			CombinedLogger::GetLogger() << "Received STOP command" << std::endl;
		else
			CombinedLogger::GetLogger() << "Received STOP command, but system is not in an active state (state = " << GetStateName() << ")" << std::endl;
	}
	else if (receivedMessage.command == CmdReset)
	{
		if (state == StateCooling || state == StateError)
			CombinedLogger::GetLogger() << "Received RESET command" << std::endl;
		else
			CombinedLogger::GetLogger() << "Received RESET command, but system is not in resettable state (state = " << GetStateName() << ")" << std::endl;
	}
	else
	{
		CombinedLogger::GetLogger() << "Received unknown command from front end:  " << receivedMessage.command << std::endl;
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
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		std::string containing the file name
//
//==========================================================================
std::string SousVide::GetLogFileName(void) const
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
		<< " cooking.log";

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
	// TODO:  This can be improved by breaking active and idle times apart and reporting statistics independently for each category
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

		CombinedLogger::GetLogger() << "Timing statistics for last " << maxElements << " frames:" << std::endl;
		CombinedLogger::GetLogger() << "    Min total period: " << *std::min_element(frameTimes.begin(), frameTimes.end()) << " sec" << std::endl;
		CombinedLogger::GetLogger() << "    Max total period: " << *std::max_element(frameTimes.begin(), frameTimes.end()) << " sec" << std::endl;
		CombinedLogger::GetLogger() << "    Avg total period: " << totalAverage << " sec" << std::endl;
		CombinedLogger::GetLogger() << "    Min busy period: " << *std::min_element(busyTimes.begin(), busyTimes.end()) << " sec" << std::endl;
		CombinedLogger::GetLogger() << "    Max busy period: " << *std::max_element(busyTimes.begin(), busyTimes.end()) << " sec" << std::endl;
		CombinedLogger::GetLogger() << "    Avg busy period: " << busyAverage << " sec ("
			<< busyAverage / totalAverage * 100.0 << "%)" << std::endl;
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
