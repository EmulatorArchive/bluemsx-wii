#include <wiisprite.h>
#include <set>
#include "imageloader.h"

using std::set;
using std::pair;

using namespace wsp;

Image *ImageLoader::get(const char *name)
{
  string key(name);
  map<string,Image*>::iterator it;
  it = imgs.find(key);
  if(it == imgs.end())
    return NULL;
  
  return it->second;
}

Image *ImageLoader::load(unsigned const char * buffer, const char *name)
{
  map<string,Image*>::iterator it;
  string key(name);
  
  Image *newimg = new Image();
  if(newimg->LoadImage(buffer)) {
    // failed to load
    delete newimg;
    it = imgs.find(key);
    if(it == imgs.end())
      return NULL;
    return it->second;
  }
  
  it = imgs.find(key);
  if(it != imgs.end()) {
    delete it->second;
    imgs.erase(it);
  }
  
  pair<string,Image*> pr(key,newimg);
  imgs.insert(pr);
  return newimg;
}


Image *ImageLoader::load(const char *filename)
{
  map<string,Image*>::iterator it;
  string key(filename);
  
  Image *newimg = new Image();
  if(newimg->LoadImage(filename)) {
    // failed to load
    delete newimg;
    it = imgs.find(key);
    if(it == imgs.end())
      return NULL;
    return it->second;
  }
  
  it = imgs.find(key);
  if(it != imgs.end()) {
    delete it->second;
    imgs.erase(it);
  }
  
  pair<string,Image*> pr(key,newimg);
  imgs.insert(pr);
  return newimg;
}

DrawableImage* ImageLoader::createImageText(const char * name,int sizeX, int sizeY, TextRender *tr, int fontSize, int *color)
{
	map<string,DrawableImage*>::iterator it;
	string key(name);
  
  
	DrawableImage *newimg = new DrawableImage();
	it = txtImgs.find(key);
	if(it != txtImgs.end()) {
		delete it->second;
		txtImgs.erase(it);
	} else
    newimg->CreateImage(sizeX, sizeY);
	newimg->SetFont(tr);
	Color colorT = Color(color[1],color[2],color[3]);
	newimg->SetColor(colorT.getGXColor());
	newimg->SetSize(fontSize);
	newimg->RenderText("teeesssttttt");

	pair<string,DrawableImage*> pr(key,newimg);
	txtImgs.insert(pr);

	return newimg;
}
DrawableImage* ImageLoader::getTextImage(const char * name)
{
	  string key(name);
  map<string,DrawableImage*>::iterator it;
  it = txtImgs.find(key);
  if(it == txtImgs.end())
    return NULL;
  
  return it->second;
}

DrawableImage* ImageLoader::setText(const char * name, const char * text)
{
  string key(name);
  map<string,DrawableImage*>::iterator it;
  it = txtImgs.find(key);
  if(it == txtImgs.end())
    return NULL;
  
  it->second->RenderText(text);
  return it->second;

}

bool ImageLoader::reloadAll()
{
  map<string,Image*>::iterator it;
  set<string> toreload;
  set<string>::iterator sit;
  bool ret = true;

  for(it = imgs.begin(); it != imgs.end(); ++it)
    toreload.insert(it->first);
  
  for(sit = toreload.begin(); sit != toreload.end(); ++sit) {
    if(!this->load(sit->c_str()))
      ret = false;
  }
  
  return ret;
}

bool ImageLoader::release(const char *filename)
{
  string key(filename);
  map<string,Image*>::iterator it = imgs.find(key);
  bool ret = false;
  
  if(it != imgs.end()) {
    delete it->second;
    imgs.erase(it);
    ret = true;
  }
  
  return ret;
}

bool ImageLoader::releaseAll()
{
	map<string,Image*>::iterator it1;
	for(it1 = imgs.begin(); it1 != imgs.end(); ++it1)
		delete it1->second;
	imgs.clear();
	map<string, DrawableImage*>::iterator it2;
	for(it2 = txtImgs.begin(); it2 != txtImgs.end(); ++it2)
		delete it2->second;
	txtImgs.clear();
  
  return true;
}
