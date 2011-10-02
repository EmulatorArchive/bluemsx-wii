/***************************************************************
 *
 * Copyright (C) 2008-2011 Tim Brugman
 *
 * This file may be licensed under the terms of of the
 * GNU General Public License Version 2 (the ``GPL'').
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the GPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the GPL along with this
 * program. If not, go to http://www.gnu.org/licenses/gpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 ***************************************************************/
#ifndef _GUI_ROOTCONTAINER_H
#define _GUI_ROOTCONTAINER_H

#include <map>

#include "GameWindow.h"
#include "GuiContainer.h"
#include "Mutex.h"

#define GUI_MAX_LAYERS 1000

extern GameWindow *g_poGwd;

class GuiRootContainer;
class GuiTextImage;
class GuiImage;

typedef void (*GUIFUNC_MAIN)(GuiRootContainer *);

typedef std::map<GuiAtom*, int> TAtomRefMap;
typedef std::pair<GuiAtom*, int> TAtomRefPair;
typedef std::map<GuiAtom*, int>::iterator TAtomRefIterator;


class GuiRootContainer : public GuiContainer {
public:
    GuiRootContainer();
    virtual ~GuiRootContainer();

    GameWindow gwd;

    void Run(void);
    GameWindow *GetGameWindow(void) { return &gwd; };
    void Lock(void);
    void Unlock(void);
    void SetMode(GW_VIDEO_MODE mode);
    GW_VIDEO_MODE GetMode(void);
    float GetWidth(void);
    float GetHeight(void);

    // Image management
    static void RegisterAtom(GuiAtom *atom);
    static bool IsAtomRegistered(GuiAtom *atom);
    static void UseAtom(GuiAtom *atom);
    static void ReleaseAtom(GuiAtom *atom);
    static void AbandonAtom(GuiAtom *atom);
    void DeleteAllAtoms(void);

protected:
    static void RunMainFunc(void *context);
    static bool DrawFuncWrapper(void *context);
    virtual void Main(void) = 0;
    bool DrawFunc();

private:
    CMutex mutex;
    bool stop_requested;

    // Atom ref counting
    static CMutex atom_lock;
    static TAtomRefMap atom_ref;
};

#endif

