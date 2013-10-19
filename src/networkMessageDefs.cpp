// File:  networkMessageDefs.cpp
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Network message definitions for communication between front end (web
//        interface) and back end (this).

// Standard C++ headers
#include <string>

// Local headers
#include "networkMessageDefs.h"

//==========================================================================
// Class:			JSONKeys
// Function:		None
//
// Description:		Static Member Definition
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
const std::string JSONKeys::CommandKey				= "Command";
const std::string JSONKeys::PlateauTemperatureKey	= "SetTemp";
const std::string JSONKeys::SoakTimeKey				= "SoakTime";
const std::string JSONKeys::StateKey				= "State";
const std::string JSONKeys::ErrorMessageKey			= "ErrMesg";
const std::string JSONKeys::CommandedTemperatureKey	= "CmdTemp";
const std::string JSONKeys::ActualTemperatureKey	= "ActTemp";
