/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#ifdef _MSC_VER
#include <minwindef.h>
#include <string>
#include <processthreadsapi.h>
#include <handleapi.h>
#include <Windows.h>
#include <tchar.h>
#include <Psapi.h>
#endif

namespace SystemFunctions
{
#ifdef _MSC_VER
	// Returns true if a process with _processID has name _processName.
	inline bool MatchProcessName(DWORD _processID, const std::wstring& _processName)
	{
		TCHAR szProcessName[MAX_PATH] = TEXT("<noname>");

		// get a handle to the process
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, _processID);

		// get the process name
		if (hProcess)
		{
			HMODULE hModule;
			DWORD memoryNeeded;
			if (EnumProcessModules(hProcess, &hModule, sizeof(hModule), &memoryNeeded))
				GetModuleBaseName(hProcess, hModule, szProcessName,	sizeof(szProcessName) / sizeof(TCHAR));
		}

		const bool found = !_tcscmp(szProcessName, _processName.c_str());
		CloseHandle(hProcess);
		return found;
	}
#endif

#ifdef _MSC_VER
	/// Returns number of running process with the specified name.
	inline size_t ActiveInstancesCount(const std::wstring& _processName)
	{
		DWORD processes[1024], memoryNeeded;
		if (!EnumProcesses(processes, sizeof(processes), &memoryNeeded))
			return 1;

		// calculate how many process identifiers were found
		const DWORD totalProcessesFound = memoryNeeded / sizeof(DWORD);

		// count processes with the needed name
		size_t processesCount = 0;
		for (DWORD i = 0; i < totalProcessesFound; ++i)
			if (processes[i] != 0 && MatchProcessName(processes[i], _processName))
				processesCount++;

		return processesCount;
	}
#endif
}
