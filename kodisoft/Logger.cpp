#include <windows.h>
#include <tchar.h>
#include <iomanip>
#include <iostream>

#include "Macro.h"
#include "Logger.h"

using std::runtime_error;
using std::setfill;
using std::setw;
using std::endl;
using std::flush;


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

	tstring t(tstream.str());

	return t;
}

Logger::Logger() {

	logEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
}

void Logger::log(tstring & tstr) {

	WaitForSingleObject(logEvent, INFINITE);
	
	tstringstream tstream;
	tstream << _T(" [ ") << getTime() << _T(" ] ") << tstr << flush;
	logRoutine(tstream.str());

	SetEvent(logEvent);
}

tstring Logger::getInfo() const {

	WaitForSingleObject(logEvent, INFINITE);
	tstring t(getInfoRoutine());
	SetEvent(logEvent);

	return t;
}

Logger::~Logger() {

	WaitForSingleObject(logEvent, INFINITE);
	CloseHandle(logEvent);
}

ConsoleLogger::ConsoleLogger() {}

ConsoleLogger::ConsoleLogger(const ConsoleLogger & c) {}

void ConsoleLogger::logRoutine(tstring & tstr) {

	tout << tstr << endl;
}

tstring ConsoleLogger::getInfoRoutine() const {

	return tstring(_T("ConsoleLogger"));
}

ConsoleLogger & ConsoleLogger::operator = (const ConsoleLogger & c) {

	return *this;
}

ConsoleLogger::~ConsoleLogger() {}

FileLogger::FileLogger(tstring & fileName) : fileName(fileName) {

	tfout.open(fileName, std::ios_base::out | std::ios_base::app);
	if (!tfout.good())
		throw(runtime_error("Logger: Unable to open the file"));
}

FileLogger::FileLogger() : FileLogger(tstring(_T("log.txt"))) {}

FileLogger::FileLogger(const FileLogger & f) : fileName(f.fileName){

	tfout.open(fileName, std::ios_base::out | std::ios_base::app);
}

void FileLogger::logRoutine(tstring & tstr) {

	tfout << tstr << endl;
}

tstring FileLogger::getInfoRoutine() const {
	
	tstringstream tstream;
	tstream << _T("FileLogger(\"") << fileName << _T("\")") << flush;
	return tstream.str();
}

FileLogger & FileLogger::operator = (const FileLogger & f) {

	if (&f == this)
		return *this;
	
	tfout.close();
	tfout.open(fileName = f.fileName, std::ios_base::out | std::ios_base::app);
	return *this;
}

FileLogger::~FileLogger() {

	tfout.close();
}