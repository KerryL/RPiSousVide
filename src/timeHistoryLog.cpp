// File:  timeHistorylog.cpp
// Date:  9/4/2013
// Auth:  K. Loux
// Desc:  Class for generating plottable time-history data, using measured system time
//        (down to microseconds - unsure of accuracy) to generate time column.  NOT thread-safe.

// Standard C++ headers
#include <cassert>

// *nix headers
#include <unistd.h>

// Local headers
#include "timeHistoryLog.h"

TimeHistoryLog::TimeHistoryLog(std::ostream& str, char delimiter) : std::ostream(&buffer),
	buffer(str), delimiter(delimiter)
{
	headerWritten = false;
}

void TimeHistoryLog::WriteHeader(void)
{
	assert(!headerWritten);
	headerWritten = true;

	unsigned int i;

	static_cast<std::ostream&>(*this) << "Time";
	for (i = 0; i < columnHeadings.size(); i++)
		*this << columnHeadings[i].first;
	*this << std::endl;

	static_cast<std::ostream&>(*this) << "[sec]";
	for (i = 0; i < columnHeadings.size(); i++)
		*this << columnHeadings[i].second;
	*this << std::endl;

	buffer.MarkStartTime();
}

void TimeHistoryLog::AddColumn(std::string title, std::string units)
{
	assert(!headerWritten);
	columnHeadings.push_back(std::make_pair(title, "[" + units + "]"));
}

std::string TimeHistoryLog::TimeHistoryStreamBuffer::GetTime(void)
{
	unsigned long seconds, useconds;
	double elapsed;
	struct timeval now;
	gettimeofday(&now, NULL);

	seconds  = now.tv_sec  - start.tv_sec;
	useconds = now.tv_usec - start.tv_usec;

	elapsed = seconds + useconds / 1000000.0 + 0.0005;

	std::stringstream ss;
	ss << elapsed;
	return ss.str();
}

void TimeHistoryLog::TimeHistoryStreamBuffer::MarkStartTime(void)
{
	gettimeofday(&start, NULL);
	started = true;
}
