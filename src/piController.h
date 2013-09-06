// File:  piController.cpp
// Date:  8/30/2013
// Auth:  K. Loux
// Desc:  Basic PI controller.  Uses "ideal" form of controller: Kp * (1 + 1 / (Ti * s)).

#ifndef PI_CONTROLLER_H_
#define PI_CONTROLLER_H_

class PIController
{
public:
	PIController(double timeStep, double kp = 1.0, double ti = 0.0);

	void SetKp(double kp);
	void SetTi(double ti);
	void SetOutputClamp(double limit);
	void SetOutputClamp(double limit1, double limit2);

	void Reset(void);
	void Reset(double value);
	double Update(double error);

	// TODO:  Options for: filtering?  Equation form?

private:
	const double timeStep;// [sec]
	double kp, ti;
	double errorIntegral;
	double highLimit, lowLimit;
};

#endif// PI_CONTROLLER_H_