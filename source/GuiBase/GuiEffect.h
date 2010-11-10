#ifndef _GUI_EFFECT_H
#define _GUI_EFFECT_H

class GuiLayer;

class GuiEffect {
public:
    GuiEffect();
    virtual ~GuiEffect();

    virtual void Initialize(GuiLayer *from, GuiLayer *to) = 0;
    virtual bool Run(void) = 0;
};

#endif

