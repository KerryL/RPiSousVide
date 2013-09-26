// File:  gnuPlotTest.cpp
// Date:  9/24/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Application for testing GNUPlotter class.  It is based upon
//        gnuplot_i, which was written by N. Devillard <nDevil@eso.org>.

// Standard C++ headers
#include <cstdlib>
#include <iostream>
#include <cmath>

// Local headers
#include "gnuPlotter.h"

using namespace std;

void PlotTwoSineWaves(void);
void MakeThreePlotsByAppendingData(void);

// Application entry point
int main(int, char *[])
{
	srand(time(NULL));

	PlotTwoSineWaves();
	MakeThreePlotsByAppendingData();

	return 0;
}

void PlotTwoSineWaves(void)
{
	// Create some data
	const unsigned int nPts(1000);
	std::vector<double> data1, data2, time;
	const double timeStep(0.01);

	time.push_back(0.0);
	data1.push_back(0.0);
	data2.push_back(0.0);

	unsigned int i;
	for (i = 1; i < nPts; i++)
	{
		time.push_back(time[i - 1] + timeStep);
		data1.push_back(3.0 * sin(time[i] * 10.0) - 1.0);
		data2.push_back(8.0 * sin(time[i] * 3.0));
	}

	/* This code should generate a plot equivalent to (except for the actual
	waveforms, which in the example come from data files) the following gnuplot
	commands (included with semi-colons here for ease of copy/paste):

	set terminal png size 800,600;
	set output "plotTest.png";
	set multiplot;
	set yrange [-10:10];
	set grid;
	set style line 1 lt 1 lc rgb "red" lw 2;
	set style line 2 lt 1 lc rgb "blue" lw 2;
	set title "Two Sine Waves";
	set xlabel "Time [sec]";
	set ylabel "Values [-]";
	set key at 10,9.8;
	plot 3*sin(x*10)-1 ls 1;
	set key at 10,9;
	plot 8*sin(x*3) ls 2;
	replot;
	unset multiplot;
	*/

	GNUPlotter plotter;
	if (!plotter.PipeIsOpen())
	{
		cout << "gnuPlot pipe is broken" << endl;
		exit(1);
	}

	// Optionally, we could check the return value from every call,
	// but we'll assume the above check is enough (it should be)

	// Omitting these first two calls will pop open a window with the plot
	// On a headless Raspberry Pi, you'll want these first two lines
	plotter.SendCommand("set terminal png size 800,600");
	plotter.SendCommand("set output \"plotTest.png\"");

	plotter.SendCommand("set multiplot");
	plotter.SendCommand("set yrange [-10:10]");
	plotter.SendCommand("set grid");

	plotter.SendCommand("set title \"Two Sine Waves\"");
	plotter.SendCommand("set xlabel \"Time [sec]\"");
	plotter.SendCommand("set ylabel \"Values [-]\"");

	plotter.SendCommand("set style line 1 lt 1 lc rgb \"red\" lw 2");
	plotter.SendCommand("set style line 2 lt 1 lc rgb \"blue\" lw 2");

	plotter.SendCommand("set key at 10,9.8");
	// Not sure why "with lines" is required here and on the next plot call below
	// When I type into the console I don't need it, but with this interface, I do?
	plotter.PlotYAgainstX(0, time, data1, "title \"Sine 1\" ls 1 with lines");

	plotter.SendCommand("set key at 10,9");
	plotter.PlotYAgainstX(1, time, data2, "title \"Sine 2\" ls 2 with lines");

	plotter.SendCommand("replot");
	plotter.SendCommand("unset multiplot");
	plotter.WaitForGNUPlot();

	// No cleanup required - handled in GNUPlotter destructor
}

void MakeThreePlotsByAppendingData(void)
{
	const unsigned int nPts(1000);
	std::vector<double> data;

	unsigned int i;
	for (i = 0; i < nPts; i++)
		data.push_back(i * 3.4 - 5.0);

	GNUPlotter plotter;
	if (!plotter.PipeIsOpen())
	{
		cout << "gnuPlot pipe is broken" << endl;
		exit(1);
	}

	plotter.SendCommand("set terminal png");
	plotter.SendCommand("set grid");
	plotter.SendCommand("unset key");

	plotter.SendCommand("set output \"appendTest1.png\"");
	plotter.PlotYAgainstIndex(0, data, "with lines");
	plotter.SendCommand("replot");
	plotter.WaitForGNUPlot("wait_test_1");

	plotter.SendCommand("set output \"appendTest2.png\"");
	plotter.PlotYAgainstIndex(0, data, "with lines");
	plotter.SendCommand("replot");
	plotter.WaitForGNUPlot("wait_test_2");

	plotter.SendCommand("set output \"appendTest3.png\"");
	plotter.PlotYAgainstIndex(0, data, "with lines");
	plotter.SendCommand("replot");
	plotter.WaitForGNUPlot("wait_test_3");
}
