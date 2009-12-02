
#include "GuiFonts.h"
#include "font_arial.inc"
#include "font_impact.inc"

TextRender *g_fontArial = NULL;
TextRender *g_fontImpact = NULL;

void GuiFontInit(void)
{
    g_fontArial = new TextRender;
    g_fontArial->SetFont(font_arial, sizeof(font_arial));
    g_fontImpact = new TextRender;
    g_fontImpact->SetFont(font_impact, sizeof(font_impact));
}

void GuiFontClose(void)
{
    delete g_fontArial;
    g_fontArial = NULL;
    delete g_fontImpact;
    g_fontImpact = NULL;
}

