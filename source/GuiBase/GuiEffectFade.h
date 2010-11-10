#ifndef _GUI_EFFECT_FADE_H
#define _GUI_EFFECT_FADE_H

#include "GuiEffect.h"

class GuiEffectFade : public GuiEffect {
public:
    GuiEffectFade(int fade_frames, int delay=0);
    virtual ~GuiEffectFade();

    virtual void Initialize(GuiLayer *from, GuiLayer *to);
    virtual bool Run(void);
private:
    GuiLayer *m_poLayer;
    int m_iDelay;
    int m_iFrames;
    int m_iCount;
    int m_iStoredAplpha;
    int m_iStartAlpha;
    int m_iEndAlpha;
};

#endif

