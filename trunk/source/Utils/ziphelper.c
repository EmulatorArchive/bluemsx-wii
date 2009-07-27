/*****************************************************************************
** $Source: /cvsroot/bluemsx/blueMSX/Src/Utils/ziphelper.c,v $
**
** $Revision: 1.6 $
**
** $Date: 2008/03/30 21:38:43 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
******************************************************************************
*/
#include "ZipHelper.h"

#include "zip.h"
#include "unzip.h"
#include "ctype.h"
#include "ZipFromMem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

static void toLower(char* str) {
    while (*str) {
        *str = tolower(*str);
        str++;
    }
}

/******************************************************************************
*** Description
***     Load a file in a zip file into memory.
***
*** Arguments
***     zipName     - Name of zip file
***     fileName    - Name of file insize zipfile to load
***     size        - Output of size of file
***
*** Return
***     Pointer to allocate memory buffer with file content or NULL on
***     failure.
***
*******************************************************************************
*/
void* _zipLoadFile(const char* zipName, const char* fileName, int* size, zlib_filefunc_def* filefunc)
{
    void* buf;
    char name[256];
    unzFile zip;
    unz_file_info info;

    *size = 0;

    if (fileName[0] == '*') {
        strcpy(name, zipName);
        name[strlen(zipName) - 3] = fileName[strlen(fileName) - 3];
        name[strlen(zipName) - 2] = fileName[strlen(fileName) - 2];
        name[strlen(zipName) - 1] = fileName[strlen(fileName) - 1];
    }
    else {
        strcpy(name, fileName);
    }

    zip = unzOpen2(zipName, filefunc);
    if (!zip) {
        return NULL;
    }

    if (unzLocateFile(zip, name, 1) == UNZ_END_OF_LIST_OF_FILE) {
        unzClose(zip);
        return NULL;
    }

    if (unzOpenCurrentFile(zip) != UNZ_OK) {
        return NULL;
    }

    unzGetCurrentFileInfo(zip,&info,NULL,0,NULL,0,NULL,0);

    buf = malloc(info.uncompressed_size);
    *size = info.uncompressed_size;

    if (!buf) {
        unzCloseCurrentFile(zip);
        unzClose(zip);
        return NULL;
    }

    unzReadCurrentFile(zip, buf, info.uncompressed_size);
    unzCloseCurrentFile(zip);
    unzClose(zip);

    return buf;
}


/******************************************************************************
*** Description
***     Read cache to speed-up reading multiple files from one zip.
***
******************************************************************************/

static char *cacheData = NULL, cacheFile[512];
static zlib_filefunc_def cacheFilefunc;

void* zipLoadFile(const char* zipName, const char* fileName, int* size)
{
    if( cacheData != NULL && *cacheFile != '\0' && 0==strcmp(cacheFile, zipName) ) {
        return _zipLoadFile(cacheData, fileName, size, &cacheFilefunc);
    }else{
        return _zipLoadFile(zipName, fileName, size, NULL);
    }
}

void zipCacheReadOnlyZip(const char* zipName)
{
    *cacheFile = '\0';
    if( cacheData != NULL ) {
        free(cacheData);
        cacheData = NULL;
        free_fopen_memfunc(&cacheFilefunc);
    }
    if( zipName != NULL ) {
        FILE *file;
        file = fopen(zipName, "rb");
        if( file != NULL ) {
            unsigned int filesize;
            fseek(file, 0, SEEK_END);
            filesize = ftell(file);
            fill_fopen_memfunc(&cacheFilefunc, filesize);
            fseek(file, 0, SEEK_SET);
            cacheData = malloc(filesize);
            if( cacheData != NULL ) {
                size_t size = fread(cacheData, 1, filesize, file);
                if( size == filesize ) {
                    strcpy(cacheFile, zipName);
                }
            }
            fclose(file);
        }
    }
}


/******************************************************************************
*** Description
***     Load a file in a zip file into memory.
***
*** Arguments
***     zipName     - Name of zip file
***     fileName    - Name of file insize zipfile to save
***     buffer      - Buffer to save
***     size        - Size of buffer to save
***
*******************************************************************************
*/
int zipSaveFile(const char* zipName, const char* fileName, int append, void* buffer, int size)
{
    zipFile zip;
    zip_fileinfo zi;
    int err;

    zip = zipOpen(zipName, append ? 2 : 0);
    if (zip == NULL) {
        return 0;
    }

    memset(&zi, 0, sizeof(zi));

    err = zipOpenNewFileInZip(zip, fileName, &zi,
                              NULL, 0, NULL, 0, NULL,
                              Z_DEFLATED, Z_DEFAULT_COMPRESSION);
    if (err == ZIP_OK) {
        err = zipWriteInFileInZip(zip, buffer, size);
    }

    zipClose(zip, NULL);

    return err >= 0;
}

int zipHasFileType(char* zipName, char* ext) {
    char tempName[256];
    char extension[8];
    unzFile zip;
    unz_file_info info;
    int found = 0;
    int status;

    zip = unzOpen(zipName);
    if (!zip) {
        return 0;
    }

    strcpy(extension, ext);
    toLower(extension);

    status = unzGoToFirstFile(zip);
    unzGetCurrentFileInfo(zip,&info,tempName,256,NULL,0,NULL,0);

    while (status == UNZ_OK) {
        unzGetCurrentFileInfo(zip, &info, tempName, 256, NULL, 0, NULL, 0);

        toLower(tempName);
        if (strstr(tempName, extension) != NULL) {
            found = 1;
            break;
        }

        status = unzGoToNextFile(zip);
    }

    unzClose(zip);

    return found;
}

/******************************************************************************
*** Description
***     Checks if a file exists in a zip file.
***
*** Arguments
***     zipName     - Name of zip file
***     fileName    - Name of file insize zipfile to load
***
*** Return
***     1 = file exists, 0 = non existing zip or file in zip does not exists
***     failure.
***
*******************************************************************************
*/
int zipFileExists(const char* zipName, const char* fileName)
{
    char name[256];
    unzFile zip;

    if (fileName[0] == '*') {
        strcpy(name, zipName);
        name[strlen(zipName) - 3] = fileName[strlen(fileName) - 3];
        name[strlen(zipName) - 2] = fileName[strlen(fileName) - 2];
        name[strlen(zipName) - 1] = fileName[strlen(fileName) - 1];
    }
    else {
        strcpy(name, fileName);
    }

    zip = unzOpen(zipName);
    if (!zip) {
        return 0;
    }

    if (unzLocateFile(zip, name, 1) == UNZ_END_OF_LIST_OF_FILE) {
        unzClose(zip);
        return 0;
    }else{
        unzClose(zip);
        return 1;
    }
}

/******************************************************************************
*** Description
***     Creates a list of file names inside a zip that matches a given
***     extension.
***
*** Arguments
***     zipName     - Name of zip file
***     ext         - Extension to check
***     count       - Output for number of matching files in zip file.
***
*** Return
***     1 if files with the given extension exists in the zip file,
***     0 otherwise.
***
*******************************************************************************
*/
char* zipGetFileList(const char* zipName, const char* ext, int* count) {
    char tempName[256];
    char extension[8];
    unzFile zip;
    unz_file_info info;
    char* fileArray = NULL;
    int totalLen = 0;
    int status;

    *count = 0;

    zip = unzOpen(zipName);
    if (!zip) {
        return 0;
    }

    strcpy(extension, ext);
    toLower(extension);

    status = unzGoToFirstFile(zip);
    unzGetCurrentFileInfo(zip,&info,tempName,256,NULL,0,NULL,0);

    while (status == UNZ_OK) {
        char tmp[256];

        unzGetCurrentFileInfo(zip, &info, tempName, 256, NULL, 0, NULL, 0);

        strcpy(tmp, tempName);

        toLower(tmp);
        if (strstr(tmp, extension) != NULL) {
            int entryLen = strlen(tempName) + 1;
            fileArray = realloc(fileArray, totalLen +  entryLen + 1);
            strcpy(fileArray + totalLen, tempName);
            totalLen += entryLen;
            fileArray[totalLen] = '\0'; // double null termination at end

            *count = *count + 1;
        }

        status = unzGoToNextFile(zip);
    }

    unzClose(zip);

    return fileArray;
}

/******************************************************************************
*** Description
***     zipExtractCurrentfile - Extracts the current file from the zip
***     zipExtract - Extracts the whole zip file
***
*** Arguments
***     uf        - The zip file
***     overwrite - 1 = overwrite files, 0 = do not overwrite files
***     password  - Optional password for the zip
***
*** Return
***     1 okay,
***     0 failed.
***
*******************************************************************************
*/
#define WRITEBUFFERSIZE 8192

static int makedir(const char *newdir)
{
    char *buffer;
    char *p;
    int len = (int)strlen(newdir);

    if (len <= 0) return 0;

    buffer = (char*)malloc(len+1);
    strcpy(buffer,newdir);

    if (buffer[len-1] == '/') {
        buffer[len-1] = '\0';
    }
    if (mkdir(buffer, 0777) == 0) {
        free(buffer);
        return 1;
    }

    p = buffer+1;
    while (1) {
        char hold;

        while(*p && *p != '\\' && *p != '/') p++;
        hold = *p;
        *p = 0;
        if ((mkdir(buffer, 0777) == -1) && (errno == ENOENT))
        {
            printf("couldn't create directory %s\n",buffer);
            free(buffer);
            return 0;
        }
        if (hold == 0) break;
        *p++ = hold;
    }
    free(buffer);
    return 1;
}

int zipExtractCurrentfile(unzFile uf, int overwrite, const char* password)
{
    char filename_inzip[256];
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;
    uInt size_buf;

    unz_file_info file_info;
    err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);

    if( err != UNZ_OK ) {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        return 0;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);

    p = filename_withoutpath = filename_inzip;
    while ((*p) != '\0') {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    if ((*filename_withoutpath)=='\0') {
        mkdir(filename_inzip, 0777);
    }else{
        const char* write_filename;
        int skip=0;

        write_filename = filename_inzip;

        err = unzOpenCurrentFilePassword(uf,password);
        if (err!=UNZ_OK) {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        if ((overwrite==0) && (err==UNZ_OK)) {
            FILE* ftestexist = fopen(write_filename,"rb");
            if (ftestexist!=NULL) {
                fclose(ftestexist);
                skip = 1;
            }
        }

        if ((skip==0) && (err==UNZ_OK)) {
            fout=fopen(write_filename,"wb");

            /* some zipfile don't contain directory alone before file */
            if( (fout==NULL) && (filename_withoutpath!=(char*)filename_inzip) ) {
                char c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=fopen(write_filename,"wb");
            }

            if( fout == NULL ) {
                printf("error opening %s\n",write_filename);
            }
        }

        if (fout!=NULL)
        {
            printf(" extracting: %s\n",write_filename);
            do {
                err = unzReadCurrentFile(uf,buf,size_buf);
                if( err < 0 ) {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    break;
                }
                if( err > 0 ) {
                    if (fwrite(buf,err,1,fout)!=1) {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        break;
                    }
                }
            }while( err > 0 );
            if( fout ) fclose(fout);
        }

        if(err == UNZ_OK) {
            err = unzCloseCurrentFile (uf);
            if( err != UNZ_OK ) {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }else{
            unzCloseCurrentFile(uf); /* don't lose the error */
        }
    }

    free(buf);
    return 1;
}

int zipExtract(unzFile uf, int overwrite, const char* password,
               ZIP_EXTRACT_CB progress_callback)
{
    uLong i;
    unz_global_info gi;
    int err;

    err = unzGetGlobalInfo(uf,&gi);
    if (err!=UNZ_OK) {
        printf("error %d with zipfile in unzGetGlobalInfo \n",err);
        return 0;
    }

    for (i = 0; i < gi.number_entry; i++)
    {
        if( progress_callback ) {
            progress_callback(gi.number_entry, i);
        }
        if( !zipExtractCurrentfile(uf, overwrite, password) ) {
            return 0;
        }
        if ((i+1) < gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                return 0;
            }
        }
    }

    return 1;
}
