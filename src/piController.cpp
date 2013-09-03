// File:  piController.cpp
// Date:  8/30/2013
// Auth:  K. Loux
// Desc:  Basic PI controller.  Uses "ideal" form of controller: Kp * (1 + 1 / (Ti * s)).

// Standard C++ headers
#include <cmath>

// Local headers
#include "piController.h"

PIController::PIController(double timeStep, double kp, double ki) : timeStep(timeStep)
{
	SetKp(kp);
	SetTi(ti);

	Reset();
	SetOutputClamp(0.0);
}

void PIController::SetKp(double kp)
{
	this->kp = fabs(kp);
}

void PIController::SetTi(double ti)
{
	this->ti = fabs(ti);
}

void PIController::SetOutputClamp(double limit)
{
	SetOutputClamp(limit, -limit);
}

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

void PIController::Reset(void)
{
	Reset(0.0);
}

void PIController::Reset(double value)
{
	errorIntegral = value;
}

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