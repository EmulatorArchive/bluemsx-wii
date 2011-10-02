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
#ifndef _GUI_EFFECT_H
#define _GUI_EFFECT_H

#include "../GuiBase/GuiAtom.h"
#include "../GuiBase/GuiLayer.h"

typedef enum {
    ET_SHOW,
    ET_HIDE
} GuiEffectType;

class GuiEffect : public GuiAtom {
public:
    GuiEffect();
    virtual ~GuiEffect();

    // Required interface
    virtual void Initialize(GuiLayer *layer, GuiEffectType type,
                            LayerTransform tfrom=LayerTransform(), LayerTransform tto=LayerTransform()) = 0;
    virtual GuiEffect* Clone(void) = 0;
    virtual void Cancel(LayerTransform *transform) = 0;
    virtual bool Run(void) = 0;
};

#endif

