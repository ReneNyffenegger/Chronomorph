#include "stdafx.h"
#include "CmdLine.h"
#include "LibInjector.h"
#include <Strsafe.h>
#include "ChronoModeEnum.h"

#define CHRONOMORPH_LIB L"ChronomorphLib.dll"

using namespace TCLAP;
using namespace std;

void StartChildProcess(const string& pathToExecutable)
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

	if (DebugMode)
	{
		wcout << L"About to inject lib at " << szDllToInject << endl;
		wcout << L"Targeting process " << szExeFile << endl;
	}

	InjectLibAtStartup(szExeFile, szDllToInject);
}

void InjectTimeOffset(const string& pathToExecutable, int timeOffset)
{
	// Add mode and timeoffset to env so child process can inherit it.
	//
	wstring modeCode = to_wstring(ChronoModeEnum::TimeOffset);
	SetEnvironmentVariable(CHRONO_MODE_ARG, modeCode.c_str());
	wstring inputArg = to_wstring(timeOffset);
	SetEnvironmentVariable(CHRONO_MODE_ARG_1, inputArg.c_str());

	if (DebugMode)
	{
		wcout << L"Injecting offset " << inputArg.c_str() << endl;
	}

	StartChildProcess(pathToExecutable);
}

void InjectTimeFlowRatio(const string& pathToExecutable, double timeFlowRatio)
{
	// Add mode and timeflow ratio to env so child process can inherit it.
	//
	wstring modeCode = to_wstring(ChronoModeEnum::TimeFlowRatio);
	SetEnvironmentVariable(CHRONO_MODE_ARG, modeCode.c_str());
	wstring inputArg = to_wstring(timeFlowRatio);
	SetEnvironmentVariable(CHRONO_MODE_ARG_1, inputArg.c_str());

	if (DebugMode)
	{
		wcout << L"Injecting time flow ratio " << inputArg.c_str() << endl;
	}

	StartChildProcess(pathToExecutable);
}

int main(int argc, char *argv[])
{
	CmdLine cmd("Time morpher", ' ' /* argument separator */);

	ValueArg<string> pathToExecutable("e", "executable", "Full path to the executable to me morphed", true, "Param is mandatory", "string");
	cmd.add(pathToExecutable);

	ValueArg<int> timeOffset("o", "timeOffset", "Time offset in seconds from current time.", false, 0, "int");
	cmd.add(timeOffset);

	ValueArg<double> timeFlowRatio("f", "timeFlowRatio", "Ratio that will slow down or speed up the target process (e.g. 2. means that time will go two times faster, 0.5 that it will go two times slower).", false, 1.0, "double");
	cmd.add(timeFlowRatio);

	// Shoud this be a switch?
	//
	ValueArg<bool> debugMode("d", "debugMode", "Extra printing for debugging.", false, false, "bool");
	cmd.add(debugMode);

	cmd.parse(argc, argv);

	DebugMode = debugMode.getValue();

	// TODO: Can't set two parameters at once.
	// TODO: TimeFlowRatio can't be 0.

	if (timeOffset.isSet())
	{
		InjectTimeOffset(pathToExecutable.getValue(), timeOffset.getValue());
	}
	else if (timeFlowRatio.isSet())
	{
		InjectTimeFlowRatio(pathToExecutable.getValue(), timeFlowRatio.getValue());
	}

    return 0;
}