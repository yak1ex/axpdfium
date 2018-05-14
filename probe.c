#include <windows.h>
#include <stdio.h>

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	char buf[4096];
	FILE *fp;
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		GetModuleFileName(hModule, buf, sizeof(buf));
		lstrcat(buf, ".txt");
		fp = fopen(buf, "w");
		fclose(fp);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
