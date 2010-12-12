/*
 * GuiBase - GuiSprite, Rect
 */

#ifndef GUIBASE_SPRITE
#define GUIBASE_SPRITE

#include <stdarg.h>
#include <stdlib.h>
#include <gccore.h>

#include "GuiLayer.h"
#include "GuiImage.h"

#ifndef WII
class hgeSprite;
#endif
class GuiContainer;
class GuiTiles;
class DrawableImage;
class TextRender;

//!Basic data for a rectangle.
typedef struct {
  f32 x, //!<X position.
      y; //!<Y position.
  f32 width,  //!<Width of rectangle.
      height; //!<Height of rectangle.
} Rect;

typedef enum {
  COLL_NO_COLLISION,
  COLL_COLLISION,
  COLL_UNKNOWN
} COLL;
  
//!A basic drawable object with animation support.
class GuiSprite : public GuiLayer {
public:
    //!Constructors.
    GuiSprite(GuiContainer *parent, const char *name, GuiImage* image=NULL, int x=0, int y=0,
              int clipw=0, int cliph=0, int clipx=0, int clipy=0);
    //!Destructor.
    virtual ~GuiSprite();

    void CleanUp(void);

    //!Assigns this sprite to an image.
    //!If there is already an image with the same size, no data gets changed.
    //!\param image The image for this sprite.
    //!\param frameWidth The width of the frame. Should be a multiple of image->GetWidth() or 0 if it should get the same width as the image.
    //!\param frameHeight The height of the frame. Should be a multiple of image->GetHeight() or 0 if it should get the same height as the image.
    void SetImage(GuiImage* image, u32 clipWidth = 0, u32 clipHeight = 0, u32 clipOffsetX = 0, u32 clipOffsetY = 0);
    void SetImage(DrawableImage* drawimage, u32 clipWidth = 0, u32 clipHeight = 0, u32 clipOffsetX = 0, u32 clipOffsetY = 0);
    //!Gets the assigned image.
    //!\return A pointer to the image. NULL if no image was assigned.
    GuiImage* GetImage() const;

    // Image
    bool LoadImage(const unsigned char *buf);
    bool LoadImage(const char *file);

    // DrawableImage
    void CreateDrawImage(int width, int height, int format = GX_TF_RGBA8);
    void CreateTextImageVA(TextRender* font, int size, int minwidth, int yspace, bool center,
                             GXColor color, const char *fmt, va_list valist);
    void CreateTextImage(TextRender* font, int size, int minwidth, int yspace, bool center,
                             GXColor color, const char *fmt, ...);
    void FillSolidColor(u8 r, u8 g, u8 b);
    u8 *GetTextureBuffer(void);
    void FlushBuffer(void);

    //!Defines a collision rectangle. On startup it's the same as Image width and height.
    //!\param x Offset from the upper left corners position of the sprite.
    //!\param y Offset from the upper left corners position of the sprite.
    //!\param width The width of the collision rectangle.
    //!\param height The height of the collision rectangle.
    void DefineCollisionRectangle(f32 x, f32 y, f32 width, f32 height);
    //!Gets the current collision rectangle.
    //!\return A pointer to the rectangle.
    const Rect* GetCollisionRectangle() const;
    //!Checks if another rectangle collides with this sprite.
    //!\param rect The rectangle to check.
    //!\param x The position of the rectangle, since x in rectangle is only an offset.
    //!\param y The position of the rectangle, since y in rectangle is only an offset.
    //!\return true if it is colliding, false if not.
    COLL CollidesWith(const Rect* rect, f32 x = 0, f32 y = 0);
    //!Checks if another sprite collides with this sprite.
    //!\param sprite The sprite to check.
    //!\param complete Set this to true, if you also want to use zoom and rotation with the collision detecting.
    //!\return true if it is colliding, false if not.
    COLL CollidesWith(GuiSprite* sprite, bool complete = false);
    //!Checks if the sprite does cross a tile on the tiledlayer which number is not 0.
    //!\param tiledlayer the tiledlayer to check.
    //!\return true if it is colliding, false if not.
    COLL CollidesWith(GuiTiles* tiledlayer);

    //!Draws the GuiSprite.
    void Draw(void);
protected:
private:
    void SetImageIntern(GuiImage* image, DrawableImage* drawimage,
                        u32 clipWidth, u32 clipHeight, u32 clipOffsetX, u32 clipOffsetY);
#ifndef WII
    hgeSprite *spr;
#endif
    GuiImage* _image;
    DrawableImage* _draw_image;
    bool _image_owner;

    Rect* _colRect;
};

#endif
