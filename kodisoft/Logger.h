#ifndef LOGGER_H
#define LOGGER_H

#include <windows.h>

#include "Macro.h"


class Logger
{
	private:
		tstring getTime();
		HANDLE logEvent;

	public:
		Logger();
		void log(LPTSTR);
		virtual void logRoutine(LPTSTR) = 0;
		virtual ~Logger();
};

class ConsoleLogger : public Logger
{
	public:
		ConsoleLogger();
		void logRoutine(LPTSTR);
		~ConsoleLogger();
};

class FileLogger : public Logger
{
	private:
		tofstream tfout;

	public:
		FileLogger(LPTSTR);
		void logRoutine(LPTSTR);
		~FileLogger();
};

#endif // LOGGER_H