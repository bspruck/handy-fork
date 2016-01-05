//
// Copyright (c) 2004 K. Wilkins
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

// FullScreenLynx.h: interface for the FullScreenDirectX class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_FULLSCREENLYNX_H_)
#define _FULLSCREENLYNX_H_

#include <afxwin.h>
#include <afxcmn.h>
#include <afxdlgs.h>
#include <mmsystem.h>
#include "machine.h"
#include "directx.h"
#include "pixblend.h"
#include "lynxrender.h"

/////////////////////////////////////////////////////////////////////////////
// Full Screen Management object

class CFullScreenDirectXLynxLCD : public CLynxRender
{
    private:
        IDirectDraw2Ptr       mIDD;
        IDirectDrawSurfacePtr mPrimarySurface;
        IDirectDrawSurfacePtr mBackSurface;
        IDirectDrawSurfacePtr mIntermediateSurface;

		long	m_cwnd_style;
		long	m_cwnd_exstyle;
		HMENU	m_cwnd_hmenu;
		CRect	m_cwnd_rect;

		int		mInitOK;
		UBYTE*	mBackBuffer;
		int		mBackBufferPitch;

    public:
		CFullScreenDirectXLynxLCD()
		{
			mBackBuffer=NULL;
			mInitOK=false;
		}


		virtual ~CFullScreenDirectXLynxLCD()
		{
			mInitOK=false;
		}


        bool Create(CWnd *pcwnd,int src_width, int src_height, int scrn_width, int scrn_height, int zoom)
		{
			mWidth=scrn_width;
			mHeight=scrn_height;
			mSrcWidth=src_width;
			mSrcHeight=src_height;
			mZoom=3;

			// Save window style, position, size, menu

			// Safety check before proceeding on creation
			if(mWidth<(mSrcWidth*mZoom)) return FALSE;
			if(mHeight<(mSrcHeight*mZoom)) return FALSE;

			m_cwnd_style=::GetWindowLong(pcwnd->m_hWnd,GWL_STYLE);
			m_cwnd_exstyle=::GetWindowLong(pcwnd->m_hWnd,GWL_EXSTYLE);
			m_cwnd_hmenu=::GetMenu(pcwnd->m_hWnd);
			pcwnd->GetWindowRect(&m_cwnd_rect);

			// Set window style, position, size, menu

			::SetWindowLong(pcwnd->m_hWnd,GWL_STYLE, WS_POPUP);
			::SetWindowLong(pcwnd->m_hWnd,GWL_EXSTYLE,0);
			::SetMenu(pcwnd->m_hWnd,NULL);
			::SetWindowPos(pcwnd->m_hWnd,HWND_TOP,0,0,mWidth,mHeight,SWP_SHOWWINDOW);
			::ShowWindow(pcwnd->m_hWnd,SW_SHOW);

			// -- setup direct draw --
			_hresult hr;

			try 
			{

				// -- construct a direct draw object and set to windowed mode --
				mIDD = IDirectDraw2Ptr (CLSID_DirectDraw);
				hr = mIDD->Initialize (NULL);
				hr = mIDD->SetCooperativeLevel (pcwnd->m_hWnd, DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN );

				// Change video mode
				hr = mIDD->SetDisplayMode(mWidth, mHeight, 16, 0, 0);
		
				// -- create primary surface -- 
				DXStruct<DDSURFACEDESC> primaryDesc;
				primaryDesc.dwFlags = DDSD_CAPS;
				primaryDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
				hr = mIDD->CreateSurface (&primaryDesc, &mPrimarySurface, NULL);

				// -- create back buffer -- 
				DXStruct<DDSURFACEDESC> backDesc;
				backDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
				backDesc.dwWidth  = mSrcWidth;
				backDesc.dwHeight = mSrcHeight;
				backDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
				hr = mIDD->CreateSurface(&backDesc, &mBackSurface, NULL);

				// -- create intermediate buffer -- 
				DXStruct<DDSURFACEDESC> intDesc;
				intDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
				intDesc.dwWidth  = 3*mSrcWidth;
				intDesc.dwHeight = 3*mSrcHeight;
				intDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
				hr = mIDD->CreateSurface(&intDesc, &mIntermediateSurface, NULL);

				// Lock the buffer to get a pointer back for rendering
				hr=mBackSurface->Lock(NULL,&backDesc,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL);
				mBackBuffer=(UBYTE*)backDesc.lpSurface;
				mBackBufferPitch=backDesc.lPitch;
			}

			catch (_com_error& ex)
			{
				mIDD->SetCooperativeLevel(pcwnd->m_hWnd, DDSCL_NORMAL);

				CString message ("Error during Direct Draw create: \n");
				message += DXAppErrorToString (ex.Error());
//				AfxMessageBox (message);

				// Reset window style, position, size, menu

				::SetWindowLong(pcwnd->m_hWnd,GWL_STYLE, m_cwnd_style);
				::SetWindowLong(pcwnd->m_hWnd,GWL_EXSTYLE,m_cwnd_exstyle);
				::SetMenu(pcwnd->m_hWnd,m_cwnd_hmenu);
				::SetWindowPos(pcwnd->m_hWnd,HWND_TOP,m_cwnd_rect.left,m_cwnd_rect.top,m_cwnd_rect.Width(),m_cwnd_rect.Height(),0);

				// Return failure
				return false;
			}

			DDBLTFX ddbltfx;
			ddbltfx.dwSize = sizeof(ddbltfx);
			ddbltfx.dwFillColor = 0;
			mPrimarySurface->Blt(NULL,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&ddbltfx);

			// Allow operation
			mInitOK=true;

			ShowCursor(false);
			return true;
		}

        
		bool Destroy(CWnd *pcwnd)
		{
			if (mIDD != NULL)
			{
				mIDD->SetCooperativeLevel(pcwnd->m_hWnd, DDSCL_NORMAL);

				// Back buffer is always kept locked for display usage
				mBackSurface->Unlock(NULL);
				mBackBuffer=NULL;

				// Now we can release surfaces
				if(mBackSurface!=NULL) mBackSurface.Release();
				if(mIntermediateSurface!=NULL) mIntermediateSurface.Release();
				if(mPrimarySurface!=NULL) mPrimarySurface.Release();
			}
			ShowCursor(true);

			// Reset window style, position, size, menu

			::SetWindowLong(pcwnd->m_hWnd,GWL_STYLE, m_cwnd_style);
			::SetWindowLong(pcwnd->m_hWnd,GWL_EXSTYLE,m_cwnd_exstyle);
			::SetMenu(pcwnd->m_hWnd,m_cwnd_hmenu);
			::SetWindowPos(pcwnd->m_hWnd,HWND_TOP,m_cwnd_rect.left,m_cwnd_rect.top,m_cwnd_rect.Width(),m_cwnd_rect.Height(),0);

			mInitOK=false;

			return true;
		}

		
		bool Render(int dest_x,int dest_y)
		{
			int	xloop,yloop;
			bool retval=true;
			_hresult hr;

			if(!mInitOK) return FALSE;

			// Protect against overlarge screens
			if(dest_x<0 || dest_y<0)
			{
				retval=FALSE;
				return retval;
			}

			// Copy/modify from Back buffer to intermediate surface
			DXStruct<DDSURFACEDESC> intDesc;
			hr=mIntermediateSurface->Lock(NULL,&intDesc,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL);
			UBYTE *IntermediateBuffer=(UBYTE*)intDesc.lpSurface;
			int IntermediateBufferPitch=intDesc.lPitch;

			UWORD mask[3];
			switch(PixelFormat())
			{
				case RENDER_16BPP_555:
					mask[0]=0x7c00;
					mask[1]=0x03e0;
					mask[2]=0x001f;
					break;
				case RENDER_16BPP_565:
					mask[0]=0xf800;
					mask[1]=0x07e0;
					mask[2]=0x001f;
					break;
				default:
					break;
			}

			
			switch(PixelFormat())
			{
				case RENDER_16BPP_555:
				case RENDER_16BPP_565:
					if(mSrcWidth>mSrcHeight)
					{
						UWORD *linebase1,*linebase2,*linebase3,*srcline,srcdata;
						srcline=(UWORD*)mBackBuffer;
						for(yloop=0;yloop<mSrcHeight;yloop++)
						{
							linebase1=(UWORD*)IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							linebase2=(UWORD*)IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							linebase3=(UWORD*)IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							for(xloop=0;xloop<mSrcWidth;xloop++)
							{
								srcdata=*(srcline+xloop);
								*(linebase1++)=*(linebase2++)=*(linebase3++)=srcdata&mask[0];
								*(linebase1++)=*(linebase2++)=*(linebase3++)=srcdata&mask[1];
								*(linebase1++)=*(linebase2++)=*(linebase3++)=srcdata&mask[2];
							}
							srcline+=mBackBufferPitch/2;
						}
					}
					else
					{
						UWORD *linebase1,*linebase2,*linebase3,*srcline,srcdata;
						srcline=(UWORD*)mBackBuffer;
						for(yloop=0;yloop<mSrcHeight;yloop++)
						{
							linebase1=(UWORD*)IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							linebase2=(UWORD*)IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							linebase3=(UWORD*)IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							for(xloop=0;xloop<mSrcWidth;xloop++)
							{
								srcdata=*(srcline+xloop);
								*(linebase1++)=srcdata&((UWORD)mask[0]); *(linebase1++)=srcdata&((UWORD)mask[0]); *(linebase1++)=srcdata&((UWORD)mask[0]);
								*(linebase2++)=srcdata&((UWORD)mask[1]); *(linebase2++)=srcdata&((UWORD)mask[1]); *(linebase2++)=srcdata&((UWORD)mask[1]);
								*(linebase3++)=srcdata&((UWORD)mask[2]); *(linebase3++)=srcdata&((UWORD)mask[2]); *(linebase3++)=srcdata&((UWORD)mask[2]);
							}
							srcline+=mBackBufferPitch/sizeof(UWORD);
						}
					}
					break;
				case RENDER_24BPP:
				case RENDER_32BPP:
				case RENDER_ERROR:
					retval=FALSE;
					break;

			}

			// Unlock intermediate for blit
			hr=mIntermediateSurface->Unlock(NULL);

			// Calculate where the destination rectangle should land and
			// how large it should be
			CRect rect(dest_x,dest_y,dest_x+(mSrcWidth*mZoom),dest_y+(mSrcHeight*mZoom));

			while(1)
			{
				// Make the primary buffer do the blit & stretch to the screen
				int res=mPrimarySurface->Blt(&rect,mIntermediateSurface,NULL,0,NULL);

				if(res==DD_OK)
				{
					break;
				}
				else if(res==DDERR_SURFACELOST)
				{
					// Restore the lost surface
					mPrimarySurface->Restore();
				}
				else
				{
					retval=FALSE;
				}
			}

			return retval;
		}

		
		int PixelFormat(void)
		{
			int format;
			DXStruct<DDPIXELFORMAT> pixel;
			mPrimarySurface->GetPixelFormat(&pixel);
			if(pixel.dwRBitMask==0x7c00 && pixel.dwGBitMask==0x03e0 && pixel.dwBBitMask==0x001f)
			{
				format=RENDER_16BPP_555;
			}
			else if(pixel.dwRBitMask==0xf800 && pixel.dwGBitMask==0x07e0 && pixel.dwBBitMask==0x001f)
			{
				format=RENDER_16BPP_565;
			}
			else
			{
				format=RENDER_ERROR;
			}
			return format;
		}


		int		Windowed(void) { return FALSE; }
        IDirectDrawSurfacePtr GetFrontSurface(void ) { return mPrimarySurface; }
		UBYTE*	BackBuffer(void) { return mBackBuffer; }
		int		BufferPitch(void) { return mBackBufferPitch; }

    protected:
};

/////////////////////////////////////////////////////////////////////////////

#endif
