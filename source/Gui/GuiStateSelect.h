
#ifndef _GUI_STATE_SELECT_H
#define _GUI_STATE_SELECT_H

#include <time.h>
#include "GuiDialog.h"

#define NUM_STATE_ITEMS   5

struct Properties;

class GuiSelectionList;

class GuiStateSelect : public GuiDialog
{
public:
    GuiStateSelect(GuiManager *man);
    virtual ~GuiStateSelect();

    // GuiDialog interface
    virtual void OnUpdateScreen(GuiRunner *runner);

    char *DoModal(Properties *properties, char *directory);
private:
    GuiManager *manager;
    GuiRunner *runner;
    GuiSelectionList *list;
    int last_selected;
    int last_index;
    int num_states;
    int posx;
    int posy;
    int sizex;
    int sizey;
    char *filenames[256];
    char *timestrings[256];
    time_t filetimes[256];
    Image *imgScreenShot;
    Sprite *sprScreenShot;

    void CreateStateFileList(Properties *properties, char *directory);
    void SetSelected(int index, int selected);
    void FreeStateFileList(void);
    void UpdateScreenShot(char *file);
};

#endif

