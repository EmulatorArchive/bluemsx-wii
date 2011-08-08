/*
** Haaf's Game Engine 1.8
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** Core functions implementation: graphics
*/

#include "hge_impl.h"
#include <d3d8.h>
#ifndef UNDER_CE
#include <d3dx8.h>
#endif


void CALL HGE_Impl::Gfx_Clear(DWORD color)
{
    if(pCurTarget)
    {
        if(pCurTarget->pDepth)
            pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0 );
        else
            pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET, color, 1.0f, 0 );
    }
    else
    {
        if(bZBuffer)
            pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0 );
        else
            pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET, color, 1.0f, 0 );
    }
}

void CALL HGE_Impl::Gfx_SetClipping(int x, int y, int w, int h)
{
    D3DVIEWPORT8 vp;
    int scr_width, scr_height;

    if(!pCurTarget) {
        scr_width=pHGE->System_GetStateInt(HGE_SCREENWIDTH);
        scr_height=pHGE->System_GetStateInt(HGE_SCREENHEIGHT);
    }
    else {
        scr_width=Texture_GetWidth((HTEXTURE)pCurTarget->pTex);
        scr_height=Texture_GetHeight((HTEXTURE)pCurTarget->pTex);
    }

    if(!w) {
        vp.X=0;
        vp.Y=0;
        vp.Width=scr_width;
        vp.Height=scr_height;
    }
    else
    {
        if(x<0) { w+=x; x=0; }
        if(y<0) { h+=y; y=0; }

        if(x+w > scr_width) w=scr_width-x;
        if(y+h > scr_height) h=scr_height-y;

        vp.X=x;
        vp.Y=y;
        vp.Width=w;
        vp.Height=h;
    }

    vp.MinZ=0.0f;
    vp.MaxZ=1.0f;

    _render_batch();
    pD3DDevice->SetViewport(&vp);

    D3DXMATRIX tmp;
    D3DXMatrixScaling(&matProj, 1.0f, -1.0f, 1.0f);
    D3DXMatrixTranslation(&tmp, -0.5f, +0.5f, 0.0f);
    D3DXMatrixMultiply(&matProj, &matProj, &tmp);
    D3DXMatrixOrthoOffCenterLH(&tmp, (float)vp.X, (float)(vp.X+vp.Width), -((float)(vp.Y+vp.Height)), -((float)vp.Y), vp.MinZ, vp.MaxZ);
    D3DXMatrixMultiply(&matProj, &matProj, &tmp);
#ifdef UNDER_CE
    pD3DDevice->SetTransform(D3DTS_PROJECTION, (D3DMMATRIX*)&matProj, D3DMFMT_D3DMVALUE_FLOAT);
#else
    pD3DDevice->SetTransform(D3DTS_PROJECTION, &matProj);
#endif
}

bool CALL HGE_Impl::Gfx_BeginScene(HTARGET targ)
{
    LPDIRECT3DSURFACE8 pSurf=0, pDepth=0;
    D3DDISPLAYMODE Mode;
    CRenderTargetList *target=(CRenderTargetList *)targ;

    //
    // Test if we currently have access to the 3D surface
    // When not, for example because we lost focus, return false and try again later.
    // When we just got it back, restore some stuff
    //
    HRESULT hr = pD3DDevice->TestCooperativeLevel();
    if (hr == D3DERR_DEVICELOST) return false;
    else if (hr == D3DERR_DEVICENOTRESET)
    {
        if(bWindowed)
        {
            if(FAILED(pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &Mode)) || Mode.Format==D3DFMT_UNKNOWN) 
            {
                _PostError("Can't determine desktop video mode");
                return false;
            }

            d3dppW.BackBufferFormat = Mode.Format;
            if(_format_id(Mode.Format) < 4) nScreenBPP=16;
            else nScreenBPP=32;
        }

        if(!_GfxRestore()) return false; 
    }

    //
    // Sanity check, make sure we're not alreay busy drawing a scene
    //
    if(VertArray)
    {
        _PostError("Gfx_BeginScene: Scene is already being rendered");
        return false;
    }
    
    if(target != pCurTarget)
    {
        if(target)
        {
            target->pTex->GetSurfaceLevel(0, &pSurf);
            pDepth=target->pDepth;
        }
        else
        {
            pSurf=pScreenSurf;
            pDepth=pScreenDepth;
        }
        if(FAILED(pD3DDevice->SetRenderTarget(pSurf, pDepth)))
        {
            if(target) pSurf->Release();
            _PostError("Gfx_BeginScene: Can't set render target");
            return false;
        }
        if(target)
        {
            pSurf->Release();
            if(target->pDepth) pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ); 
            else pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE ); 
            _SetProjectionMatrix(target->width, target->height);
        }
        else
        {
            if(bZBuffer) pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ); 
            else pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
            _SetProjectionMatrix(nScreenWidth, nScreenHeight);
        }

#ifdef UNDER_CE
        pD3DDevice->SetTransform(D3DTS_PROJECTION, (D3DMMATRIX*)&matProj, D3DMFMT_D3DMVALUE_FLOAT);
        pD3DDevice->SetTransform(D3DTS_VIEW, (D3DMMATRIX*)&matView, D3DMFMT_D3DMVALUE_FLOAT);
#else
        pD3DDevice->SetTransform(D3DTS_PROJECTION, &matProj);
        pD3DDevice->SetTransform(D3DTS_VIEW, &matView);
#endif
        D3DXMatrixIdentity(&matView);

        pCurTarget=target;
    }

    pD3DDevice->BeginScene();
#ifdef UNDER_CE
    pVB->Lock( 0, 0, (void**)&VertArray, 0 );
#else
    pVB->Lock( 0, 0, (BYTE**)&VertArray, 0 );
#endif
    return true;
}

void CALL HGE_Impl::Gfx_EndScene()
{
    _render_batch(true);
    pD3DDevice->EndScene();
    if(!pCurTarget) pD3DDevice->Present( NULL, NULL, NULL, NULL );
}

void CALL HGE_Impl::Gfx_RenderQuad(const hgeQuad *quad)
{
	if(VertArray)
	{
		if(CurPrimType!=HGEPRIM_QUADS || nPrim>=VERTEX_BUFFER_SIZE/HGEPRIM_QUADS || CurTexture!=quad->tex || CurBlendMode!=quad->blend)
		{
			_render_batch();

			CurPrimType=HGEPRIM_QUADS;
			if(CurBlendMode != quad->blend) _SetBlendMode(quad->blend);
			if(quad->tex != CurTexture)
			{
				pD3DDevice->SetTexture( 0, (LPDIRECT3DTEXTURE8)quad->tex );
				CurTexture = quad->tex;
			}
		}

		memcpy(&VertArray[nPrim*HGEPRIM_QUADS], quad->v, sizeof(hgeVertex)*HGEPRIM_QUADS);
		nPrim++;
	}
}

HTARGET CALL HGE_Impl::Target_Create(int width, int height, bool zbuffer)
{
    CRenderTargetList *pTarget;
    D3DSURFACE_DESC TDesc;

    pTarget = new CRenderTargetList;
    pTarget->pTex=0;
    pTarget->pDepth=0;

    if(FAILED(D3DXCreateTexture(pD3DDevice, width, height, 1, D3DUSAGE_RENDERTARGET,
                        d3dpp->BackBufferFormat, D3DPOOL_DEFAULT, &pTarget->pTex)))
    {
        _PostError("Can't create render target texture");
        delete pTarget;
        return 0;
    }

    pTarget->pTex->GetLevelDesc(0, &TDesc);
    pTarget->width=TDesc.Width;
    pTarget->height=TDesc.Height;

    if(zbuffer)
    {
        if(FAILED(pD3DDevice->CreateDepthStencilSurface(pTarget->width, pTarget->height,
                        D3DFMT_D16, D3DMULTISAMPLE_NONE, &pTarget->pDepth)))
        {   
            pTarget->pTex->Release();
            _PostError("Can't create render target depth buffer");
            delete pTarget;
            return 0;
        }
    }

    pTarget->next=pTargets;
    pTargets=pTarget;

    return (HTARGET)pTarget;
}

void CALL HGE_Impl::Target_Free(HTARGET target)
{
    CRenderTargetList *pTarget=pTargets, *pPrevTarget=NULL;

    while(pTarget)
    {
        if((CRenderTargetList *)target == pTarget)
        {
            if(pPrevTarget)
                pPrevTarget->next = pTarget->next;
            else
                pTargets = pTarget->next;

            if(pTarget->pTex) pTarget->pTex->Release();
            if(pTarget->pDepth) pTarget->pDepth->Release();

            delete pTarget;
            return;
        }

        pPrevTarget = pTarget;
        pTarget = pTarget->next;
    }
}

HTEXTURE CALL HGE_Impl::Target_GetTexture(HTARGET target)
{
    CRenderTargetList *targ=(CRenderTargetList *)target;
    if(target) return (HTEXTURE)targ->pTex;
    else return 0;
}

HTEXTURE CALL HGE_Impl::Texture_Create(int width, int height)
{
    LPDIRECT3DTEXTURE8 pTex;

#ifdef UNDER_CE
    int w, h;
    for(w = 256; w < width; w <<= 1);
    for(h = 2; h < height; h <<= 1);
    if( FAILED( D3DXCreateTexture( pD3DDevice, w, h,
                                        1,                  // Mip levels
                                        D3DMUSAGE_DYNAMIC | D3DMUSAGE_LOCKABLE, // Usage
                                        D3DFMT_A8R8G8B8,    // Format
                                        D3DMPOOL_VIDEOMEM,  // Memory pool
                                        &pTex ) ) )
#else
    if( FAILED( D3DXCreateTexture( pD3DDevice, width, height,
                                        1,                  // Mip levels
                                        0,                  // Usage
                                        D3DFMT_A8R8G8B8,    // Format
                                        D3DPOOL_MANAGED,    // Memory pool
                                        &pTex ) ) )
#endif
    {   
        _PostError("Can't create texture");
        return NULL;
    }

    return (HTEXTURE)pTex;
}

void CALL HGE_Impl::Texture_Free(HTEXTURE tex)
{
    LPDIRECT3DTEXTURE8 pTex=(LPDIRECT3DTEXTURE8)tex;

    if(pTex != NULL) pTex->Release();
}

int CALL HGE_Impl::Texture_GetWidth(HTEXTURE tex)
{
    D3DSURFACE_DESC TDesc;
    LPDIRECT3DTEXTURE8 pTex=(LPDIRECT3DTEXTURE8)tex;

    if(FAILED(pTex->GetLevelDesc(0, &TDesc))) return 0;
    else return TDesc.Width;
}


int CALL HGE_Impl::Texture_GetHeight(HTEXTURE tex)
{
    D3DSURFACE_DESC TDesc;
    LPDIRECT3DTEXTURE8 pTex=(LPDIRECT3DTEXTURE8)tex;

    if(FAILED(pTex->GetLevelDesc(0, &TDesc))) return 0;
    else return TDesc.Height;
}


DWORD * CALL HGE_Impl::Texture_Lock(HTEXTURE tex, bool bReadOnly, int left, int top, int width, int height)
{
    LPDIRECT3DTEXTURE8 pTex=(LPDIRECT3DTEXTURE8)tex;
    D3DSURFACE_DESC TDesc;
    D3DLOCKED_RECT TRect;
    RECT region, *prec;
    int flags;

    pTex->GetLevelDesc(0, &TDesc);
    if(TDesc.Format!=D3DFMT_A8R8G8B8 && TDesc.Format!=D3DFMT_X8R8G8B8) return 0;

    if(width && height)
    {
        region.left=left;
        region.top=top;
        region.right=left+width;
        region.bottom=top+height;
        prec=&region;
    }
    else prec=0;

    if(bReadOnly) flags=D3DLOCK_READONLY;
    else flags=0;

    if(FAILED(pTex->LockRect(0, &TRect, prec, flags)))
    {
        _PostError("Can't lock texture");
        return 0;
    }

    return (DWORD *)TRect.pBits;
}


void CALL HGE_Impl::Texture_Unlock(HTEXTURE tex)
{
    LPDIRECT3DTEXTURE8 pTex=(LPDIRECT3DTEXTURE8)tex;
    pTex->UnlockRect(0);
}

//////// Implementation ////////

void HGE_Impl::_render_batch(bool bEndScene)
{
    if(VertArray)
    {
        pVB->Unlock();
        
        if(nPrim)
        {
            switch(CurPrimType)
            {
                case HGEPRIM_QUADS:
#ifdef UNDER_CE
                    pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, nPrim<<2, 0, nPrim<<1);
#else
                    pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, nPrim<<2, 0, nPrim<<1);
#endif
                    break;

                case HGEPRIM_TRIPLES:
                    pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, nPrim);
                    break;

                case HGEPRIM_LINES:
                    pD3DDevice->DrawPrimitive(D3DPT_LINELIST, 0, nPrim);
                    break;
            }

            nPrim=0;
        }

        if(bEndScene) VertArray = 0;
#ifdef UNDER_CE
        else pVB->Lock( 0, 0, (void**)&VertArray, 0 );
#else
        else pVB->Lock( 0, 0, (BYTE**)&VertArray, 0 );
#endif
    }
}

void HGE_Impl::_SetBlendMode(int blend)
{
    if((blend & BLEND_ALPHABLEND) != (CurBlendMode & BLEND_ALPHABLEND))
    {
        if(blend & BLEND_ALPHABLEND) pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        else pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    }

    if((blend & BLEND_ZWRITE) != (CurBlendMode & BLEND_ZWRITE))
    {
        if(blend & BLEND_ZWRITE) pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        else pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    }           
    
    if((blend & BLEND_COLORADD) != (CurBlendMode & BLEND_COLORADD))
    {
        if(blend & BLEND_COLORADD) pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
        else pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    }

    CurBlendMode = blend;
}

void HGE_Impl::_SetProjectionMatrix(int width, int height)
{
    D3DXMATRIX tmp;
    D3DXMatrixScaling(&matProj, 1.0f, -1.0f, 1.0f);
    D3DXMatrixTranslation(&tmp, -0.5f, height+0.5f, 0.0f);
    D3DXMatrixMultiply(&matProj, &matProj, &tmp);
    D3DXMatrixOrthoOffCenterLH(&tmp, 0, (float)width, 0, (float)height, 0.0f, 1.0f);
    D3DXMatrixMultiply(&matProj, &matProj, &tmp);
}

bool HGE_Impl::_GfxInit()
{
    static const char *szFormats[]={"UNKNOWN", "R5G6B5", "X1R5G5B5", "A1R5G5B5", "X8R8G8B8", "A8R8G8B8"};
    D3DADAPTER_IDENTIFIER8 AdID;
    D3DDISPLAYMODE Mode;
    D3DFORMAT Format=D3DFMT_UNKNOWN;
    UINT nModes, i;
    
// Init D3D
                            
#ifdef UNDER_CE
    pD3D=Direct3DCreate8(D3DM_SDK_VERSION); //120); // D3D_SDK_VERSION
#else
    pD3D=Direct3DCreate8(120); // D3D_SDK_VERSION
#endif
    if(pD3D==NULL)
    {
        _PostError("Can't create D3D interface");
        return false;
    }

// Get adapter info

    pD3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, D3DENUM_NO_WHQL_LEVEL, &AdID);
    System_Log("D3D Driver: %s",AdID.Driver);
    System_Log("Description: %s",AdID.Description);
    System_Log("Version: %d.%d.%d.%d",
            HIWORD(AdID.DriverVersion.HighPart),
            LOWORD(AdID.DriverVersion.HighPart),
            HIWORD(AdID.DriverVersion.LowPart),
            LOWORD(AdID.DriverVersion.LowPart));

// Set up Windowed presentation parameters
    
    if(FAILED(pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &Mode)) || Mode.Format==D3DFMT_UNKNOWN) 
    {
        _PostError("Can't determine desktop video mode");
        if(bWindowed) return false;
    }
    
    ZeroMemory(&d3dppW, sizeof(d3dppW));

    d3dppW.BackBufferWidth  = nScreenWidth;
    d3dppW.BackBufferHeight = nScreenHeight;
    d3dppW.BackBufferFormat = Mode.Format;
    d3dppW.BackBufferCount  = 1;
//  d3dppW.Flags            |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER; // required to be able to lock the back buffer
    d3dppW.MultiSampleType  = D3DMULTISAMPLE_NONE;
    d3dppW.hDeviceWindow    = hwnd;
    d3dppW.Windowed         = TRUE;

    if(nHGEFPS==HGEFPS_VSYNC) d3dppW.SwapEffect = D3DSWAPEFFECT_COPY_VSYNC;
    else                      d3dppW.SwapEffect = D3DSWAPEFFECT_COPY;

    if(bZBuffer)
    {
        d3dppW.EnableAutoDepthStencil = TRUE;
        d3dppW.AutoDepthStencilFormat = D3DFMT_D16;
    }

// Set up Full Screen presentation parameters

    nModes=pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT);

    for(i=0; i<nModes; i++)
    {
        pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, i, &Mode);
        if(Mode.Width != (UINT)nScreenWidth || Mode.Height != (UINT)nScreenHeight) continue;
        if(nScreenBPP==16 && (_format_id(Mode.Format) > _format_id(D3DFMT_A1R5G5B5))) continue;
        if(_format_id(Mode.Format) > _format_id(Format)) Format=Mode.Format;
    }

    if(Format == D3DFMT_UNKNOWN)
    {
        _PostError("Can't find appropriate full screen video mode");
        if(!bWindowed) return false;
    }

    ZeroMemory(&d3dppFS, sizeof(d3dppFS));

    d3dppFS.BackBufferWidth  = nScreenWidth;
    d3dppFS.BackBufferHeight = nScreenHeight;
    d3dppFS.BackBufferFormat = Format;
    d3dppFS.BackBufferCount  = 1;
//  d3dppFS.Flags            |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER; // required to be able to lock the back buffer
    d3dppFS.MultiSampleType  = D3DMULTISAMPLE_NONE;
    d3dppFS.hDeviceWindow    = hwnd;
    d3dppFS.Windowed         = FALSE;

    d3dppFS.SwapEffect       = D3DSWAPEFFECT_FLIP;
    d3dppFS.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

    if(nHGEFPS==HGEFPS_VSYNC) d3dppFS.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    else                      d3dppFS.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    if(bZBuffer)
    {
        d3dppFS.EnableAutoDepthStencil = TRUE;
        d3dppFS.AutoDepthStencilFormat = D3DFMT_D16;
    }

    d3dpp = bWindowed ? &d3dppW : &d3dppFS;

    if(_format_id(d3dpp->BackBufferFormat) < 4) nScreenBPP=16;
    else nScreenBPP=32;
    
// Create D3D Device

    if( FAILED( pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
                                  D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                  d3dpp, &pD3DDevice ) ) )
    {
        _PostError("Can't create D3D device");
        return false;
    }

    _AdjustWindow();

    System_Log("Mode: %d x %d x %s\n",nScreenWidth,nScreenHeight,szFormats[_format_id(Format)]);

// Create vertex batch buffer

    VertArray=0;

// Init all stuff that can be lost

    _SetProjectionMatrix(nScreenWidth, nScreenHeight);
    D3DXMatrixIdentity(&matView);
    
    if(!_init_lost()) return false;

    Gfx_Clear(0);

    return true;
}

int HGE_Impl::_format_id(D3DFORMAT fmt)
{
    switch(fmt) {
        case D3DFMT_R5G6B5:     return 1;
        case D3DFMT_X1R5G5B5:   return 2;
        case D3DFMT_A1R5G5B5:   return 3;
        case D3DFMT_X8R8G8B8:   return 4;
        case D3DFMT_A8R8G8B8:   return 5;
        default:                return 0;
    }
}

void HGE_Impl::_AdjustWindow()
{
    RECT *rc;
    LONG style;

    if(bWindowed) {rc=&rectW; style=styleW; }
    else  {rc=&rectFS; style=styleFS; }
    SetWindowLong(hwnd, GWL_STYLE, style);

    style=GetWindowLong(hwnd, GWL_EXSTYLE);
    if(bWindowed)
    {
        SetWindowLong(hwnd, GWL_EXSTYLE, style & (~WS_EX_TOPMOST));
        SetWindowPos(hwnd, HWND_NOTOPMOST, rc->left, rc->top, rc->right-rc->left, rc->bottom-rc->top, SWP_FRAMECHANGED);
    }
    else
    {
        SetWindowLong(hwnd, GWL_EXSTYLE, style | WS_EX_TOPMOST);
        SetWindowPos(hwnd, HWND_TOPMOST, rc->left, rc->top, rc->right-rc->left, rc->bottom-rc->top, SWP_FRAMECHANGED);
    }
}

void HGE_Impl::_Resize(int width, int height)
{
    if(hwndParent)
    {
        //if(procFocusLostFunc) procFocusLostFunc();

        d3dppW.BackBufferWidth=width;
        d3dppW.BackBufferHeight=height;
        nScreenWidth=width;
        nScreenHeight=height;

        _SetProjectionMatrix(nScreenWidth, nScreenHeight);
        _GfxRestore();

        //if(procFocusGainFunc) procFocusGainFunc();
    }
}

void HGE_Impl::_GfxDone()
{
    CRenderTargetList *target=pTargets, *next_target;
    
    if(pScreenSurf) { pScreenSurf->Release(); pScreenSurf=0; }
    if(pScreenDepth) { pScreenDepth->Release(); pScreenDepth=0; }

    while(target)
    {
        if(target->pTex) target->pTex->Release();
        if(target->pDepth) target->pDepth->Release();
        next_target=target->next;
        delete target;
        target=next_target;
    }
    pTargets=0;

    if(pIB)
    {
#ifdef UNDER_CE
        pD3DDevice->SetIndices(NULL);
#else
        pD3DDevice->SetIndices(NULL,0);
#endif
        pIB->Release();
        pIB=0;
    }
    if(pVB)
    {
        if(VertArray) { pVB->Unlock(); VertArray=0; }
        pD3DDevice->SetStreamSource( 0, NULL, sizeof(hgeVertex) );
        pVB->Release();
        pVB=0;
    }
    if(pD3DDevice) { pD3DDevice->Release(); pD3DDevice=0; }
    if(pD3D) { pD3D->Release(); pD3D=0; }
}


bool HGE_Impl::_GfxRestore()
{
    CRenderTargetList *target=pTargets;

    //if(!pD3DDevice) return false;
    //if(pD3DDevice->TestCooperativeLevel() == D3DERR_DEVICELOST) return;

    if(pScreenSurf) pScreenSurf->Release();
    if(pScreenDepth) pScreenDepth->Release();

    while(target)
    {
        if(target->pTex) target->pTex->Release();
        if(target->pDepth) target->pDepth->Release();
        target=target->next;
    }

    if(pIB)
    {
#ifdef UNDER_CE
    pD3DDevice->SetIndices(NULL);
#else
    pD3DDevice->SetIndices(NULL,0);
#endif
        pIB->Release();
    }
    if(pVB)
    {
        pD3DDevice->SetStreamSource( 0, NULL, sizeof(hgeVertex) );
        pVB->Release();
    }

    pD3DDevice->Reset(d3dpp);

    if(!_init_lost()) return false;

    if(procGfxRestoreFunc) return procGfxRestoreFunc();

    return true;
}


bool HGE_Impl::_init_lost()
{
    CRenderTargetList *target=pTargets;

// Store render target

    pScreenSurf=0;
    pScreenDepth=0;

    pD3DDevice->GetRenderTarget(&pScreenSurf);
    pD3DDevice->GetDepthStencilSurface(&pScreenDepth);
    
    while(target)
    {
        if(target->pTex)
            D3DXCreateTexture(pD3DDevice, target->width, target->height, 1, D3DUSAGE_RENDERTARGET,
                              d3dpp->BackBufferFormat, D3DPOOL_DEFAULT, &target->pTex);
        if(target->pDepth)
            pD3DDevice->CreateDepthStencilSurface(target->width, target->height,
                                                  D3DFMT_D16, D3DMULTISAMPLE_NONE, &target->pDepth);
        target=target->next;
    }

// Create Vertex buffer
    
    if( FAILED (pD3DDevice->CreateVertexBuffer(VERTEX_BUFFER_SIZE*sizeof(hgeVertex),
                                              D3DUSAGE_WRITEONLY,
                                              D3DFVF_HGEVERTEX,
                                              D3DPOOL_DEFAULT, &pVB )))
    {
        _PostError("Can't create D3D vertex buffer");
        return false;
    }

#ifndef UNDER_CE
    pD3DDevice->SetVertexShader( D3DFVF_HGEVERTEX );
#endif
    pD3DDevice->SetStreamSource( 0, pVB, sizeof(hgeVertex) );

// Create and setup Index buffer

    if( FAILED( pD3DDevice->CreateIndexBuffer(VERTEX_BUFFER_SIZE*6/4*sizeof(WORD),
                                              D3DUSAGE_WRITEONLY,
                                              D3DFMT_INDEX16,
                                              D3DPOOL_DEFAULT, &pIB ) ) )
    {
        _PostError("Can't create D3D index buffer");
        return false;
    }

    WORD *pIndices, n=0;
#ifdef UNDER_CE
    if( FAILED( pIB->Lock( 0, 0, (void**)&pIndices, 0 ) ) )
#else
    if( FAILED( pIB->Lock( 0, 0, (BYTE**)&pIndices, 0 ) ) )
#endif
    {
        _PostError("Can't lock D3D index buffer");
        return false;
    }

    for(int i=0; i<VERTEX_BUFFER_SIZE/4; i++) {
        *pIndices++=n;
        *pIndices++=n+1;
        *pIndices++=n+2;
        *pIndices++=n+2;
        *pIndices++=n+3;
        *pIndices++=n;
        n+=4;
    }

    pIB->Unlock();
#ifdef UNDER_CE
  pD3DDevice->SetIndices(pIB);
#else
  pD3DDevice->SetIndices(pIB,0);
#endif

// Set common render states

    //pD3DDevice->SetRenderState( D3DRS_LASTPIXEL, FALSE );
    pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
    pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    
    pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
    pD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
    pD3DDevice->SetRenderState( D3DRS_ALPHAREF,        0x01 );
    pD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

    pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
    pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

    pD3DDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_POINT);

    if(bTextureFilter)
    {
        pD3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_LINEAR);
        pD3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_LINEAR);
    }
    else
    {
        pD3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_POINT);
        pD3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_POINT);
    }

    nPrim=0;
    CurPrimType=HGEPRIM_QUADS;
    CurBlendMode = BLEND_DEFAULT;
    CurTexture = NULL;

#ifdef UNDER_CE
  pD3DDevice->SetTransform(D3DTS_VIEW, (D3DMMATRIX*)&matView, D3DMFMT_D3DMVALUE_FLOAT);
  pD3DDevice->SetTransform(D3DTS_PROJECTION, (D3DMMATRIX*)&matProj, D3DMFMT_D3DMVALUE_FLOAT);
#else
  pD3DDevice->SetTransform(D3DTS_VIEW, &matView);
  pD3DDevice->SetTransform(D3DTS_PROJECTION, &matProj);
#endif

    return true;
}
