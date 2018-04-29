#include "stdafx.h"
#include "CmdLine.h"
#include "LibInjector.h"
#include <Strsafe.h>
#include "ChronoModeEnum.h"

#define CHRONOMORPH_LIB L"ChronomorphLib.dll"

using namespace TCLAP;
using namespace std;

void InjectTimeOffset(string& pathToExecutable, int timeOffset)
{
	wstring pathToExecutableWide = std::wstring(pathToExecutable.begin(), pathToExecutable.end());
	
	// Since c_str points to the const string and LibInjector requires non const we need to reallocate it in new buffer.
	//
	wchar_t szExeFile[MAX_PATH];
	StringCchCopy(szExeFile, MAX_PATH, pathToExecutableWide.c_str());

	// Now need to craft an absolute path to the lib to be injected.
	//
	wchar_t szPathname[MAX_PATH];
	GetModuleFileNameW(nullptr, szPathname, _countof(szPathname));
	wstring pathToLib = wstring(szPathname);
	pathToLib = pathToLib.substr(0, pathToLib.find_last_of(L"/\\"));
	wchar_t szDllToInject[MAX_PATH];
	StringCchPrintf(szDllToInject, MAX_PATH, L"%ls\\%ls", pathToLib.c_str(), CHRONOMORPH_LIB);

	// Add mode and timeoffset to env so child process can inherit it.
	//
	SetEnvironmentVariable(CHRONO_MODE_ARG, ChronoModeDesc[ChronoModeEnum::TimeOffset]);
	wstring inputArg = to_wstring(timeOffset);
	SetEnvironmentVariable(CHRONO_MODE_ARG_1, inputArg.c_str());

	if (DebugMode)
	{
		wcout << L"About to inject lib at " << szDllToInject << endl;
		wcout << L"Targeting process " << szExeFile << endl;
		wcout << L"Injecting offset " << inputArg.c_str() << endl;
	}

	InjectLibAtStartup(szExeFile, szDllToInject);
}

int main(int argc, char *argv[])
{
	CmdLine cmd("Time morpher", ' ' /* argument separator */);

	ValueArg<string> pathToExecutable("e", "executable", "Full path to the executable to me morphed", true, "Param is mandatory", "string");
	cmd.add(pathToExecutable);

	ValueArg<string> timeZoneToInject ("t", "timezone", "Name of timezone to use.", false, "UTC", "string");
	cmd.add(timeZoneToInject);

	ValueArg<int> timeOffset("o", "timeOffset", "Time offset in seconds from current time.", false, 0, "int");
	cmd.add(timeOffset);

	// Shoud this be a switch?
	//
	ValueArg<bool> debugMode("d", "debugMode", "Extra printing for debugging.", false, false, "bool");
	cmd.add(debugMode);

	cmd.parse(argc, argv);

	DebugMode = debugMode.getValue();

	if (timeOffset.isSet())
	{
		InjectTimeOffset(pathToExecutable.getValue(), timeOffset.getValue());
	}

    return 0;
}