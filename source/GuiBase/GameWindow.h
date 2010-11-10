/*
 * GuiBase - GameWindow
 */

#ifndef GUIBASE_GAMEWINDOW
#define GUIBASE_GAMEWINDOW

#include <stdlib.h>
#include <gccore.h>

#include "InputDevices.h"

class GuiImage;

#ifndef WII
class HGE;
extern HGE *g_hge;
#endif

typedef void (*GUI_FUNC)(void *);
typedef bool (*RENDER_FUNC)(void *);

typedef enum {
    GW_VIDEO_MODE_INVALID,
    GW_VIDEO_MODE_PAL50_528,
    GW_VIDEO_MODE_PAL50_440,
    GW_VIDEO_MODE_PAL60_440,
    GW_VIDEO_MODE_NTSC_440
} GW_VIDEO_MODE;
//!Basic rendering unit.
class GameWindow{
    public:
        //!Constructor.
        GameWindow();
        //!Destructor. Does call StopVideo() automatically.
        virtual ~GameWindow();

        InputDevices input;

        void SetMode(GW_VIDEO_MODE mode);
        GW_VIDEO_MODE GetMode(void);
        static void ToggleFullScreen(void);

        //!Initializes the whole video subsystem.
        //!Should be the first command called with the library.
        void InitVideo();
        //!Shuts the video subsystem down. It won't work if Video wasn't initialized before.
        void StopVideo();
        //!Checks if the video subsystem is already initialized.
        //!\return true if initialized, false if not.
        static bool IsInitialized();
        //!Sets the background clear color
        //!\param bgcolor is a GXColor containing r, g, b and a.
        void SetBackground(int r, int g, int b, int a);
        //!Finishes rendering.
        void Flush();
        //!Write screenshot from framebuffer.
        int WriteScreenshot(const char *fname);
        //!Run GUI.
        void Run(GUI_FUNC cbGui, RENDER_FUNC cbRender, void *context);

        static void Lock();
        static void Unlock();

        static void SetGetClipping(int *x, int *y, int *w, int *h);

        //!Gets the width of the screen.
        //!\return The width of the screen if intialized. 0 if the video subsystem is not initialized.
        static u32 GetWidth();
        //!Gets the height of the screen.
        //!\return The height of the screen if intialized. 0 if the video subsystem is not initialized.
        static u32 GetHeight();
        static GuiImage* _lastimage;
        static bool _lastimagebilinear;
    protected:
        static bool FrameFuncWrapper();
        bool FrameFunc();
        static bool RenderFuncWrapper();
        bool RenderFunc();
        static bool ExitFuncWrapper();
        bool ExitFunc();
        static void GuiFuncWrapper();
        void GuiFunc();
#ifdef WII
        static void *DisplayThreadWrapper(void *arg);
        void DisplayThread(void);
#endif
    private:
        static GameWindow *_this;
        static CMutex _mutex;
#ifdef WII
        lwp_t _gui_thread;
        void *_gui_thread_stack;
#else
        static bool _screen_toggle;
        HANDLE _gui_thread;
#endif
        bool _stop_requested;
        RENDER_FUNC _gui_render;
        GUI_FUNC _gui_main;
        void *_gui_context;
#ifdef WII
        void* _frameBuffer[2]; u32 _fb;
        bool _first;
        GXRModeObj* _rmode;
        void* _gp_fifo;
#endif
        static GW_VIDEO_MODE _mode;
        static u32 _width, _height;
        static bool _initialized;
        static int _clipx, _clipy, _clipw, _cliph;

    friend class GuiImage; // Lets image access the _lastimage and _lastimagebilinear member.
};

/*! \mainpage libwiisprite Documentation
 * \image html libwiisprite.png
 * \section intro_sec Introduction
 *
 * libwiisprite is a sprite library, created to help developers make homebrew games
 * and applications for the Wii. It was coded in C++ and makes good use of OOP concepts.
 *
 * libwiisprite makes it easy and fun to make games for the Wii.
 *
 * \section Quickstart
 * Just browse the Classes from the menu above. To add all of them to your project with
 * one command, use
\code
#include <wiisprite.h>
\endcode
 *
 * If you want to start as soon as possible, use the template example in the libwiisprite/examples folder as your base, and check out the spritetest and invaders examples. These will help you understand the concepts of this library.
 *
 * \section concepts_sec Explanations
 * This is for quick explaining what we mean with all these terms we use throughout the
 * library. They can help you get started on the more "advanced" features of this library.
 *
 * \ref global_coord_page\n
 * \ref image_loadbuffer_page\n
 * \ref layermanager_append_page\n
 * \ref layermanager_viewwindows_page\n
 * \ref sprite_sequences_page\n
 * \ref tiledlayer_animations_page\n
 *
 * \section contact_sec Contact
 *
 * If you have any suggestions for the library or documentation, please mail
 * Chaosteil [at] gmail [dot] com or gummybassist [at] gmail [dot] com.
 * We are also happy to help with most of the problems that aren't explained in this
 * documentation.
 *
*/
/*! \page global_coord_page Global - Coordinate system
 * The coordinate system are the X and Y positions on the screen and in which direction everything
 * goes. It is a two dimensional (2D) cartesian coordinate system with inverted vertical axis.
 *
 * This snippet explains it in a simple way:
 \verbatim
 Coordinates:
 .------------> X (640, horizontal)
 |
 |
 |    Coordinate system starts at (0;0).
 |
 |
 V Y (480, vertical)
 \endverbatim
 * The top left is (0;0), the bottom right is (640;480). Of course the positions
 * can also be negative or bigger than said bottom right corner.
 */

#endif
