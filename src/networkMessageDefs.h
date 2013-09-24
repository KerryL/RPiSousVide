// File:  networkMessageDefs.h
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Network message definitions for communication between front end (web
//        interface) and back end (this).

#ifndef NETWORK_MESSAGE_DEFS_H_
#define NETWORK_MESSAGE_DEFS_H_

// Standard C++ headers
#include <string>

// Local headers
#include "sousVide.h"

struct JSONKeys
{
	static const std::string CommandKey;
	static const std::string PlateauTemperatureKey;
	static const std::string SoakTimeKey;

	static const std::string StateKey;
	static const std::string CommandedTemperatureKey;
	static const std::string ActualTemperatureKey;
};

// Structures for passing in and out of network interface
// (we're keeping the JSON stuff all within NetworkInterface)
struct FrontToBackMessage
{
	SousVide::Command command;// See sousVide.h

	double plateauTemperature;// [deg F]
	double soakTime;// [sec]
};

struct BackToFrontMessage
{
	std::string state;

	double commandedTemperature;// [deg F]
	double actualTemperature;// [deg F]
};

#endif// NETWORK_MESSAGE_DEFS_H_
