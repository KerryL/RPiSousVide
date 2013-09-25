// File:  timeHistorylog.h
// Date:  9/4/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Class for generating plottable time-history data, using measured system time
//        (accurate to milliseconds only) to generate time column.  NOT thread-safe.

#ifndef TIME_HISTORY_LOG_H_
#define TIME_HISTORY_LOG_H_

// Standard C++ headers
#include <iostream>
#include <sstream>
#include <vector>
#include <ctime>

// *nix headers
#include <sys/time.h>

class TimeHistoryLog : public std::ostream
{
private:
	class TimeHistoryStreamBuffer : public std::stringbuf
	{
	public:
		TimeHistoryStreamBuffer(std::ostream &str) : output(str) { started = false; };

		virtual int sync(void)
		{
			// TODO:  Ensure proper number of columns?
			// NOTE:  Time stamp is generated when std::endl is passed (or stream is flushed),
			//        thus, when used, it is recommended to pass all data and the endl as close
			//        together as possible (i.e. the data in the first column may be older than
			//        the data in the last column, but they will share a common time stamp).
			if (started)
				output << GetTime() << str();
			else
				output << str();
			str("");
			output.flush();
			return 0;
		};

		void MarkStartTime(void);

	private:
		std::ostream& output;

		bool started;

		std::string GetTime(void);
		struct timeval start;
	} buffer;

	const char delimiter;
	bool headerWritten;
	std::vector<std::pair<std::string, std::string> > columnHeadings;

	void WriteHeader(void);

public:
	TimeHistoryLog(std::ostream& str, char delimiter = ',');

	void AddColumn(std::string title, std::string units);

	template<typename T>
	friend TimeHistoryLog& operator<<(TimeHistoryLog& log, T const& value);
};

template<typename T>
TimeHistoryLog& operator<<(TimeHistoryLog& log, T const& value)
{
	if (!log.headerWritten)
		log.WriteHeader();

	static_cast<std::ostream&>(log) << log.delimiter << value;

	return log;
}

#endif// TIME_HISTORY_LOG_H_
