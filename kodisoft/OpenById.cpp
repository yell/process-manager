#include <tchar.h>

#include <memory>

#include "OpenById.h"
#include "Process.h"

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//_CrtMemState S1, S2, S3;
//_CrtMemCheckpoint(&S2);
//if (_CrtMemDifference(&S3, &S1, &S2))
//_CrtMemDumpStatistics(&S3);

using std::runtime_error;
using std::unique_ptr;

Process::Process(DWORD pid, Logger * logger) : processID(pid), logger(logger), id(++count) {

	// open the process
	DWORD err = 0;
	processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	
	if (processHandle == nullptr)
		throw(runtime_error("\nProcess: OpenProcess failed"));

	// determine if 64 or 32-bit processor
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);

	// determine if this process is running on WOW64
	BOOL wow;
	IsWow64Process(GetCurrentProcess(), &wow);

	// use WinDbg "dt ntdll!_PEB" command and search for ProcessParameters offset to find the truth out
	DWORD ProcessParametersOffset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x20 : 0x10;
	DWORD CommandLineOffset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x70 : 0x40;

	// read basic info to get ProcessParameters address, we only need the beginning of PEB
	DWORD pebSize = ProcessParametersOffset + 8;
	unique_ptr<BYTE> peb(new BYTE [pebSize]);
	ZeroMemory(&(*peb), pebSize);

	// read basic info to get CommandLine address, we only need the beginning of ProcessParameters
	DWORD ppSize = CommandLineOffset + 16;
	unique_ptr<BYTE> pp(new BYTE [ppSize]);
	ZeroMemory(&(*pp), ppSize);

	if (wow) {
		// we're running as a 32-bit process in a 64-bit OS
		PROCESS_BASIC_INFORMATION_WOW64 pbi;
		ZeroMemory(&pbi, sizeof(pbi));

		// get process information from 64-bit world
		_NtQueryInformationProcess query = (_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64QueryInformationProcess64");
		err = query(processHandle, 0, &pbi, sizeof(pbi), NULL);
		
		if (err != 0) {
			CloseHandle(processHandle);
			throw(runtime_error("\nProcess: NtWow64QueryInformationProcess64 failed"));
		}

		// read PEB from 64-bit address space
		_NtWow64ReadVirtualMemory64 read = (_NtWow64ReadVirtualMemory64)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64ReadVirtualMemory64");
		err = read(processHandle, pbi.PebBaseAddress, &(*peb), pebSize, NULL);
		if (err != 0) {
			CloseHandle(processHandle);
			throw(runtime_error("\nProcess: NtWow64ReadVirtualMemory64 PEB failed"));
		}

		// read ProcessParameters from 64-bit address space
		PBYTE* parameters = (PBYTE*)*(LPVOID*)(&(*peb) + ProcessParametersOffset); // address in remote process adress space
		err = read(processHandle, parameters, &(*pp), ppSize, NULL);
		if (err != 0) {
			CloseHandle(processHandle);
			throw(runtime_error("\nProcess: NtWow64ReadVirtualMemory64 Parameters failed"));
		}

		// read CommandLine
		UNICODE_STRING_WOW64* pCommandLine = (UNICODE_STRING_WOW64*)(&(*pp) + CommandLineOffset);
		
		commandLine = new TCHAR[pCommandLine->MaximumLength + 1];

		err = read(processHandle, pCommandLine->Buffer, commandLine, pCommandLine->MaximumLength, NULL);
		if (err != 0) {
			CloseHandle(processHandle);
			throw(runtime_error("\nProcess: NtWow64ReadVirtualMemory64 Parameters failed"));
		}
	} 
	else {
		// we're running as a 32-bit process in a 32-bit OS, or as a 64-bit process in a 64-bit OS
		PROCESS_BASIC_INFORMATION pbi;
		ZeroMemory(&pbi, sizeof(pbi));

		// get process information
		_NtQueryInformationProcess query = (_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
		err = query(processHandle, 0, &pbi, sizeof(pbi), NULL);
		if (err != 0) {
			CloseHandle(processHandle);
			throw(runtime_error("\nProcess: NtQueryInformationProcess failed"));
		}

		// read PEB
		if (!ReadProcessMemory(processHandle, pbi.PebBaseAddress, &(*peb), pebSize, NULL)) {
			CloseHandle(processHandle);
			throw(runtime_error("\nProcess: ReadProcessMemory PEB failed"));
		}

		// read ProcessParameters
		PBYTE* parameters = (PBYTE*)*(LPVOID*)(&(*peb) + ProcessParametersOffset); // address in remote process adress space
		if (!ReadProcessMemory(processHandle, parameters, &(*pp), ppSize, NULL)) {
			CloseHandle(processHandle);
			throw(runtime_error("\nProcess: ReadProcessMemory Parameters failed"));
		}

		// read CommandLine
		UNICODE_STRING* pCommandLine = (UNICODE_STRING*)(&(*pp) + CommandLineOffset);
		
		commandLine = new TCHAR[pCommandLine->MaximumLength + 1];

		if (!ReadProcessMemory(processHandle, pCommandLine->Buffer, commandLine, pCommandLine->MaximumLength, NULL)) {
			CloseHandle(processHandle);
			throw(runtime_error("\nProcess: ReadProcessMemory Parameters failed"));
		}
	}

	status = IsWorking;
	generalEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	stopResumeEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	watchingThread = CreateThread(NULL, 0, watchingThreadFunc, this, 0, NULL);
	log(_T("started ..............."));
}