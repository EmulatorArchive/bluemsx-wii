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

#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpng/png.h>
#include "gamewindow.h"

#if 1
#define DEFAULT_FIFO_SIZE (256*1024)
#else
#define DEFAULT_FIFO_SIZE (1024*1024)
#endif

#ifdef WII
#define GUI_DISP_STACK_SIZE 64*1024
#endif

#ifndef WII
#include <hge/hge.h>
HGE *g_hge = NULL;
bool GameWindow::_screen_toggle = false;
#endif

// Initializes the static members
GW_VIDEO_MODE GameWindow::_mode = GW_VIDEO_MODE_INVALID;
int GameWindow::_width = 0;
int GameWindow::_height = 0;
bool GameWindow::_initialized = false;
#ifdef WII
GuiImage* GameWindow::_lastimage = NULL;
bool GameWindow::_lastimagebilinear = false;
#endif
GameWindow* GameWindow::_this = NULL;
CMutex GameWindow::_mutex;
int GameWindow::_clipx = 0, GameWindow::_clipy = 0;
int GameWindow::_clipw = 0, GameWindow::_cliph = 0;

GameWindow::GameWindow()
{
    if(_initialized)return; // Don't mess with me!
    _this = this;
    _stop_requested = false;
#ifdef WII
    _fb = NULL;
    _first = true;
    _rmode = NULL;
    _gp_fifo = NULL;
    _frameBuffer[0] = NULL; _frameBuffer[1] = NULL;
#endif
}
GameWindow::~GameWindow()
{
    StopVideo(); // Stops video automatically
}

bool GameWindow::FrameFuncWrapper()
{
    return _this->FrameFunc();
}

bool GameWindow::FrameFunc()
{
    //--------------------------------------------
    // Handle input devices
    //--------------------------------------------
    input.ScanButtons();

    //--------------------------------------------
    // Switch fullscreen
    //--------------------------------------------
#ifndef WII
    if( _screen_toggle ) {
        Lock();
        if( g_hge->System_GetState(HGE_WINDOWED) ) {
            g_hge->System_SetState(HGE_WINDOWED, false);
        }else{
            g_hge->System_SetState(HGE_WINDOWED, true);
        }
        _screen_toggle = false;
        Unlock();
    }
#endif
    return _stop_requested;
}

bool GameWindow::RenderFuncWrapper()
{
    return _this->RenderFunc();
}

bool GameWindow::RenderFunc()
{
    //float dt=g_hge->Timer_GetDelta();

    //--------------------------------------------
    // Handle graphics
    //--------------------------------------------

    _clipx = 0;
    _clipy = 0;
    _clipw = 640;
    _cliph = 440;
#ifndef WII
    Lock();
    g_hge->Gfx_BeginScene();
    g_hge->Gfx_SetClipping(_clipx, _clipy+20, _clipw, _cliph);
    Unlock();
#endif
    if( _gui_render(_gui_context) ) {
        _stop_requested = true;
    }
#ifndef WII
    Lock();
    g_hge->Gfx_EndScene();
    Unlock();
#endif
    return false;
}

void GameWindow::Lock()
{
  _mutex.Lock();
}

void GameWindow::Unlock()
{
  _mutex.Unlock();
}

bool GameWindow::ExitFuncWrapper()
{
    return _this->ExitFunc();
}

bool GameWindow::ExitFunc()
{
    _stop_requested = true;
#ifdef WII
    // TODO: Implement
#else
    TerminateThread(_gui_thread, 0);
#endif
    return false;
}

void GameWindow::GuiFuncWrapper()
{
    _this->GuiFunc();
}

void GameWindow::GuiFunc()
{
    _gui_main(_gui_context);
    _stop_requested = true;
}


void GameWindow::SetGetClipping(int *x, int *y, int *w, int *h)
{
  int xx = *x, yy = *y, ww = *w, hh = *h;
  *x = _clipx;
  *y = _clipy;
  *w = _clipw;
  *h = _cliph;
  _clipx = xx;
  _clipy = yy;
  _clipw = ww;
  _cliph = hh;
#ifndef WII
  g_hge->Gfx_SetClipping(_clipx, _clipy+20, _clipw, _cliph);
#endif
}

GW_VIDEO_MODE GameWindow::GetMode(void)
{
    return _mode;
}
void GameWindow::SetMode(GW_VIDEO_MODE mode)
{
#ifdef WII
    int yoffset = 0;
    if( mode == _mode )
        return;
    switch( mode ) {
        case GW_VIDEO_MODE_PAL50_528:
            _rmode = &TVPal528IntDf;
            break;
        case GW_VIDEO_MODE_PAL50_440:
            _rmode = &TVPal574IntDfScale;
            yoffset = 20;
            break;
        case GW_VIDEO_MODE_PAL60_440:
            _rmode = &TVEurgb60Hz480IntDf;
            yoffset = 20;
            break;
        case GW_VIDEO_MODE_NTSC_440:
            _rmode = &TVNtsc480IntDf;
            yoffset = 20;
            break;
        default:
            return;
    }
    _mode = mode;

    // Do some Init work
    VIDEO_SetBlack(false);
    VIDEO_Configure(_rmode);
    VIDEO_SetNextFramebuffer(_frameBuffer[_fb]);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if(_rmode->viTVMode&VI_NON_INTERLACE){
         VIDEO_WaitVSync();
    }
    _fb ^= 1;

    // Use these values for GetWidth() and GetHeight()
    _width = _rmode->fbWidth; _height = _rmode->efbHeight;
    if( _mode == GW_VIDEO_MODE_PAL50_440 ||
        _mode == GW_VIDEO_MODE_PAL60_440 ||
        _mode == GW_VIDEO_MODE_NTSC_440) {
        _height = 440;
    }

    // Init GX (once)
    if(_first == true){
        _gp_fifo = memalign(32, DEFAULT_FIFO_SIZE);
        memset(_gp_fifo, 0, DEFAULT_FIFO_SIZE);
        GX_Init(_gp_fifo, DEFAULT_FIFO_SIZE);
        _first = false;
    }

    // Set our background
    GXColor background = {0x00, 0x00, 0x00, 0x00};
    GX_SetCopyClear(background, 0x00ffffff);

    // Set up the display
    float yscale = 0; int xfbHeight = 0;
    yscale = GX_GetYScaleFactor(_rmode->efbHeight, _rmode->xfbHeight);
    xfbHeight = GX_SetDispCopyYScale(yscale);
    GX_SetViewport(0, 0,_rmode->fbWidth,_rmode->efbHeight, 0, 1);
    GX_SetScissor(0, 0, _rmode->fbWidth, _rmode->efbHeight);
    GX_SetDispCopySrc(0, 0, _rmode->fbWidth, _rmode->efbHeight);
    GX_SetDispCopyDst(_rmode->fbWidth, xfbHeight);
    GX_SetCopyFilter(_rmode->aa, _rmode->sample_pattern, GX_TRUE, _rmode->vfilter);

    // Some additional Init code
    GX_SetFieldMode(_rmode->field_rendering, ((_rmode->viHeight == 2*_rmode->xfbHeight)?GX_ENABLE:GX_DISABLE));

    if(_rmode->aa){
        GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    }else{
        GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    }

    GX_SetCullMode(GX_CULL_NONE);
    GX_CopyDisp(_frameBuffer[0],GX_TRUE);
    GX_SetDispCopyGamma(GX_GM_1_0);

    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0); // Positions given in 2 f32's (f32 x, f32 y)
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0); // Texture coordinates given in 2 f32's
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    GX_SetNumChans(1);
    GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHTNULL, GX_DF_NONE, GX_AF_NONE);

    GX_SetNumTexGens(1);
    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    GX_InvalidateTexAll();

    Mtx GXmodelView2D;
    Mtx44 perspective;
    // Reset the model view matrix
    guMtxIdentity(GXmodelView2D);
    guMtxTransApply(GXmodelView2D, GXmodelView2D, 0.0f, 0.0f, -5.0f);
    // Apply changes to model view matrix
    GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);

    // Set the viewing matrix to use orthographic projection
    guOrtho(perspective, 0, _rmode->efbHeight, 0, _rmode->fbWidth, 0, 300);

    // Apply changes to the projection matrix
    GX_LoadProjectionMtx(perspective, GX_ORTHOGRAPHIC);

    GX_SetViewport(0, 0, _rmode->fbWidth, _rmode->efbHeight, 0, 1);
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GX_SetAlphaUpdate(GX_TRUE);

    // The final scissor box
    GX_SetScissorBoxOffset(0, -yoffset);
    GX_SetScissor(0, 0, _width, _height);
#else
    _mode = mode;
    switch( mode ) {
        case GW_VIDEO_MODE_PAL50_528:
            _width = 640;
            _height = 528;
            break;
        case GW_VIDEO_MODE_PAL50_440:
        case GW_VIDEO_MODE_PAL60_440:
        case GW_VIDEO_MODE_NTSC_440:
            _width = 640;
            _height = 440;
            break;
        default:
            return;
    }
#endif
}

void GameWindow::InitVideo()
{
    // This Code is taken from many examples, but modified for this lib
    if(_initialized)return;

#ifdef WII
    // Allocate two framebuffers for double buffering
    TVPal574IntDfScale.xfbHeight += 16;
    if(_frameBuffer[0] == NULL ) {
        _frameBuffer[0] = MEM_K0_TO_K1(_SYS_AllocateFramebuffer(&TVPal574IntDfScale));
    }
    if(_frameBuffer[1] == NULL ) {
        _frameBuffer[1] = MEM_K0_TO_K1(_SYS_AllocateFramebuffer(&TVPal574IntDfScale));
    }
    TVPal574IntDfScale.xfbHeight -= 16;

    // Start initializing
    VIDEO_Init();

    _rmode = VIDEO_GetPreferredMode(NULL);
    if(_rmode == NULL){
        exit(0);
        return;
    }

    switch( (_rmode->viTVMode >> 2) & 7 ) {
        case VI_NTSC:
        case VI_MPAL:
        case VI_DEBUG:
            SetMode(GW_VIDEO_MODE_NTSC_440);
            break;
        case VI_EURGB60:
            SetMode(GW_VIDEO_MODE_PAL60_440);
            break;
        case VI_PAL:
        case VI_DEBUG_PAL:
        default:
            SetMode(GW_VIDEO_MODE_PAL50_440);
            break;
     }
#else
    SetMode(GW_VIDEO_MODE_PAL50_440);
    // HGE
    g_hge = hgeCreate(HGE_VERSION);
    g_hge->System_SetState(HGE_LOGFILE, L"test.log");
    g_hge->System_SetState(HGE_FRAMEFUNC, FrameFuncWrapper);
    g_hge->System_SetState(HGE_RENDERFUNC, RenderFuncWrapper);
    g_hge->System_SetState(HGE_EXITFUNC, ExitFuncWrapper);
    g_hge->System_SetState(HGE_TITLE, L"HGE Test");
    g_hge->System_SetState(HGE_USESOUND, false);
    g_hge->System_SetState(HGE_WINDOWED, true);
    g_hge->System_SetState(HGE_SCREENWIDTH, 640);
    g_hge->System_SetState(HGE_SCREENHEIGHT, 480);
    g_hge->System_SetState(HGE_SCREENBPP, 32);
    g_hge->System_SetState(HGE_FPS, HGEFPS_VSYNC);
    bool bHgeOk = g_hge->System_Initiate();
    assert( bHgeOk );
#endif
    _initialized = true;
}

void GameWindow::StopVideo(){
    if(!_initialized)return;
#ifdef WII
    void *fb1 = _frameBuffer[0];
    void *fb2 = _frameBuffer[1];

    // Prevent garbage on screen
    memset((void*)0xC1710000, 0, 640*480*2);
    _frameBuffer[_fb] = (void*)0xC1710000;
    Flush();

    // Dhewg.. You rescued our asses again.
    // This code should be run before exiting the app.
    GX_AbortFrame();

    // Thx to jepler for these quite obvious hints
    free(MEM_K1_TO_K0(fb1));
    free(MEM_K1_TO_K0(fb2));
    _frameBuffer[0] =_frameBuffer[1] = NULL;
    free(_gp_fifo); _gp_fifo = NULL;
#else
    // HGE
    g_hge->System_Shutdown();
    g_hge->Release();
#endif
    _initialized = false;
}

#ifdef WII
void *GameWindow::DisplayThreadWrapper(void *arg)
{
    GameWindow *my = (GameWindow *)arg;
    my->DisplayThread();
    return NULL;
}

void GameWindow::DisplayThread(void)
{
    while( !_stop_requested ) {
        FrameFunc();
        RenderFunc();
        Flush();
    }
}
#endif

void GameWindow::ToggleFullScreen(void)
{
#ifndef WII
    _screen_toggle = true;
#endif
}

void GameWindow::Run(GUI_FUNC cbGui, RENDER_FUNC cbRender, void *context)
{
    _gui_main = cbGui;
    _gui_render = cbRender;
    _gui_context = context;
#ifdef WII
    // Start displaying
    _gui_thread_stack = malloc(GUI_DISP_STACK_SIZE);
    LWP_CreateThread(&_gui_thread, DisplayThreadWrapper, this,
                     _gui_thread_stack, GUI_DISP_STACK_SIZE, 90);

    // Call GUI
    GuiFunc();

    // Stop display thread
    LWP_JoinThread(_gui_thread, NULL);
    free(_gui_thread_stack);
#else
    // Start GUI thread
    _gui_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GuiFuncWrapper, NULL, 0, NULL);
    // Let's rock now!
    g_hge->System_Start();
    // Close thread
    (void)WaitForSingleObject(_gui_thread, INFINITE);
    CloseHandle(_gui_thread);
#endif
}

void GameWindow::SetBackground(int r, int g, int b, int a)
{
#ifdef WII
    GXColor bgcolor;
    bgcolor.r = r;
    bgcolor.g = g;
    bgcolor.b = b;
    bgcolor.a = a;
    GX_SetCopyClear(bgcolor, 0x00ffffff);
#else
    // TODO: Implement
#endif
}

bool GameWindow::IsInitialized()
{
    return _initialized;
}

void GameWindow::Flush()
{
#ifdef WII
    GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);
    GX_CopyDisp(_frameBuffer[_fb],GX_TRUE);
    GX_DrawDone();

    VIDEO_SetNextFramebuffer(_frameBuffer[_fb]);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    _fb ^= 1;       // Flip framebuffer

    GX_InvalidateTexAll(); // Fixes some texture garbles
#endif
}

float GameWindow::GetWidth(){
    if(!_initialized)return 0;
    return (float)_width;
}

float GameWindow::GetHeight(){
    if(!_initialized)return 0;
    return (float)_height;
}

static void MyPngErrorFunction(png_structp png_ptr, const char *err_msg)
{
    fprintf(stderr, "%s", err_msg);
}

static void MyPngWarningFunction(png_structp png_ptr, const char *warn_msg)
{
    return;
}

int GameWindow::WriteScreenshot(const char *fname)
{
#ifndef WII
    // TODO: Implement
    (void)fname;
    return 1;
#else
    png_structp png_ptr = NULL;
    png_infop  info_ptr = NULL;
    u32 **row_pointers;
    int i, x, y;
    int retval = 0;
    int width = 640;
    int height = 528;
    u32 *pfb;

    FILE *outfp = fopen(fname, "wb");
    if( !outfp ) {
        fprintf(stderr, "Error creating file '%s'\n", fname);
        goto done;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                      NULL, MyPngErrorFunction, MyPngWarningFunction);
    if (!png_ptr) {
        fprintf(stderr, "Error creating write_struct\n");
        goto done;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fprintf(stderr, "Error creating write_info_struct\n");
        goto done;
    }

    png_init_io(png_ptr, outfp);

    /* set png header */
    png_set_IHDR(png_ptr, info_ptr, width, height, 8,
                 PNG_COLOR_TYPE_RGBA, PNG_FILTER_TYPE_BASE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);

    /* create 32 bpp image */
    row_pointers = (u32**)malloc(height * sizeof(char*));
    pfb = (u32 *)_frameBuffer[_fb];
    for(y = 0; y < (int)height; y++) {
        u32 *pd = (u32 *)malloc(width*sizeof(u32));
        row_pointers[y] = pd;
        for(x = 0; x < (int)(width>>1) ; x++) {
            u8 Y1, Y2, Cb, Cr;
            int R, G, B;
            u32 pix = *pfb++;
            Y1 = (pix >> 24) & 0xff;
            Cb = (pix >> 16) & 0xff;
            Y2 = (pix >> 8) & 0xff;
            Cr = pix & 0xff;
            R = (int)Y1 + 1.371f * (float)(Cr - 128);
            G = (int)Y1 - 0.698f * (float)(Cr - 128) - 0.336f * (float)(Cb - 128);
            B = (int)Y1 + 1.732f * (float)(Cb - 128);
            R = (R < 0)? 0 : ((R > 255)? 255 : R);
            G = (G < 0)? 0 : ((G > 255)? 255 : G);
            B = (B < 0)? 0 : ((B > 255)? 255 : B);
            *pd++ = (R << 24) | (G << 16) | (B << 8) | 0xff;
            R = (int)Y2 + 1.371f * (float)(Cr - 128);
            G = (int)Y2 - 0.698f * (float)(Cr - 128) - 0.336f * (float)(Cb - 128);
            B = (int)Y2 + 1.732f * (float)(Cb - 128);
            R = (R < 0)? 0 : ((R > 255)? 255 : R);
            G = (G < 0)? 0 : ((G > 255)? 255 : G);
            B = (B < 0)? 0 : ((B > 255)? 255 : B);
            *pd++ = (R << 24) | (G << 16) | (B << 8) | 0xff;
        }
    }

    png_write_image(png_ptr, (png_byte**)row_pointers);
    png_write_end(png_ptr, info_ptr);

    /* free row_pointers array */
    for(i = 0; i < (int)height; i++) {
        free(row_pointers[i]);
    }
    free(row_pointers);

    retval = 1;
done:
    if(png_ptr) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
    }
    if(outfp) {
        fclose(outfp);
    }
    return retval;
#endif
}

