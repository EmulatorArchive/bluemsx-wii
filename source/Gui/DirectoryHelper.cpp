
#include <string.h>
#include "DirectoryHelper.h"

char DirectoryHelper::root_dir[MAX_DIRMANAGER_PATH] = "";
char DirectoryHelper::current_dir[MAX_DIRMANAGER_PATH*2] = "";
CMutex DirectoryHelper::dir_lock;

DirectoryHelper::DirectoryHelper()
{
    create_path[0] = '\0';
}

DirectoryHelper::~DirectoryHelper()
{
}

void DirectoryHelper::CleanupPath(char *path, bool isroot)
{
    char *p;
    // Check if empty
    if( *path == '\0' ) {
        *path++ = DIR_SLASH;
        *path = '\0';
        return;
    }
    // Replace slashes
    for( p = path; *p != '\0'; p++ ) {
        if( *p == DIR_BAD_SLASH ) {
            *p = DIR_SLASH;
        }
    }
    // Make sure it ends with a slash
    if( *(p-1) != DIR_SLASH ) {
        *p++ = DIR_SLASH;
        *p = '\0';
    }
    // Skip/remove trailing slashes
    for( p = path; *p == DIR_SLASH; ) {
        if( isroot ) {
            p++;
        }else{
            strcpy(path, p+1);
        }
    }
    // Remove redundant paths (e.g. /dir_a/dir_b/../dir_c/ -> /dir_a/dir_c/)
    int dirs = 0;
    while( *p != '\0' ) {
        if( *p == DIR_SLASH ) {
            char *subdir = p;
            bool isdot = true;
            int goback = -1;
            p++;
            // scan this subdir
            while( *p != '\0' && *p != DIR_SLASH ) {
                if( *p++ == '.' ) {
                    goback++; // one more '.', go back one more dir
                }else{
                    isdot = false; // other character, it's a normal directory
                }
            }
            if( isdot && goback > -1 ) {
                // it is some '.' directory, remove the proper amount of subdirs if possible
                while( goback-- > 0 ) {
                    if( dirs > 0 ) {
                        subdir--;
                        while( *subdir != DIR_SLASH ) {
                            subdir--;
                        }
                    }
                }
                strcpy(subdir, p);
                p = subdir;
            }else{
                dirs++;
            }
        }else{
            p++;
        }
    }
    // Remove trailing slash
    if( *(p-1) == DIR_SLASH ) {
        *(p-1) = '\0';
    }
}

void DirectoryHelper::SetRootDirectory(const char *path)
{
    dir_lock.Lock();
    strcpy(root_dir, path);
    CleanupPath(root_dir, true);
    dir_lock.Unlock();
}

void DirectoryHelper::ChangeDirectory(const char *path)
{
    bool relative = true;
    while( *path == DIR_SLASH || *path == DIR_BAD_SLASH ) {
        relative = false;
        path++;
    }

    dir_lock.Lock();
    if( relative ) {
        strcat(current_dir, DIR_SLASH_S);
        strcat(current_dir, path);
    }else{
        strcpy(current_dir, path);
    }
    CleanupPath(current_dir, false);
    dir_lock.Unlock();
}

char* DirectoryHelper::CreatePath(const char *path1, const char *path2)
{
    bool relative = true;
    while( *path1 == DIR_SLASH || *path1 == DIR_BAD_SLASH ) {
        relative = false;
        path1++;
    }

    dir_lock.Lock();
    strcpy(create_path, root_dir);
    if( relative ) {
        strcat(create_path, DIR_SLASH_S);
        strcat(create_path, current_dir);
    }
    dir_lock.Unlock();

    strcat(create_path, DIR_SLASH_S);
    strcat(create_path, path1);
    CleanupPath(create_path, true);

    if( path2 != NULL ) {
        strcat(create_path, DIR_SLASH_S);
        strcat(create_path, path2);
        CleanupPath(create_path, true);
    }

    return create_path;
}

