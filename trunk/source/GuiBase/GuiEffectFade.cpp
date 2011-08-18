
#include "GuiEffectFade.h"

#include <assert.h>
#include <math.h>

#include "GuiRootContainer.h"
#include "GuiLayer.h"

GuiEffectFade::GuiEffectFade(int fade_frames, int delay, bool zoom,
                             float rotations, bool clockwise,
                             float posx, float posy)
{
    m_oTransform.valid = false;
    m_iFrames = fade_frames;
    m_iDelay = delay;
    m_bZoom = zoom;
    m_fRotations = rotations;
    m_bClockwise = clockwise;
    if( posx >= 0.0f && posy >= 0.0f ) {
        m_bMove = true;
        m_iPosX = (int)posx * GuiContainer::GetRootContainer()->GetWidth();
        m_iPosY = (int)posy * GuiContainer::GetRootContainer()->GetHeight();
    }else{
        m_bMove = false;
        m_iPosX = 0;
        m_iPosY = 0;
    }
}

GuiEffectFade::~GuiEffectFade()
{
}

void GuiEffectFade::Initialize(GuiLayer *from, GuiLayer *to, LayerTransform tfrom, LayerTransform tto)
{
    assert( from || to );
    if( from ) {
        m_poLayer = from;
        m_bFadeIn = false;
        m_fStartRotation = tfrom.rotation;
        m_fEndRotation = -360.0f * m_fRotations;
        m_fStartZoomX = tfrom.stretchWidth;
        m_fEndZoomX = m_bZoom? 0.0f : 1.0f;
        m_fStartZoomY = tfrom.stretchHeight;
        m_fEndZoomY = m_bZoom? 0.0f : 1.0f;
        m_iStartAlpha = tfrom.alpha;
        m_iEndAlpha = 0;
        m_fStartOffsetX = tfrom.valid? tfrom.offsetX : 0.0f;
        m_fStartOffsetY = tfrom.valid? tfrom.offsetY : 0.0f;
    }else{
        m_poLayer = to;
        m_bFadeIn = true;
        m_fStartRotation = tto.valid? tto.rotation : (360.0f * m_fRotations);
        m_fEndRotation = 0;
        m_fStartZoomX = m_bZoom? (tto.valid? tto.stretchWidth : 0.0f) : 1.0f;
        m_fEndZoomX = 1.0f;
        m_fStartZoomY = m_bZoom? (tto.valid? tto.stretchHeight : 0.0f) : 1.0f;
        m_fEndZoomY = 1.0f;
        m_iStartAlpha = tto.valid? tto.alpha : 0;
        m_iEndAlpha = 255;
        m_fStartOffsetX = tto.valid? tto.offsetX : 0.0f;
        m_fStartOffsetY = tto.valid? tto.offsetY : 0.0f;
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

bool GuiEffectFade::CancelLayer(GuiLayer *layer, LayerTransform *transform)
{
    if( m_poLayer == layer ) {
        // Since we're dealing with just one layer canceling this layer
        //   means the effect is done.
        *transform = m_oTransform;
        return true;
    }else{
        return false;
    }
}

bool GuiEffectFade::Run(void)
{
    bool bDone = false;

    // Do the effect
    f32 factor = (f32)sin(((f32)m_iCount / m_iFrames) * GUI_PI_2);

    // Fade
    m_oTransform.alpha = (u8)( (1.0f-factor) * m_iStartAlpha + factor * m_iEndAlpha );

    // Zoom
    m_oTransform.stretchWidth = ( (1.0f-factor) * m_fStartZoomX + factor * m_fEndZoomX );
    m_oTransform.stretchHeight = ( (1.0f-factor) * m_fStartZoomY + factor * m_fEndZoomY );

    // Rotation
    m_oTransform.rotation = ( (1.0f-factor) * m_fStartRotation + factor * m_fEndRotation );

    // Position movement
    LayerTransform tr = m_poLayer->GetTransform();
    float refx = m_poLayer->GetRefPixelX() * m_poLayer->GetStretchWidth();
    float refy = m_poLayer->GetRefPixelY() * m_poLayer->GetStretchHeight();
    if( m_bMove ) {
        float move_factor = m_bFadeIn? 1.0f - factor : factor;
        m_oTransform.offsetX = ((float)m_iPosX - tr.offsetX - m_fStartOffsetX - refx) * move_factor;
        m_oTransform.offsetY = ((float)m_iPosY - tr.offsetY - m_fStartOffsetY - refy) * move_factor;
    }else{
        m_oTransform.offsetX = 0.0f;
        m_oTransform.offsetY = 0.0f;
    }
    m_oTransform.offsetX += m_fStartOffsetX;
    m_oTransform.offsetY += m_fStartOffsetY;
    m_oTransform.valid = true;

    // Apply
    m_poLayer->DoTransform(m_oTransform);

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

