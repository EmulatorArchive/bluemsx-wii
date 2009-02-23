
#ifndef _GUI_STATE_SELECT_H
#define _GUI_STATE_SELECT_H

#include <time.h>
#include "GuiSelectionList.h"
extern "C" {
#include "Properties.h"
}

#define NUM_STATE_ITEMS   4

class GuiStateSelect : public GuiSelectionList {
public:
    GuiStateSelect(GuiManager *man);
    virtual ~GuiStateSelect();

    void OnSetSelected(int index, int selected);
    char *DoModal(Properties *properties, char *directory);
private:
    int num_states;
    char *filenames[256];
    char *timestrings[256];
    time_t filetimes[256];
    Image *imgScreenShot;
    Sprite sprScreenShot;

    void CreateStateFileList(Properties *properties, char *directory);
    void FreeStateFileList(void);
    void UpdateScreenShot(char *file);
};

#endif

