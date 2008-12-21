/* 	Code by Untitled

	spritemanager.h

	Simple program to store and manage sprites

	work with libwiisprite, freetype and DragonMinded modified textrender
	require :
		+libwiisprite
		+freetype
		+ SpriteManager
		 -spritemanager.cpp
		 -imageloader.h			//inspired by ulti code (modified by untitled)
		 -imageloader.cpp		//inspired by ulti code (modified by untitled)
		+ TextRender
		 -TextRender.h			//Coded by DragonMinded (modified)
		 -TextRender.cpp		//Coded by DragonMinded (modified)
		 -DrawableImage.h		//Coded by DragonMinded (modified)
		 -DrawableImage.cpp		//Coded by DragonMinded (modified)

		
		
	The point of this code is to prevent memory leak due to undesallocated
	pointers of sprites, images... spritemanager can manage all you sprites
	containing images or text and free memory when deleted.



*/



#ifndef __SPRITEMANAGER_H__
#define __SPRITEMANAGER_H__

#include <map>
#include <string>
#include "imageloader.h"

using std::map;
using std::string;

using namespace wsp;

class wsp::Image;

class SpriteManager
{
public:

	/*Container where the sprites are stored indexed by a key string.
	This map is public, so you can get and modify directly the 
	sprite wanted if you know the key string.*/
	map<string,Sprite *> sprites;
	
	
	
	//Constructor
	SpriteManager();
	
	
	
	//Destructor
	~SpriteManager();
	
	
	
	/* --- Sprite *get(const char *filename) ---
	
	Returns the sprite pointer indexed by the key string "const char * name"
	*/
	
	Sprite *get(const char *filename);



	/* --- Sprite *loadImage(const char *filename) ---
	
	Load an Image from the path "const char * filename" and store it in a sprite
	indexed by the key string "const char * filname".
	The image is stored in "ImageLoader * images" and indexed by the same key string
	as the sprite. 
	"u32 frameWidth" and "u32 frameHeight" are the size of the frame image (submultiple of the total size)
	if you want to load and image sequence from a tiled image.
	"images" is cleared and deleted when the actual SpriteManager is deleted.
	
	Returns the pointer of the stored sprite.
	*/
	Sprite *loadImage(const char *filename, u32 frameWidth = 0, u32 frameHeight = 0);
	
	
	
	/* --- Sprite *loadImage(unsigned const char * buffer, const char *name) ---
	
	Load an Image or an image sequence from the buffer in "unsigned const char * buffer" and store it 
	in a sprite indexed by the key string "char * name".
	The image is stored in "ImageLoader * images" and indexed by the same key string
	as the sprite. 
	"u32 frameWidth" and "u32 frameHeight" are the size of the frame image (submultiple of the total size)
	if you want to load and image sequence from a tiled image.
	"images" is cleared and deleted when the actual SpriteManager is deleted.
	
	Returns the pointer of the stored sprite.
	*/
	Sprite *loadImage(unsigned const char * buffer, const char *name, u32 frameWidth = 0, u32 frameHeight = 0);
	
	
	

	
	
		/* -- bool setSequence(const char * name, u32* seq, int seqLength) ---
	
	Change the animation sequence of the sprite indexed by "const char * name".
	"u32* seq" is the desired sequence ( {0,1,3,4,5} for example) where each u32 of that sequence
	must not be larger than the total numbre of frames of the tiled image.
	"int seqLength" is the length of the sequence "u32* seq".
	
	Returns true if success.
	*/
	bool setSequence(const char * name, u32* seq, int seqLength);
	
	
	
	/* --- loadImageText(const char * name,int sizeX, int sizeY, TextRender *tr,
													 int fontSize, int * color) ---
	
	Load an image (DrawableImage* type) create using DragonMinded textrender and store it in 
	a sprite indexed by "const char * name".
	"int sizeX" and "int sizeY" are the size of the DrawableImage created.
	"int fontSize" and "int * color" are the size of the font and the color of the text.
	"TextRender *tr" is the textrender containing the font to apply to the text.
	The DrawableImage is stored in "ImageLoader * images" and indexed by the same key string
	as the sprite. "images" is cleared and deleted when the actual SpriteManager is deleted.
	
	Returns the pointer of the stored sprite.

	
	Example :
	
	#include "TextRender.h"
	...
	LayerManager manager(3);	
	SpriteManger *spManager = new SpriteManager();
	int color[3] = {255,0,0};
	...
	Textrender *tr = nex TextRender();
	tr->setFont("myfont.ttf");
	spManager->loadImageText("hello", 640, 40, tr, 18, color);
	spManager->setText("hello", "Hello World!");
	manager.Append(spManager->get("hello"));
	*/
	
	
	Sprite *loadImageText(const char * name,int sizeX, int sizeY, TextRender *tr, int fontSize, int * color);


	/* --- bool setText(const char * name, const char * text) ---
	
	Define or redefine the text of a sprite indexed by "const char * name" containing 
	a text image (DrawableImage).
	"const char * text" is the new value of the text.
	
	Returns true if success.
	*/
	
	
	
	bool setText(const char * name, const char * text);
		
	/* --- Sprite *setPosition(const char * name, int x, int y) ---
	
	Set the spite indexed by the strin gkey "const char * name" at the position defined by
	"int x" and "int y"
	Returns the pointer of the sprite.
	
	Return the pointer of the modified sprite.	
	*/
	
	
	
	Sprite *setPosition(const char * name, int x, int y);
	
	/* --- Sprite *changeImage(unsigned const char * buffer, const char *imageName, 
															const char * spriteName) ---
															
	Change the image of the sprite indexed by the key string "const char * spriteName" by
	the image loaded from the buffer "unsigned const char * buffer" that will be stored
	in the ImageLoder * images and indexed in this imageloader by the key string
	"const char *imageName"
	
	Return the pointer of the modified sprite.
	*/
	
	Sprite *changeImage(unsigned const char * buffer, const char *imageName, const char * spriteName);
	
	
	
	/* --- bool release(const char *name) ---
	
	release the sprite indexed by the string key "const char * name"
	and delete its content.
	
	Returns true if success.
	*/
	bool release(const char *name);
	
	
	/* --- bool releaseAll() ---
	
	release all the sprites and delete their content.
	
	Returns true if success.
	*/
	
	bool releaseAll();
	
	
	
	/**********************************|
	|------------- WARNING -------------
	|**********************************/
	
	/* !!! ABOUT THE FOLLOWING FUNCTIONS !!!
	IT NOT RECOMMENDED TO USE THE FOLLOWING FUNCTIONS IN A ANIMATION LOOP BECAUSE
	THEY HAVE TO SEARCH IN THE ALL MAP THE DESIRED SPRITE. THOSE FUNCTION ARE
	DESIGNED TO APPLY SOME CHANGES TO A SPRITE WHEN YOU DO NOT KNOW IF THIS SPRITE
	STILL EXISTE OR IS ALLREADY DELETED.
	
	IF YOU KNOW YOUR SPRITE STILL EXISTs, I STRONGLY ADVISE YOU TO USE :
	
		myspritemanager->sprites["nameofmysprite"]->SetTrasnparency(tr);
		myspritemanager->sprites["nameofmysprite"]->SetZoom(zoom);
		myspritemanager->sprites["nameofmysprite"]->SetRotation(angle);
		
	INSTEAD OF :
	
		myspritemanager->get("nameofmysprite")->setTransaprency(tr);
		myspritemanager->get("nameofmysprite")->setZoom(zoom);
		myspritemanager->get("nameofmysprite")->setRotation(angle);
	
	THAT IS THE REASON WHY "map<string,Sprite *> sprites" IS DECLARED AS "public"
	*/
	
	
	
	
	
	
	/* --- bool setTransparency(const char * name, u32 tr) ---
	
	Set the transparency of the sprite indexed to "const char * name" to "u32 tr"
	tr == 0 (or 0x00) --> sprite transparent.
	tr == 255 (or 0xFF) --> sprite solid.
	
	*/
	bool setTransparency(const char * name, u32 tr);
	
	
	/* --- bool setZoom(const char * name, f32 zoom) ---
	
	Set the zoom of the sprite indexed to "const char * name" to "f32 zoom"
	zoom = 1 --> initial size.
	zoom must be > 0.
	
	*/
	bool setZoom(const char * name, f32 zoom);
	
	
	/* --- bool setRotation(const char * name, f32 angle) ---
	
	Set the angle of the sprite indexed to "const char * name" to "f32 angle"
	(in degrees).
	*/
	bool setRotation(const char * name, f32 angle);
	

    //bool reloadAll();


protected:
  ImageLoader * images;
};

#endif
