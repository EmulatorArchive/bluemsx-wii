
#include "GuiRootContainer.h"
#include "GuiImage.h"
#include "GuiTextImage.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

GameWindow *g_poGwd = NULL;

CMutex GuiRootContainer::atom_lock;
TAtomRefMap GuiRootContainer::atom_ref;

GuiRootContainer::GuiRootContainer() :
                  GuiContainer(NULL, "root")
{
    SetRootContainer(this);
    g_poGwd = &gwd;
    stop_requested = false;
}

GuiRootContainer::~GuiRootContainer()
{
    DeleteAllAtoms();
    gwd.StopVideo();
    SetRootContainer(NULL);
}

/*---------------------*/

bool GuiRootContainer::DrawFuncWrapper(void *context)
{
    return ((GuiRootContainer*)context)->DrawFunc();
}

bool GuiRootContainer::DrawFunc()
{
    LayerTransform transform;
    transform.offsetX = transform.offsetY = 0.0f;
    transform.stretchWidth = transform.stretchHeight = 1.0f;
    transform.rotation = 0.0f;
    transform.alpha = 255;
    ResetTransform(transform);
    // Draw layers
    Draw();

    return stop_requested;
}

void GuiRootContainer::RunMainFunc(void *context)
{
    ((GuiRootContainer*)context)->Main();
}

void GuiRootContainer::Run(void)
{
    // Initialize GameWindow
    gwd.InitVideo();
    gwd.SetBackground(0, 0, 0, 255);

    SetWidth(gwd.GetWidth());
    SetHeight(gwd.GetHeight());

    gwd.Run(RunMainFunc, DrawFuncWrapper, this);
}

/*---------------------*/

void GuiRootContainer::Lock(void)
{
    mutex.Lock();
}

void GuiRootContainer::Unlock(void)
{
    mutex.Unlock();
}

GW_VIDEO_MODE GuiRootContainer::GetMode(void)
{
    return gwd.GetMode();
}

u32 GuiRootContainer::GetWidth(void)
{
    return gwd.GetWidth();
}

u32 GuiRootContainer::GetHeight(void)
{
    return gwd.GetHeight();
}

void GuiRootContainer::SetMode(GW_VIDEO_MODE mode)
{
    gwd.SetMode(mode);
}

// Image management

void GuiRootContainer::RegisterAtom(GuiAtom *atom)
{
    atom_lock.Lock();
    TAtomRefIterator it = atom_ref.find(atom);
    assert( it == atom_ref.end() );
    atom_ref.insert(TAtomRefPair(atom, 1));
    atom_lock.Unlock();
}

bool GuiRootContainer::IsAtomRegistered(GuiAtom *atom)
{
    atom_lock.Lock();
    TAtomRefIterator it = atom_ref.find(atom);
    bool bRegistered = ( it != atom_ref.end() );
    atom_lock.Unlock();
    return bRegistered;
}

void GuiRootContainer::UseAtom(GuiAtom *atom)
{
    atom_lock.Lock();
    TAtomRefIterator it = atom_ref.find(atom);
    assert( it != atom_ref.end() );
    (*it).second++;
    atom_lock.Unlock();
}

void GuiRootContainer::ReleaseAtom(GuiAtom *atom)
{
    atom_lock.Lock();
    TAtomRefIterator it = atom_ref.find(atom);
    assert( it != atom_ref.end() );
    if( --(*it).second == 0 ) {
        atom_ref.erase(it);
        delete atom;
    }
    atom_lock.Unlock();
}

void GuiRootContainer::DeleteAllAtoms(void)
{
    atom_lock.Lock();
    TAtomRefIterator it = atom_ref.begin();
#ifdef DEBUG
    if( it != atom_ref.end() ) {
        printf("There are unfreed resources detected\n");
        DebugBreak();
    }
#endif
    while( it != atom_ref.end() ) {
        GuiAtom *atom = (*it).first;
#ifdef DEBUG
        printf("Unfreed GuiAtom 0x%x ref = %d\n", (*it).first, (*it).second);
#endif
        atom_ref.erase(it);
        delete atom;
        it = atom_ref.begin();
    }
    atom_lock.Unlock();
}

