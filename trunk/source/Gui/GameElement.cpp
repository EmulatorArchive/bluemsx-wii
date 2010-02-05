#include <fat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wiisprite.h>
#include "GameElement.h"
#include "GuiImages.h"

/*************************************************
  Game Element
 *************************************************/

GameElement::GameElement()
{
    next = NULL;
    name = NULL;
    cmdline = NULL;
    screenshot[0] = NULL;
    screenshot[1] = NULL;
    image[0] = NULL;
    image[1] = NULL;
    cheatfile = NULL;
    properties = 0;
    memset(key_map, 0, sizeof(key_map)); /* set to EC_NONE */
}

GameElement::GameElement(GameElement *parent)
{
    next = NULL;
    name = NULL;
    cmdline = NULL;
    screenshot[0] = NULL;
    screenshot[1] = NULL;
    image[0] = NULL;
    image[1] = NULL;
    cheatfile = NULL;
    if( parent->name != NULL ) name = strdup(parent->name);
    if( parent->cmdline != NULL ) cmdline = strdup(parent->cmdline);
    if( parent->screenshot[0] != NULL ) screenshot[0] = strdup(parent->screenshot[0]);
    if( parent->screenshot[1] != NULL ) screenshot[1] = strdup(parent->screenshot[1]);
    if( parent->cheatfile != NULL ) cheatfile = strdup(parent->cheatfile);
    memcpy(key_map, parent->key_map, sizeof(key_map));
}

GameElement::~GameElement()
{
    if( name ) free(name);
    if( cmdline ) free(cmdline);
    if( screenshot[0] ) free(screenshot[0]);
    if( screenshot[1] ) free(screenshot[1]);
    if( image[0] ) delete image[0];
    if( image[1] ) delete image[1];
    if( cheatfile ) free (cheatfile);
}

unsigned GameElement::CalcCRC(unsigned crc)
{
    if( name ) crc = crc32(crc, (const unsigned char *)name, strlen(name)+1);
    if( cmdline ) crc = crc32(crc, (const unsigned char *)cmdline, strlen(cmdline)+1);
    if( screenshot[0] ) crc = crc32(crc, (const unsigned char *)screenshot[0], strlen(screenshot[0])+1);
    if( screenshot[1] ) crc = crc32(crc, (const unsigned char *)screenshot[1], strlen(screenshot[1])+1);
    if( cheatfile ) crc = crc32(crc, (const unsigned char *)cheatfile, strlen(cheatfile)+1);
    return crc;
}


void GameElement::SetName(const char *str)
{
    if( name ) free(name);
    if( str ) {
        name = strdup(str);
    }else{
        name = NULL;
    }
}

void GameElement::SetCommandLine(const char *str)
{
    if( cmdline ) free(cmdline);
    if( str ) {
        cmdline = strdup(str);
    }else{
        cmdline = NULL;
    }
}

void GameElement::SetScreenShot(int number, const char *str)
{
    if( number < 2 ) {
        if( screenshot[number] ) free(screenshot[number]);
        if( str ) {
            screenshot[number] = strdup(str);
        }
    }
}

void GameElement::SetKeyMapping(KEY key, int event)
{
    key_map[key] = event;
}

void GameElement::SetCheatFile(const char *str)
{
    if( cheatfile ) free(cheatfile);
    if( str ) {
        cheatfile = strdup(str);
    }else{
        cheatfile = NULL;
    }
}

void GameElement::SetProperty(GEP prop, bool value)
{
    if( value ) {
        properties |= (1 << (int)prop);
    }else{
        properties &= ~(1 << (int)prop);
    }
}

bool GameElement::GetProperty(GEP prop)
{
    return !!(properties & (1 << (int)prop));
}

char* GameElement::GetName(void)
{
    return name;
}

char* GameElement::GetCommandLine(void)
{
    return cmdline;
}

char* GameElement::GetScreenShot(int number)
{
    if( number < 2 ) {
        return screenshot[number];
    }else{
        return NULL;
    }
}

char* GameElement::GetCheatFile(void)
{
    return cheatfile;
}

int GameElement::GetKeyMapping(KEY key)
{
    return key_map[key];
}

void GameElement::FreeImage(int number)
{
    if( image[number] ) {
        delete image[number];
        image[number] = NULL;
    }
}

Image* GameElement::GetImage(int number)
{
    if( image[number] == NULL ) {
        char *filename = GetScreenShot(number);
        if( filename ) {
            char str[256];
            strcpy(str, "Screenshots/");
            strcat(str, filename);
            image[number] = new Image;
            if(image[number]->LoadImage(str) != IMG_LOAD_ERROR_NONE) {
                delete image[number];
                image[number] = NULL;
            }
        }
    }
    if( image[number] != NULL ) {
        return image[number];
    }else{
        return g_imgNoise;
    }
}

void GameElement::DeleteImage(int number)
{
    FreeImage(number);
    if( image[number] == NULL ) {
        char *filename = GetScreenShot(number);
        if( filename ) {
            char str[256];
            strcpy(str, "Screenshots/");
            strcat(str, filename);
            unlink(str);
        }
    }
}

