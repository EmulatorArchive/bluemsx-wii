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

#include "GuiLayer.h"
#include "GuiContainer.h"
#include <math.h>

GuiLayer::GuiLayer(GuiContainer *parent, const char *name)
{
    m_poParent = parent;
    m_psName = name;
    m_fRotation = 0.0f;
    m_fHeight = 0.0f;
    m_fWidth = 0.0f;
    m_fAlpha = 1.0f;
    m_fX = 0.0f;
    m_fY = 0.0f;
    m_bVisible = true;
    m_fRefPixelX = 0.0f;
    m_fRefPixelY = 0.0f;
    m_fZoomX = 1.0f;
    m_fZoomY = 1.0f;

    m_oTransform.valid = false;
    m_oTransform.offsetX = 0;
    m_oTransform.offsetY = 0;
    m_oTransform.stretchWidth = 1.0f;
    m_oTransform.stretchHeight = 1.0f;
    m_oTransform.rotation = 0.0f;
    m_oTransform.alpha = 1.0f;
}

GuiLayer::~GuiLayer(){
}

void GuiLayer::SetScaledWidth(float scaled_width)
{
    m_fZoomX = (scaled_width-1) / m_fWidth;
}
void GuiLayer::SetScaledHeight(float scaled_height)
{
    m_fZoomY = (scaled_height-1) / m_fHeight;
}

float GuiLayer::GetScaledWidth(void)
{
    return m_fWidth * m_fZoomX;
}

float GuiLayer::GetScaledHeight(void)
{
    return m_fHeight * m_fZoomY;
}

bool GuiLayer::IsBusy(void)
{
    return false;
}

bool GuiLayer::IsInVisibleArea(float x, float y)
{
    return x >= m_fX && y >= m_fY && x < (m_fX + m_fWidth * m_fZoomX) && y < (m_fY + m_fHeight * m_fZoomY);
}

bool GuiLayer::IsInVisibleArea(GuiLayer *layer)
{
    return IsInVisibleArea(layer->GetX() + layer->GetRefPixelX(),
                           layer->GetY() + layer->GetRefPixelY());
}

void GuiLayer::ResetTransform(LayerTransform transform)
{
    m_oTransform.offsetX = m_fX;
    m_oTransform.offsetY = m_fY;
    m_oTransform.stretchWidth = m_fZoomX;
    m_oTransform.stretchHeight = m_fZoomY;
    m_oTransform.rotation = m_fRotation;
    m_oTransform.alpha = m_fAlpha;
    m_oTransform.valid = true;
    DoTransform(transform);
}

void GuiLayer::DoTransform(LayerTransform transform)
{
    m_oTransform.offsetX += transform.offsetX;
    m_oTransform.offsetY += transform.offsetY;
    m_oTransform.stretchWidth *= transform.stretchWidth;
    m_oTransform.stretchHeight *= transform.stretchHeight;
    m_oTransform.rotation = (float)fmod(m_oTransform.rotation + transform.rotation, 360.0f);
    m_oTransform.alpha = m_oTransform.alpha * transform.alpha;
}

void GuiLayer::Draw(void)
{
    // Can be overloaded to do something usefull
}

