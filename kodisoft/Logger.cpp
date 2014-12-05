#include <windows.h> // CreateEvent ...
#include <string> 
#include <time.h>    // getTime
#include <tchar.h>

#include "Macro.h"
#include "Logger.h"

using std::endl;
using std::flush;
using std::string;
using std::wstring;
using std::runtime_error;

Logger::Logger() {

	logEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
}

Logger::~Logger() {

	WaitForSingleObject(logEvent, INFINITE);
	CloseHandle(logEvent);
}

tstring Logger::getTime() {

	string timeStr;
	time_t rawTime;
	time(&rawTime);
	timeStr = ctime(&rawTime);
	string s =  timeStr.substr(0, timeStr.size() - 1);

	#ifdef _UNICODE
		wstring w(s.begin(), s.end());
		return w;
	#else
		return s;
	#endif
}

void Logger::log(tstring & str) {

	WaitForSingleObject(logEvent, INFINITE);
	
	tstringstream tstream;
	tstream << _T(" [ ") << getTime() << _T(" ] ") << str << flush;
	tstring t(tstream.str());
		
	logRoutine(t);

	SetEvent(logEvent);
}

ConsoleLogger::ConsoleLogger() {}

void ConsoleLogger::logRoutine(tstring & str) {
	tout << str << endl;
}

tstring ConsoleLogger::getInfo() const {
	return tstring(_T("ConsoleLogger"));
}

ConsoleLogger::~ConsoleLogger() {}

FileLogger::FileLogger(tstring & fileName) : fileName(fileName) {
	tfout.open(fileName, std::ios_base::out | std::ios_base::app);
	if (!tfout.good())
		throw(runtime_error("Logger: Unable to open the file"));
}

void FileLogger::logRoutine(tstring & str) {
	tfout << str << endl;
}

tstring FileLogger::getInfo() const {
	
	tstringstream tstream;
	tstream << _T("FileLogger(\"") << fileName << _T("\")") << flush;
	tstring t(tstream.str());
	
	return t;
}

FileLogger::~FileLogger() {
	tfout.close();
}