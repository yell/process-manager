#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <iomanip>

#include "Macro.h"
#include "Process.h"

using std::endl;
using std::setw;
using std::setfill;
using std::runtime_error;


int Process::count;

static LPTSTR msgs[] = { _T("process is being watched"), 
						 _T("shutdowned ............"),
						 _T("manually stopped ......"),
						 _T("manually resumed ......"),    
						 _T("crashed. restarting ..."),   
						 _T("restarted after crash ."),   
						 _T("manually restarting ......................"),
						 _T("manually restarted ......................."),
						 _T("logger has been switched"),
						 _T("(mogla bit v drugom loggere)"),
						 _T("an attempt to stop already stopped process"),
						 _T("an attempt to resume active process......."),
}; 

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

			((Process*)arg)->status = Restarting;
			((Process*)arg)->log(msgs[4]);
			((Process*)arg)->onProcCrash();

			((Process*)arg)->closeRoutine();
			((Process*)arg)->startRoutine();
			
			((Process*)arg)->status = IsWorking;
			((Process*)arg)->log(msgs[5]);
			((Process*)arg)->onProcStart();
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

	LPTSTR temp = new TCHAR[_tcslen(commandLine) + 3];
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

void Process::closeRoutine() {

	if (isStillActive()) {
		TerminateProcess(processHandle, 0);
		CloseHandle(processHandle);
	}
}

Process::Process(LPTSTR cmd, Logger * logger) : logger(logger), id(++count) {

	commandLine = new TCHAR[_tcslen(cmd) + 3]; // +2 for additional "" 's
	commandLine[0] = _T('"'); 
	_tcscpy(commandLine + 1, cmd);
	lstrcat(commandLine, _T("\""));	

	startRoutine();
	generalEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	stopResumeEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	watchingThread = CreateThread(NULL, 0, watchingThreadFunc, this, 0, NULL);
	log(msgs[0]);
	
	//activeProcPids.insert(processID);
	//printSet();
}

Process::Process(DWORD pid) : Process(pid, new FileLogger(_T("log.txt"))) {}

Process::Process(LPTSTR cmd) : Process(cmd, new FileLogger(_T("log.txt"))) {}

Process::Process(Logger * logger) : Process(_T("calc"), logger) {}

Process::Process() : Process(new FileLogger(_T("log.txt"))) {}

void Process::switchLogger(Logger * logger) {
	log(msgs[8]);
	this->logger.reset(logger);
	log(msgs[9]);
}

void Process::stop() {

	WaitForSingleObject(generalEvent, INFINITE);

	if (status == IsWorking) {

		WaitForSingleObject(stopResumeEvent, INFINITE);
		status = Stopped;
		log(msgs[2]);
		onProcManualStop();
		closeRoutine();
		//processID = 0;
		//processHandle = 0;
		//SetEvent(Event);
	}
	else
		log(msgs[10]);

	SetEvent(generalEvent);
}

void Process::resume() {

	WaitForSingleObject(generalEvent, INFINITE);

	if (status == Stopped) {

		//WaitForSingleObject(stopResumeEvent, INFINITE);
		startRoutine();
		status = IsWorking;
		log(msgs[3]);
		onProcManualResume();
		SetEvent(stopResumeEvent);
	}
	else
		log(msgs[11]);

	SetEvent(generalEvent);
}

void Process::restart() {

	WaitForSingleObject(generalEvent, INFINITE);
	
	status = Restarting;
	log(msgs[6]);
	onProcManualRestart();
	closeRoutine();
	startRoutine();
	status = IsWorking;
	log(msgs[7]);
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

LPTSTR Process::getStatus() const {
	static LPTSTR statusStr[] = { _T("is working"), 
								  _T("is stopped"), 
								  _T("restarting") };
	return statusStr[status];
}

tstring Process::getInfo() const {
	tstringstream tstream;
	tstream << _T(" [ id: ") << setfill(_T(' ')) << setw(4) << getId();
	tstream << _T(" ] [ pid: ") << setfill(_T(' ')) << setw(6) << getProcessID();
	tstream << _T(" ] [ handle: 0x") << getProcessHandle();
	tstream << _T(" ] [ status: ") << getStatus();
	tstream << _T(" ] [ cmd: ") << getCommandLine() << _T(" ] ") << std::flush;
	tstring t(tstream.str());
	return t;
}

Process::~Process() {

	log(msgs[1]);

	//WaitForSingleObject(watchingThread, INFINITE);
	TerminateThread(watchingThread, 0);
	CloseHandle(watchingThread);

	//WaitForSingleObject(generalEvent, INFINITE);
	CloseHandle(generalEvent);
	//WaitForSingleObject(stopResumeEvent, INFINITE);
	CloseHandle(stopResumeEvent);
	closeRoutine();
	onProcManualShutdown();

	delete[] commandLine;
}