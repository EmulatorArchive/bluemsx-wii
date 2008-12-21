// Code by DragonMinded
// Please attribute, nothing more

#ifndef __TEXTRENDER_H
#define __TEXTRENDER_H

#define DEFAULT_X 5
#define DEFAULT_Y 5

#define DEFAULT_Y_CUSHION 2
#define DEFAULT_TAB_SPACE 50

#include <stdint.h>
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <gccore.h>

using std::string;

class TextRender
{
	public:
		TextRender();
		~TextRender();

		void SetFont(string font);
		void SetFont(const unsigned char* font, u32 size);
		void SetColor(GXColor c);
		void SetSize(int s);

		void SetBuffer(uint8_t *buf, int width, int height);
		void RenderSimple(const char *out);
		void Render(const char *fmt, ...);
	private:
		void Blit(FT_Bitmap *bmp, int bmpWidth, int bmpHeight);

		FT_Library library;
		FT_Face face;

		uint8_t *_buf;
		int _width;
		int _height;
		int _fontheight;
		GXColor _color;
};

#endif


