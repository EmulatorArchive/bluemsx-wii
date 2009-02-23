// Code by DragonMinded
// Please attribute, nothing more

#ifndef __DRAWABLE_H
#define __DRAWABLE_H

#include <stdint.h>
#include <image.h>

#include "TextRender.h"

using namespace wsp;

class DrawableImage : public Image
{
	public:
		DrawableImage();
		~DrawableImage();

		void CreateImage(int width, int height, int format = GX_TF_RGBA8);
        void DestroyImage(void);

		u32 GetWidth() const;
		u32 GetHeight() const;
		bool IsInitialized() const;
		void BindTexture(bool bilinear = true);

		Image *GetImage();
		void CopyBuffer(u8 *buf);

		void SetFont(TextRender* text);
		void SetColor(GXColor c);
		void SetSize(int s);
        void SetYSpacing(int s);
		void RenderText(const char *fmt, ...);
		void RenderText(bool center, const char *fmt, ...);
        void GetTextSize(int *sx, int *sy, const char *fmt, ...);
        u8 *GetTextureBuffer(void);
        void FlushBuffer(void);
	private:
        void RenderTextVA(bool center, const char *fmt, va_list list);

		u32 _width, _height;
		bool _initialized;
		GXTexObj _texObj;
		u8* _pixels;
        int _format;
        int _font_size;
        int _font_yspacing;
        GXColor _font_color;

		TextRender* font;
};

#endif
