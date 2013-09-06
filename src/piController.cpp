// File:  piController.cpp
// Date:  8/30/2013
// Auth:  K. Loux
// Desc:  Basic PI controller.  Uses "ideal" form of controller: Kp * (1 + 1 / (Ti * s)).

// Standard C++ headers
#include <cmath>

// Local headers
#include "piController.h"

//==========================================================================
// Class:			PIController
// Function:		PIController
//
// Description:		Constructor for PIController class.
//
// Input Arguments:
//		timeStep	= double [sec]
//		kp			= double, initial value of proportional gain
//		ti			= double, initial value of integral time constant [sec]
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
PIController::PIController(double timeStep, double kp, double ti) : timeStep(timeStep)
{
	SetKp(kp);
	SetTi(ti);

	Reset();
	SetOutputClamp(0.0);
}

//==========================================================================
// Class:			PIController
// Function:		SetKp
//
// Description:		Sets the proportional gain.
//
// Input Arguments:
//		kp	= double, value of proportional gain
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PIController::SetKp(double kp)
{
	this->kp = fabs(kp);
}

//==========================================================================
// Class:			PIController
// Function:		SetTi
//
// Description:		Sets the integral time constant.
//
// Input Arguments:
//		ti	= double, value of integral time constant [sec]
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PIController::SetTi(double ti)
{
	this->ti = fabs(ti);
}

//==========================================================================
// Class:			PIController
// Function:		SetOutputClamp
//
// Description:		Sets the upper and lower limits on the PI controller output
//					to by symmetric about zero by the specified amount.
//
// Input Arguments:
//		limit	= double
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PIController::SetOutputClamp(double limit)
{
	SetOutputClamp(limit, -limit);
}

//==========================================================================
// Class:			PIController
// Function:		SetOutputClamp
//
// Description:		Sets the upper and lower limits on the PI controller output.
//					When limit1 == limit2, the clamp function is disabled.
//
// Input Arguments:
//		limit1	= double
//		limit2	= double
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PIController::SetOutputClamp(double limit1, double limit2)
{
	if (limit1 > limit2)
	{
		highLimit = limit1;
		lowLimit = limit2;
	}
	else
	{
		highLimit = limit2;
		lowLimit = limit1;
	}
}

//==========================================================================
// Class:			PIController
// Function:		Reset
//
// Description:		Resets the integral of the error signal to zero.
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
void PIController::Reset(void)
{
	Reset(0.0);
}

//==========================================================================
// Class:			PIController
// Function:		Reset
//
// Description:		Resets the integral of the error signal to the specified value.
//
// Input Arguments:
//		value	= double
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PIController::Reset(double value)
{
	errorIntegral = value;
}

//==========================================================================
// Class:			PIController
// Function:		Update
//
// Description:		Updates the PI controller calculations and output.  Must
//					be called once per frame (once every timeStep seconds).
//
// Input Arguments:
//		error	= double, new error value
//
// Output Arguments:
//		None
//
// Return Value:
//		double, new control signal
//
//==========================================================================
double PIController::Update(double error)
{
	double control;
	if (ti == 0.0)// TODO:  nearly zero?
		control = kp * error;
	else
	{
		errorIntegral += error * timeStep;
		control = kp * (error + errorIntegral / ti);
	}

	if (highLimit != lowLimit)// TODO:  nearly zero?
	{
		if (control > highLimit)
			control = highLimit;
		else if (control < lowLimit)
			control = lowLimit;
	}

	return control;
}