#ifndef __IMAGELOADER_H__
#define __IMAGELOADER_H__

#include <map>
#include <string>
#include "color.h"
#include "TextRender.h"
#include "DrawableImage.h"

using std::map;
using std::string;

using namespace wsp;

class wsp::Image;

class ImageLoader
{
public:
  //Load an image from a file and store it in the protected map with the string key "char * filename"
  //Return the pointer of the stored image
  Image *load(const char *filename);
  
  //Load an image from the buffer and store it in the protected map with the string key "char *name".
  //Return the pointer of the stored image
  Image *load(unsigned const char *buffer, const char *name);
  
  //Return the image pointer corresponding to "char *filename" 
  Image *get(const char *name);

  //
  DrawableImage *createImageText(const char * name,int sizeX, int sizeY, TextRender *tr, int fontSize, int color[]);

  DrawableImage *setText(const char * name, const char * text);
  
  DrawableImage *getTextImage(const char * name);
  bool reloadAll();

  bool release(const char *filename);
  bool releaseAll();

protected:
  map<string,Image *> imgs;
  map<string, DrawableImage *> txtImgs;
};

#endif
