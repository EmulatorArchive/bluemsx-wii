
#include "GuiFonts.h"

#include <stddef.h>

#include "font_arial.inc"
#include "font_impact.inc"

#include "../GuiBase/TextRender.h"
#include "../GuiBase/GuiRootContainer.h"

TextRender *g_fontArial = NULL;
TextRender *g_fontImpact = NULL;

void GuiFontInit(void)
{
    g_fontArial = new TextRender;
    g_fontArial->SetFont(font_arial, sizeof(font_arial));
    g_fontImpact = new TextRender;
    g_fontImpact->SetFont(font_impact, sizeof(font_impact));
}

void GuiFontClose(GuiRootContainer *root)
{
    root->Delete(g_fontArial);
    g_fontArial = NULL;
    root->Delete(g_fontImpact);
    g_fontImpact = NULL;
}

