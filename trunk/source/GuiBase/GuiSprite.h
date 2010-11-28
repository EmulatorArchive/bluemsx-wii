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
    GuiSprite(GuiImage* image=NULL, int x=0, int y=0);
    //!Destructor.
    virtual ~GuiSprite();

    void CleanUp(void);

    //!Assigns this sprite to an image.
    //!If there is already an image with the same size, no data gets changed.
    //!\param image The image for this sprite.
    //!\param frameWidth The width of the frame. Should be a multiple of image->GetWidth() or 0 if it should get the same width as the image.
    //!\param frameHeight The height of the frame. Should be a multiple of image->GetHeight() or 0 if it should get the same height as the image.
    void SetImage(GuiImage* image, u32 frameWidth = 0, u32 frameHeight = 0);
    void SetImage(DrawableImage* drawimage, u32 frameWidth = 0, u32 frameHeight = 0);
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

    //!Gets the current frame of the sprite.
    //!\return The frame this sprite is at.
    //!\sa \ref sprite_sequences_page
    u32 GetFrame() const;
    //!Gets the current position of the sequence.
    //!Equals to GetFrame() if sequence was not changed.
    //!\return The current position of the sequence.
    //!\sa \ref sprite_sequences_page
    u32 GetFrameSequencePos() const;
    //!Gets how long the current frame sequence is.
    //!Same as GetRawFrameCount() if no frame sequence speficied.
    //!\sa \ref sprite_sequences_page
    u32 GetFrameSequenceLength() const;
    //!Gets how many frames there are at all.
    //!\sa \ref sprite_sequences_page
    u32 GetRawFrameCount() const;
    //!Sets the current frame to a frame at the sequence.
    //!\param sequenceIndex The index in the current frame sequence.
    //!\sa \ref sprite_sequences_page
    void SetFrame(u32 sequenceIndex);
    //!Sets the current frame to the next frame in the sequence. Goes back to the first member if no frames left in the sequence.
    //!\sa \ref sprite_sequences_page
    void NextFrame();
    //!Sets the current frame to the previous frame in the sequence. Goes to the last member if the current frame is 0.
    //!\sa \ref sprite_sequences_page
    void PrevFrame();
    //!Sets the current frame sequence.
    //!\param sequence An array with data for each sequence. Each member cannot be bigger than the maximum amount of frames.
    //!\param length The length of the sequence. Gets length amount of data.
    //!\sa \ref sprite_sequences_page
    void SetFrameSequence(u32* sequence, u32 length);

    //!Draws the GuiSprite.
    void Draw(void);
protected:
private:
    void SetImageIntern(GuiImage* image, DrawableImage* drawimage, u32 frameWidth, u32 frameHeight);
    void _CalcFrame();

#ifndef WII
    hgeSprite *spr;
#endif
    GuiImage* _image;
    DrawableImage* _draw_image;
    bool _image_owner;

    Rect* _colRect;

    u32 _frame, _frameRawCount;
    u32* _frameSeq; u32 _frameSeqLength, _frameSeqPos;
    f32 _txCoords[4];
};

/*! \page sprite_sequences_page GuiSprite - Animation Sequences
 *  Animation sequences help the develeloper handle animations more easily.
 *
 *  Sequences are arrays of u32 variables. Each of these variables is a frame index
 *  of the sprite.
 *
 *  The starting sequence looks like this:
\verbatim
When you do NextFrame() on a Sprite which has 4 frames without modifying its
sequence, frames get changed like this:
[0] -> [1] -> [2] -> [3] -> [0] -> ...
This happens, when you do PrevFrame()
[0] -> [3] -> [2] -> [1] -> [0] -> ...
\endverbatim
 *  As you can see, it is pretty straightforward.
 *
 *  Now we assign an own sequence.\n
 *  When assigning a sequence, each of its members musn't be bigger than actual frames available,
 *  or it won't get set. So if we assign the sequence to u32 seq[6] = {0, 3, 1, 1, 2, 3}; this happens:
\verbatim
On NextFrame():
[0] -> [3] -> [1] -> [1] -> [2] -> [3] -> [0] -> ...
On PrevFrame():
[0] -> [3] -> [2] -> [1] -> [1] -> [3] -> [0] -> ...
\endverbatim
 *
 * The sequence just keeps on looping and only changes when you assign a new one or change to an
 * Image with fewer frames than the current one. So as you may see, sequencing can help you a lot
 * with your animation needs.
 */

#endif
