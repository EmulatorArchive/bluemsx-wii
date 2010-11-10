
#include <assert.h>
#include "GuiEffectFade.h"
#include "GuiLayer.h"

GuiEffectFade::GuiEffectFade(int fade_frames, int delay)
{
    m_iFrames = fade_frames;
    m_iDelay = delay;
}

GuiEffectFade::~GuiEffectFade()
{
}

void GuiEffectFade::Initialize(GuiLayer *from, GuiLayer *to)
{
    assert( from || to );
    if( from ) {
        m_poLayer = from;
        m_iStartAlpha = m_iStoredAplpha = m_poLayer->GetTransparency();
        m_iEndAlpha = 0;
    }else{
        m_poLayer = to;
        m_iStartAlpha = 0;
        m_iEndAlpha = m_iStoredAplpha = m_poLayer->GetTransparency();
    }
    m_poLayer->SetTransparency(m_iStartAlpha);
    m_iCount = 0;
}

bool GuiEffectFade::Run(void)
{
    bool bDone = false;
    if( m_iDelay ) {
        m_iDelay--;
    }else{
        if( m_iFrames ) {
            m_poLayer->SetTransparency(((m_iFrames - m_iCount) * m_iStartAlpha +
                                        m_iCount * m_iEndAlpha) / m_iFrames);
        }else{
            m_poLayer->SetTransparency(m_iEndAlpha);
        }
        if( m_iCount < m_iFrames ) {
          m_iCount++;
        }else{
          m_poLayer->SetTransparency(m_iStoredAplpha);
          bDone = true;
        }
    }
    return bDone;
}

