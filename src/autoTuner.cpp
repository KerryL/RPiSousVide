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
	
	if (time.size() < 2)
		return false;

	Matrix x(3,1);
	if (!ComputeRegressionCoefficients(time, temperature, x))
	{
		outStream << "Failed to compute regression coefficients" << std::endl;
		return false;
	}
	/*x(0,0)=800.0/6432201.0;
	x(1,0)=6367801.0/6432201.0;
	x(2,0)=-1422222.0/714689.0;*/
	std::cout << "x = \n" << x << std::endl;

	if (!ComputeParametersFromCoefficients(x, ComputeMeanSampleTime(time)))
	{
		outStream << "Failed to compute system parameters" << std::endl;
		return false;
	}
	
	// TODO:  Fix this stuff
	double a(desiredDamping * desiredBandwidth * feedForwardScale * maxRateScale * referenceTemperature * ambTempSegments);
	a = a * a;

	/*ComputeAmbientTemperature(croppedTime, croppedTemp, ambTempSegments);
	ComputeMaxHeatRate(maxRateScale, referenceTemperature);
	ComputeRecommendedGains(desiredBandwidth, desiredDamping, feedForwardScale);*/

	return MembersAreValid();
}

//==========================================================================
// Class:			AutoTuner
// Function:		ComputeRegressionCoefficients
//
// Description:		Assembles matrices and performs linear regression.  The
//					regression is based on finding the coefficients for the
//					difference equation for the open loop system.  We also
//					assume that the input to the system was 100% during the
//					time that the auto-tune data was collected.  This allows
//					us to simplify the numerator of the discrete time transfer
//					function, leaving us with only three coefficients.
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
	
	// Ignore first two rows of data, so we subtract 2 from the size
	Matrix A(time.size() - 2, 3), b(time.size() - 2, 1);
	unsigned int i;
	for (i = 0; i < A.GetNumberOfRows(); i++)
	{
		A(i, 0) = 1.0;
		A(i, 1) = -temperature[i];// 2 time steps ago
		A(i, 2) = -temperature[i + 1];// 1 time step ago
		b(i, 0) = temperature[i + 2];// current time step
	}
	
	std::cout << A.GetSubMatrix(0,0,5,3) << std::endl;
	
	if (!A.LeftDivide(b, x))
		return false;
		
	return true;
}

//==========================================================================
// Class:			AutoTuner
// Function:		ComputeParametersFromCoefficients
//
// Description:		The system parameters (c1, c2 and tau) are determined by
//					setting the coefficients of the difference equation equal
//					to the hand-solved coefficients from the model, and
//					solving for the system parameters.  The discrete time
//					transfer function is:
//
//					Y(z)   c2 * ( 1 + 2 * z^-1 + z^-2) * T
//					---- = -------------------------------
//					U(z)       O * z^-2 + P * z^-1 + Q
//
//					where
//
//					O = 4 * tau - 2 * T * (c1 * tau + 1) + c1 * T^2
//					P = 2 * c1 * T^2 - 8 * tau
//					Q = c1 * T^2 + 2 * T * (c1 * tau + 1) + 4 * tau
//					and T is the sampe time for the data
//
//					Making the assumtion that the input to the system is 1 for
//					all time (during the data collection for the auto-tune
//					process), we can reduce the numerator to:
//					N = 4 * c2 * T^2
//
//					From this and the the z-domain transfer function, we know:
//					Y(k) = (N - O * Y(k - 2) - P * Y(k - 1)) / Q
//
//					So we have:
//					x(0) = N / Q
//					x(1) = O / Q
//					x(2) = P / Q
//
// Input Arguments:
//		x			= const Matrix&
//		sampleTime	= const double& [sec]
//
// Output Arguments:
//		None
//
// Return Value:
//		true for success, false otherwise
//
//==========================================================================
bool AutoTuner::ComputeParametersFromCoefficients(const Matrix &x,
	const double &sampleTime)
{
	c1 = (x(1,0) - 1 + sqrt(x(2,0) * x(2,0) - 4 * x(1,0)))
		/ (sampleTime / 2 * (x(2,0) - x(1,0) - 1));
    tau = (sampleTime * sampleTime * c1 * (2 - x(2,0)) - 2 * sampleTime * x(2,0))
		/ (2 * x(2,0) * (c1 * sampleTime + 2) + 8);
    c2 = x(0,0) / 4 / sampleTime / sampleTime
		* (c1 * sampleTime * sampleTime
		+ 2 * sampleTime * (c1 * tau + 1) + 4 * tau);
		
	return true;
}

//==========================================================================
// Class:			AutoTuner
// Function:		ComputeMeanSampleTime
//
// Description:		Computes the mean sample time and prints some statistical
//					information about the time vector.
//
// Input Arguments:
//		time	= const std::vector<double>&
//
// Output Arguments:
//		None
//
// Return Value:
//		double, average sample time
//
//==========================================================================
double AutoTuner::ComputeMeanSampleTime(const std::vector<double> &time) const
{
	double mean((time[time.size() - 1] - time[0]) / (time.size() - 1.0));
	double d(0.0), minDT(time[time.size() - 1]), maxDT(0.0), dt;
	unsigned int i;
	for (i = 1; i < time.size(); i++)
	{
		dt = time[i] - time[i - 1];
		if (dt < minDT)
			minDT = dt;
		if (dt > maxDT)
			maxDT = dt;
			
		d += (dt - mean) * (dt - mean);
	}
	
	outStream << "Average sample time = " << mean << " sec" << std::endl;
	/*outStream << "Standard deviation = " << sqrt(d / (time.size() - 1)) << " sec" << std::endl;
	outStream << "Maximum sample time = " << maxDT << " sec" << std::endl;
	outStream << "Minimum sample time = " << minDT << " sec" << std::endl;*/
	
	return mean;
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
	// TODO:  Update this
	assert(time.size() == temperature.size());
	assert(time.size() > 1);
	
	if (time.size() <= segments)
	{
		outStream << "Warning:  Number of points used to compute ambient temperature is below " << segments << std::endl;
		segments = time.size() - 1;
	}

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
	// TODO:  Update for pole placement with 3rd order characteristic polynomial
	
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
	
	if (tau <= 0.0)
	{
		outStream << "Invalid Auto-Tune Result:  Model parameter tau is negative (tau = "
			<< tau << " sec)" << std::endl;
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
	double initialTemperature, double ambientTemperature)
{
	assert(time.size() == control.size());
	
	if (!MembersAreValid())
		return false;
		
	BuildSimulationMatrices(initialTemperature, ambientTemperature, 0.0);// TODO:  Parameterize last argument
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
// Description:		Computes the minimum time to collect data for good auto-tune results.
//
// Input Arguments:
//		sampleRate		= double, frequency at which data is collected [Hz]
//		ambTempSegments	= unsigned int, minimum number of data points to collect
//
// Output Arguments:
//		None
//
// Return Value:
//		double, [sec]
//
//==========================================================================
double AutoTuner::GetMinimumAutoTuneTime(double /*sampleRate*/,
	unsigned int /*ambTempSegments*/)
{
	return 0.0;//ignoreInitialTime + (ambTempSegments + 1) / sampleRate;
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