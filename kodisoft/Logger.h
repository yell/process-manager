#ifndef LOGGER_H
#define LOGGER_H

#include <windows.h>

#include "Macro.h"

class Logger {

	private:
		HANDLE logEvent;
		tstring getTime();

	protected:
		virtual void logRoutine(tstring &) = 0;
		virtual tstring getInfoRoutine() const = 0;

	public:
		Logger();

		void log(tstring &);
		tstring getInfo() const;

		virtual ~Logger();
};

class ConsoleLogger : public Logger
{
	protected:
		void logRoutine(tstring &);
		tstring getInfoRoutine() const;

	public:
		ConsoleLogger();
		ConsoleLogger(const ConsoleLogger &);
	
		ConsoleLogger & operator = (const ConsoleLogger &);
		~ConsoleLogger();
};

class FileLogger : public Logger
{
	private:
		tofstream tfout;
		tstring fileName;

	protected:
		void logRoutine(tstring &);
		tstring getInfoRoutine() const;

	public:
		FileLogger(tstring &);
		FileLogger();
		FileLogger(const FileLogger &);

		FileLogger & operator = (const FileLogger &);
		~FileLogger();
};

#endif // LOGGER_H