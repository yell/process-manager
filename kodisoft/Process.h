#ifndef PROCESS_H
#define PROCESS_H

#include <windows.h>
#include <memory>

#include "Macro.h"
#include "Callback.h"
#include "Logger.h"
#include "OpenByPid.h"

using std::unique_ptr;

enum Status { IS_WORKING, STOPPED, RESTARTING, FINISHING };

class Process {

	private:
		static int count;
		bool killAtTheEnd;
		int monitorId;
		DWORD processId;
		DWORD threadId;	// for PostThreadMessage		
		HANDLE processHandle;
		HANDLE processMutex;
		HANDLE watchingThread;
		tstring commandLine;
		unique_ptr<Logger> logger;
		Status status;
		
		static DWORD WINAPI watchingThreadFunc(void *);
		bool isStillActive() const;
		void startRoutine();
		void closeRoutine();
		void log(tstring &) const;

		Process & operator = (const Process &);
		Process(const Process &);

	public:	
		Process(tstring &, Logger *, bool);
		Process(tstring &, Logger *);
		Process(Logger *, bool);
		Process(tstring &, bool);
		Process(tstring &);
		Process(Logger *);
		Process();
		Process(DWORD, Logger *, bool);
		Process(DWORD, Logger *);
		Process(DWORD, bool);
		Process(DWORD);

		void stop();
		void resume();
		void restart();
		void switchLogger(Logger *);
		void switchLogger();

		bool isKillAtTheEnd() const;
		int getMonitorId() const;
		DWORD getProcessId() const;
		HANDLE getProcessHandle() const;
		tstring getCommandLine() const;
		tstring getStatus() const;
		tstring getInfo() const;
		tstring getLoggerInfo() const;	

		Callback onProcStart;
		Callback onProcCrash;
		Callback onProcManualStop;
		Callback onProcManualResume;
		Callback onProcManualRestart;
		Callback onProcManualShutdown;

		~Process();
};

#endif // PROCESS_H