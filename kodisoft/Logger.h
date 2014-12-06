#ifndef LOGGER_H
#define LOGGER_H

#include <windows.h>

#include "Macro.h"

class Logger
{
	private:
		HANDLE logEvent;
		tstring getTime();

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
		ConsoleLogger(const ConsoleLogger &);

		void logRoutine(tstring &);
		tstring getInfo() const;
		
		ConsoleLogger & operator = (const ConsoleLogger &);
		~ConsoleLogger();
};

class FileLogger : public Logger
{
	private:
		tofstream tfout;
		tstring fileName;

	public:
		FileLogger(tstring &);
		FileLogger();
		FileLogger(const FileLogger &);

		void logRoutine(tstring &);
		tstring getInfo() const;

		FileLogger & operator = (const FileLogger &);
		~FileLogger();
};

#endif // LOGGER_H