
#ifndef _GUI_STATE_SELECT_H
#define _GUI_STATE_SELECT_H

#include <time.h>
#include "../GuiBase/GuiDialog.h"

#define NUM_STATE_ITEMS   5

struct Properties;

class GuiLayFrame;
class GuiElmSelectionList;

class GuiDlgStateSelect : public GuiDialog
{
public:
    GuiDlgStateSelect(GuiContainer *cntr, Properties *properties, char *directory);
    virtual ~GuiDlgStateSelect();

    // GuiDialog interface
    virtual void OnUpdateScreen(void);

    char *DoModal(void);
private:
    GuiLayFrame *frame;
    GuiElmSelectionList *list;
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
    GuiSprite *sprScreenShot;

    void CreateStateFileList(Properties *properties, char *directory);
    void SetSelectedState(int index, int selected);
    void FreeStateFileList(void);
    void UpdateScreenShot(char *file);
};

#endif

