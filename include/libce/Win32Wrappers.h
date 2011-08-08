#ifndef _H_WIN32WRAPPERS
#define _H_WIN32WRAPPERS

#include <windows.h>

#define GetFileAttributesA _GetFileAttributesA
#define CreateDirectoryA _CreateDirectoryA
#define FindNextFileA _FindNextFileA
#define FindFirstFileA _FindFirstFileA

#ifdef __cplusplus
extern "C" {
#endif

DWORD _GetFileAttributesA(LPCSTR lpFileName);
BOOL _CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
BOOL _FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData);
HANDLE _FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);

#ifdef __cplusplus
}
#endif

#endif

