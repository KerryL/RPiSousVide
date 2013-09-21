// File:  autoTuner.cpp
// Date:  9/19/2013
// Auth:  K. Loux
// Copy:  (c) Kerry Loux 2013
// Desc:  Auto-tuner for PI + FF controller for heated tank.  This performs some
//        system identification, sends results to the specified output stream,
//        and makes recommendations for Kp, Ti, and Kf.  Details on assumptions,
//        model form and gain selection criteria are in the header.

// Standard C++ headers
#include <cassert>
#include <cmath>

// Local headers
#include "autoTuner.h"
#include "matrix.h"

//==========================================================================
// Class:			AutoTuner
// Function:		AutoTuner
//
// Description:		Constructor for AutoTuner class.
//
// Input Arguments:
//		outStream	= std::ostream&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
AutoTuner::AutoTuner(std::ostream &outStream) : outStream(outStream)
{
	InitializeMembers();
}

//==========================================================================
// Class:			AutoTuner
// Function:		InitializeMembers
//
// Description:		Initializes all member variables.
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
void AutoTuner::InitializeMembers(void)
{
	// Choose invalid values so it's obvious when we have not yet run
	c1 = -1.0;
	c2 = -1.0;

	kp = -1.0;
	ti = -1.0;
	kf = -1.0;

	maxHeatRate = -1.0;
	ambientTemperature = -500.0;
}

//==========================================================================
// Class:			AutoTuner
// Function:		ProcessAutoTuneData
//
// Description:		Processes the data and generates model parameters and
//					recommended controller values.
//
// Input Arguments:
//		time					= const std::vector<double>& [sec]
//		temperature				= const std::vector<double>& [deg F]
//      desiredBandwidth		= double, target closed-loop system bandwidth [rad/sec]
//		desiredDamping			= double, target closed-loop system damping [-]
//		maxRateScale			= double, fraction of max rate to use when
//								  recommending a max heat rate - less than 1
//								  is recommended, since this calculation is
//								  valid only at the reference temperature [%]
//		referenceTemperature	= double, temperature to use when computing the max.
//								  heating rate [deg F]
//		feedForwardScale		= double, fraction of "perfect" feed forward to
//								  recommend - it is suggested that a value less
//								  than 1 be used, since we cannot force-cool
//								  the tank (i.e. we cannot "push the rope" to
//								  slow it down) [%]
//		ambTempSegments			= unsigned int, number of segments to break
//								  data into when approximating the ambient
//								  temperature
//
// Output Arguments:
//		None
//
// Return Value:
//		true for success, false otherwise
//
//==========================================================================
bool AutoTuner::ProcessAutoTuneData(const std::vector<double> &time,
	const std::vector<double> &temperature, double desiredBandwidth,
		double desiredDamping, double maxRateScale, double referenceTemperature,
		double feedForwardScale, unsigned int ambTempSegments)
{
	assert(time.size() == temperature.size());
	assert(time.size() > 2);// Really it should be much larger, but we'll crash if it's smaller than 2

	// TODO:  Necessary to filter data?

	const double ignoreInitialTime(5.0);// [sec]
	std::vector<double> croppedTime, croppedTemp, dTdt;
	unsigned int i;
	for (i = 1; i < time.size(); i++)
	{
		if (time[i] > ignoreInitialTime)
		{
			croppedTime.push_back(time[i]);
			croppedTemp.push_back(temperature[i]);
			dTdt.push_back((temperature[i] - temperature[i - 1]) / (time[i] - time[i - 1]));
		}

	}

	if (!ComputeC1(croppedTime, dTdt))
	{
		outStream << "Encoutered errors while estimating model parameter c1" << std::endl;
		return false;
	}

	if (!ComputeC2(croppedTime, dTdt))
	{
		outStream << "Encoutered errors while estimating model parameter c2" << std::endl;
		return false;
	}

	ComputeAmbientTemperature(croppedTime, croppedTemp, ambTempSegments);
	ComputeMaxHeatRate(maxRateScale, referenceTemperature);
	ComputeRecommendedGains(desiredBandwidth, desiredDamping, feedForwardScale);

	return MembersAreValid();
}

//==========================================================================
// Class:			AutoTuner
// Function:		ComputeC1
//
// Description:		Computes the model parameter c1 and stores it in the member
//					variable.
//
// Input Arguments:
//		time	= const std::vector<double>& [sec]
//		dTdt	= const std::vector<double>& [deg F/sec]
//
// Output Arguments:
//		None
//
// Return Value:
//		true for success, false otherwise
//
//==========================================================================
bool AutoTuner::ComputeC1(const std::vector<double> &time, const std::vector<double> &dTdt)
{
	assert(time.size() == dTdt.size());

	// The first step in the curve fitting process is to fit a curve of the form
	// y = m * x + b to the natural log of the rate of change of the temperature.
	// the oppostie of the slope of this curve is equal to the model parameter c1.
	// In other words, we're fitting an equation of the form y = -c1 * time + b.

	Matrix A(time.size(), 2), b(time.size(), 1);
	unsigned int i;
	for (i = 0; i < A.GetNumberOfRows(); i++)
	{
		A(i, 0) = time[i];
		A(i, 1) = 1.0;
		b(i, 0) = log(dTdt[i]);
	}

	// TODO:  Check for NaN of Inf in b vector?

	c1 = -A.LeftDivide(b)(0,0);

	return true;
}

//==========================================================================
// Class:			AutoTuner
// Function:		ComputeC2
//
// Description:		Computes the model parameter c2 and stores it in the member
//					variable.
//
// Input Arguments:
//		time	= const std::vector<double>& [sec]
//		dTdt	= const std::vector<double>& [deg F/sec]
//
// Output Arguments:
//		None
//
// Return Value:
//		true for success, false otherwise
//
//==========================================================================
bool AutoTuner::ComputeC2(const std::vector<double> &time, const std::vector<double> &dTdt)
{
	assert(time.size() == dTdt.size());

	// The second step in the curve fitting process is to fit an exponential curve
	// to the rate of change data.  The rate of change should decay exponentially,
	// assuming we have a constant heat input (the heat input is constant, but the
	// heat escaping into the room slowly increases as the delta temperature between
	// the tank and the room increases).
	// Normally, fitting an exponential would be non-linear, but we linearized and
	// solved for the coefficient in ComputeC1, so we can now find the coefficient
	// to the exponential, which is the model parameter c2.

	Matrix A(time.size(), 1), b(time.size(), 1);
	unsigned int i;
	for (i = 0; i < A.GetNumberOfRows(); i++)
	{
		A(i, 0) = exp(-c1 * time[i]);
		b(i, 0) = dTdt[i];
	}

	c2 = A.LeftDivide(b)(0, 0);

	return true;
}

//==========================================================================
// Class:			AutoTuner
// Function:		ComputeAmbientTemperature
//
// Description:		Estimates the ambient temperature and stores it in the
//					member variable.
//
// Input Arguments:
//		time		= const std::vector<double>& [sec]
//		temperature	= const std::vector<double>& [deg F]
//		segments	= unsigned int, number of segments to break data into
//					  when approximating the ambient temperature
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void AutoTuner::ComputeAmbientTemperature(const std::vector<double> &time,
		const std::vector<double> &temperature, unsigned int segments)
{
	assert(time.size() == temperature.size());
	assert(time.size() > segments);

	double total(0.0), rate;
	unsigned int i, startIndex(0), endIndex;

	for (i = 0; i < segments; i++)
	{
		endIndex = (unsigned int)floor((time.size() - 1.0) * (i + 1.0) / (double)segments);
		rate = (temperature[endIndex] - temperature[startIndex]) /
			(time[endIndex] - time[startIndex]);
		total += (rate - c2) / c1 + temperature[startIndex];
		startIndex = endIndex;
	}

	ambientTemperature = total / (double)segments;
}

//==========================================================================
// Class:			AutoTuner
// Function:		ComputeMaxHeatRate
//
// Description:		Computes the recommended maximum heat rate to use for
//					temperature ramps.
//
// Input Arguments:
//		maxRateScale			= double, fraction of max rate to use when
//								  recommending a max heat rate - less than 1
//								  is recommended, since this calculation is
//								  valid only at the reference temperature [%]
//		referenceTemperature	= double, temperature to use when computing the max.
//								  heating rate [deg F]
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void AutoTuner::ComputeMaxHeatRate(double maxRateScale, double referenceTemperature)
{
	// From the main differential equation:
	//   dT/dt = c1 * (Tamb - Ttank) + c2 * H
	// We compute the heat rate we can achieve at the reference temperature, given
	// that we only want to apply a percentage of the available heater power (to
	// leave some head room in case our model isn't good enough).  We choose a
	// reference temperature near our target operating range to ensure we don't
	// ever try to heat faster than we really can (this would spool up the
	// controller's integral term and result in temperature oscilaltions)
	maxHeatRate = c1 * (ambientTemperature - referenceTemperature) + maxRateScale * c2;
}

//==========================================================================
// Class:			AutoTuner
// Function:		ComputeRecommendedGains
//
// Description:		Computes the recommended controller gains.
//
// Input Arguments:
//      desiredBandwidth		= double, target closed-loop system bandwidth [rad/sec]
//		desiredDamping			= double, target closed-loop system damping [-]
//		feedForwardScale		= double, fraction of "perfect" feed forward to
//								  recommend - it is suggested that a value less
//								  than 1 be used, since we cannot force-cool
//								  the tank (i.e. we cannot "push the rope" to
//								  slow it down) [%]
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void AutoTuner::ComputeRecommendedGains(double desiredBandwidth, double desiredDamping,
	double feedForwardScale)
{
	// The denominator of the closed-loop system transfer function is:
	//   s^2 / c2 + (Kp + c1 / c2) * s + Kp / Ti
	// If we multiply the entire equation by c2 to make the first term s^2,
	// we have:
	//   s^2 + (Kp * c2 + c1) * s + c2 * Kp / Ti
	// This equation fits the form of s^2 + 2 * wn * zeta * s + wn^2.  We can
	// easily solve for Kp and Ti such that wn and zeta match our desired values.
	kp = 2 * desiredBandwidth * desiredDamping - c1 / c2;
	ti = c2 * kp / (desiredBandwidth * desiredBandwidth);

	// The feed-forward gain is calculated by going back to the driving
	// differential equation:
	//   dT/dt = c1 * (Tamb - Ttank) + c2 * H
	// If we assume the tank temperature is equal to the ambient temperature and
	// that we want to choose H as a function of dT/dt (i.e. H = Kf * dT/dt) such
	// that the resulting temperature rate is exactly equal to the desired rate,
	// the equation becomes:
	//   dT/dt = c2 * Kf * dT/dt
	// We then assume that we only want the feed-forward path to account for some
	// fraction of the desired rate (scale by feedForwardScale).
	// We do this apparent non-ideal scaling because if our model is wrong, and we
	// accidentially heat too fast, we do not have the ability to force the
	// temperature to drop.  This is analagous to controlling the position of
	// an object on a table by pulling on it with a rope - if we pull to far,
	// we cannot push on the rope to make it go back.  Thus, we choose a
	// feed-forward gain that is less than the 1:1 value, and let the PI
	// controller make up the difference as needed.
	kf = feedForwardScale / c2;
}

//==========================================================================
// Class:			AutoTuner
// Function:		MembersAreValid
//
// Description:		Checks values of all member variables for validity.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool AutoTuner::MembersAreValid(void) const
{
	bool valid(true);

	if (c1 <= 0.0)
	{
		outStream << "Invalid Auto-Tune Result:  Model parameter c1 is negative (c1 = "
			<< c1 << " 1/sec)" << std::endl;
		valid = false;
	}

	if (c2 <= 0.0)
	{
		outStream << "Invalid Auto-Tune Result:  Model parameter c2 is negative (c2 = "
			<< c2 << " deg F/BTU)" << std::endl;
		valid = false;
	}

	if (kp <= 0.0)
	{
		outStream << "Invalid Auto-Tune Result:  Recommended proportional gain is negative (Kp = "
			<< kp << " %/deg F)" << std::endl;
		valid = false;
	}

	if (ti <= 0.0)
	{
		outStream << "Invalid Auto-Tune Result:  Recommended integral time constant is negative (Ti = "
			<< ti << " sec)" << std::endl;
		valid = false;
	}

	if (kf <= 0.0)
	{
		outStream << "Invalid Auto-Tune Result:  Recommended feed-forward gain is negative (Kf = "
			<< kf << " %-sec/deg F)" << std::endl;
		valid = false;
	}

	if (maxHeatRate <= 0.0)
	{
		outStream << "Invalid Auto-Tune Result:  Recommended max. heat rate is negative (max. heat rate = "
			<< maxHeatRate << " deg F/sec)" << std::endl;
		valid = false;
	}

	if (ambientTemperature < -459.670000)
	{
		outStream << "Invalid Auto-Tune Result:  Ambient temperature is below absolue zero (ambient temperature = "
			<< ambientTemperature << " deg F)" << std::endl;
		valid = false;
	}

	if (kf * maxHeatRate > 1.0)
	{
		outStream << "Invalid Auto-Tune Result:  Product of Kf and max. heat rate is greater than one (Kf = "
			<< kf << " %-sec/deg G, max. heat rate = " << maxHeatRate << " deg F/sec)" << std::endl;
		valid = false;
	}

	return valid;
}

//==========================================================================
// Class:			AutoTuner
// Function:		GetSimulatedOpenLoopResponse
//
// Description:		Generates a predicted time history of the open-loop response.
//					Will use internal ambient temperature for initial temperature
//					and simulation ambient temperature.
//
//
// Input Arguments:
//		time		= const std::vector<double>& [sec]
//		control		= const std::vector<double>& [%] (must be clamped to 0..1)
//
// Output Arguments:
//		temperature	= std::vector<double>&, response the specified control vector [deg F]
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool AutoTuner::GetSimulatedOpenLoopResponse(const std::vector<double> &time,
	const std::vector<double> &control, std::vector<double> &temperature) const
{
	return GetSimulatedOpenLoopResponse(time, control, temperature,
		ambientTemperature, ambientTemperature);
}

//==========================================================================
// Class:			AutoTuner
// Function:		GetSimulatedOpenLoopResponse
//
// Description:		Generates a predicted time history of the open-loop response.
//					Will use internal ambient temperature for simulation ambient
//					temperature.
//
//
// Input Arguments:
//		time				= const std::vector<double>& [sec]
//		control				= const std::vector<double>& [%] (must be clamped to 0..1)
//		initialTemperature	= double [deg F]
//
// Output Arguments:
//		temperature	= std::vector<double>&, response the specified control vector [deg F]
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool AutoTuner::GetSimulatedOpenLoopResponse(const std::vector<double> &time,
	const std::vector<double> &control, std::vector<double> &temperature,
	double initialTemperature) const
{
	return GetSimulatedOpenLoopResponse(time, control, temperature,
		initialTemperature, ambientTemperature);
}

//==========================================================================
// Class:			AutoTuner
// Function:		GetSimulatedOpenLoopResponse
//
// Description:		Generates a predicted time history of the open-loop response.
//
//
// Input Arguments:
//		time				= const std::vector<double>& [sec]
//		control				= const std::vector<double>& [%] (must be clamped to 0..1)
//		initialTemperature	= double [deg F]
//		ambientTemperature	= double [deg F]
//
// Output Arguments:
//		temperature	= std::vector<double>&, response the specified control vector [deg F]
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool AutoTuner::GetSimulatedOpenLoopResponse(const std::vector<double> &time,
	const std::vector<double> &control, std::vector<double> &temperature,
	double initialTemperature, double ambientTemperature) const
{
	if (!MembersAreValid())
		return false;

	temperature.clear();

	// Create the first data point
	// We don't just push the initial temeprature, in case the time series we're
	// using doesn't start at zero
	temperature.push_back(GetSimulatedOpenLoopResponse(time[0], control[0],
		initialTemperature, ambientTemperature));

	unsigned int i;
	for (i = 1; i < time.size(); i++)
	{
		temperature.push_back(GetSimulatedOpenLoopResponse(
			time[i] - time[i - 1], control[i - 1], temperature[i - 1], ambientTemperature));
	}

	return true;
}

//==========================================================================
// Class:			AutoTuner
// Function:		GetSimulatedOpenLoopResponse
//
// Description:		Generates the next expected temperature point.
//
//
// Input Arguments:
//		deltaT				= double [sec]
//		control				= double [%] (must be clamped to 0..1)
//		tankTemperature		= double [deg F] at beginning of time step
//		ambientTemperature	= double [deg F]
//
// Output Arguments:
//		None
//
// Return Value:
//		double, next temperature value [deg F]
//
//==========================================================================
double AutoTuner::GetSimulatedOpenLoopResponse(double deltaT, double control,
	double tankTemperature, double ambientTemperature) const
{
	// NOTE:  The integrator we use here is just the forward Euler method -
	//        chosen because it's easy to implement, not because it's accurate!
	//        In using AB3 and RK4, though, the results are very similar (possibly
	//        due to relativly high system inertia and constant control input?).

	// TODO:  This could be cleaned up, maybe provide an integrator option?
	// Any integrator with options should probably be it's own class...
	/*double nextTemperature;
	const unsigned int integrator(0);
	if (integrator == 0)// Forward Euler
		nextTemperature =*/return tankTemperature + deltaT * PredictRateOfChange(control, tankTemperature, ambientTemperature);
	/*else if (integrator == 1)// Adams-Bashforth 3rd order
	{
		double rate = PredictRateOfChange(control, tankTemperature, ambientTemperature);
		nextTemperature = tankTemperature + deltaT * (23.0 / 12.0 * rate - 4.0 / 3.0 * lastRate + 5.0 / 12.0 * lastLastRate);
		lastLastRate = lastRate;
		lastRate = rate;
	}
	else if (integrator == 2)// Runge-Kutta 4th order
	{
		// TODO:  Really, control should be interpolated, too (not an issues as we're currently using it, since control is constant...)
		double k1, k2, k3, k4;
		k1 = PredictRateOfChange(control, tankTemperature, ambientTemperature);
		k2 = PredictRateOfChange(control, tankTemperature + deltaT * 0.5 * k1, ambientTemperature);
		k3 = PredictRateOfChange(control, tankTemperature + deltaT * 0.5 * k2, ambientTemperature);
		k4 = PredictRateOfChange(control, tankTemperature + deltaT * k3, ambientTemperature);
		nextTemperature = tankTemperature + 1.0 / 6.0 * deltaT * (k1 + 2.0 * k2 + 2.0 * k3 + k4);
	}
	else
		assert(false);

	return nextTemperature;*/
}

//==========================================================================
// Class:			AutoTuner
// Function:		PredictRateOfChange
//
// Description:		Generates the next expected temperature point.
//
//
// Input Arguments:
//		deltaT				= double [sec]
//		control				= double [%] (must be clamped to 0..1)
//		initialTemperature	= double [deg F]
//		ambientTemperature	= double [deg F]
//
// Output Arguments:
//		None
//
// Return Value:
//		double, next temperature value [deg F]
//
//==========================================================================
double AutoTuner::PredictRateOfChange(double control, double tankTemperature,
		double ambientTemperature) const
{
	assert(control >= 0.0 && control <= 1.0);
	return c1 * (ambientTemperature - tankTemperature) + c2 * control;
}
