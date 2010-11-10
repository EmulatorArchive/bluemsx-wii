#include <stdio.h>
#include <string.h>
#include "../Utils/xmlwriter.h"
#include "GameList.h"
#include "../Input/InputEvent.h"

/*************************************************
  Game List
 *************************************************/

void XMLCALL GameList::startElement(void *userData, const char *name, const char **atts)
{
    GameList *my = (GameList *)userData;

    switch( my->current_container ) {
    case CONTAINER_ROOT:
        if( strcmp("GameList", name)==0 ) {
            my->current_container = CONTAINER_GAMELIST;
        }
        break;
    case CONTAINER_GAMELIST:
        if( strcmp("Game", name)==0 && strcmp("Title", atts[0])==0 ) {
            my->current_container = CONTAINER_GAME;
            if( my->current_element != NULL ) {
                // throw away unfinished elements
                delete my->current_element;
            }
            my->current_element = new GameElement();
            my->current_element->SetName(atts[1]);
        }
        break;
    case CONTAINER_GAME:
        if( strcmp("CommandLine", name)==0 ) {
            my->current_container = CONTAINER_COMMANDLINE;
        }
        if( strcmp("ScreenShot", name)==0 ) {
            my->current_container = CONTAINER_SCREENSHOT;
        }
        if( strcmp("KeyMap", name)==0 ) {
            my->current_container = CONTAINER_KEYMAP;
            for(int i = 0; atts[i] != NULL; i += 2) {
                const char *key = atts[i];
                const char *val = atts[i+1];
                if( val != NULL ) {
                    int event = inputEventStringToCode(val);
                    if( event ) {
                        for(int k = 1; k < BTN_LAST; k++) {
                            if( InputDevices::CheckButtonName((BTN)k, key) ) {
                                my->current_element->SetKeyMapping((BTN)k, event);
                            }
                        }
                    }
                }
            }
        }
        if( strcmp("Settings", name)==0 ) {
            my->current_container = CONTAINER_SETTINGS;
            for(int i = 0; atts[i] != NULL; i += 2) {
                const char *key = atts[i];
                const char *val = atts[i+1];
                if( strcmp("KeyboardJoystick", key)==0 && strcmp("true", val)==0 ) {
                    my->current_element->SetProperty(GEP_KEYBOARD_JOYSTICK, true);
                }
            }
        }
        if( strcmp("CheatFile", name)==0 ) {
            my->current_container = CONTAINER_CHEATFILE;
        }
        break;
    case CONTAINER_COMMANDLINE:
    case CONTAINER_SCREENSHOT:
    case CONTAINER_KEYMAP:
    case CONTAINER_SETTINGS:
    case CONTAINER_CHEATFILE:
        break;
    }
}

void XMLCALL GameList::endElement(void *userData, const char *name)
{
    GameList *my = (GameList *)userData;

    switch( my->current_container ) {
    case CONTAINER_ROOT:
        break;
    case CONTAINER_GAMELIST:
        my->current_container = CONTAINER_ROOT;
        break;
    case CONTAINER_GAME:
        my->current_container = CONTAINER_GAMELIST;
        // if first element, set it to first_element
        if( my->first_element == NULL ){
            my->first_element = my->current_element;
        }
        // append to linked-list
        if( my->last_element != NULL ){
            my->last_element->next = my->current_element;
        }
        my->elements++;
        // set as last element
        my->last_element = my->current_element;
        // current element again free to use
        my->current_element = NULL;
        break;
    case CONTAINER_COMMANDLINE:
        my->current_container = CONTAINER_GAME;
        my->current_element->SetCommandLine(my->receiving_string);
        if( my->receiving_string != NULL ) {
            free(my->receiving_string);
            my->receiving_string = NULL;
        }
        break;
    case CONTAINER_SCREENSHOT:
        my->current_container = CONTAINER_GAME;
        if( my->current_element->GetScreenShot(0) == NULL ) {
            my->current_element->SetScreenShot(0, my->receiving_string);
        }else{
            my->current_element->SetScreenShot(1, my->receiving_string);
        }
        if( my->receiving_string != NULL ) {
            free(my->receiving_string);
            my->receiving_string = NULL;
        }
        break;
    case CONTAINER_CHEATFILE:
        my->current_container = CONTAINER_GAME;
        my->current_element->SetCheatFile(my->receiving_string);
        if( my->receiving_string != NULL ) {
            free(my->receiving_string);
            my->receiving_string = NULL;
        }
    case CONTAINER_KEYMAP:
    case CONTAINER_SETTINGS:
        my->current_container = CONTAINER_GAME;
        break;
    }
}

void XMLCALL GameList::dataHandler(void *userData, const XML_Char *s, int len)
{
    GameList *my = (GameList *)userData;

    // strip off trailing rubbish
    if( my->receiving_string == NULL ) {
        while( len && (*s == '\t' || *s == ' ' || *s == '\r' || *s == '\n') ) {
            len--;
            s++;
        }
    }
    // still data to store?
    if( len > 0 ) {
        // is our container open?
        if( my->current_element != NULL ) {
            if( my->receiving_string == NULL ) {
                // first data chunk
                my->receiving_string = (char *)malloc(len+1);
                memcpy(my->receiving_string, s, len);
                my->receiving_string[len] = '\0';
            }else{
                // append data
                int curlen = strlen(my->receiving_string);
                my->receiving_string = (char *)realloc(my->receiving_string, curlen+len+1);
                memcpy(&my->receiving_string[curlen], s, len);
                my->receiving_string[curlen+len] = '\0';
            }
        }
    }
}

unsigned GameList::CalcCRC(unsigned crc)
{
    GameElement *p = first_element;
    while( p ) {
        crc = p->CalcCRC(crc);
        p = p->next;
    }
    return crc;
}

int GameList::Load(const char *filename)
{
    char buf[1024];
    FILE *f;

    // Clear first
    Clear();

    // Parse XML
    f = fopen(filename, "rb");
    if( f == NULL ) {
        fprintf(stderr, "Unable to open XML file\n");
    }else{
        XML_Parser parser = XML_ParserCreate(NULL);
        int done;
        XML_SetUserData(parser, this);
        XML_SetElementHandler(parser, startElement, endElement);
        XML_SetCharacterDataHandler(parser, dataHandler);
        do {
            unsigned len = (int)fread(buf, 1, sizeof(buf), f);
            done = len < sizeof(buf);
            if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
              fprintf(stderr,
                      "%s at line %lu\n",
                      XML_ErrorString(XML_GetErrorCode(parser)),
                      XML_GetCurrentLineNumber(parser));
              break;
            }
        } while (!done);
        XML_ParserFree(parser);
        fclose(f);
    }
    return elements;
}

void GameList::Save(const char *filename)
{
    XmlWriter MyXml(filename);

    MyXml.CreateTag("GameList");

    GameElement *p = first_element;
    while( p ) {
        MyXml.AddAtributes("Title", p->GetName()); 
        MyXml.CreateTag("Game");
        
        MyXml.CreateChild("CommandLine", p->GetCommandLine());

        if( p->GetProperty(GEP_KEYBOARD_JOYSTICK) ) {
            MyXml.AddAtributes("KeyboardJoystick","true"); 
            MyXml.CreateTag("Settings", true);
        }

        bool keymaps = false;
        for(int k = 1; k < BTN_LAST; k++) {
            int event = p->GetKeyMapping((BTN)k);
            if( event != EC_NONE ) {
                MyXml.AddAtributes(InputDevices::GetButtonName((BTN)k), inputEventCodeToString(event));
                keymaps = true;
            }
        }
        if( keymaps ) {
            MyXml.CreateTag("KeyMap", true);
        }
        
        MyXml.CreateChild("ScreenShot", p->GetScreenShot(0));
        MyXml.CreateChild("ScreenShot", p->GetScreenShot(1));
        
        MyXml.CloseLastTag(); // close 'Game'

        p = p->next;
    }

    MyXml.CloseAllTags();
}

void GameList::Clear(void)
{
    // Free list
    elements = 0;
    GameElement *p = first_element;
    while( p ) {
        GameElement *n = p->next;
        delete p;
        p = n;
    }
    first_element = NULL;
    last_element = NULL;
    // Free element in process
    if( current_element != NULL ) {
        delete current_element;
        current_element = NULL;
    }
    // Free strings
    if( receiving_string != NULL ) {
        free(receiving_string);
        receiving_string = NULL;
    }
}

int GameList::GetNumberOfGames(void)
{
    return elements;
}

GameElement* GameList::GetGame(int index)
{
    GameElement *p = first_element;
    while(p && index--) {
        p = p->next;
    }
    return p;
}

GameElement* GameList::RemoveFromList(int index)
{
    GameElement *p = first_element;
    GameElement *retval = NULL;
    if( index == 0 ) {
        if( first_element ) {
            elements--;
            retval = first_element;
            first_element = first_element->next;
        }
    }else{
        while( p && --index ) {
            p = p->next;
        }
        if( p && p->next ) {
            elements--;
            retval = p->next;
            p->next = p->next->next;
        }
    }
    return retval;
}

bool GameList::AddToList(int index, GameElement *element)
{
    if( !element ) {
        return false;
    }
    if( index == 0 ) {
        element->next = first_element;
        first_element = element;
    }else{
        GameElement *p = first_element;
        while( p && p->next && --index ) {
            p = p->next;
        }
        element->next = p->next;
        p->next = element;
    }
    elements++;
    return true;
}

void GameList::DeleteItem(int index)
{
    GameElement *element = RemoveFromList(index);
    if( element != NULL ) {
        delete element;
    }
}

void GameList::MoveUp(int index)
{
    if( index > 0 ) {
        AddToList(index - 1, RemoveFromList(index));
    }
}

void GameList::MoveDown(int index)
{
    AddToList(index + 1, RemoveFromList(index));
}

GameList::GameList()
{
    // Init member variables
    elements = 0;
    current_container = CONTAINER_ROOT;
    current_element = NULL;
    first_element = NULL;
    last_element = NULL;
    receiving_string = NULL;
}

GameList::~GameList()
{
    Clear();
}
