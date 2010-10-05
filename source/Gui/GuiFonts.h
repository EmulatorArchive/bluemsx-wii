#ifndef _GUI_FONTS_H
#define _GUI_FONTS_H

#include "../WiiSprite/TextRender.h"

extern TextRender *g_fontArial;
extern TextRender *g_fontImpact;

extern void GuiFontInit(void);
extern void GuiFontClose(void);

#endif
