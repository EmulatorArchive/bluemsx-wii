#ifndef _GUI_EFFECT_FADE_H
#define _GUI_EFFECT_FADE_H

#include "GuiEffect.h"
#include "GuiLayer.h"

class GuiEffectFade : public GuiEffect {
public:
    GuiEffectFade(int fade_frames, int delay=0, bool zoom=false,
                  float rotations=0.0f, bool clockwise=false,
                  float posx=-1, float posy=-1);
    virtual ~GuiEffectFade();

    virtual void Initialize(GuiLayer *from, GuiLayer *to);
    virtual bool Run(void);
private:
    GuiLayer *m_poLayer;
    bool m_bFadeIn;
    bool m_bZoom;
    bool m_bMove;
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
    int m_iPosX;
    int m_iPosY;
    float m_fStartOffsetX;
    float m_fStartOffsetY;
    float m_fEndOffsetX;
    float m_fEndOffsetY;
};

#endif

