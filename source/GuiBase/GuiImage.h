/***************************************************************
 *
 * libwiisprite 0.3.0d source code license.
 * Copyright 2008, 2009, 2010 by Chaosteil, Feesh!, Arikado.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 *    must not claim that you wrote the original software. If you
 *    use this software in a product, an acknowledgment in the
 *    product documentation would be appreciated but is not required.
 * 2. Any commercial application using this software is required to
 *    give a percentage of it's sales to the authors determined via
 *    an an arranged agreement between the author and the commercial
 *    seller.
 * 3. Altered source versions must be plainly marked as such, and
 *    must not be misrepresented as being the original software.
 *    They are subject to the same restrictions listed here as the
 *    unaltered source.
 * 4. This notice may not be removed or altered from any source
 *    distribution.
 *
 ***************************************************************/
#ifndef GUIBASE_IMAGE
#define GUIBASE_IMAGE

#include <stdlib.h>
#ifndef WII
#include <hge/hge.h>
#endif
#include <gccore.h>
#include <libpng/png.h>

#include "GuiAtom.h"

//!Error codes when loading images.
enum IMG_LOAD_ERROR{
    IMG_LOAD_ERROR_NONE = 0, //!< GuiImage successfully loaded.
    IMG_LOAD_ERROR_NOT_FOUND, //!< File was not found.
    IMG_LOAD_ERROR_INV_PNG, //!< Invalid PNG header.
    IMG_LOAD_ERROR_PNG_FAIL, //!< Invalid PNG data.
    IMG_LOAD_ERROR_WRONG_SIZE, //!< The images width and height should both be a multiple of 4.
    IMG_LOAD_ERROR_ALREADY_INIT //!< Returns this if the image was already initialized.
};

//!Specifies how to load the image.
enum IMG_LOAD_TYPE{
    IMG_LOAD_TYPE_PATH = 0, //!< Loads the image from a path on the filesystem.
    IMG_LOAD_TYPE_BUFFER //!< Loads the image from the provided buffer.
};

//!Stores imagedata and is capable of loading pngs.
class GuiImage : public GuiAtom {
public:
    //!Constructors.
    GuiImage();
    //!Destructor.
    virtual ~GuiImage();

#ifndef WII
    HTEXTURE GetTEX() { return m_texObj; };
#endif

    //!Loads an image from a file. Width and height have to be a multiple of 4, or it won't load.
    //!Once an GuiImage is initialized, it can't be initialized again.
    //!Uses fopen(), so some basic filesystem initialization is required.
    //!\param path The path to the file.
    //!\param loadtype Set this to how you want to load this image. (This is probably a path)
    //!\return An error code based on loading status.
    //!\sa \ref image_loadbuffer_page
    IMG_LOAD_ERROR LoadImage(const char* path, IMG_LOAD_TYPE loadtype = IMG_LOAD_TYPE_PATH);
    //!\overload
    //!\param path The path to the file. (This is probably a buffer)
    //!\param loadtype Set this to how you want to load this image.
    //!\sa \ref image_loadbuffer_page
    IMG_LOAD_ERROR LoadImage(const unsigned char* path, IMG_LOAD_TYPE loadtype = IMG_LOAD_TYPE_BUFFER);

    //!Cleans up the image to be able to reload it again.
    virtual void DestroyImage();
    //!Gets the width of this image.
    //!\return The width of the image. Returns 0 if no image is loaded.
    virtual float GetWidth() const;
    //!Gets the height of this image.
    //!\return The height of the image. Returns 0 if no image is loaded.
    virtual float GetHeight() const;

    //!Checks if the image is loaded.
    //!\return true if an image is stored, false if not.
    virtual bool IsInitialized() const;

    //!Used to select this image for the next texturing process.
    //!Most of the time you won't gonna use this.
    //!\param bilinear Turns on bilinear filtering.
    virtual void BindTexture(bool bilinear = true);
protected:
    //!Initializes a clear image to draw on. It will be displayed as an RGBA8 texture, so its format should be ARGB. Don't mess with it if you don't know what you're doing.
    //!Once an GuiImage is initialized, it can't be initialized again.
    //!\param width The new width of the image.
    //!\param height The new height of the image.
    //!\return True if it was successfully initialized, false if not.
    bool _InitializeImage(int width, int height, int bytespp = 4);
    //!Use this method if you've drawn something in the pixeldata.
    void _Flush();


#ifdef WII
    GXTexObj m_texObj;
    u8* m_pixels; //!< Stores the pixeldata of this image. Use carefully.
#else
    HTEXTURE m_texObj;
    int m_tex_width, m_tex_height;
#endif
    png_uint_32 m_width, m_height, m_bytespp;
    bool m_initialized;
private:
    void _ConvertTexture(u8 *blitbuf, png_byte color_type, int channels, png_bytep* row_pointers);
};

/*! \page image_loadbuffer_page GuiImage - Loading from buffer
 * \section image_loadbuffer_intro_sec Intro
 * Most of the time you'll want to load an image from the SD card. This is done easily by just specifying
 * the path of the image based on the location of the boot.[dol|elf| file. That means, if your boot file
 * is in /apps/foo and your image is /apps/foo/bar.png, then
\code
image.LoadImage("/apps/foo/bar.png");
\endcode
 * equals to
\code
image.LoadImage("bar.png");
\endcode
 *
 * If you're using wiiload, think that the boot file is running from root.
 *
 * There are some situations however, where you want to load from a buffer. Say, you have a very small
 * application you just want to run very fast. Or maybe you have your own package system where you
 * can extract each file with its own buffer. Anyway, you will need a way to load an image directly,
 * and not through the filesystem.
 *
 * \section image_loadbuffer_preparing_sec Preparing the image
 * If you have your own method for loading something into an unsigned char array, skip this section.
 *
 * Since you are using devkit, there are some nice tools for you already available. If you look into
 * the devkitPPC/bin folder, there is a tool called raw2c. It can convert any file to a unsigned char
 * buffer. We're going to use this tool. To use it, simply specify you image as an argument when starting
 * the program. Or you could just drag and drop your image onto it. Either way you are presented with
 * a *.c and *.h file. Copy these files into your source folder and include the header file at the
 * point where you want to have access to it. The header file will have one unsigned char[] member,
 * which we will are going to use for loading.
 *
 * \section image_loadbuffer_loading_sec Loading the buffer
 * libwiisprite assumes that if you're loading a unsigned char array, that it's a buffer. If you're
 * just entering a simple char array ("foo/bar.png") it will assume that it's loading from a path.
 * You can take control over this behaviour with IMG_LOAD_TYPE.
 *
 * Let's say you converted bar.png with raw2c and included the header file in your source.
 * Now you just need to type the command
\code
image.LoadImage(bar);
\endcode
 * and it should load just fine!
 *
 */

#endif
