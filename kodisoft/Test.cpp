#include <tchar.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Process.h"

using std::cout;
using std::wcout;
using std::endl;
using std::string;
using std::ofstream;

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
//_CrtMemState S1, S2, S3;
//_CrtMemCheckpoint(&S2);
//
//if (_CrtMemDifference(&S3, &S1, &S2))
//_CrtMemDumpStatistics(&S3);

void m(const char * s) { std::cout << s << std::endl; }

int main() {

	try{
		/*string s("yo");
		wstring w(s.begin(), s.end());
		wcout << w << endl;*/
		
		/*Logger * p = new FileLogger (string("log.txt"));
		p->log(string("brand new msg"));
		delete p;*/
		
		//HANDLE h = CreateThread(NULL, 0, yo, NULL, 0, NULL);

	/*	Process *p[10];
		for (int i = 0; i < 10; i++)
			p[i] = new Process();*/

		/*Process p(q.getProcessID());
		Process r(q.getProcessID());*/

		Logger * l = new ConsoleLogger();
		l->log(tstring(_T("l::yo")));

		Process p(l, true);
		//std::getchar();
		//p.restart();

		//p.onProcStart.set([&p]() { p.stop(); });
		//p.onProcManualStop.set([&p]() { p.resume(); });

		//p.onProcManualShutdown = p.onProcCrash;

		//p.stop();

		//std::getchar();
		//p.stop();
		/*std::getchar();
		p.switchLogger();
		std::getchar();*/

		//Process flux(1900 ,new ConsoleLogger());
		//uTorrent.stop();
		//uTorrent.restart();
		//Process p(_T("calc"), new FileLogger(tstring(_T("log.txt"))));// (new ConsoleLogger());
		/*std::getchar();
		p.stop();
		p.stop();
		p.stop();
		std::getchar();
		p.resume();
		p.resume();*/
		
		//uTorrent.restart();
		//p.stop();
		//p.restart();
		//p.resume();

		//p.resume();
		//p.restart();
		
		
		//p.switchLogger(new FileLogger(tstring(_T("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.txt"))));

		//std::getchar();

	/*	for (int i = 0; i < 10; i++)
			delete p[i];
*/
		//CloseHandle(h);

	/*	for (int i = 0; i < 100; i++) {
			p.restart();
			Sleep(100);
		}*/
		//delete l;
	}
	catch (std::runtime_error & e) {
		std::cout << "Caught a runtime_error exception: " << e.what() << std::endl;
	}

	_CrtDumpMemoryLeaks();
	return 0;
}