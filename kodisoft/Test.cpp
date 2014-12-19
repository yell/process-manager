#include <windows.h>
#include <tchar.h>
#include <cctype>
#include <iostream>

#include "Process.h"

using std::tolower;
using std::cout;
using std::endl;

void pause(char * str) {
	cout << str << endl;
	system("pause");
	cout << endl;
}

int main() {

	try{
		char c;
		DWORD pid;

		do{
			cout << "open Process by pid? [y/n]: " << std::flush;
			std::cin >> c;
		} while (!std::cin.fail() && tolower(c) != 'y' && tolower(c) != 'n');

		if (tolower(c) == 'y'){
			cout << "pid: " << std::flush;
			std::cin >> pid;
			Process p(pid);
			cout << endl;
			pause("try to crash this process and after that monitor will be closed \n(but process will be still working)");
		}

		Process q(tstring(_T("notepad")), true);

		pause("notepad should have been started");

		q.onProcStart.set([](){ cout << "\ncrash!" << std::flush; });
		q.onProcCrash = q.onProcStart;
		q.onProcStart.reset();
		pause("on crash it should print \"crash\" into the console");

		cout << "q.onProcStart.isEmpty(): " << q.onProcStart.isEmpty() << endl;
		cout << "q.onProcCrash.isEmpty(): " << q.onProcCrash.isEmpty() << endl;

		pause("");

		Logger * f = new FileLogger(tstring(_T("log2.txt")));
		q.switchLogger(f);

		pause("switched logger to log2.txt");

		q.resume();
		pause("an attempt to resume");

		q.stop();
		q.stop();
		pause("stopped (twice :D)");

		q.restart();
		pause("restarted. after that notepad will be closed");
	}

	catch (std::runtime_error & e) {
		std::cout << "Caught a runtime_error exception: " << e.what() << std::endl;
	}

	return 0;
}