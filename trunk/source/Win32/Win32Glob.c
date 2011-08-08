/*****************************************************************************
** $Source: WiiGlob.c,v $
**
** $Revision: 1.2 $
**
** $Date: 2006/06/24 02:27:08 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
******************************************************************************
*/
#include "../Arch/ArchGlob.h"
#include "../Arch/ArchFile.h"
#include <windows.h>
#include <stdlib.h>
#ifdef UNDER_CE
#include <Win32Wrappers.h>
#endif

// This glob only support very basic globbing, dirs in the patterns are only
// supported without any wildcards

ArchGlob* archGlob(const char* pattern, int flags)
{
    char *p;
    char oldPath[MAX_PATH];
    char findPath[MAX_PATH];
    const char* filePattern;
    ArchGlob* glob;
    WIN32_FIND_DATAA wfd;
    HANDLE handle;

    strcpy(oldPath, archGetCurrentDirectory());

    while((p = strchr(pattern, '\\')) != NULL) {
        *p = '/';
    }

    filePattern = strrchr(pattern, '/');
    if (filePattern == NULL) {
        filePattern = pattern;
    }
    else {
        char relPath[MAX_PATH];
        strcpy(relPath, pattern);
        relPath[filePattern - pattern] = '\0';
        pattern = filePattern + 1;
        if( relPath[0] == '/' || relPath[1] == ':' ) {
            // absolute path already
            strcpy(oldPath, relPath);
        }else{
            strcat(oldPath, "/");
            strcat(oldPath, relPath);
        }
    }

    strcpy(findPath, oldPath);
    strcat(findPath, "/");
    strcat(findPath, pattern);
    handle = FindFirstFileA(findPath, &wfd);
    if (handle == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    glob = (ArchGlob*)calloc(1, sizeof(ArchGlob));

    do {
        char *fullpath = (char*)malloc(MAX_PATH);
        DWORD fa;
        if (0 == strcmp(wfd.cFileName, ".") || 0 == strcmp(wfd.cFileName, "..")) {
            continue;
        }
        strcpy(fullpath, oldPath);
        strcat(fullpath, "/");
        strcat(fullpath, wfd.cFileName);
        fa = GetFileAttributesA(fullpath);
        if (((flags & ARCH_GLOB_DIRS) && (fa & FILE_ATTRIBUTE_DIRECTORY) != 0) ||
            ((flags & ARCH_GLOB_FILES) && (fa & FILE_ATTRIBUTE_DIRECTORY) == 0))
        {
            glob->count++;
            glob->pathVector = realloc(glob->pathVector, sizeof(char*) * glob->count);
            glob->pathVector[glob->count - 1] = fullpath;
        }else{
            free(fullpath);
        }
    } while (FindNextFileA(handle, &wfd));

    FindClose(handle);

    return glob;
}

void archGlobFree(ArchGlob* globHandle)
{
    int i;

    if (globHandle == NULL) {
        return;
    }

    for (i = 0; i < globHandle->count; i++) {
        free(globHandle->pathVector[i]);
    }
    free(globHandle->pathVector);
    free(globHandle);
}
