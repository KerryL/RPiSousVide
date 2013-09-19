// File:  derivativeFilter.h
// Date:  9/18/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Discrete-time implementation of a 1st order derivative filter.  Provides
//        smoother output than a standard backwards difference derivative.
//        Transfer function:
//        Y(s) / U(s) = s / (tc * s + 1)
//        where tc is the time constant in seconds

// Standard C++ headers
#include <cmath>

// Local headers
#include "derivativeFilter.h"

//==========================================================================
// Class:			DerivativeFilter
// Function:		DerivativeFilter
//
// Description:		Constructor for DerivativeFilter class.
//
// Input Arguments:
//		timeStep		= double [sec]
//		timeConstant	= double, initial value of filter time constant [sec]
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
DerivativeFilter::DerivativeFilter(double timeStep, double timeConstant) : timeStep(timeStep)
{
	SetTimeConstant(timeConstant);
	Reset(0.0);
}

//==========================================================================
// Class:			DerivativeFilter
// Function:		SetTimeConstant
//
// Description:		Sets the time constant to the specified value.
//
// Input Arguments:
//		timeConstant	= double, value of filter time constant [sec]
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void DerivativeFilter::SetTimeConstant(double timeConstant)
{
	a = timeStep * 0.5 - timeConstant;
	b = timeConstant + timeStep * 0.5;
}

//==========================================================================
// Class:			DerivativeFilter
// Function:		Reset
//
// Description:		Resets the filter to the specified rate.
//
// Input Arguments:
//		rate	= double, rate value to initialize to [input units/sec]
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void DerivativeFilter::Reset(double in, double rate)
{
	oldIn = in;
	oldRate = rate;
}

//==========================================================================
// Class:			DerivativeFilter
// Function:		Apply
//
// Description:		Updates the filter output.  Must be called at a rate of
//					once per timeStep.
//
// Input Arguments:
//		in	= double
//
// Output Arguments:
//		None
//
// Return Value:
//		double, filtered value
//
//==========================================================================
double DerivativeFilter::Apply(double in)
{
	rate = (in - oldIn - oldRate * a) / b;

	oldRate = rate;
	oldIn = in;

	return rate;
}