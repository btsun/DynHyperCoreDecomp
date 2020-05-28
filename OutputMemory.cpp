#include <windows.h>
#include <psapi.h>
#include <iostream>
using namespace std;

double outputMemory() {
	HANDLE handle = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
	double mem = pmc.PeakWorkingSetSize / 1024.0 / 1024.0;
	cerr << "Memory: " << mem << "MB" << endl;
	return mem;
}
