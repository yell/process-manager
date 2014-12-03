#ifndef PROCESS_H
#define PROCESS_H

#include <windows.h>
#include <memory>
#include <set>

#include "Callback.h"
#include "Logger.h"
#include "OpenById.h"

using std::string;
using std::wstring;
using std::unique_ptr;
using std::set;

class Process {

	private:
		DWORD  processID;
		HANDLE processHandle;

		HANDLE watchingThread;
		HANDLE generalEvent;
		HANDLE stopResumeEvent;
		LPTSTR commandLine;

		enum Status { IsWorking, Stopped, Restarting } status;

		unique_ptr<Logger> logger;

		void log(LPTSTR) const;

		static DWORD WINAPI watchingThreadFunc(void *);

		bool isStillActive() const;
		void startRoutine();
		void closeRoutine();
		
		//static LPTSTR defaultPath;

		Process & operator = (const Process &);
		Process(const Process &);

	public:	
		Process(DWORD, Logger *);
		Process(LPTSTR, Logger *);
		Process(DWORD);
		Process(LPTSTR);
		Process(Logger *);
		Process();

		void stop();
		void resume();
		void restart();

		DWORD  getProcessID() const;
		HANDLE getProcessHandle() const;
		LPTSTR getCommandLine() const;
		LPTSTR getStatus() const;
		tstring getInfo() const;

		Callback onProcStart;
		Callback onProcCrash;
		Callback onProcManualStop;
		Callback onProcManualResume;
		Callback onProcManualRestart;
		Callback onProcManualShutdown;

		//switch Logger;

		~Process();
};

#endif // PROCESS_H