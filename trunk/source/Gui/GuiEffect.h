#ifndef _GUI_EFFECT_H
#define _GUI_EFFECT_H

#include "../WiiSprite/Layer.h"

class GuiEffect {
public:
    GuiEffect();
    virtual ~GuiEffect();

    virtual void Initialize(Layer *from, Layer *to) = 0;
    virtual bool Run(void) = 0;
};

#endif

