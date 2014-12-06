#include <windows.h>
#include <tchar.h>
#include <iomanip>

#include "Macro.h"
#include "Logger.h"

using std::endl;
using std::flush;
using std::setw;
using std::setfill;
using std::runtime_error;

Logger::Logger() {

	logEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
}

tstring Logger::getTime() {

	SYSTEMTIME st;
	GetLocalTime(&st);

	tstringstream tstream;
	tstream << setfill(_T('0')) << setw(2) << st.wDay << _T("-");
	tstream << setfill(_T('0')) << setw(2) << st.wMonth << _T("-");
	tstream << st.wYear << _T(" ");

	tstream << setfill(_T('0')) << setw(2) << st.wHour << _T(":");
	tstream << setfill(_T('0')) << setw(2) << st.wMinute << _T(":");
	tstream << setfill(_T('0')) << setw(2) << st.wSecond << _T(".");
	tstream << setfill(_T('0')) << setw(3) << st.wMilliseconds << flush;

	return tstream.str();
}

void Logger::log(tstring & str) {

	WaitForSingleObject(logEvent, INFINITE);
	
	tstringstream tstream;
	tstream << _T(" [ ") << getTime() << _T(" ] ") << str << flush;
	logRoutine(tstream.str());

	SetEvent(logEvent);
}

Logger::~Logger() {

	WaitForSingleObject(logEvent, INFINITE);
	CloseHandle(logEvent);
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
	return tstream.str();
}

FileLogger::~FileLogger() {

	tfout.close();
}