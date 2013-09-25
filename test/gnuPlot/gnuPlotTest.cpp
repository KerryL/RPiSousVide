// File:  gnuPlotTest.cpp
// Date:  9/24/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Application for testing gnuPlot interface (3rd party).
//        gnuplot_pipes was obtained here: http://www.ee.iitb.ac.in/~relab/gnu/
//        It was written by N. Devillard <nDevil@eso.org>.

// Standard C++ headers
#include <cstdlib>
#include <iostream>
#include <cmath>

// Local headers
#include "gnuplot_i.h"

using namespace std;

// Application entry point
int main(int, char *[])
{
	// Create some data
	const unsigned int nPts(1000);
	double data1[nPts], data2[nPts], time[nPts];
	const double timeStep(0.01);

	time[0] = 0.0;
	data1[0] = 0.0;
	data2[0] = 0.0;

	unsigned int i;
	for (i = 1; i < nPts; i++)
	{
		time[i] = time[i - 1] + timeStep;
		data1[i] = 3.0 * sin(time[i] * 10.0) - 1.0;
		data2[i] = 8.0 * sin(time[i] * 3.0);
	}

	// Create a plot
	gnuplot_ctrl *handle = gnuplot_init();

	// Instruct gnuplot to save to file
	gnuplot_cmd(handle, "set terminal png size 800,600");
	gnuplot_cmd(handle, "set output \"plotTest.png\"");

	gnuplot_setstyle(handle, "lines");
	gnuplot_cmd(handle, "set multiplot");
	gnuplot_cmd(handle, "set yrange [-10:10]");// Need to force the y-range, otherwise multiple curves each have their own range
	gnuplot_cmd(handle, "set grid");

	gnuplot_set_xlabel(handle, "Time [sec]");
	gnuplot_set_ylabel(handle, "Value [-]");
	gnuplot_cmd(handle, "set title \"Two Sine Waves\"");

	gnuplot_cmd(handle, "set style line 1 lt 1 lc rgb \"red\" lw 2");// style #1, solid line (lt 1), red, width of 2
	gnuplot_cmd(handle, "set style line 2 lt 1 lc rgb \"blue\" lw 2");// style #2, solid line (lt 1), blue, width of 2

	gnuplot_cmd(handle, "set key at 10,9.8");// Uses axes scales to determine placement
	gnuplot_plot_xy(handle, time, data1, nPts, "Sine 1\" ls 1");// Use injection to force the line style (no clean mechanism for this)

	gnuplot_cmd(handle, "set key at 10,9");// Uses axes scales to determine placement
	gnuplot_plot_xy(handle, time, data2, nPts, "Sine 2\" ls 2");// Use injection to force the line style (no clean mechanism for this)

	gnuplot_resetplot(handle);
	gnuplot_cmd(handle, "unset multiplot");

	// Clean up
	gnuplot_close(handle);

	return 0;
}
