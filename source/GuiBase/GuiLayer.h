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
#ifndef GUIBASE_LAYER
#define GUIBASE_LAYER

#include <stdlib.h>
#include <gccore.h>

#include "GuiAtom.h"
#include "Mutex.h"

#ifndef GUI_PI
#define GUI_2PI     6.28318530717958647692f
#define GUI_PI      3.14159265358979323846f
#define GUI_PI_2    1.57079632679489661923f
#define GUI_PI_4    0.785398163397448309616f
#define GUI_1_PI    0.318309886183790671538f
#define GUI_2_PI    0.636619772367581343076f
#endif

#define ROUND(f) (int)((float)(f) + 0.5f)

class GuiContainer;

class LayerTransform {
public:
  LayerTransform()
  {
    valid = false;
    offsetX = 0;
    offsetY = 0;
    stretchWidth = 1.0f;
    stretchHeight = 1.0f;
    rotation = 0.0f;
    alpha = 1.0f;
  };
  ~LayerTransform()
  {
  };
  bool valid;
  float offsetX;
  float offsetY;
  float stretchWidth;
  float stretchHeight;
  float rotation;
  float alpha;
};

class GuiLayer : public GuiAtom {
public:
    GuiLayer(GuiContainer *parent, const char *name);
    virtual ~GuiLayer();

    GuiContainer* GetParent(void) const { return m_poParent; }
    const char* GetName(void) const { return m_psName; }

    float GetHeight(void) const { return m_fHeight; }
    float GetWidth(void) const { return m_fWidth; }

    float GetX(void) const { return m_fX; }
    float GetY(void) const { return m_fY; }

    bool IsVisible() const { return m_bVisible; }
    void SetVisible(bool visible) { m_bVisible = visible; }

    void SetAlpha(float alpha) { m_fAlpha = alpha; }
    float GetAlpha(void) const { return m_fAlpha; }

    void SetZoom(float zoom) { m_fZoomX = m_fZoomY = zoom; }
    void SetZoomX(float zoom_x) { m_fZoomX = zoom_x; }
    void SetZoomY(float zoom_y) { m_fZoomY = zoom_y; }
    float GetZoomX(void) const { return m_fZoomX; }
    float GetZoomY(void) const { return m_fZoomY; }
    void SetScaledWidth(float scaled_width);
    void SetScaledHeight(float scaled_height);
    float GetScaledWidth(void);
    float GetScaledHeight(void);

    void SetRefPixelPosition(float x, float y) { m_fRefPixelX = x; m_fRefPixelY = y; }
    void SetRefPixelPosition(int x, int y) { m_fRefPixelX = (float)x; m_fRefPixelY = (float)y; }
    float GetRefPixelX(void) const { return m_fRefPixelX; }
    float GetRefPixelY(void) const { return m_fRefPixelY; }

    void SetRotation(float rotation) { m_fRotation = rotation; }
    float GetRotation(void) const { return m_fRotation; }

    void SetPosition(float x, float y) { m_fX = x; m_fY = y; }
    void SetPosition(int x, int y) { m_fX = (float)x; m_fY = (float)y; }

    void SetWidth(float width) { m_fWidth = width; }
    void SetWidth(int width) { m_fWidth = (float)width; }
    void SetHeight(float height) { m_fHeight = height; }
    void SetHeight(int height) { m_fHeight = (float)height; }

    virtual bool IsInVisibleArea(float x, float y);
    virtual bool IsInVisibleArea(GuiLayer *layer);
    virtual bool IsBusy(void);
    virtual void Draw(void);

    virtual void ResetTransform(LayerTransform transform);
    virtual void DoTransform(LayerTransform transform);
    virtual LayerTransform GetTransform(void) const { return m_oTransform; }

private:
    const char *m_psName;
    GuiContainer *m_poParent;
    bool m_bVisible;
    LayerTransform m_oTransform;
    float m_fHeight, m_fWidth;
    float m_fX, m_fY;
    float m_fAlpha, m_fAlphaoff;
    float m_fZoomX, m_fZoomY;
    float m_fRefPixelX, m_fRefPixelY, m_fRefWidth, m_fRefHeight;
    float m_fRotation;
};

#endif
