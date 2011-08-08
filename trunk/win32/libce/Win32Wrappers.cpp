
#include <Win32Wrappers.h>

/*
 * Returns the UTF-16 form of a UTF-8 string. The result should be
 * freed with free() when no longer needed.
 */
wchar_t *utf8_to_utf16(const char *str)
{
  int n;
  wchar_t *retval;

  n = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);

  if( n == 0 ) {
    return NULL;
  }

  retval = (wchar_t*)malloc(sizeof(wchar_t)*n);

  if (!retval) {
    return NULL;
  }

  if (MultiByteToWideChar(CP_UTF8, 0, str, -1, retval, n) != n) {
    free(retval);
    return NULL;
  }

  return retval;
}

/*
 * Returns the UTF-8 form of a UTF-16 string. The result should be
 * freed with free() when no longer needed.
 */
char *utf16_to_utf8(const wchar_t *str)
{
  int n;
  char *retval;

  n = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);

  if (n == 0) {
    return NULL;
  }

  retval = (char*)malloc(n);

  if (!retval) {
    return NULL;
  }

  if (WideCharToMultiByte(CP_UTF8, 0, str, -1, retval, n, NULL, NULL) != n) {
    free(retval);
    return NULL;
  }

  return retval;
}

DWORD _GetFileAttributesA(LPCSTR lpFileName)
{
  wchar_t *filename;
  DWORD result;
  int err;

  filename = utf8_to_utf16(lpFileName);
  if (!filename)
    return INVALID_FILE_ATTRIBUTES;

  result = GetFileAttributesW (filename);

  err = GetLastError ();
  free(filename);
  SetLastError (err);
  return result;
}

BOOL _CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
  wchar_t *pathname;
  BOOL result;
  int err;

  pathname = utf8_to_utf16(lpPathName);
  if (!pathname)
    return FALSE;

  result = CreateDirectoryW (pathname, lpSecurityAttributes);

  err = GetLastError();
  free(pathname);
  SetLastError(err);
  return result;
}

static BOOL convert_find_data (LPWIN32_FIND_DATAW fdw, LPWIN32_FIND_DATAA fda)
{
  char *filename;
  int len;

  fda->dwFileAttributes = fdw->dwFileAttributes;
  fda->ftCreationTime = fdw->ftCreationTime;
  fda->ftLastAccessTime = fdw->ftLastAccessTime;
  fda->ftLastWriteTime = fdw->ftLastWriteTime;
  fda->nFileSizeHigh = fdw->nFileSizeHigh;
  fda->nFileSizeLow = fdw->nFileSizeLow;

  filename = utf16_to_utf8(fdw->cFileName);
  if (!filename)
    return FALSE;

  len = sizeof (fda->cFileName);
  strncpy (fda->cFileName, filename, len);
  fda->cFileName[len - 1] = '\0';
  
  return TRUE;
}

BOOL _FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
{
  WIN32_FIND_DATAW find_file_data;
  BOOL result;

  result = FindNextFileW (hFindFile, &find_file_data);
  if (result)
    result = convert_find_data (&find_file_data, lpFindFileData);

  return result;  
}

HANDLE _FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
{
  wchar_t *pathname;
  WIN32_FIND_DATAW find_file_data;
  HANDLE result;
  int err;

  pathname = utf8_to_utf16(lpFileName);
  if (!pathname)
    return INVALID_HANDLE_VALUE;

  result = FindFirstFileW (pathname, &find_file_data);
  if (result != INVALID_HANDLE_VALUE)
    {
      BOOL res = convert_find_data(&find_file_data, lpFindFileData);
      if (! res)
        {
          err = GetLastError ();
          FindClose (result);
          SetLastError (err);
          result = INVALID_HANDLE_VALUE;
        }
    }

  err = GetLastError ();
  free(pathname);
  SetLastError (err);
  return result;
}


