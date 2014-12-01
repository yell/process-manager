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

//template< typename log_policy >
//std::string logger< log_policy >::get_logline_header()
//{
//	std::stringstream header;
//	header.str("");
//	header.fill('0');
//	header.width(7);
//	header << log_line_number++ << " < " << get_time() << " - ";
//	header.fill('0');
//	header.width(7);
//	header << clock() << " > ~ ";
//	return header.str();
//}

void m(const char * s) { std::cout << s << std::endl; }


int main() {

	try{
		/*string s("yo");
		wstring w(s.begin(), s.end());
		wcout << w << endl;*/
		
		/*Logger * p = new FileLogger (string("log.txt"));
		p->log(string("brand new msg"));
		delete p;*/

		Process p;
		std::getchar();
	}
	catch (std::runtime_error & e) {
		std::cout << "Caught a runtime_error exception: " << e.what() << std::endl;
	}

	_CrtDumpMemoryLeaks();
	return 0;
}