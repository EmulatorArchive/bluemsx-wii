#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include "gamewindow.h"

#if 1
#define DEFAULT_FIFO_SIZE (256*1024)
#else
#define DEFAULT_FIFO_SIZE (1024*1024)
#endif

namespace wsp{
    // Initializes the static members
    GW_VIDEO_MODE GameWindow::_mode = GW_VIDEO_MODE_INVALID;
    u32 GameWindow::_width = 0;
    u32 GameWindow::_height = 0;
    bool GameWindow::_initialized = false;
    Image* GameWindow::_lastimage = NULL;
    bool GameWindow::_lastimagebilinear = false;

    GameWindow::GameWindow() :
        _fb(0), _first(true), _rmode(NULL), _gp_fifo(NULL)
    {
        if(_initialized)return; // Don't mess with me!
        _frameBuffer[0] = NULL; _frameBuffer[1] = NULL;
    }
    GameWindow::~GameWindow(){
        StopVideo(); // Stops video automatically
    }
    GW_VIDEO_MODE GameWindow::GetMode(void)
    {
        return _mode;
    }
    void GameWindow::SetMode(GW_VIDEO_MODE mode) {
        if( mode == _mode )
            return;
        switch( mode ) {
            case GW_VIDEO_MODE_PAL50_528:
                _rmode = &TVPal528IntDf;
                break;
            case GW_VIDEO_MODE_PAL50_448:
                _rmode = &TVPal574IntDfScale;
                _rmode->efbHeight = 480+8;
                _rmode->viYOrigin = 22;
                break;
            case GW_VIDEO_MODE_PAL60_448:
                _rmode = &TVEurgb60Hz480IntDf;
                _rmode->viYOrigin = 18;
                break;
            case GW_VIDEO_MODE_NTSC_448:
                _rmode = &TVNtsc480IntDf;
                _rmode->viYOrigin = 18;
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
        _width = (u32)_rmode->fbWidth; _height = (u32)_rmode->efbHeight;
        if( _mode == GW_VIDEO_MODE_PAL50_448 ||
            _mode == GW_VIDEO_MODE_PAL60_448 ||
            _mode == GW_VIDEO_MODE_NTSC_448) {
            _height = 448;
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
        f32 yscale = 0; u32 xfbHeight = 0;
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
        GX_SetScissorBoxOffset(0, 0);
        GX_SetScissor(0, 0, _width, _height);
    }

    void GameWindow::InitVideo(){
        // This Code is taken from many examples, but modified for this lib
        if(_initialized)return;

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
                SetMode(GW_VIDEO_MODE_NTSC_448);
                break;
            case VI_EURGB60:
                SetMode(GW_VIDEO_MODE_PAL60_448);
                break;
            case VI_PAL:
            case VI_DEBUG_PAL:
            default:
                SetMode(GW_VIDEO_MODE_PAL50_448);
                break;
         }

        _initialized = true;
    }

    void GameWindow::StopVideo(){
        if(!_initialized)return;

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

        _initialized = false;
    }

    void GameWindow::SetBackground(GXColor bgcolor){
        GX_SetCopyClear(bgcolor, 0x00ffffff);
    }

    bool GameWindow::IsInitialized(){
        return _initialized;
    }

    void GameWindow::Flush(){
        GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
        GX_SetColorUpdate(GX_TRUE);
        GX_CopyDisp(_frameBuffer[_fb],GX_TRUE);
        GX_DrawDone();

        VIDEO_SetNextFramebuffer(_frameBuffer[_fb]);
        VIDEO_Flush();
        VIDEO_WaitVSync();
        _fb ^= 1;       // Flip framebuffer

        GX_InvalidateTexAll(); // Fixes some texture garbles
    }

    u32 GameWindow::GetWidth(){
        if(!_initialized)return 0;
        return _width;
    }

    u32 GameWindow::GetHeight(){
        if(!_initialized)return 0;
        return _height;
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
    }
}

