
#include <assert.h>
#include <math.h>
#include "GuiEffectRotate.h"
#include "GuiLayer.h"

GuiEffectRotate::GuiEffectRotate(int fade_frames, int delay, float rotations, bool clockwise)
{
    m_iFrames = fade_frames;
    m_iDelay = delay;
    m_fRotations = rotations;
    m_bClockwise = clockwise;
}

GuiEffectRotate::~GuiEffectRotate()
{
}

void GuiEffectRotate::Initialize(GuiLayer *from, GuiLayer *to)
{
    assert( from || to );
    if( from ) {
        m_poLayer = from;
        m_fStartRotation = 0;
        m_fEndRotation = -360.0f * m_fRotations;
        m_fStartZoom = 1.0f;
        m_fEndZoom = 0.0f;
        m_iStartAlpha = 255;
        m_iEndAlpha = 255;
    }else{
        m_poLayer = to;
        m_fStartRotation = 360.0f * m_fRotations;
        m_fEndRotation = 0;
        m_fStartZoom = 0.0f;
        m_fEndZoom = 1.0f;
        m_iStartAlpha = 255;
        m_iEndAlpha = 255;
    }
    if( m_bClockwise ) {
        m_fStartRotation = -m_fStartRotation;
    }
    m_oTransform.offsetX = m_oTransform.offsetY = 0;
    m_oTransform.rotation = 0.0f;
    m_oTransform.alpha = 255;
    m_oTransform.stretchWidth = m_oTransform.stretchHeight = m_fStartZoom;
    m_iCount = 0;
    if( m_iFrames == 0 ) {
        m_iFrames++;
        m_iCount++;
    }
}

bool GuiEffectRotate::Run(void)
{
    bool bDone = false;
    if( m_iDelay ) {
        // Delay before effect
        m_iDelay--;
    }else{
        // Do the effect
        f32 factor = sin(((f32)m_iCount / m_iFrames) * GUI_PI_2);
        m_oTransform.rotation = ( (1.0f-factor) * m_fStartRotation + factor * m_fEndRotation );
        m_oTransform.stretchHeight = ( (1.0f-factor) * m_fStartZoom + factor * m_fEndZoom );
        m_oTransform.stretchWidth = m_oTransform.stretchHeight;
        m_oTransform.alpha = (u8)( (1.0f-factor) * m_iStartAlpha + factor * m_iEndAlpha );
        m_poLayer->DoTransform(m_oTransform);
        // Next step
        if( m_iCount < m_iFrames ) {
          m_iCount++;
        }else{
          bDone = true;
        }
    }
    return bDone;
}

