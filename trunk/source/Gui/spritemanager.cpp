#include <wiisprite.h>
#include <set>
#include "spritemanager.h"
#include "TextRender.h"

using std::set;
using std::pair;

using namespace wsp;

SpriteManager::SpriteManager()
{
	images = new ImageLoader();
	
}


SpriteManager::~SpriteManager()
{
	this->releaseAll();
}


Sprite *SpriteManager::get(const char *name)
{
	string key(name);
	map<string, Sprite*>::iterator it;
	it = sprites.find(key);
	if (it == sprites.end())
		return NULL;
	else
		return it->second;

}

Sprite *SpriteManager::loadImage(const char * filename, u32 frameWidth, u32 frameHeight)
{
  
	map<string,Sprite*>::iterator it;
	string key(filename);
	Sprite *newSprite = new Sprite();
	it = sprites.find(key);
	if(it != sprites.end())
	{
		delete newSprite;
		return it->second;
	}
	else if(!images->load(filename))
		return NULL;
	else
	{
		newSprite->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
		newSprite->SetImage(images->get(filename),frameWidth,frameHeight);
		pair<string, Sprite*> pr(key, newSprite);
		sprites.insert(pr);
		return newSprite;
	}
}

Sprite *SpriteManager::loadImage(unsigned const char * buffer, const char *name, u32 frameWidth, u32 frameHeight)
{
  
	map<string,Sprite*>::iterator it;
	string key(name);
	Sprite *newSprite = new Sprite();
	it = sprites.find(key);
	if(it != sprites.end())
	{
		return it->second;
	}
	else if(!images->load(buffer, name))
		return NULL;

	
		newSprite->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
		newSprite->SetImage(images->get(name),frameWidth,frameHeight);
		pair<string, Sprite*> pr(key, newSprite);
		sprites.insert(pr);
		return newSprite;
}

Sprite *SpriteManager::changeImage(unsigned const char * buffer, const char *imageName, const char * spriteName)
{
	map<string,Sprite*>::iterator it;
	string key(spriteName);
	it = sprites.find(key);
	
	if(it == sprites.end())
		return NULL;
	else
	{
		it->second->SetImage(images->load(buffer, imageName));
		return it->second;
	}
}



Sprite * SpriteManager::loadImageText(const char * name,int sizeX, int sizeY, TextRender *tr, int fontSize, int* color)
{
	map<string,Sprite*>::iterator it;
	string key(name);
	Sprite *newSprite = new Sprite();
	it = sprites.find(key);
	if(it != sprites.end())
	{
		delete newSprite;
		return it->second;
	}
	else if(!images->createImageText(name, sizeX, sizeY, tr, fontSize, color))
		return NULL;
	else
	{
		newSprite->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
		newSprite->SetImage(images->getTextImage(name)->GetImage());
		pair<string, Sprite*> pr(key, newSprite);
		sprites.insert(pr);
		return newSprite;
	}
}

bool SpriteManager::setText(const char * name, const char * text)
{
	if(images->setText(name, text));
		return FALSE;
	return TRUE;
}	




bool SpriteManager::setSequence(const char * name, u32* seq, int seqLength)
{
	Sprite * tmpSprite = this->get(name);
	if(tmpSprite == NULL)
		return false;
	tmpSprite->SetFrameSequence(seq, seqLength);
	return true;
}

Sprite * SpriteManager::setPosition(const char * name, int x, int y)
{
	Sprite * tmpSprite = this->get(name);
	if(tmpSprite == NULL)
		return false;
	tmpSprite->SetPosition(x,y);
	return tmpSprite;

}

bool SpriteManager::setTransparency(const char * name, u32 tr)
{
	Sprite * tmpSprite = this->get(name);
	if(tmpSprite == NULL)
		return false;
	tmpSprite->SetTransparency(tr);
	return true;
}

bool SpriteManager::setZoom(const char * name, f32 zoom)
{
	Sprite * tmpSprite = this->get(name);
	if(tmpSprite == NULL)
		return false;
	tmpSprite->SetZoom(zoom);
	return true;
}

bool SpriteManager::setRotation(const char * name, f32 angle)
{
	Sprite * tmpSprite = this->get(name);
	if(tmpSprite == NULL)
		return false;
	tmpSprite->SetRotation(angle);
	return true;
}

bool SpriteManager::release(const char *name)
{
  string key(name);
  map<string,Sprite*>::iterator it = sprites.find(key);
  bool ret = false;
  
  if(it != sprites.end()) {
    delete it->second;
    sprites.erase(it);
    ret = true;
  }
  
  return ret;
}
bool SpriteManager::releaseAll()
{
  map<string,Sprite*>::iterator it;
  for(it = sprites.begin(); it != sprites.end(); ++it)
    delete it->second;
  sprites.clear();
  
  return true;
}


/*
bool SpriteManager::reloadAll()
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
*/



