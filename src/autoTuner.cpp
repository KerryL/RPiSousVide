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

//==========================================================================
// Class:			AutoTuner
// Function:		Constant definitions
//
// Description:		Constant definitions for AutoTuner class.
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
double AutoTuner::switchTime(30.0);// [sec]

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
	tau = -1.0;

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
		double feedForwardScale)
{
	assert(time.size() == temperature.size());
	
	if (time.size() < 2)
		return false;

	Matrix x(3,1);
	if (!ComputeRegressionCoefficients(time, temperature, x))
	{
		outStream << "Failed to compute regression coefficients" << std::endl;
		return false;
	}

	ComputeMaxHeatRate(maxRateScale, referenceTemperature);
	ComputeRecommendedGains(desiredBandwidth, desiredDamping, feedForwardScale);

	return MembersAreValid();
}

//==========================================================================
// Class:			AutoTuner
// Function:		ComputeRegressionCoefficients
//
// Description:		Assembles matrices and performs linear regression.  The
//					regression is based on finding values for the system
//					parameters and ambient temperature that result in the
//					modeled rate of temperature change best matching the
//					measured rate of temperature change.  Due to the inclusion
//					of the lag on the heating element, there are actually two
//					coupled differential equations.  We use a hill-climbing
//					method to find the time constant for the lag, then use
//					traditional least-squares regression for the remaining
//					parameters.
//
// Input Arguments:
//		time		= const std::vector<double>& [sec]
//		temperature	= const std::vector<double>& [deg F]
//
// Output Arguments:
//		x			= Matrix&
//
// Return Value:
//		true for success, false otherwise
//
//==========================================================================
bool AutoTuner::ComputeRegressionCoefficients(const std::vector<double> &time,
	const std::vector<double> &temperature, Matrix &x)
{
	assert(time.size() == temperature.size());
	Matrix A(time.size() - 1, 3), b(time.size() - 1, 1);
	unsigned int i;
	for (i = 0; i < b.GetNumberOfRows(); i++)
	{
		A(i,0) = 1.0;
		A(i,1) = -temperature[i];
		// Assign A(i,2) later
		b(i,0) = (temperature[i + 1] - temperature[i]) / (time[i + 1] - time[i]);
	}

	if (!PerformHillClimbSearchForTau(time, A, b, tau))
	{
		outStream << "Failure while searching for tau" << std::endl;
		return false;
	}

	AssignHeatStateValue(time, A, tau);
	if (!A.LeftDivide(b, x))
		return false;

	c1 = x(1,0);
	c2 = x(2,0);
	ambientTemperature = x(0,0) / c1;
		
	return SystemParametersAreValid();
}

//==========================================================================
// Class:			AutoTuner
// Function:		PerformHillClimbSearchForTau
//
// Description:		This method performs a hill-climbing in tau search to maximize
//					the coefficient of determination.
//
// Input Arguments:
//		time	= const std::vector<double>& [sec]
//		A		= Matrix
//		b		= const Matrix&
//
// Output Arguments:
//		tau		= double [sec]
//
// Return Value:
//		true for success, false otherwise
//
//==========================================================================
bool AutoTuner::PerformHillClimbSearchForTau(const std::vector<double> &time,
	Matrix A, const Matrix &b, double &tau) const
{
	const double tolerance(0.01);// [sec], stops the loop when best tau is known within this value
	const double scaleFactor(1.01);
	double minTauGuess(0.1), maxTauGuess(1000.0), tauGuess, rSq1, rSq2;
	const unsigned int iterationLimit(1000);
	unsigned int iteration;
	Matrix x;

	for (iteration = 0; iteration < iterationLimit; iteration++)
	{
		tauGuess = minTauGuess + 0.5 * (maxTauGuess - minTauGuess);
		AssignHeatStateValue(time, A, tauGuess);
		if (!A.LeftDivide(b, x))
			return false;
		rSq1 = ComputeCoefficientOfDetermination(b, A * x);

		tauGuess *= scaleFactor;
		AssignHeatStateValue(time, A, tauGuess);
		if (!A.LeftDivide(b, x))
			return false;
		rSq2 = ComputeCoefficientOfDetermination(b, A * x);

		if (rSq2 > rSq1)// Positive slope
			minTauGuess = tauGuess / scaleFactor;
		else
			maxTauGuess = tauGuess / scaleFactor;

		if (maxTauGuess - minTauGuess < tolerance)
			break;
	}

	tau = minTauGuess + 0.5 * (maxTauGuess - minTauGuess);

	return true;
}

//==========================================================================
// Class:			AutoTuner
// Function:		ComputeCoefficientOfDetermination
//
// Description:		Given two vectors of values, returns the coefficient of
//					determination indicating the "goodness of fit" for the
//					modeled data to the measured data.
//
// Input Arguments:
//		measured	= const Matrix&
//		modeled		= const Matrix&
//
// Output Arguments:
//		None
//
// Return Value:
//		double
//
//==========================================================================
double AutoTuner::ComputeCoefficientOfDetermination(const Matrix &measured,
	const Matrix &modeled) const
{
	double modeledAverage(0.0);
	unsigned int i;
	for (i = 0; i < modeled.GetNumberOfRows(); i++)
		modeledAverage += modeled(i,0);
	modeledAverage /= modeled.GetNumberOfRows();

	double sumSqResiduals(0.0), sumSqTotal(0.0);
	for (i = 0; i < modeled.GetNumberOfRows(); i++)
	{
		sumSqResiduals += (measured(i,0) - modeled(i,0)) * (measured(i,0) - modeled(i,0));
		sumSqTotal += (measured(i,0) - modeledAverage) * (measured(i,0) - modeledAverage);
	}

	return 1.0 - sumSqResiduals / sumSqTotal;
}

//==========================================================================
// Class:			AutoTuner
// Function:		AssignHeatStateValue
//
// Description:		Assigns the values of the third column of the A matrix
//					according to the predicted value of the heater state
//					variable as a function of tau.
//
// Input Arguments:
//		time	= const std::vector<double>& [sec]
//		A		= Matrix&
//		tau		= double [sec]
//
// Output Arguments:
//		A		Matrix&
//
// Return Value:
//		None
//
//==========================================================================
void AutoTuner::AssignHeatStateValue(const std::vector<double> &time, Matrix &A,
	double tau) const
{
	double heatState(0.0);
	unsigned int i;
	for (i = 0; i < A.GetNumberOfRows(); i++)
	{
		A(i,2) = heatState;
		heatState += (time[i + 1] - time[i]) * (GetControlSignal(time[i]) - heatState) / tau;
	}
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
	// TODO:  Update for pole placement with 3rd order characteristic polynomial
	// The information below is valid for a system with tau ~= 0, but for real systems
	// the results will be inexact.  It is likely that this is good enough, so we'll
	// leave it for now.  If we need to readress it, we may also need to add a derivative
	// term to the controller to allow us to place all three poles where we want them.
	
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
	valid = SystemParametersAreValid() && valid;
	valid = ControllerParametersAreValid() && valid;

	return valid;
}

//==========================================================================
// Class:			AutoTuner
// Function:		SystemParametersAreValid
//
// Description:		Checks values of system parameters for validity (results of
//					system identification).
//
// Input Arguments:
//		checkTemperature	= bool, indicating whether or not we should check
//							  the value of ambientTemperature
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool AutoTuner::SystemParametersAreValid(bool checkTemperature) const
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
	
	if (tau <= 0.0)
	{
		outStream << "Invalid Auto-Tune Result:  Model parameter tau is negative (tau = "
			<< tau << " sec)" << std::endl;
		valid = false;
	}

	if (checkTemperature && ambientTemperature < -459.670000)
	{
		outStream << "Invalid Auto-Tune Result:  Ambient temperature is below absolue zero (ambient temperature = "
			<< ambientTemperature << " deg F)" << std::endl;
		valid = false;
	}

	return valid;
}

//==========================================================================
// Class:			AutoTuner
// Function:		ControllerParametersAreValid
//
// Description:		Checks values of controller parameters for validity.
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
bool AutoTuner::ControllerParametersAreValid(void) const
{
	bool valid(true);

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
	const std::vector<double> &control, std::vector<double> &temperature)
{
	return GetSimulatedOpenLoopResponse(time, control, temperature,
		ambientTemperature);
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
	double initialTemperature)
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
//		initialHeatOutput	= double [%]
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
	double initialTemperature, double ambientTemperature, double initialHeatOutput)
{
	assert(time.size() == control.size());
	
	// System instead of model because ambient temperature is specified explicitly
	if (!SystemParametersAreValid(false))
		return false;
		
	BuildSimulationMatrices(initialTemperature, ambientTemperature, initialHeatOutput);
	temperature.clear();

	// Create the first data point
	// We don't blindly push the initial temeprature, in case the time series we're
	// using doesn't start at zero
	ComputeNextTimeStep(control[0], time[0]);
	temperature.push_back((output * state)(0,0));

	unsigned int i;
	for (i = 1; i < time.size(); i++)
	{
		ComputeNextTimeStep(control[i], time[i] - time[i - 1]);
		temperature.push_back((output * state)(0,0));
	}

	return true;
}

//==========================================================================
// Class:			AutoTuner
// Function:		ComputeNextTimeStep
//
// Description:		Computes the next time step using the state-space model.
//
// Input Arguments:
//		control		= const double& [%]
//		deltaTime	= const double& [sec]
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void AutoTuner::ComputeNextTimeStep(const double &control, const double &deltaTime)
{
	Matrix stateDot(state);
	stateDot = system * state + input * control;
	state += stateDot * deltaTime;
}

//==========================================================================
// Class:			AutoTuner
// Function:		GetMinimumAutoTuneTime
//
// Description:		Computes the minimum time to collect data for good
//					auto-tune results.
//
// Input Arguments:
//		sampleRate	= double, frequency at which data is collected [Hz]
//
// Output Arguments:
//		None
//
// Return Value:
//		double, [sec]
//
//==========================================================================
double AutoTuner::GetMinimumAutoTuneTime(double sampleRate)
{
	return 3.0 * switchTime / sampleRate;
}

//==========================================================================
// Class:			AutoTuner
// Function:		BuildSimulationMatrices
//
// Description:		Builds the state-space system matrices as described in
//					autoTuner.h.
//
// Input Arguments:
//		initialTemperature	= double [deg F]
//		ambientTemperature	= double [deg F]
//		initialHeatLevel	= double [%]
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void AutoTuner::BuildSimulationMatrices(double initialTemperature,
	double ambientTemperature, double initialHeatLevel)
{
	system = Matrix(3, 3, -c1, c1, c2, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0 / tau);
	input = Matrix(3, 1, 0.0, 0.0, 1.0 / tau);
	output = Matrix(1, 3, 1.0, 0.0, 0.0);
	state = Matrix(3,1, initialTemperature, ambientTemperature, initialHeatLevel);
	
	/*outStream << "Built simulaiton matrices:\n";
	outStream << "A =\n" << system << "\n\n";
	outStream << "B =\n" << input << "\n\n";
	outStream << "C =\n" << output << "\n\n";
	outStream << "x =\n" << state << "\n" << std::endl;*/
}

//==========================================================================
// Class:			AutoTuner
// Function:		GetControlSignal
//
// Description:		Returns the control signal corresponding to the specified
//					time.  This method can be called from the controller while
//					generating auto-tune data, and then again when fitting the
//					data to ensure that the actual command used is the one
//					expected by the auto tuner.
//
// Input Arguments:
//		time	= double [sec]
//
// Output Arguments:
//		None
//
// Return Value:
//		double
//
//==========================================================================
double AutoTuner::GetControlSignal(double time)
{
	return (int)floor(time / switchTime) % 2 == 0 ? 1.0 : 0.5;
}

//==========================================================================
// Class:			AutoTuner
// Function:		DefineParameters
//
// Description:		Provides a means of directly setting the system parameters.
//
// Input Arguments:
//		c1	= double
//		c2	= double
//		tau	= double
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void AutoTuner::DefineParameters(double c1, double c2, double tau)
{
	this->c1 = c1;
	this->c2 = c2;
	this->tau = tau;
}
