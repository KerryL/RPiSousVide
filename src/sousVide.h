// File:  sousVide.h
// Date:  8/30/2013
// Auth:  K. Loux
// Desc:  Main object for sous vide machine.

#ifndef SOUS_VIDE_H_
#define SOUS_VIDE_H_

// Standard C++ headers
#include <string>
#include <ctime>
#include <fstream>
#include <vector>

// Local headers
#include "sousVideConfig.h"

// Local forward declarations
class NetworkInterface;
class TemperatureController;
class GPIO;
class TimeHistoryLog;
struct FrontToBackMessage;
struct BackToFrontMessage;

class SousVide
{
public:
	SousVide(bool autoTune = false);
	~SousVide();

	void Run(void);

	enum State
	{
		StateOff,
		StateInitializing,
		StateReady,
		StateHeating,
		StateSoaking,
		StateCooling,
		StateError,
		//StateAutoTune,

		StateCount
	};

	enum Command
	{
		CmdStart,
		CmdStop,
		CmdReset,
		CmdNone
	};

	static void PrintUsageInfo(std::string name);

private:
	static const std::string configFileName;
	SousVideConfig configuration;
	bool ReadConfiguration(void);

	double timeStep;// [sec]
	double plateauTemperature;// [deg F]
	double soakTime;// [sec]

	// For maintaining timing statistics
	void UpdateTimingStatistics(double elapsed);
	static double AverageVector(const std::vector<double> &values);
	unsigned int keyElement, maxElements;
	std::vector<double> frameTimes, busyTimes;// [sec]
	clock_t lastUpdate;

	NetworkInterface *ni;
	void ProcessMessage(const FrontToBackMessage &recievedMessage);
	BackToFrontMessage AssembleMessage(void) const;

	TemperatureController *controller;
	GPIO *pumpRelay;

	TimeHistoryLog *thLog;
	std::ofstream *thLogFile;
	std::string GetLogFileName(void) const;
	void SetUpTimeHistoryLog(void);
	void CleanUpTimeHistoryLog(void);

	// Finite state machine
	State state, nextState;
	time_t stateStartTime;

	void UpdateState(void);
	void EnterState(void);
	void ProcessState(void);
	void ExitState(void);

	std::string GetStateName(void);

	bool InterlocksOK(void);
	void EnterActiveState(void);
	void ExitActiveState(void);

	Command command;
};

#endif// SOUS_VIDE_H_
