/*
** Haaf's Game Engine 1.6
** Copyright (C) 2003-2006, Relish Games
** hge.relishgames.com
**
** hge_tut07 - Thousand of Hares
*/


// Copy the files "font2.fnt", "font2.png", "bg2.png"
// and "zazaka.png" from the folder "precompiled" to
// the folder with executable file. Also copy hge.dll
// to the same folder.


#include <hge/hge.h>
#include <windows.h>
#include "../source/Gui/GuiMain.h"
#include "../source/Gui/GuiManager.h"
#include "../source/Win32/Win32Input.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  // GUI init
  GuiManager *manager = new GuiManager();

  // Init keyboard
  keyboardInit(manager);

  // Let's rock now!
  manager->Run(GuiMain);

  keyboardClose();

  delete manager;

  return 0;
}
