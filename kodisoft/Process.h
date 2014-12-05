#ifndef PROCESS_H
#define PROCESS_H

#include <windows.h>
#include <memory>

#include "Macro.h"
#include "Callback.h"
#include "Logger.h"
#include "OpenById.h"

using std::unique_ptr;

class Process {

	private:
		static int count;

		int id;
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

		Process & operator = (const Process &);
		Process(const Process &);

	public:	
		Process(DWORD, Logger *);
		Process(LPTSTR, Logger *);
		Process(DWORD);
		Process(LPTSTR);
		Process(Logger *);
		Process();

		void switchLogger(Logger *);

		void stop();
		void resume();
		void restart();

		int getId() const;
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

		~Process();
};

#endif // PROCESS_H