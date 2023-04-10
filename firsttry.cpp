#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>

HANDLE hproc;
DWORD procid;
using namespace std;
boolean attachproc(const wchar_t* procname) {
	PROCESSENTRY32 proc32;
	proc32.dwSize = sizeof(PROCESSENTRY32);
	auto hprocsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hprocsnap == INVALID_HANDLE_VALUE) {
		printf("Failed  to take snapshot of process\n");
	}
	while (Process32Next(hprocsnap, &proc32)) {
		cout << "Currently opening " << proc32.szExeFile << endl;
		//remove Typecasting if program throws error
		if (!_wcsicmp(procname,proc32.szExeFile)) {
			cout << "Found PRocess " << proc32.szExeFile << " With process id " << proc32.th32ProcessID << endl;
			hproc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, proc32.th32ProcessID);
			procid = proc32.th32ProcessID;
			if (hproc == NULL) {
				cout << "Failed to get Handle " << endl;
			}
			CloseHandle(hprocsnap);
			return true;
		}

	}
	printf("Couldnt find proc name\n");
}
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	uintptr_t moduleBaseAddress = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 moduleEntry;
		moduleEntry.dwSize = sizeof(MODULEENTRY32);

		if (Module32First(hSnap, &moduleEntry))
		{
			do
			{
				if (!_wcsicmp(moduleEntry.szModule, modName))
				{
					moduleBaseAddress = (uintptr_t)moduleEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &moduleEntry));
		}
	}
	CloseHandle(hSnap);

	return moduleBaseAddress;
}
uintptr_t FindDma(HANDLE Hproc, uintptr_t ptr, vector<unsigned> offsets) {

	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i) {
		ReadProcessMemory(Hproc, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += offsets[i];
	}
	return addr;
}

wchar_t* myconverter(const char *c)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);

	return wc;
}

uintptr_t baser(DWORD procId, const wchar_t* modName) {
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_wcsicmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}
uintptr_t wpaddress(uintptr_t ptr) {
	uintptr_t wp;
	ReadProcessMemory(hproc, (BYTE*)ptr, &wp, sizeof(wp), NULL);
	wp += 0x150;
	return wp;
}


int main() {
	boolean mybool = attachproc(L"ac_client.exe");
	int x = 111;
	if (mybool == true) {
		printf("\nAttached successfully to proces\n");
	}
	else
		cout << "Couldnt attach to process" << endl;
	uintptr_t base = GetModuleBaseAddress(procid, L"ac_client.exe");
	base = base + 0x10F4F4;
	uintptr_t wpoffset = base + 0x150;
	vector<unsigned int> offset = { 0x374,0x14,0x0 };
	cout << "dynamic base address is : " <<"0x"<< hex << base << endl;
	uintptr_t weapon = wpaddress(base);
	cout << "Weapon address is " << "0x" << hex << weapon << endl;
	WriteProcessMemory(hproc, (byte*)weapon, &x, sizeof(int), NULL);
	Sleep(30000);
}