#include <windows.h>
#include <tchar.h>
#include <iomanip>
#include <iostream>

#include "Macro.h"
#include "Process.h"

using std::runtime_error;
using std::boolalpha;
using std::noboolalpha;
using std::setw;
using std::endl;
using std::flush;

int Process::count;

DWORD WINAPI Process::watchingThreadFunc(void * arg) {

	HANDLE h[2] = { ((Process*)arg)->processHandle, ((Process*)arg)->processMutex };

	while (((Process*)arg)->status != FINISHING) {

		WaitForMultipleObjects(2, h, TRUE, 0);		

		if (((Process*)arg)->status == IS_WORKING && !(((Process*)arg)->isStillActive())) {

			((Process*)arg)->status = RESTARTING;
			((Process*)arg)->log(tstring(_T("crashed. restarting ...")));
			((Process*)arg)->onProcCrash();

			((Process*)arg)->closeRoutine();
			((Process*)arg)->startRoutine();

			((Process*)arg)->status = IS_WORKING;
			((Process*)arg)->log(tstring(_T("restarted after crash")));
			((Process*)arg)->onProcStart();
		}
		ReleaseMutex(((Process*)arg)->processMutex);
	}
	return 0;
}

bool Process::isStillActive() const {

	DWORD exitCode = 0;
	GetExitCodeProcess(processHandle, &exitCode);
	return (exitCode == STILL_ACTIVE);
}

void Process::startRoutine() {

	if (!isStillActive()) {
		
		tstring t(commandLine);
		LPTSTR temp = &t[0];

		STARTUPINFO startInfo;
		PROCESS_INFORMATION processInfo;

		ZeroMemory(&startInfo, sizeof(startInfo));
		ZeroMemory(&processInfo, sizeof(processInfo));

		if (!CreateProcess(NULL, temp, NULL, NULL, FALSE, 0, NULL, NULL, &startInfo, &processInfo))
			throw(runtime_error("\nProcess: Unable to start process"));

		processHandle = processInfo.hProcess;
		processId = processInfo.dwProcessId;
	}
}

void Process::closeRoutine() {

	if (isStillActive()) {
		TerminateProcess(processHandle, 0);
		CloseHandle(processHandle);
	}
}

void Process::log(tstring & tstr) const {

	tstringstream tstream;
	tstream << _T("\n") << setw(18) << _T("message: ") << tstr << _T("\n") << getInfo() << endl;
	logger->log(tstream.str());
}

Process::Process(tstring & commandLine, Logger * logger, bool killAtTheEnd) :
	 commandLine(commandLine), logger(logger), killAtTheEnd(killAtTheEnd), monitorId(++count) {

	status = IS_WORKING;
	startRoutine();
	processMutex = CreateMutex(NULL, FALSE, NULL);
	watchingThread = CreateThread(NULL, 0, watchingThreadFunc, this, 0, &threadId);
	log(tstring(_T("started")));
}

Process::Process(tstring & commandLine, Logger * logger) : 
	 Process(commandLine, logger, false) {}

Process::Process(Logger * logger, bool killAtTheEnd) :
	 Process(tstring(_T("calc")), logger, killAtTheEnd) {}

Process::Process(tstring & commandLine, bool killAtTheEnd) :
	 Process(commandLine, new FileLogger(), killAtTheEnd) {}

Process::Process(tstring & commandLine) : 
	 Process(commandLine, new FileLogger(), false) {}

Process::Process(Logger * logger) : 
	 Process(tstring(_T("calc")), logger, false) {}

Process::Process() : 
	 Process(tstring(_T("calc")), new FileLogger(), false) {}

Process::Process(DWORD pid, Logger * logger) : 
	 Process(pid, logger, false) {}

Process::Process(DWORD pid, bool killAtTheEnd) :
	 Process(pid, new FileLogger(), killAtTheEnd) {}

Process::Process(DWORD pid) : 
	 Process(pid, new FileLogger(), false) {}

void Process::stop() {

	WaitForSingleObject(processMutex, INFINITE);

	if (status == IS_WORKING) {

		status = STOPPED;
		closeRoutine();
		log(tstring(_T("manually stopped")));
		onProcManualStop();
	}
	else
		log(tstring(_T("an attempt to stop already stopped process")));

	ReleaseMutex(processMutex);
}

void Process::resume() {

	WaitForSingleObject(processMutex, INFINITE);

	if (status == STOPPED) {

		startRoutine();
		status = IS_WORKING;
		log(tstring(_T("manually resumed")));
		onProcManualResume();
	}
	else
		log(tstring(_T("an attempt to resume active process")));

	ReleaseMutex(processMutex);
}

void Process::restart() {

	WaitForSingleObject(processMutex, INFINITE);
	
	status = RESTARTING;
	log(tstring(_T("manually restarting ...")));
	
	closeRoutine();
	startRoutine();
	
	status = IS_WORKING;
	log(tstring(_T("manually restarted")));

	onProcManualRestart();

	ReleaseMutex(processMutex);
}

void Process::switchLogger(Logger * logger) {

	WaitForSingleObject(processMutex, INFINITE);

	tstringstream tstream;
	tstream << _T("switched logger to ") << logger->getInfo() << flush;
	log(tstream.str());

	tstring prev = this->logger->getInfo();
	this->logger.reset(logger);

	tstream.str(_T(""));
	tstream << _T("continuing logging after ") << prev << flush;
	log(tstream.str());

	ReleaseMutex(processMutex);
}

void Process::switchLogger() {

	WaitForSingleObject(processMutex, INFINITE);
	switchLogger(new FileLogger(tstring(_T("log.txt"))));
	ReleaseMutex(processMutex);
}

bool Process::isKillAtTheEnd() const {

	WaitForSingleObject(processMutex, INFINITE);
	bool b = killAtTheEnd;
	ReleaseMutex(processMutex);

	return b;
}

int Process::getMonitorId() const {

	WaitForSingleObject(processMutex, INFINITE);
	int i = monitorId;
	ReleaseMutex(processMutex);

	return i;
}

DWORD Process::getProcessId() const { 

	WaitForSingleObject(processMutex, INFINITE);
	DWORD d = processId;
	ReleaseMutex(processMutex);

	return d;
}

HANDLE Process::getProcessHandle() const { 

	WaitForSingleObject(processMutex, INFINITE);
	HANDLE h = processHandle;
	ReleaseMutex(processMutex);

	return processHandle;
}

tstring Process::getCommandLine() const { 

	WaitForSingleObject(processMutex, INFINITE);
	tstring t(commandLine);
	ReleaseMutex(processMutex);

	return t;
}

tstring Process::getStatus() const {

	WaitForSingleObject(processMutex, INFINITE);

	tstring statusStr[] = { _T("is working"), 
				_T("is stopped"), 
			        _T("restarting"),
				_T("finishing")};
	
	tstring t(statusStr[status]);

	ReleaseMutex(processMutex);

	return t;
}

tstring Process::getInfo() const {

	WaitForSingleObject(processMutex, INFINITE);

	tstringstream tstream;
	tstream << setw(18) << _T("monitor id: ") << getMonitorId() << endl;
	tstream << setw(18) << _T("pid: ") << getProcessId() << endl;
	tstream << setw(20) << _T("handle: 0x") << getProcessHandle() << endl;
	tstream << setw(18) << _T("status: ") << getStatus() << endl;
	tstream << setw(18) << _T("command line: ") << getCommandLine() << endl;
	tstream << setw(18) << _T("kill at the end: ") << boolalpha << isKillAtTheEnd() << noboolalpha << endl;
	tstream << setw(18) << _T("logger: ") << getLoggerInfo() << flush;
	tstring t(tstream.str());

	ReleaseMutex(processMutex);

	return t;
}

tstring Process::getLoggerInfo() const {

	WaitForSingleObject(processMutex, INFINITE);
	tstring t(logger->getInfo());
	ReleaseMutex(processMutex);

	return t;
}

Process::~Process() {

	status = FINISHING;

	PostThreadMessage(threadId, WM_QUIT, 0, 0);
	WaitForSingleObject(watchingThread, INFINITE);
	CloseHandle(watchingThread);

	WaitForSingleObject(processMutex, INFINITE);
	CloseHandle(processMutex);

	if (killAtTheEnd) {
		log(tstring(_T("manually shutdowned")));
		closeRoutine();
		onProcManualShutdown();
	}

	log(tstring(_T("finished")));
}