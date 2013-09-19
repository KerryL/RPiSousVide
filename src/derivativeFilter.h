// File:  derivativeFilter.h
// Date:  9/18/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Discrete-time implementation of a 1st order derivative filter.  Provides
//        smoother output than a standard backwards difference derivative.
//        Transfer function:
//        Y(s) / U(s) = s / (tc * s + 1)
//        where tc is the time constant in seconds

#ifndef DERIVATIVE_FILTER_H_
#define DERIVATIVE_FILTER_H_

class DerivativeFilter
{
public:
	DerivativeFilter(double timeStep, double timeConstant);

	void SetTimeConstant(double timeConstant);
	void Reset(double in, double rate = 0.0);

	double Apply(double in);
	double GetRate(void) const { return rate; };

private:
	const double timeStep;// [sec]

	double oldIn;
	double rate, oldRate;// [input units/sec]

	double a, b;
};

#endif// DERIVATIVE_FILTER_H_