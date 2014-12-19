#include <tchar.h>
#include <memory>
#include <iostream>

#include "Macro.h"
#include "OpenByPid.h"
#include "Process.h"

using std::runtime_error;
using std::unique_ptr;
using std::flush;

Process::Process(DWORD pid, Logger * logger, bool killAtTheEnd) : 
	 processId(pid), logger(logger), killAtTheEnd(killAtTheEnd), monitorId(++count) {

	#ifndef _UNICODE
		throw(runtime_error("\nProcess: Unable to retrieve command line in multi-byte character mode (use unicode instead)"));
	#endif

	processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, FALSE, pid);
	
	if (processHandle == nullptr)
		throw(runtime_error("\nProcess: OpenProcess failed"));

	SYSTEM_INFO si;
	GetNativeSystemInfo(&si); // determines whether 32 or 64-bit processor

	DWORD ProcessParametersOffset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x20 : 0x10;
	DWORD CommandLineOffset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x70 : 0x40;

	BOOL wow;
	IsWow64Process(GetCurrentProcess(), &wow);

	DWORD pebSize = ProcessParametersOffset + 8; // we only need the beginning of PEB structure to get ProcessParameters address 
	unique_ptr<BYTE> peb(new BYTE [pebSize]);
	ZeroMemory(&(*peb), pebSize);

	DWORD ppSize = CommandLineOffset + 16; // again, we only need the beginning of ProcessParameters to get commandLine address
	unique_ptr<BYTE> pp(new BYTE [ppSize]);
	ZeroMemory(&(*pp), ppSize);

	unique_ptr<TCHAR> cmd;

	if (wow) { // we're running as a 32-bit process in a 64-bit OS
		
		PROCESS_BASIC_INFORMATION_WOW64 pbi;
		ZeroMemory(&pbi, sizeof(pbi));

		// gets process information from 64-bit world :
		_NtQueryInformationProcess query = (_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64QueryInformationProcess64");	
		if (query(processHandle, 0, &pbi, sizeof(pbi), NULL)) {
			CloseHandle(processHandle);
			throw(runtime_error("\nProcess: NtWow64QueryInformationProcess64 failed"));
		}

		// reads PEB from 64-bit address space :
		_NtWow64ReadVirtualMemory64 read = (_NtWow64ReadVirtualMemory64)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64ReadVirtualMemory64");
		if (read(processHandle, pbi.PebBaseAddress, &(*peb), pebSize, NULL)) {
			CloseHandle(processHandle);
			throw(runtime_error("\nProcess: NtWow64ReadVirtualMemory64 PEB failed"));
		}

		// reads ProcessParameters from 64-bit address space :
		PBYTE* parameters = (PBYTE*)*(LPVOID*)(&(*peb) + ProcessParametersOffset); // <- address in remote process adress space
		if (read(processHandle, parameters, &(*pp), ppSize, NULL)) {
			CloseHandle(processHandle);
			throw(runtime_error("\nProcess: NtWow64ReadVirtualMemory64 Parameters failed"));
		}

		UNICODE_STRING_WOW64* pCommandLine = (UNICODE_STRING_WOW64*)(&(*pp) + CommandLineOffset); // reads command line
		cmd.reset(new TCHAR[pCommandLine->MaximumLength + 1]);
		if (read(processHandle, pCommandLine->Buffer, &(*cmd), pCommandLine->MaximumLength, NULL)) {
			CloseHandle(processHandle);
			throw(runtime_error("\nProcess: NtWow64ReadVirtualMemory64 Parameters failed"));
		}
	} 
	else { // we're running as a 32-bit process in a 32-bit OS, or as a 64-bit process in a 64-bit OS

		PROCESS_BASIC_INFORMATION pbi;
		ZeroMemory(&pbi, sizeof(pbi));

		// gets process information :
		_NtQueryInformationProcess query = (_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");	
		if (query(processHandle, 0, &pbi, sizeof(pbi), NULL)) {
			CloseHandle(processHandle);
			throw(runtime_error("\nProcess: NtQueryInformationProcess failed"));
		}

		if (!ReadProcessMemory(processHandle, pbi.PebBaseAddress, &(*peb), pebSize, NULL)) { // reads PEB
			CloseHandle(processHandle);
			throw(runtime_error("\nProcess: ReadProcessMemory PEB failed"));
		}

		// read ProcessParameters :
		PBYTE* parameters = (PBYTE*)*(LPVOID*)(&(*peb) + ProcessParametersOffset); // <- address in remote process adress space
		if (!ReadProcessMemory(processHandle, parameters, &(*pp), ppSize, NULL)) {
			CloseHandle(processHandle);
			throw(runtime_error("\nProcess: ReadProcessMemory Parameters failed"));
		}

		UNICODE_STRING* pCommandLine = (UNICODE_STRING*)(&(*pp) + CommandLineOffset); // reads command line
		
		cmd.reset(new TCHAR[pCommandLine->MaximumLength + 1]);

		if (!ReadProcessMemory(processHandle, pCommandLine->Buffer, &(*cmd), pCommandLine->MaximumLength, NULL)) {
			CloseHandle(processHandle);
			throw(runtime_error("\nProcess: ReadProcessMemory Parameters failed"));
		}
	}

	tstringstream tstream;
	tstream << &(*cmd) << flush;
	commandLine = tstream.str();

	status = IS_WORKING;
	processMutex = CreateMutex(NULL, FALSE, NULL);
	watchingThread = CreateThread(NULL, 0, watchingThreadFunc, this, 0, NULL);
	log(tstring(_T("started")));
}