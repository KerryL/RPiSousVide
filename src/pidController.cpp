// File:  pidController.cpp
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Basic PI controller.  Uses "ideal" form of controller:
//        Kp * (1 + 1 / (Ti * s) + Kd * s / (Td * s + 1)) * E(s) + F(s)
//        Additional feed forward term (F) is:
//        F(s) = Kf * s / (Tf * s + 1) * U(s)

// Standard C++ headers
#include <cmath>

// Local headers
#include "pidController.h"

//==========================================================================
// Class:			PIDController
// Function:		Constant definitions
//
// Description:		Constant definitions for PIDController class.
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
const double PIDController::nearlyZero = 1.0e-16;

//==========================================================================
// Class:			PIDController
// Function:		PIDController
//
// Description:		Constructor for PIDController class.
//
// Input Arguments:
//		timeStep	= double [sec]
//		kp			= double, initial value of proportional gain
//		ti			= double, initial value of integral time constant [sec]
//		kd			= double, initial value of derivative gain
//		kf			= double, initial value of feed-forward gain
//		td			= double, initial value of derivative filter time constant [sec]
//		tf			= double, initial value of feed-forward filter time constant [sec]
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
PIDController::PIDController(double timeStep, double kp, double ti,
		double kd, double kf, double td, double tf) : timeStep(timeStep),
		errorDerivative(timeStep, td), commandDerivative(timeStep, tf)
{
	SetKp(kp);
	SetTi(ti);
	SetKd(kd);
	SetKf(kf);

	Reset();
	SetOutputClamp(0.0);
}

//==========================================================================
// Class:			PIDController
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
void PIDController::SetKp(double kp)
{
	this->kp = fabs(kp);
}

//==========================================================================
// Class:			PIDController
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
void PIDController::SetTi(double ti)
{
	this->ti = fabs(ti);
}

//==========================================================================
// Class:			PIDController
// Function:		SetKd
//
// Description:		Sets the derivative gain.
//
// Input Arguments:
//		kd	= double, value of derivative gain
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PIDController::SetKd(double ti)
{
	this->kd = fabs(kd);
}

//==========================================================================
// Class:			PIDController
// Function:		SetKf
//
// Description:		Sets the feed-forward gain.
//
// Input Arguments:
//		kf	= double, value of feed-forward gain
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PIDController::SetKf(double ff)
{
	this->kf = fabs(kf);
}

//==========================================================================
// Class:			PIDController
// Function:		SetTd
//
// Description:		Sets the derivative filter time constant.
//
// Input Arguments:
//		td	= double, time constant for derivative filter [sec]
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PIDController::SetTd(double td)
{
	errorDerivative.SetTimeConstant(td);
}

//==========================================================================
// Class:			PIDController
// Function:		SetTf
//
// Description:		Sets the feed-forward filter time constant.
//
// Input Arguments:
//		tf	= double, time constant for feed-forward filter [sec]
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PIDController::SetTf(double tf)
{
	commandDerivative.SetTimeConstant(tf);
}

//==========================================================================
// Class:			PIDController
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
void PIDController::SetOutputClamp(double limit)
{
	SetOutputClamp(limit, -limit);
}

//==========================================================================
// Class:			PIDController
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
void PIDController::SetOutputClamp(double limit1, double limit2)
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
// Class:			PIDController
// Function:		Reset
//
// Description:		Resets the integral of the error signal to the specified value.
//
// Input Arguments:
//		reference	= double
//		value		= double
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PIDController::Reset(double reference, double value)
{
	error = 0.0;
	errorIntegral = value;

	errorDerivative.Reset(0.0);
	commandDerivative.Reset(reference);
}

//==========================================================================
// Class:			PIDController
// Function:		Update
//
// Description:		Updates the PI controller calculations and output.  Must
//					be called once per frame (once every timeStep seconds).
//
// Input Arguments:
//		reference	= double
//		feedback	= double
//
// Output Arguments:
//		None
//
// Return Value:
//		double, new control signal
//
//==========================================================================
double PIDController::Update(double reference, double feedback)
{
	error = reference - feedback;
	double errorRate = commandDerivative.Apply(error);
	double commandRate = commandDerivative.Apply(reference);

	double integralTerm(0.0);
	if (fabs(ti) > nearlyZero)
	{
		errorIntegral += error * timeStep;
		integralTerm = errorIntegral / ti;
	}

	double control = kp * (error + integralTerm + errorRate * kd) + commandRate * kf;

	if (fabs(highLimit - lowLimit) < nearlyZero)
	{
		if (control > highLimit)
			control = highLimit;
		else if (control < lowLimit)
			control = lowLimit;
	}

	return control;
}

//==========================================================================
// Class:			PIDController
// Function:		GetErrorRate
//
// Description:		Returns the rate of change of the error, as determined
//					by the derivative filter.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		double
//
//==========================================================================
double PIDController::GetErrorRate(void) const
{
	return errorDerivative.GetRate();
}

//==========================================================================
// Class:			PIDController
// Function:		GetCommandRate
//
// Description:		Returns the rate of change of the command, as determined
//					by the derivative filter.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		double
//
//==========================================================================
double PIDController::GetCommandRate(void) const
{
	return commandDerivative.GetRate();
}
