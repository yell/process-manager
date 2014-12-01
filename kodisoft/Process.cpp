#include <windows.h>
#include <tchar.h> // _T
#include <iostream>
#include <string>

#include "Process.h"

using std::endl;
using std::string;
using std::to_string;
using std::wstring;
using std::to_wstring;
using std::runtime_error;

void Process::log(LPTSTR str) const {

	tstring info = getInfo();
	LPTSTR t = &info[0];

	LPTSTR temp = new TCHAR[_tcsclen(str) + _tcsclen(t) + 1];

	_tcscpy(temp, str);
	lstrcat(temp, t);

	logger->log(temp);

	delete[] temp;
}

DWORD WINAPI Process::watchingThreadFunc(void * arg) {

	HANDLE h[3] = { ((Process*)arg)->processHandle, ((Process*)arg)->generalEvent, ((Process*)arg)->stopResumeEvent };

	while (1) {

		WaitForMultipleObjects(3, h, TRUE, INFINITE);

		if (!(((Process*)arg)->isStillActive())) {
			((Process*)arg)->onProcCrash();
			((Process*)arg)->restartRoutine();
			((Process*)arg)->log(_T("crash! restarting ..."));
		}
		SetEvent(((Process*)arg)->generalEvent);
		SetEvent(((Process*)arg)->stopResumeEvent);
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

		if (!CreateProcess(NULL, temp, NULL, NULL, FALSE, 0, NULL, NULL, &startInfo, &processInfo)) { // if CreateProcess (...) else log("bad news");
			delete[] temp;
			throw(runtime_error("\nProcess: unable to start process"));
		}
		
		processHandle = processInfo.hProcess;
		processID = processInfo.dwProcessId;

		status = IsWorking;
		onProcStart();
		log(_T("start!"));

		delete[] temp;
	}
}

//isStillActive
void Process::closeRoutine() {

	//INFINITE? 0?
	//std::cout << "wait:" << (WaitForSingleObject(processHandle, 1000) == WAIT_OBJECT_0);

	if (isStillActive()) {
		TerminateProcess(processHandle, 0);
		CloseHandle(processHandle);
	}
}

void Process::restartRoutine() {

	status = Restarting;
	closeRoutine();
	startRoutine();
}

Process::Process(LPTSTR cmd, Logger * logger) : logger(logger) {

	commandLine = new TCHAR[_tcslen(cmd) + 1];
	_tcscpy(commandLine, cmd);

	startRoutine();
	generalEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	stopResumeEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	watchingThread = CreateThread(NULL, 0, watchingThreadFunc, this, 0, NULL);
}

Process::Process(DWORD pid) : Process(pid, new FileLogger(_T("log.txt"))) {}

Process::Process(LPTSTR cmd) : Process(cmd, new FileLogger(_T("log.txt"))) {}

Process::Process(Logger * logger) : Process(_T("calc"), logger) {}

Process::Process() : Process(new FileLogger(_T("log.txt"))) {}

void Process::stop() {

	HANDLE h[2] = { generalEvent, stopResumeEvent };

	WaitForMultipleObjects(2, h, TRUE, INFINITE);

	if (status == IsWorking) {
		status = Stopped;
		log(_T("manual shutdown!"));
		onProcManuallyStopped();
		closeRoutine();
	}
	SetEvent(generalEvent);
}

void Process::resume() {

	WaitForSingleObject(generalEvent, INFINITE);
	if (status == Stopped) {

		startRoutine();

		status = IsWorking;
		onProcManuallyResumed();
	}
	SetEvent(generalEvent);
	SetEvent(stopResumeEvent);
}

void Process::restart() {

	WaitForSingleObject(generalEvent, INFINITE);
	restartRoutine();
	SetEvent(generalEvent);
}

DWORD Process::getProcessID() const { 
	bool b = isStillActive();
	return b ? processID : 0; 
}

HANDLE Process::getProcessHandle() const { 
	bool b = isStillActive();
	return b ? processHandle : 0; 
}

LPTSTR Process::getCommandLine() const { 
	return commandLine; 
}

LPTSTR Process::getStatus() const {
	static LPTSTR statusStr[] = { _T("is working"), _T("stopped"), _T("restarting") };
	return statusStr[status];
}

tstring Process::getInfo() const {
	tstringstream tstream;
	tstream << _T(" [ pid: ") << (getProcessID());
	tstream << _T(" ] [ handle: 0x") << getProcessHandle();
	tstream << _T(" ] [ status: ") << getStatus();
	tstream << _T(" ] [ cmd: ") << getCommandLine() << _T(" ] ") << std::flush;
	tstring t(tstream.str());
	return t;
}

Process::~Process() {

	//WaitForSingleObject() ?
	//if(commandLine != nullptr)

	log(_T("manual shutdown!"));

	TerminateThread(watchingThread, 0);
	CloseHandle(watchingThread);

	CloseHandle(generalEvent);
	CloseHandle(stopResumeEvent);
	closeRoutine();
	onProcManuallyShutdown();

	delete[] commandLine;
}