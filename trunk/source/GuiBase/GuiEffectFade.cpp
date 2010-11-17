
#include "GuiEffectFade.h"

#include <assert.h>
#include <math.h>

#include "GuiRootContainer.h"
#include "GuiLayer.h"

GuiEffectFade::GuiEffectFade(int fade_frames, int delay, bool zoom,
                             float rotations, bool clockwise,
                             float posx, float posy)
{
    m_iFrames = fade_frames;
    m_iDelay = delay;
    m_bZoom = zoom;
    m_fRotations = rotations;
    m_bClockwise = clockwise;
    if( posx >= 0.0f && posy >= 0.0f ) {
        m_bMove = true;
        m_iPosX = posx * GuiContainer::GetRootContainer()->GetWidth();
        m_iPosY = posy * GuiContainer::GetRootContainer()->GetHeight();
    }else{
        m_bMove = false;
        m_iPosX = 0;
        m_iPosY = 0;
    }
}

GuiEffectFade::~GuiEffectFade()
{
}

void GuiEffectFade::Initialize(GuiLayer *from, GuiLayer *to)
{
    assert( from || to );
    LayerTransform t;
    if( from ) {
        m_poLayer = from;
        m_bFadeIn = false;
        m_fStartRotation = 0;
        m_fEndRotation = -360.0f * m_fRotations;
        m_fStartZoom = 1.0f;
        m_fEndZoom = m_bZoom? 0.0f : 1.0f;
        m_iStartAlpha = 255;
        m_iEndAlpha = 0;
    }else{
        m_bFadeIn = true;
        m_poLayer = to;
        m_fStartRotation = 360.0f * m_fRotations;
        m_fEndRotation = 0;
        m_fStartZoom = m_bZoom? 0.0f : 1.0f;
        m_fEndZoom = 1.0f;
        m_iStartAlpha = 0;
        m_iEndAlpha = 255;
    }
    if( m_bClockwise ) {
        m_fStartRotation = -m_fStartRotation;
    }
    m_iCount = 0;
    if( m_iFrames == 0 ) {
        m_iFrames++;
        m_iCount++;
    }
}

bool GuiEffectFade::Run(void)
{
    LayerTransform oTransform;
    bool bDone = false;

    // Do the effect
    f32 factor = sin(((f32)m_iCount / m_iFrames) * GUI_PI_2);

    // Fade
    oTransform.alpha = (u8)( (1.0f-factor) * m_iStartAlpha + factor * m_iEndAlpha );

    // Zoom
    oTransform.stretchHeight = ( (1.0f-factor) * m_fStartZoom + factor * m_fEndZoom );
    oTransform.stretchWidth = oTransform.stretchHeight;

    // Rotation
    oTransform.rotation = ( (1.0f-factor) * m_fStartRotation + factor * m_fEndRotation );

    // Position movement
    LayerTransform tr = m_poLayer->GetTransform();
    float refx = m_poLayer->GetRefPixelX() * m_poLayer->GetStretchWidth();
    float refy = m_poLayer->GetRefPixelY() * m_poLayer->GetStretchHeight();
    if( m_bMove ) {
        float move_factor = m_bFadeIn? 1.0f - factor : factor;
        oTransform.offsetX = ((float)m_iPosX - tr.offsetX - refx) * move_factor;
        oTransform.offsetY = ((float)m_iPosY - tr.offsetY - refy) * move_factor;
    }else{
        oTransform.offsetX = 0.0f;
        oTransform.offsetY = 0.0f;
    }

    // Apply
    m_poLayer->DoTransform(oTransform);

    // Next step
    if( m_iDelay ) {
        // Delay before effect
        m_iDelay--;
    }else{
        // Count
        if( m_iCount < m_iFrames ) {
          m_iCount++;
        }else{
          bDone = true;
        }
    }
    return bDone;
}

