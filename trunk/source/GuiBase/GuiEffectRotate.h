#ifndef _GUI_EFFECT_ROTATE_H
#define _GUI_EFFECT_ROTATE_H

#include "GuiEffect.h"
#include "GuiLayer.h"

class GuiEffectRotate : public GuiEffect {
public:
    GuiEffectRotate(int fade_frames, int delay=0, float rotations=1.0f, bool clockwise=false);
    virtual ~GuiEffectRotate();

    virtual void Initialize(GuiLayer *from, GuiLayer *to);
    virtual bool Run(void);
private:
    GuiLayer *m_poLayer;
    LayerTransform m_oTransform;
    bool m_bClockwise;
    float m_fRotations;
    int m_iDelay;
    int m_iFrames;
    int m_iCount;
    float m_fStartZoom;
    float m_fEndZoom;
    float m_fStartRotation;
    float m_fEndRotation;
    int m_iStartAlpha;
    int m_iEndAlpha;
};

#endif

