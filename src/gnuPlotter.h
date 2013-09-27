// File:  gnuPlotter.h
// Date:  9/25/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  C++ interface to gnuplot.  Uses pipes for communication.  Based on
//        gnuplot_i by N. Devillard (http://ndevilla.free.fr/gnuplot/).

#ifndef GNU_PLOTTER_H_
#define GNU_PLOTTER_H_

// Standard C++ headers
#include <string>
#include <vector>
#include <iostream>
#include <stdio.h>

class GNUPlotter
{
public:
	GNUPlotter(std::ostream& outStream = std::cout);
	~GNUPlotter();

	bool PipeIsOpen(void) const { return gnuPipe != NULL; };

	bool SendCommand(std::string command);

	bool PlotYAgainstIndex(const std::vector<double> &y, std::string args = "");
	bool PlotYAgainstIndex(unsigned int i, const std::vector<double> &y,
		std::string args = "", bool append = true);

	bool PlotYAgainstX(const std::vector<double> &x,
		const std::vector<double> &y, std::string args = "");
	bool PlotYAgainstX(unsigned int i, const std::vector<double> &x,
		const std::vector<double> &y, std::string args = "", bool append = true);

	void WaitForGNUPlot(void);

private:
	static const std::string gnuPlotName;
	std::ostream& outStream;
	std::vector<std::string> tempFileNames;

	FILE *gnuPipe;

	static std::string GetTemporaryFileName(void);
	bool WriteTemporaryFile(unsigned int i, const std::vector<double> &y,
		bool append) const;
	bool WriteTemporaryFile(unsigned int i, const std::vector<double> &x,
		const std::vector<double> &y, bool append) const;
};


#endif// GNU_PLOTTER_H_
