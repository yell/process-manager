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
		void log(tstring &);
		virtual void logRoutine(tstring &) = 0;
		virtual tstring getInfo() const = 0;
		virtual ~Logger();
};

class ConsoleLogger : public Logger
{
	public:
		ConsoleLogger();
		void logRoutine(tstring &);
		tstring getInfo() const;
		~ConsoleLogger();
};

class FileLogger : public Logger
{
	private:
		tofstream tfout;
		tstring fileName;

	public:
		FileLogger(tstring &);
		void logRoutine(tstring &);
		tstring getInfo() const;
		~FileLogger();
};

#endif // LOGGER_H