#include <windows.h>
#include <stdio.h>

#include <string>
#include <vector>

std::vector<std::string> targets = {
	"GDIPLUS.DLL", // OLDA, NEWA
	"GDI32.DLL", // OLDA, NEWA
	"icudt.dll", // OLDA
	"WINMM.dll", // OLDA
	"KERNEL32.dll", // OLDA, OLDW, NEWA, NEWW
	"USER32.dll", // OLDA, OLDW, NEWA
	"GDI32.dll", // OLDA, NEWA
	"ADVAPI32.dll", // OLDA, OLDW, NEWW
	"COMCTL32.dll", // OLDA, NEWA
	"dbghelp.dll", // OLDW
	"mscoree.dll", // OLDW, NEWS
	"combase.dll", // OLDW
	"api-ms-win-core-synch-l1-2-0.dll", // NEWW
	"api-ms-win-core-registry-l1-1-0.dll" // NEWW
};

int main(int argc, char **argv)
{
	if(argc > 1) {
		for(auto &v : targets) {
			CopyFile("probe.dll", v.c_str(), FALSE);
			HMODULE hModule = LoadLibrary(argv[1]);
			Sleep(1000);
			if(hModule) {
				FreeLibrary(hModule);
			}
			DeleteFile(v.c_str());
		}
	}
	return 0;
}
