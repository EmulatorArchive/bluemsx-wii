#ifndef _GUI_FONTS_H
#define _GUI_FONTS_H

class TextRender;
class GuiRootContainer;

extern TextRender *g_fontArial;
extern TextRender *g_fontImpact;

extern void GuiFontInit(void);
extern void GuiFontClose(GuiRootContainer *root);

#endif
