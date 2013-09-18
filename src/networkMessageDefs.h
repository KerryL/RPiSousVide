// File:  networkMessageDefs.h
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Network message definitions for communication between front end (web
//        interface) and back end (this).

#ifndef NETWORK_MESSAGE_DEFS_H_
#define NETWORK_MESSAGE_DEFS_H_

// Local headers
#include "sousVide.h"

struct FrontToBackMessage
{
	SousVide::Command command;

	double plateauTemperature;// [deg F]
	double soakTime;// [sec]
};

struct BackToFrontMessage
{
	SousVide::State state;// see sousVide.h

	double commandedTemperature;// [deg F]
	double actualTemperature;// [deg F]
};

#endif// NETWORK_MESSAGE_DEFS_H_
