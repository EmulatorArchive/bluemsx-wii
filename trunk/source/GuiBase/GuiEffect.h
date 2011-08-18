#ifndef _GUI_EFFECT_H
#define _GUI_EFFECT_H

#include "../GuiBase/GuiAtom.h"
#include "../GuiBase/GuiLayer.h"

class GuiEffect : public GuiAtom {
public:
    GuiEffect();
    virtual ~GuiEffect();

    virtual void Initialize(GuiLayer *from, GuiLayer *to,
                            LayerTransform tfrom=LayerTransform(), LayerTransform tto=LayerTransform()) = 0;
    virtual bool CancelLayer(GuiLayer *layer, LayerTransform *transform) = 0;
    virtual bool Run(void) = 0;
};

#endif

