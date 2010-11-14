#ifndef _GUI_EFFECT_H
#define _GUI_EFFECT_H

#ifndef GUI_PI
#define GUI_PI      3.14159265358979323846f
#define GUI_PI_2    1.57079632679489661923f
#define GUI_PI_4    0.785398163397448309616f
#define GUI_1_PI    0.318309886183790671538f
#define GUI_2_PI    0.636619772367581343076f
#endif

class GuiLayer;

class GuiEffect {
public:
    GuiEffect();
    virtual ~GuiEffect();

    virtual void Initialize(GuiLayer *from, GuiLayer *to) = 0;
    virtual bool Run(void) = 0;
};

#endif

