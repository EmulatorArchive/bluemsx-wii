#ifndef _H_DIR_HELPER
#define _H_DIR_HELPER

#include "../GuiBase/Mutex.h"

#define MAX_DIRMANAGER_PATH 256

#define DIR_BAD_SLASH '\\'
#define DIR_SLASH '/'
#define DIR_SLASH_S "/"

class DirectoryHelper
{
public:
    DirectoryHelper();
    virtual ~DirectoryHelper();

    static void SetRootDirectory(const char *path);
    void ChangeDirectory(const char *path);
    char *CreatePath(const char *path1, const char *path2 = NULL);

private:
    static void CleanupPath(char *path, bool isroot);

    char create_path[MAX_DIRMANAGER_PATH*3];
    static char root_dir[MAX_DIRMANAGER_PATH];
    static char current_dir[MAX_DIRMANAGER_PATH*2];
    static CMutex dir_lock;
};

#endif

