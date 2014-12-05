#include <windows.h>
#include <tchar.h>
#include <iostream>

#include "Macro.h"
#include "Process.h"

using std::endl;
using std::flush;
using std::runtime_error;


int Process::count;

void Process::log(tstring & str) const {

	tstringstream tstream;
	tstream << _T("[ msg: ") << str << _T(" ]\n") << getInfo() << _T("\n") << flush;
	tstring t(tstream.str());

	logger->log(t);
}

DWORD WINAPI Process::watchingThreadFunc(void * arg) {

	HANDLE h[2] = { ((Process*)arg)->processHandle, ((Process*)arg)->generalEvent };

	while (((Process*)arg)->status != Finishing) {

		WaitForMultipleObjects(2, h, TRUE, 0);

		if (((Process*)arg)->status == IsWorking && !(((Process*)arg)->isStillActive())) {

			((Process*)arg)->status = Restarting;
			((Process*)arg)->log(tstring(_T("crashed. restarting ...")));
			((Process*)arg)->onProcCrash();

			((Process*)arg)->closeRoutine();
			((Process*)arg)->startRoutine();

			((Process*)arg)->status = IsWorking;
			((Process*)arg)->log(tstring(_T("restarted after crash")));
			((Process*)arg)->onProcStart();
		}	
		SetEvent(((Process*)arg)->generalEvent);
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
		LPTSTR temp = new TCHAR[_tcslen(commandLine) + 1];
		_tcscpy(temp, commandLine);

		PROCESS_INFORMATION processInfo;
		STARTUPINFO startInfo;

		ZeroMemory(&startInfo, sizeof(startInfo));
		ZeroMemory(&processInfo, sizeof(processInfo));

		if (!CreateProcess(NULL, temp, NULL, NULL, FALSE, 0, NULL, NULL, &startInfo, &processInfo)) {
			delete[] temp;
			throw(runtime_error("\nProcess: Unable to start process"));
		}

		processHandle = processInfo.hProcess;
		processID = processInfo.dwProcessId;

		delete[] temp;
	}
}

void Process::closeRoutine() {

	if (isStillActive()) {
		TerminateProcess(processHandle, 0);
		CloseHandle(processHandle);
	}
}

Process::Process(LPTSTR cmd, Logger * logger) : logger(logger), id(++count) {

	commandLine = new TCHAR[_tcslen(cmd) + 3];
	commandLine[0] = _T('"'); 
	_tcscpy(commandLine + 1, cmd);
	lstrcat(commandLine, _T("\""));	

	status = IsWorking;
	startRoutine();
	generalEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	watchingThread = CreateThread(NULL, 0, watchingThreadFunc, this, 0, &threadId);
	log(tstring(_T("started")));
}

Process::Process(DWORD pid) : Process(pid, new FileLogger(tstring(_T("log.txt")))) {}

Process::Process(LPTSTR cmd) : Process(cmd, new FileLogger(tstring(_T("log.txt")))) {}

Process::Process(Logger * logger) : Process(_T("calc"), logger) {}

Process::Process() : Process(new FileLogger(tstring(_T("log.txt")))) {}

void Process::switchLogger(Logger * logger) {

	tstringstream tstream;
	tstream << _T("switched logger to ") << logger->getInfo() << flush;
	log(tstream.str());
	
	tstring prev = this->logger->getInfo();
	
	this->logger.reset(logger);
	
	tstream.str(_T(""));
	tstream << _T("continuing logging after ") << prev << flush;
	log(tstream.str());
}

void Process::stop() {

	WaitForSingleObject(generalEvent, INFINITE);

	if (status == IsWorking) {

		status = Stopped;
		onProcManualStop();
		closeRoutine();
		//processID = 0;
		//processHandle = 0;
		log(tstring(_T("manually stopped")));
	}
	else
		log(tstring(_T("an attempt to stop already stopped process")));

	SetEvent(generalEvent);
}

void Process::resume() {

	WaitForSingleObject(generalEvent, INFINITE);

	if (status == Stopped) {

		startRoutine();

		status = IsWorking;
		log(tstring(_T("manually resumed")));
		onProcManualResume();
	}
	else
		log(tstring(_T("an attempt to resume active process")));

	SetEvent(generalEvent);
}

void Process::restart() {

	WaitForSingleObject(generalEvent, INFINITE);
	
	status = Restarting;
	log(tstring(_T("manually restarting ...")));
	onProcManualRestart();

	closeRoutine();
	startRoutine();
	
	status = IsWorking;
	log(tstring(_T("manually restarted")));

	SetEvent(generalEvent);
}

int Process::getId() const {

	return id;
}

DWORD Process::getProcessID() const { 

	return processID;
}

HANDLE Process::getProcessHandle() const { 

	return processHandle;
}

LPTSTR Process::getCommandLine() const { 

	return commandLine; 
}

tstring Process::getStatus() const {

	tstring statusStr[] = { _T("is working"), 
				_T("is stopped"), 
			        _T("restarting"),
				_T("finishing")};
	
	return statusStr[status];
}

tstring Process::getInfo() const {

	tstringstream tstream;

	tstream << _T(" [ id: ") << getId();
	tstream << _T(" ] [ pid: ") << getProcessID();
	tstream << _T(" ] [ handle: 0x") << getProcessHandle();
	tstream << _T(" ] [ status: ") << getStatus();
	tstream << _T(" ] [ cmd: ") << getCommandLine();
	tstream << _T(" ] [ logger: ") << getLoggerInfo() << _T(" ] ") << flush;

	tstring t(tstream.str());
	return t;
}

tstring Process::getLoggerInfo() const {

	return logger->getInfo();
}

Process::~Process() {

	status = Finishing;
	log(tstring(_T("manually shutdowned")));
	onProcManualShutdown();

	PostThreadMessage(threadId, WM_QUIT, 0, 0);
	WaitForSingleObject(watchingThread, INFINITE);
	CloseHandle(watchingThread);

	WaitForSingleObject(generalEvent, INFINITE);
	CloseHandle(generalEvent);

	closeRoutine();

	delete[] commandLine;
}