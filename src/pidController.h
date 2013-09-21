// File:  pidController.cpp
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Basic PI controller.  Uses "ideal" form of controller:
//        Kp * (1 + 1 / (Ti * s) + Kd * s / (Td * s + 1)) * E(s) + F(s)
//        Additional feed forward term (F) is:
//        F(s) = Kf * s / (Tf * s + 1) * U(s)

#ifndef PID_CONTROLLER_H_
#define PID_CONTROLLER_H_

// Local headers
#include "derivativeFilter.h"

class PIDController
{
public:
	PIDController(double timeStep, double kp = 1.0, double ti = 0.0,
		double kd = 0.0, double kf = 0.0, double td = 1.0, double tf = 1.0);

	void SetKp(double kp);
	void SetTi(double ti);
	void SetKd(double ti);
	void SetKf(double kf);
	void SetTd(double td);
	void SetTf(double tf);

	void SetOutputClamp(double limit);
	void SetOutputClamp(double limit1, double limit2);

	void Reset(double reference = 0.0, double value = 0.0);
	double Update(double reference, double feedback);

	double GetError(void) const { return error; };
	double GetErrorRate(void) const;
	double GetCommandRate(void) const;

protected:
	static const double nearlyZero;

	const double timeStep;// [sec]

	double kp, ti, kd, kf;
	double error, errorIntegral;
	double highLimit, lowLimit;

	DerivativeFilter errorDerivative;
	DerivativeFilter commandDerivative;
};

#endif// PID_CONTROLLER_H_
