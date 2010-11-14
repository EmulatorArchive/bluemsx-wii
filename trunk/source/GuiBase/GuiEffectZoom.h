#ifndef _GUI_EFFECT_ZOOM_H
#define _GUI_EFFECT_ZOOM_H

#include "GuiEffect.h"
#include "GuiLayer.h"

class GuiEffectZoom : public GuiEffect {
public:
    GuiEffectZoom(int fade_frames, int delay=0);
    virtual ~GuiEffectZoom();

    virtual void Initialize(GuiLayer *from, GuiLayer *to);
    virtual bool Run(void);
private:
    GuiLayer *m_poLayer;
    LayerTransform m_oTransform;
    int m_iDelay;
    int m_iFrames;
    int m_iCount;
    float m_fStartZoom;
    float m_fEndZoom;
    int m_iStartAlpha;
    int m_iEndAlpha;
};

#endif

