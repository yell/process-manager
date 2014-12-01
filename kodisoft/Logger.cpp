#include <windows.h> // CreateEvent ...
#include <iostream>  // cout
#include <fstream>   // ofstream
#include <string>    // cout << string
#include <time.h>    // getTime
#include <tchar.h>

#include "Logger.h"

using std::cout;
using std::wcout;
using std::endl;
using std::string;
using std::wstring;
using std::ofstream;
using std::wofstream;
using std::runtime_error;

Logger::Logger() {
	logEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
}

Logger::~Logger() {
	//WaitForSingleObject(logEvent, INFINITE);
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

void Logger::log(LPTSTR str) {

	WaitForSingleObject(logEvent, INFINITE);
	
	tstringstream tstream;
	tstream << _T(" [ ") << getTime() << _T(" ] ") << std::flush;
	tstring t(tstream.str());

	LPTSTR tt = &t[0];
	LPTSTR temp = new TCHAR[_tcsclen(tt) + _tcsclen(str) + 1];
	
	_tcscpy(temp, tt);
	lstrcat(temp, str);
		
	logRoutine(temp);

	delete[] temp;
	SetEvent(logEvent);
}

ConsoleLogger::ConsoleLogger() {}

void ConsoleLogger::logRoutine(LPTSTR str) {
	tout << str << endl;
}

ConsoleLogger::~ConsoleLogger() {}

FileLogger::FileLogger(LPTSTR fileName) {
	tfout.open(fileName, std::ios_base::out | std::ios_base::app);
	if (!tfout.good())
		throw(runtime_error("Logger: unable to open the file"));
}

void FileLogger::logRoutine(LPTSTR str) {
	tfout << str << endl;
}

FileLogger::~FileLogger() {
	tfout.close();
}