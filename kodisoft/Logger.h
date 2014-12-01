#ifndef LOGGER_H
#define LOGGER_H

#include <windows.h> // HANDLE 
#include <fstream>   // ofstream
#include <string>
#include <sstream>

using std::string;
using std::to_string;
using std::stringstream;
using std::wstring;
using std::to_wstring;
using std::wstringstream;
using std::ofstream;
using std::wofstream;

#ifdef _UNICODE
#define tout wcout
#define tofstream wofstream
#define tstring wstring
#define to_tstring to_wstring
#define tstringstream wstringstream
#else
#define tout cout
#define tofstream ofstream
#define tstring string
#define to_tstring to_string
#define tstringstream stringstream
#endif

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