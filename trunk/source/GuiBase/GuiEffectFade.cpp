/***************************************************************
 *
 * Copyright (C) 2008-2011 Tim Brugman
 *
 * This file may be licensed under the terms of of the
 * GNU General Public License Version 2 (the ``GPL'').
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the GPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the GPL along with this
 * program. If not, go to http://www.gnu.org/licenses/gpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 ***************************************************************/

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
    m_fPosX = posx;
    m_fPosY = posy;
}

GuiEffectFade::~GuiEffectFade()
{
}

GuiEffect* GuiEffectFade::Clone(void)
{
   return new GuiEffectFade(m_iFrames, m_iDelay, m_bZoom, m_fRotations, m_bClockwise, m_fPosX, m_fPosY);
}

void GuiEffectFade::Initialize(GuiLayer *layer, GuiEffectType type, LayerTransform tfrom, LayerTransform tto)
{
    m_poLayer = layer;
    if( type == ET_HIDE) {
        // fade out
        m_fStartRotation = 0.0f;
        m_fEndRotation = -360.0f * m_fRotations;
        m_fStartZoomX = 1.0f;
        m_fEndZoomX = m_bZoom? 0.0f : 1.0f;
        m_fStartZoomY = 1.0f;
        m_fEndZoomY = m_bZoom? 0.0f : 1.0f;
        m_fStartAlpha = 1.0f;
        m_fEndAlpha = 0.0f;
    }else{
        // fade in
        m_fStartRotation = 360.0f * m_fRotations;
        m_fEndRotation = 0;
        m_fStartZoomX = m_bZoom? 0.0f : 1.0f;
        m_fEndZoomX = 1.0f;
        m_fStartZoomY = m_bZoom? 0.0f : 1.0f;
        m_fEndZoomY = 1.0f;
        m_fStartAlpha = 0.0f;
        m_fEndAlpha = 1.0f;
    }
    m_fStartOffsetX = 0.0f;
    m_fStartOffsetY = 0.0f;
    m_fEndOffsetX = 0.0f;
    m_fEndOffsetY = 0.0f;
    if( tfrom.valid ) {
        m_fStartRotation = tfrom.rotation;
        m_fStartZoomX = tfrom.stretchWidth;
        m_fStartZoomY = tfrom.stretchHeight;
        m_fStartAlpha = tfrom.alpha;
        m_fStartOffsetX = tfrom.offsetX;
        m_fStartOffsetY = tfrom.offsetY;
    }
    if( tto.valid ) {
        m_fEndZoomX = tto.stretchWidth;
        m_fEndZoomY = tto.stretchHeight;
        m_fEndAlpha = tto.alpha;
        m_fEndOffsetX = tto.offsetX;
        m_fEndOffsetY = tto.offsetY;
    }
    if( type == ET_HIDE) {
        if( m_fPosX >= 0 ) {
            m_fEndOffsetX += m_fPosX - m_poLayer->GetX() - m_poLayer->GetRefPixelX();
        }
        if( m_fPosY >= 0 ) {
            m_fEndOffsetY += m_fPosY - m_poLayer->GetY() - m_poLayer->GetRefPixelY();
        }
    }else{
        if( m_fPosX >= 0 ) {
            m_fStartOffsetX += m_fPosX - m_poLayer->GetX() - m_poLayer->GetRefPixelX();
        }
        if( m_fPosY >= 0 ) {
            m_fStartOffsetY += m_fPosY - m_poLayer->GetY() - m_poLayer->GetRefPixelY();
        }
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

void GuiEffectFade::Cancel(LayerTransform *transform)
{
    *transform = m_oTransform;
}

bool GuiEffectFade::Run(void)
{
    bool bDone = false;

    // Do the effect
    float factor = (float)sin(((float)m_iCount / m_iFrames) * GUI_PI_2);
 
    // Fade
    m_oTransform.alpha = (1.0f-factor) * m_fStartAlpha + factor * m_fEndAlpha;

    // Zoom
    m_oTransform.stretchWidth = (1.0f-factor) * m_fStartZoomX + factor * m_fEndZoomX;
    m_oTransform.stretchHeight = (1.0f-factor) * m_fStartZoomY + factor * m_fEndZoomY;

    // Rotation
    m_oTransform.rotation = (1.0f-factor) * m_fStartRotation + factor * m_fEndRotation;

    // Position movement
    m_oTransform.offsetX = (1.0f-factor) * m_fStartOffsetX + factor * m_fEndOffsetX;
    m_oTransform.offsetY = (1.0f-factor) * m_fStartOffsetY + factor * m_fEndOffsetY;

    // Apply
    m_oTransform.valid = true;
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

