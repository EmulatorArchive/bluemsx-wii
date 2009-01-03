#include <stdio.h>
#include <string.h>
#include <fat.h>
#include <wiisprite.h>
#include "GameElement.h"
#include "GuiImages.h"

/*************************************************
  Game Element
 *************************************************/

GameElement::GameElement()
{
    name = NULL;
    cmdline = NULL;
    screenshot[0] = NULL;
    screenshot[1] = NULL;
    image[0] = NULL;
    image[1] = NULL;
    next = NULL;
}

GameElement::~GameElement()
{
    if( name ) free(name);
    if( cmdline ) free(cmdline);
    if( screenshot[0] ) free(screenshot[0]);
    if( screenshot[1] ) free(screenshot[1]);
    if( image[0] ) delete image[0];
    if( image[1] ) delete image[1];
}

void GameElement::SetName(const char *str)
{
    if( name ) free(name);
    if( str ) {
        name = (char *)malloc(strlen(str)+1);
        strcpy(name, str);
    }else{
        name = NULL;
    }
}

void GameElement::SetCommandLine(const char *str)
{
    if( cmdline ) free(cmdline);
    cmdline = (char *)malloc(strlen(str)+1);
    strcpy(cmdline, str);
}

void GameElement::SetScreenShot(int number, const char *str)
{
    if( number < 2 ) {
        if( screenshot[number] ) free(screenshot[number]);
        screenshot[number] = (char *)malloc(strlen(str)+1);
        strcpy(screenshot[number], str);
    }
}

char* GameElement::GetName()
{
    return name;
}

char* GameElement::GetCommandLine()
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

void GameElement::FreeImage(int number)
{
    if( image[number] ) {
        delete image[number];
        image[number] = NULL;
    }
}

wsp::Image* GameElement::GetImage(int number)
{
    if( image[number] == NULL ) {
        char filename[256];
        char *p = GetScreenShot(number);
        if( p ) {
            strcpy(filename, "Screenshots/");
            strcat(filename, p);
            image[number] = new wsp::Image;
            if(image[number]->LoadImage(filename) != wsp::IMG_LOAD_ERROR_NONE) {
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

