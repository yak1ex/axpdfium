/***********************************************************************/
/*                                                                     */
/* axpdfium.cpp: Source file for axpdfium                              */
/*                                                                     */
/*     Copyright (C) 2015 Yak! / Yasutaka ATARASHI                     */
/*                                                                     */
/*     This software is distributed under the terms of a zlib/libpng   */
/*     License.                                                        */
/*                                                                     */
/*     $Id: 61260feeb364cbc8e9b6daea239472ade36e8dc8 $                 */
/*                                                                     */
/***********************************************************************/
#include <windows.h>
#include <commctrl.h>

#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <cstdio>

#include <sys/stat.h>

#include "Spi_api.h"
#include "resource.h"
#ifdef DEBUG
#include <iostream>
#include <iomanip>
#include "odstream.hpp"
#define DEBUG_LOG(ARG) yak::debug::ods ARG
#else
#define DEBUG_LOG(ARG) do { } while(0)
#endif

typedef std::pair<std::string, unsigned long> Key;
typedef std::vector<char> Data;
typedef std::pair<std::vector<SPI_FILEINFO>, std::vector<Data> > Value;
std::map<Key, Value> g_cache;

// Force null terminating version of strncpy
// Return length without null terminator
int safe_strncpy(char *dest, const char *src, std::size_t n)
{
	size_t i = 0;
	while(i<n && *src) { *dest++ = *src++; ++i; }
	*dest = 0;
	return i;
}

static HINSTANCE g_hInstance;

static std::string g_sExtension = "*.pdf";

const char* table[] = {
	"00AM",
	"A Susie plugin for PDF with PDFium rendering engine - v0.01 (2015/01/05) Written by Yak!",
	"*.pdf",
	"PDFƒtƒ@ƒCƒ‹"
};

INT PASCAL GetPluginInfo(INT infono, LPSTR buf, INT buflen)
{
	DEBUG_LOG(<< "GetPluginInfo(" << infono << ',' << buf << ',' << buflen << ')' << std::endl);
	if(0 <= infono && static_cast<size_t>(infono) < sizeof(table)/sizeof(table[0])) {
		return safe_strncpy(buf, infono == 2 ? g_sExtension.c_str() : table[infono], buflen);
	} else {
		return 0;
	}
}

static INT IsSupportedImp(LPSTR filename, LPBYTE pb)
{
	std::string name(filename);
	const char* start = g_sExtension.c_str();
	while(1) {
		while(*start && *start != '.') {
			++start;
		}
		if(!*start) break;
		const char* end = start;
		while(*end && *end != ';') {
			++end;
		}
		std::string ext(start, end);
		if(name.size() > ext.size() && !lstrcmpi(name.substr(name.size() - ext.size()).c_str(), ext.c_str())) {
			return SPI_SUPPORT_YES;
		}
		++start;
	}
	return SPI_SUPPORT_NO;
}

INT PASCAL IsSupported(LPSTR filename, DWORD dw)
{
	DEBUG_LOG(<< "IsSupported(" << filename << ',' << std::hex << std::setw(8) << std::setfill('0') << dw << ')' << std::endl);
	if(HIWORD(dw) == 0) {
		DEBUG_LOG(<< "File handle" << std::endl);
		BYTE pb[2048];
		DWORD dwSize;
		ReadFile((HANDLE)dw, pb, sizeof(pb), &dwSize, NULL);
		return IsSupportedImp(filename, pb);;
	} else {
		DEBUG_LOG(<< "Pointer" << std::endl); // By afx
		return IsSupportedImp(filename, (LPBYTE)dw);;
	}
	// not reached here
}

// Currently, 24 bits BMP is forced
static unsigned long bmpsize(DWORD dwWidth, DWORD dwHeight)
{
	return ((dwWidth * 3 + 3) / 4 * 4) * dwHeight + 54;
}

static unsigned long filesize(const char* filename)
{
	struct stat st;
	stat(filename, &st);
	return st.st_size;
}

static Key make_key(const char* filename)
{
	return std::make_pair(std::string(filename), filesize(filename));
}

static INT SetArchiveInfo(const std::vector<SPI_FILEINFO> &v, HLOCAL *lphInf)
{
	std::size_t size = v.size() * sizeof(SPI_FILEINFO);
	std::size_t tsize = size + sizeof(SPI_FILEINFO); // for terminator
	*lphInf = LocalAlloc(LMEM_MOVEABLE, tsize);
	LPVOID pv = LocalLock(*lphInf);
	ZeroMemory(pv, tsize);
	CopyMemory(pv, &v[0], size);
	LocalUnlock(*lphInf);
	return SPI_ERR_NO_ERROR;
}

static time_t filetime(const char* filename)
{
	struct stat st;
	stat(filename, &st);
	return st.st_mtime;
}

void SetErrorImage(std::vector<char> &v2)
{
	DEBUG_LOG(<< "SetErrorImage(" << v2.size() << ')' << std::endl);
	HRSRC hResource = FindResource(g_hInstance, MAKEINTRESOURCE(IDR_ERROR_IMAGE), RT_RCDATA);
	DWORD dwSize = SizeofResource(g_hInstance, hResource);
	HGLOBAL handle = LoadResource(g_hInstance, hResource);
	char* pErrorImage = static_cast<char*>(LockResource(handle));
	v2.assign(pErrorImage, pErrorImage + dwSize);
}

void SetArchiveInfo(std::vector<SPI_FILEINFO> &v1, DWORD dwSize, DWORD dwPos, DWORD timestamp)
{
	SPI_FILEINFO sfi = {
		{ 'P', 'D', 'F', 'i', 'u', 'm', '\0', '\0' },
		dwPos,
		dwSize,
		dwSize,
		timestamp,
	};
	wsprintf(sfi.filename, "%08d.bmp", v1.size());
	v1.push_back(sfi);
}

// from ax7z with modification
INT_PTR CALLBACK ProgressDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hDlgWnd, DWLP_USER, 0);
		return FALSE;
	case WM_COMMAND:
		if (HIWORD(wp) == BN_CLICKED && LOWORD(wp) == IDCANCEL)
			SetWindowLongPtr(hDlgWnd, DWLP_USER, 1);
		break;

	}
	return FALSE;
}

void SetProgress(HWND hwnd, int nNum, int nDenom)
{
	SendDlgItemMessage(hwnd, IDC_PROGRESSBAR, PBM_SETRANGE32, 0, nDenom);
	SendDlgItemMessage(hwnd, IDC_PROGRESSBAR, PBM_SETPOS, nNum, 0);
	char buf[1024];
	wsprintf(buf, "%d / %d", nNum, nDenom);
	SendDlgItemMessage(hwnd, IDC_PROGRESSTEXT, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(buf));
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

extern void ProcessPDF(std::string filename, std::vector<SPI_FILEINFO> &v1, std::vector<std::vector<char> > &v2, DWORD timestamp, HWND hwnd);

static void GetArchiveInfoImp(std::vector<SPI_FILEINFO> &v1, std::vector<std::vector<char> > &v2, LPSTR filename)
{
	DEBUG_LOG(<< "GetArchiveInfoImp(" << v1.size() << ',' << v2.size() << ',' << filename << ')' << std::endl);

	DWORD timestamp = filetime(filename);
	HWND hDlg = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROGRESS_DIALOG), NULL, ProgressDlgProc);
	ProcessPDF(filename, v1, v2, timestamp, hDlg);
	DestroyWindow(hDlg);
}

static INT GetArchiveInfoImp(HLOCAL *lphInf, LPSTR filename)
{
	DEBUG_LOG(<< "GetArchiveInfoImp(" << filename << ')' << std::endl);

	if(filename != NULL) {
		DEBUG_LOG(<< "GetArchiveInfoImp - Filename specified: check cache" << std::endl);
		Key key(make_key(filename));
		if(g_cache.count(key) != 0) {
			if(lphInf) SetArchiveInfo(g_cache[key].first, lphInf);
			return SPI_ERR_NO_ERROR;
		}
	}

	try {
		std::vector<SPI_FILEINFO> v1;
		std::vector<std::vector<char> > v2;
		GetArchiveInfoImp(v1, v2, filename);

		if(lphInf) SetArchiveInfo(v1, lphInf);

		if(filename != NULL) {
			DEBUG_LOG(<< "GetArchiveInfoImp - Filename specified: set cache" << std::endl);
			Key key(make_key(filename));
			g_cache[key].first.swap(v1);
			g_cache[key].second.swap(v2);
		}
	} catch(std::exception &e) {
		MessageBox(NULL, e.what(), "axpdfium.spi", MB_OK);
		return SPI_ERR_BROKEN_DATA;
	} catch(...) {
		return SPI_ERR_BROKEN_DATA;
	}

	return SPI_ERR_NO_ERROR;
}

INT PASCAL GetArchiveInfo(LPSTR buf, LONG len, UINT flag, HLOCAL *lphInf)
{
	DEBUG_LOG(<< "GetArchiveInfo(" << std::string(buf, std::min<DWORD>(len, 1024)) << ',' << len << ',' << std::hex << std::setw(8) << std::setfill('0') << flag << ',' << lphInf << ')' << std::endl);
	switch(flag & 7) {
	case 0:
		DEBUG_LOG(<< "File" << std::endl); // By afx
		return GetArchiveInfoImp(lphInf, buf);
	case 1:
		DEBUG_LOG(<< "Memory" << std::endl);
		return SPI_ERR_NOT_IMPLEMENTED;
	}
	return SPI_ERR_INTERNAL_ERROR;
}

INT PASCAL GetFileInfo(LPSTR buf, LONG len, LPSTR filename, UINT flag, SPI_FILEINFO *lpInfo)
{
	DEBUG_LOG(<< "GetFileInfo(" << std::string(buf, std::min<DWORD>(len, 1024)) << ',' << len << ',' << filename << ','<< std::hex << std::setw(8) << std::setfill('0') << flag << ',' << lpInfo << ')' << std::endl);
	if(flag & 128) {
		DEBUG_LOG(<< "Case-insensitive" << std::endl); // By afx
	} else {
		DEBUG_LOG(<< "Case-sensitive" << std::endl);
	}
	switch(flag & 7) {
	case 0:
		DEBUG_LOG(<< "File" << std::endl); // By afx
		break;
	case 1:
		DEBUG_LOG(<< "Memory" << std::endl);
		break;
	}
	HLOCAL hInfo;
	if(GetArchiveInfo(buf, len, flag&7, &hInfo) == SPI_ERR_NO_ERROR) {
		int (WINAPI *compare)(LPCSTR, LPCSTR) = flag&7 ? lstrcmpi : lstrcmp;
		std::size_t size = LocalSize(hInfo);
		SPI_FILEINFO *p = static_cast<SPI_FILEINFO*>(LocalLock(hInfo)), *cur = p;
		std::size_t count = size / sizeof(SPI_FILEINFO);
		while(cur < p + count && *static_cast<char*>(static_cast<void*>(cur)) != '\0') {
			if(!compare(cur->filename, filename)) {
				CopyMemory(lpInfo, cur, sizeof(SPI_FILEINFO));
				LocalUnlock(hInfo);
				LocalFree(hInfo);
				return SPI_ERR_NO_ERROR;
			}
			++cur;
		}
		LocalUnlock(hInfo);
		LocalFree(hInfo);
	}
	return SPI_ERR_INTERNAL_ERROR;
}

INT PASCAL GetFile(LPSTR buf, LONG len, LPSTR dest, UINT flag, FARPROC prgressCallback, LONG lData)
{
	DEBUG_LOG(<< "GetFile(" << ',' << len << ',' << ',' << flag << ',' << prgressCallback << ',' << lData << ')' << std::endl);
	switch(flag & 7) {
	case 0:
	  {
		DEBUG_LOG(<< "Source is file" << std::endl); // By afx
		break;
	  }
	case 1:
		DEBUG_LOG(<< "Source is memory" << std::endl);
		return SPI_ERR_NOT_IMPLEMENTED; // We cannot handle this case.
	}
	switch((flag>>8) & 7) {
	case 0:
		DEBUG_LOG(<< "Destination is file: " << dest << std::endl);
		break;
	case 1:
		DEBUG_LOG(<< "Destination is memory: " << static_cast<void*>(dest) << std::endl); // By afx
		break;
	}
	if(GetArchiveInfo(buf, len, flag&7, 0) == SPI_ERR_NO_ERROR) {
		DEBUG_LOG(<< "GetFile(): GetArvhiveInfo() returned" << std::endl);
		Key key(make_key(buf));
		if(g_cache.count(key) == 0) {
			DEBUG_LOG(<< "GetFile(): " << buf << " not cached" << std::endl);
			return SPI_ERR_INTERNAL_ERROR;
		}
		Value &value = g_cache[key];
		std::size_t size = value.first.size();
		for(std::size_t i = 0; i < size; ++i) {
			if(value.first[i].position == std::size_t(len)) {
				DEBUG_LOG(<< "GetFile(): position found" << std::endl);
				if((flag>>8) & 7) { // memory
					if(dest) {
						DEBUG_LOG(<< "GetFile(): size: " << value.second[i].size() << " head: " << value.second[i][0] << value.second[i][1] << value.second[i][2] << value.second[i][3] << std::endl);
						HANDLE *phResult = static_cast<HANDLE*>(static_cast<void*>(dest));
						*phResult = LocalAlloc(LMEM_MOVEABLE, value.second[i].size());
						void* p = LocalLock(*phResult);
						CopyMemory(p, &value.second[i][0], value.second[i].size());
						LocalUnlock(*phResult);
						return SPI_ERR_NO_ERROR;
					}
					return SPI_ERR_INTERNAL_ERROR;
				} else { // file
					std::string s(dest);
					s += '\\';
					s += value.first[i].filename;
					FILE *fp = std::fopen(s.c_str(), "wb");
					fwrite(&value.second[i][0], value.second[i].size(), 1, fp);
					fclose(fp);
					return SPI_ERR_NO_ERROR;
				}
			}
		}
	}
	DEBUG_LOG(<< "GetFile(): position not found" << std::endl);
	return SPI_ERR_INTERNAL_ERROR;
}

static LRESULT CALLBACK AboutDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
			EndDialog(hDlgWnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hDlgWnd, IDCANCEL);
			break;
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
	return TRUE;
}

INT PASCAL ConfigurationDlg(HWND parent, INT fnc)
{
	DEBUG_LOG(<< "ConfigurationDlg called" << std::endl);
	if (fnc == 0) { // About
		DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT_DIALOG), parent, (DLGPROC)AboutDlgProc);
	} else { // Configuration
		DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT_DIALOG), parent, (DLGPROC)AboutDlgProc);
	}
	return SPI_ERR_NO_ERROR;
}

extern void InitPDFium();
extern void FreePDFium();

extern "C" BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	INITCOMMONCONTROLSEX ice = { sizeof(INITCOMMONCONTROLSEX), ICC_PROGRESS_CLASS };
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		g_hInstance = (HINSTANCE)hModule;
		InitCommonControlsEx(&ice);
		InitPDFium();
		break;
	case DLL_PROCESS_DETACH:
		FreePDFium();
		break;
	}
	return TRUE;
}
