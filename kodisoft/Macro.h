#ifndef MACRO_H
#define MACRO_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#ifdef _UNICODE

	#define tout           std::wcout
	#define tofstream      std::wofstream
	#define tstring	       std::wstring
	#define to_tstring     std::to_wstring
	#define tstringstream  std::wstringstream
#else
	#define tout	       std::cout
	#define tofstream      std::ofstream
	#define tstring	       std::string
	#define to_tstring     std::to_string
	#define tstringstream  std::stringstream

#endif // _UNICODE

#endif // MACRO_H