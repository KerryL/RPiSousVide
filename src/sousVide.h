// File:  sousVide.h
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Main object for sous vide machine.

#ifndef SOUS_VIDE_H_
#define SOUS_VIDE_H_

// Standard C++ headers
#include <string>
#include <ctime>
#include <fstream>
#include <vector>
#include <time.h>

// Local forward declarations
class NetworkInterface;
class TemperatureController;
class GPIO;
class TimeHistoryLog;
struct FrontToBackMessage;
struct BackToFrontMessage;
class GNUPlotter;
class TimingUtility;
class SousVideConfig;
class CombinedLogger;

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
		StateAutoTune,

		StateCount
	};

	enum Command
	{
		CmdStart,
		CmdStop,
		CmdReset,
		CmdAutoTune,
		CmdNone
	};

	static void PrintUsageInfo(std::string name);

private:
	static const std::string configFileName;
	static const std::string autoTuneLogName;

	bool Initialize(void);

	SousVideConfig *configuration;
	bool ReadConfiguration(void);

	double plateauTemperature;// [deg F]
	double soakTime;// [sec]

	CombinedLogger *logger;
	std::ofstream logFile;

	TimingUtility *loopTimer;

	NetworkInterface *ni;
	void ProcessMessage(const FrontToBackMessage &recievedMessage);
	BackToFrontMessage AssembleMessage(void) const;
	bool sendClientMessage;

	TemperatureController *controller;
	GPIO *pumpRelay;

	TimeHistoryLog *thLog;
	std::ofstream *thLogFile;
	std::string GetLogFileName(const std::string &activity = "cooking") const;
	void SetUpTimeHistoryLog(void);
	void CleanUpTimeHistoryLog(void);

	void SetUpAutoTuneLog(void);
	bool CleanUpAutoTuneLog(std::vector<double> &time, std::vector<double> &temperature);
	double startTemperature;// [deg F]

	// Finite state machine
	State state, nextState;
	time_t stateStartTime;

	void UpdateState(void);
	void EnterState(void);
	void ProcessState(void);
	void ExitState(void);

	std::string GetStateName(void) const;

	bool InterlocksOK(void);
	bool SaturationTimeExceeded(void);
	bool TemperatureTrackingToleranceExceeded(void) const;
	bool MaximumTemperatureExceeded(void) const;
	bool TemperatureSensorFailed(void) const;
	time_t saturationStartTime;
	bool lastOutputSaturated;

	void EnterActiveState(void);
	void ExitActiveState(void);

	Command command;

	GNUPlotter *plotter;
	void ResetPlot(void);
	void UpdatePlotFile(void);
	void UpdatePlotData(double commandedTemperature, double actualTemperature);
	std::vector<double> plotTime, plotCommandedTemperature, plotActualTemperature;
	double yMin, yMax;
	static const std::string plotFileName;
	time_t plotStartTime;
};

#endif// SOUS_VIDE_H_
