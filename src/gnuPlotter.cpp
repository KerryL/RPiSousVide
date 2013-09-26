// File:  gnuPlotter.cpp
// Date:  9/25/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  C++ interface to gnuplot.  Uses pipes for communication.  Based on
//        gnuplot_i by N. Devillard (http://ndevilla.free.fr/gnuplot/).

// Standard C++ headers
#include <cstdlib>
#include <fstream>
#include <cstdio>
#include <cassert>
#include <sstream>
#include <iomanip>

// *nix headers
#include <unistd.h>

// Local headers
#include "gnuPlotter.h"

#ifdef WIN32
#define popen _popen
#define pclose _pclose
#endif

//==========================================================================
// Class:			GNUPlotter
// Function:		Constant definitions
//
// Description:		Constant definitions for GNUPlotter class.
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
#ifdef WIN32
// Not sure why I need the full path - it should be on my path...
// TODO:  Fix this (windows problem only)
const std::string GNUPlotter::gnuPlotName = "\"C:\\Program Files (x86)\\gnuplot\\bin\\pgnuplot.exe\"";
#else
const std::string GNUPlotter::gnuPlotName = "gnuplot";
#endif

//==========================================================================
// Class:			GNUPlotter
// Function:		GNUPlotter
//
// Description:		Constructor for GNUPlotter class.
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
GNUPlotter::GNUPlotter(std::ostream& outStream) : outStream(outStream)
{
	gnuPipe = popen(gnuPlotName.c_str(), "w");
    if (!gnuPipe)
		outStream << "Failed to open pipe to " << gnuPlotName << std::endl;
}

//==========================================================================
// Class:			GNUPlotter
// Function:		~GNUPlotter
//
// Description:		Destructor for GNUPlotter class.
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
GNUPlotter::~GNUPlotter()
{
	if (gnuPipe)
	{
		if (pclose(gnuPipe) == -1)
			outStream << "Failed to close pipe to " << gnuPlotName << std::endl;
	}

	unsigned int i;
	for (i = 0; i < tempFileNames.size(); i++)
	{
		if (remove(tempFileNames[i].c_str()) != 0)
			outStream << "Failed to remove temporary file '" << tempFileNames[i] << "'" << std::endl;
	}
}

//==========================================================================
// Class:			GNUPlotter
// Function:		SendCommand
//
// Description:		Sends specified command to gnuplot.
//
// Input Arguments:
//		command	= std::string
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if command could be sent (pipe is open)
//
//==========================================================================
bool GNUPlotter::SendCommand(std::string command)
{
	if (!gnuPipe)
	{
		outStream << "Failed to send command:  Pipe is not open" << std::endl;
		return false;
	}

	command.append("\n");
	fputs(command.c_str(), gnuPipe);
	fflush(gnuPipe);

	return true;
}

//==========================================================================
// Class:			GNUPlotter
// Function:		PlotYAgaintIndex
//
// Description:		Plots the specified vector of y-values against its indices.
//
// Input Arguments:
//		y		= std::vector<double>&
//		args	= std::string, any additional arguments to follow the plot command
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if command could be sent (pipe is open)
//
//==========================================================================
bool GNUPlotter::PlotYAgainstIndex(const std::vector<double> &y, std::string args)
{
	return PlotYAgainstIndex(0, y, args, false);
}

//==========================================================================
// Class:			GNUPlotter
// Function:		PlotYAgainstIndex
//
// Description:		Plots the specified vector of y-values against its indices.
//					Optionally appends data to file instead of overwritting.
//					User must keep track of data series index (first argument).
//
// Input Arguments:
//		i		= unsigned int
//		y		= std::vector<double>&
//		args	= std::string, any additional arguments to follow the plot command
//		append	= bool
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if command could be sent (pipe is open)
//
//==========================================================================
bool GNUPlotter::PlotYAgainstIndex(unsigned int i, const std::vector<double> &y,
	std::string args, bool append)
{
	assert(i <= tempFileNames.size());

	if (i == tempFileNames.size())
		tempFileNames.push_back(GetTemporaryFileName());

	if (!WriteTemporaryFile(i, y, append))
		return false;

	std::string command("plot \"" + tempFileNames[i] + "\"");
	if (!args.empty())
		command.append(" " + args);

	return SendCommand(command);
}

//==========================================================================
// Class:			GNUPlotter
// Function:		PlotYAgainstX
//
// Description:		Plots the specified vector of y-values against x-values.
//
// Input Arguments:
//		x		= std::vector<double>&
//		y		= std::vector<double>&
//		args	= std::string, any additional arguments to follow the plot command
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if command could be sent (pipe is open)
//
//==========================================================================
bool GNUPlotter::PlotYAgainstX(const std::vector<double> &x, const std::vector<double> &y, std::string args)
{
	return PlotYAgainstX(0, x, y, args, false);
}

//==========================================================================
// Class:			GNUPlotter
// Function:		PlotYAgainstX
//
// Description:		Plots the specified vector of y-values against x-values.
//					Optionally appends data to file instead of overwritting.
//					User must keep track of data series index (first argument).
//
// Input Arguments:
//		i		= unsigned int
//		x		= std::vector<double>&
//		y		= std::vector<double>&
//		args	= std::string, any additional arguments to follow the plot command
//		append	= bool
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if command could be sent (pipe is open)
//
//==========================================================================
bool GNUPlotter::PlotYAgainstX(unsigned int i, const std::vector<double> &x,
	const std::vector<double> &y, std::string args, bool append)
{
	assert(i <= tempFileNames.size());

	if (i == tempFileNames.size())
		tempFileNames.push_back(GetTemporaryFileName());

	if (!WriteTemporaryFile(i, x, y, append))
		return false;

	std::string command("plot \"" + tempFileNames[i] + "\"");
	if (!args.empty())
		command.append(" " + args);

	return SendCommand(command);
}

//==========================================================================
// Class:			GNUPlotter
// Function:		GetTemporaryFileName
//
// Description:		Generates a random temporary file name.  Random number
//					generator must have been previously seeded.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		std::string
//
//==========================================================================
std::string GNUPlotter::GetTemporaryFileName(void)
{
	std::stringstream s;

	time_t rawTime;
	struct tm *now;
	time(&rawTime);
	now = localtime(&rawTime);

	s << std::setfill('0');

	s << "tmp_gnuplot_" << std::setw(4) << now->tm_year + 1900 << "-"
		<< std::setw(2) << now->tm_mon + 1 << "-"
		<< std::setw(2) << now->tm_mday << "_"
		<< std::setw(2) << now->tm_hour << "_"
		<< std::setw(2) << now->tm_min << "_"
		<< std::setw(2) << now->tm_sec << "_"
		<< rand() << ".dat";

	return s.str();
}

//==========================================================================
// Class:			GNUPlotter
// Function:		WriteTemporaryFile
//
// Description:		Writes the specified vector to file.
//
// Input Arguments:
//		i		= unsigned int specifying which file name to use
//		y		= const std::vector<double>&
//		append	= bool, indicates whether file should be appended-to or overwritten
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if successful, false otherwise
//
//==========================================================================
bool GNUPlotter::WriteTemporaryFile(unsigned int i, const std::vector<double> &y,
	bool append) const
{
	assert(y.size() > 0);
	assert(i < tempFileNames.size());

	std::ofstream tempFile(tempFileNames[i].c_str(), append ? std::ios::app : std::ios::out);
	if (!tempFile.is_open() || !tempFile.good())
	{
		outStream << "Failed to open temporary file '" << tempFileNames[i] << "' for output" << std::endl;
		return false;
	}

	unsigned int j;
	for (j = 0; j < y.size(); j++)
		tempFile << y[j] << std::endl;

	tempFile.close();

	return true;
}

//==========================================================================
// Class:			GNUPlotter
// Function:		WriteTemporaryFile
//
// Description:		Writes the specified vectors to file.
//
// Input Arguments:
//		i		= unsigned int specifying which file name to use
//		x		= const std::vector<double>&
//		y		= const std::vector<double>&
//		append	= bool, indicates whether file should be appended-to or overwritten
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if successful, false otherwise
//
//==========================================================================
bool GNUPlotter::WriteTemporaryFile(unsigned int i, const std::vector<double> &x,
	const std::vector<double> &y, bool append) const
{
	assert(x.size() == y.size());
	assert(y.size() > 0);
	assert(i < tempFileNames.size());

	std::ofstream tempFile(tempFileNames[i].c_str(), append ? std::ios::app : std::ios::out);
	if (!tempFile.is_open() || !tempFile.good())
	{
		outStream << "Failed to open temporary file '" << tempFileNames[i] << "' for output" << std::endl;
		return false;
	}

	tempFile << std::scientific << std::setprecision(18);

	unsigned int j;
	for (j = 0; j < y.size(); j++)
		tempFile << x[j] << " " << y[j] << std::endl;

	tempFile.close();

	return true;
}

//==========================================================================
// Class:			GNUPlotter
// Function:		WaitForGNUPlot
//
// Description:		Waits for the specified file to be deleted by gnuplot.
//					useful for waiting for gnuplot to finish a task.  This is
//					not an efficient way to go about this, so only use it if
//					absolutely necessary.
//
//					If you're looking at this method thinking "my, what a
//					kludgy hack!" you'd be right.  Unfortunately, I have no
//					better method to offer right now.
//
// Input Arguments:
//		testFileName	= std::string
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void GNUPlotter::WaitForGNUPlot(std::string testFileName)
{
//	sleep(1000);
// It appears that windows has some issues writing the files fast
// enough, but Linux does.  Conversely, Linux has issues with this
// method.  We'll skip it.
#ifndef WIN32
//	return;
#endif
	// Create file
	std::ofstream out(testFileName.c_str(), std::ios::out);
	out.close();

	// Tell gnuplot to delete the file
	std::string command("system \"");
#ifdef WIN32
	command.append("del ");
#else
	command.append("rm ");
#endif
	command.append(testFileName + "\"");
	SendCommand(command);

	// Wait for it to be deleted
	while (true)
	{
		std::ifstream in(testFileName.c_str(), std::ios::in);
		if (!in.good() || !in.is_open())
			break;
		in.close();
		SendCommand(command);

#ifdef WIN32
		_sleep(100);
#else
		usleep(100000);
#endif
	}
}
