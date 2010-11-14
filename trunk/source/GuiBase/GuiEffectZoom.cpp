
#include <assert.h>
#include <math.h>
#include "GuiEffectZoom.h"
#include "GuiLayer.h"

GuiEffectZoom::GuiEffectZoom(int fade_frames, int delay)
{
    m_iFrames = fade_frames;
    m_iDelay = delay;
}

GuiEffectZoom::~GuiEffectZoom()
{
}

void GuiEffectZoom::Initialize(GuiLayer *from, GuiLayer *to)
{
    assert( from || to );
    if( from ) {
        m_poLayer = from;
        m_fStartZoom = 1.0f;
        m_fEndZoom = 0.0f;
        m_iStartAlpha = 255;
        m_iEndAlpha = 0;
    }else{
        m_poLayer = to;
        m_fStartZoom = 0.0f;
        m_fEndZoom = 1.0f;
        m_iStartAlpha = 0;
        m_iEndAlpha = 255;
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

bool GuiEffectZoom::Run(void)
{
    bool bDone = false;
    if( m_iDelay ) {
        // Delay before effect
        m_iDelay--;
    }else{
        // Do the effect
        f32 factor = sin(((f32)m_iCount / m_iFrames) * GUI_PI_2);
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

