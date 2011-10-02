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

    // Required interface
    virtual void Initialize(GuiLayer *layer, GuiEffectType type, LayerTransform tfrom, LayerTransform tto);
    virtual GuiEffect* Clone(void);
    virtual void Cancel(LayerTransform *transform);
    virtual bool Run(void);

private:
    GuiLayer *m_poLayer;
    LayerTransform m_oTransform;
    bool m_bZoom;
    bool m_bClockwise;
    float m_fRotations;
    int m_iDelay;
    int m_iFrames;
    int m_iCount;
    float m_fStartZoomX;
    float m_fEndZoomX;
    float m_fStartZoomY;
    float m_fEndZoomY;
    float m_fStartRotation;
    float m_fEndRotation;
    float m_fStartAlpha;
    float m_fEndAlpha;
    float m_fPosX;
    float m_fPosY;
    float m_fStartOffsetX;
    float m_fStartOffsetY;
    float m_fEndOffsetX;
    float m_fEndOffsetY;
};

#endif

