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

// WindowLynx.h: interface for the WindowDirectXLynx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_WINDOWLYNX_H_)
#define _WINDOWLYNX_H_

#include <afxwin.h>
#include <afxcmn.h>
#include <afxdlgs.h>
#include <mmsystem.h>
#include "../core/machine.h"
#include "directx.h"
#include "../core/pixblend.h"
#include "lynxrender.h"

/////////////////////////////////////////////////////////////////////////////
// Full Screen Management object

class CWindowDirectXLynxLCD : public CLynxRender
{
    private:
        IDirectDraw2Ptr       mIDD;     
        IDirectDrawSurfacePtr mPrimarySurface;
        IDirectDrawSurfacePtr mIntermediateSurface;
        IDirectDrawSurfacePtr mBackSurface;
		IDirectDrawClipperPtr mWindowClipper;

		int		mInitOK;
		UBYTE*	mBackBuffer;
		int		mBackBufferPitch;

    public:
		CWindowDirectXLynxLCD()
		{
			mBackBuffer=NULL;
			mInitOK=false;
		}

		virtual ~CWindowDirectXLynxLCD()
		{
			mInitOK=false;
		}

        bool Create(CWnd *pcwnd,int src_width, int src_height, int scrn_width, int scrn_height, int zoom)
		{
			// -- setup direct draw --
			_hresult hr;

			mSrcWidth=src_width;
			mSrcHeight=src_height;

			// Override ZOOM is always x3 for us
			mWidth=mSrcWidth*3;
			mHeight=mSrcHeight*3;
			mZoom=3;

			try 
			{
				// -- construct a direct draw object and set to windowed mode --
				mIDD = IDirectDraw2Ptr (CLSID_DirectDraw);
				hr = mIDD->Initialize (NULL);
				hr = mIDD->SetCooperativeLevel (pcwnd->m_hWnd, DDSCL_NORMAL);

				// -- create primary surface -- 
				DXStruct<DDSURFACEDESC> primaryDesc;
				primaryDesc.dwFlags = DDSD_CAPS;
				primaryDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
				hr = mIDD->CreateSurface (&primaryDesc, &mPrimarySurface, NULL);

				// Create clipper and attach
				hr = mIDD->CreateClipper(0,&mWindowClipper,NULL);
				hr = mWindowClipper->SetHWnd(0,pcwnd->m_hWnd);
				hr = mPrimarySurface->SetClipper(mWindowClipper);

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
				intDesc.dwWidth  = mWidth;
				intDesc.dwHeight = mHeight;
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
				AfxMessageBox (message);

				// Return failure
				return false;
			}

			// Allow operation
			mInitOK=true;

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
				if(mWindowClipper!=NULL) mWindowClipper.Release();
				if(mBackSurface!=NULL)	mBackSurface.Release();
				if(mIntermediateSurface!=NULL)	mIntermediateSurface.Release();
				if(mPrimarySurface!=NULL) mPrimarySurface.Release();
			}
			mInitOK=false;
			return true;
		}


		bool Render(int dest_x,int dest_y)
		{
			int	xloop,yloop;
			bool retval=TRUE;
			_hresult hr;

			if(!mInitOK) return FALSE;
	
			// Copy/modify from Back buffer to intermediate surface
			DXStruct<DDSURFACEDESC> intDesc;
			hr=mIntermediateSurface->Lock(NULL,&intDesc,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL);
			UBYTE *IntermediateBuffer=(UBYTE*)intDesc.lpSurface;
			int IntermediateBufferPitch=intDesc.lPitch;

			ULONG mask[3];
			switch(PixelFormat())
			{
				case RENDER_16BPP_555:
					mask[0]=0x00007c00;
					mask[1]=0x000003e0;
					mask[2]=0x0000001f;
					break;
				case RENDER_16BPP_565:
					mask[0]=0x0000f800;
					mask[1]=0x000007e0;
					mask[2]=0x0000001f;
					break;
				case RENDER_32BPP:
					mask[0]=0x000000ff;
					mask[1]=0x0000ff00;
					mask[2]=0x00ff0000;
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
								*(linebase1++)=*(linebase2++)=*(linebase3++)=srcdata&((UWORD)mask[0]);
								*(linebase1++)=*(linebase2++)=*(linebase3++)=srcdata&((UWORD)mask[1]);
								*(linebase1++)=*(linebase2++)=*(linebase3++)=srcdata&((UWORD)mask[2]);
							}
							srcline+=mBackBufferPitch/sizeof(UWORD);
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
					if(mSrcWidth>mSrcHeight)
					{
						UBYTE *linebase1,*linebase2,*linebase3,*srcline,srcdata;
						srcline=mBackBuffer;
						for(yloop=0;yloop<mSrcHeight;yloop++)
						{
							linebase1=IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							linebase2=IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							linebase3=IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							for(xloop=0;xloop<mSrcWidth*3;)
							{
								srcdata=*(srcline+(xloop++));
								*(linebase1++)=*(linebase2++)=*(linebase3++)=srcdata;
								*(linebase1++)=*(linebase2++)=*(linebase3++)=0;
								*(linebase1++)=*(linebase2++)=*(linebase3++)=0;
								srcdata=*(srcline+(xloop++));
								*(linebase1++)=*(linebase2++)=*(linebase3++)=0;
								*(linebase1++)=*(linebase2++)=*(linebase3++)=srcdata;
								*(linebase1++)=*(linebase2++)=*(linebase3++)=0;
								srcdata=*(srcline+(xloop++));
								*(linebase1++)=*(linebase2++)=*(linebase3++)=0;
								*(linebase1++)=*(linebase2++)=*(linebase3++)=0;
								*(linebase1++)=*(linebase2++)=*(linebase3++)=srcdata;
							}
							srcline+=mBackBufferPitch;
						}
					}
					else
					{
						UBYTE *linebase1,*linebase2,*linebase3,*srcline,srcdata;
						srcline=mBackBuffer;
						for(yloop=0;yloop<mSrcHeight;yloop++)
						{
							linebase1=IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							linebase2=IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							linebase3=IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							for(xloop=0;xloop<mSrcWidth*3;)
							{
								srcdata=*(srcline+(xloop++));
								*(linebase1++)=srcdata; *(linebase1++)=0; *(linebase1++)=0;
								*(linebase1++)=srcdata; *(linebase1++)=0; *(linebase1++)=0;
								*(linebase1++)=srcdata; *(linebase1++)=0; *(linebase1++)=0;
								srcdata=*(srcline+(xloop++));
								*(linebase2++)=0; *(linebase2++)=srcdata; *(linebase2++)=0;
								*(linebase2++)=0; *(linebase2++)=srcdata; *(linebase2++)=0;
								*(linebase2++)=0; *(linebase2++)=srcdata; *(linebase2++)=0;
								srcdata=*(srcline+(xloop++));
								*(linebase3++)=0; *(linebase3++)=0; *(linebase3++)=srcdata;
								*(linebase3++)=0; *(linebase3++)=0; *(linebase3++)=srcdata;
								*(linebase3++)=0; *(linebase3++)=0; *(linebase3++)=srcdata;
							}
							srcline+=mBackBufferPitch;
						}
					}
					break;
				case RENDER_32BPP:
					if(mSrcWidth>mSrcHeight)
					{
						ULONG *linebase1,*linebase2,*linebase3,*srcline,srcdata;
						srcline=(ULONG*)mBackBuffer;
						for(yloop=0;yloop<mSrcHeight;yloop++)
						{
							linebase1=(ULONG*)IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							linebase2=(ULONG*)IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							linebase3=(ULONG*)IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							for(xloop=0;xloop<mSrcWidth;xloop++)
							{
								srcdata=*(srcline+xloop);
								*(linebase1++)=*(linebase2++)=*(linebase3++)=srcdata&mask[0];
								*(linebase1++)=*(linebase2++)=*(linebase3++)=srcdata&mask[1];
								*(linebase1++)=*(linebase2++)=*(linebase3++)=srcdata&mask[2];
							}
							srcline+=mBackBufferPitch/sizeof(ULONG);
						}
					}
					else
					{
						ULONG *linebase1,*linebase2,*linebase3,*srcline,srcdata;
						srcline=(ULONG*)mBackBuffer;
						for(yloop=0;yloop<mSrcHeight;yloop++)
						{
							linebase1=(ULONG*)IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							linebase2=(ULONG*)IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							linebase3=(ULONG*)IntermediateBuffer;
							IntermediateBuffer+=IntermediateBufferPitch;
							for(xloop=0;xloop<mSrcWidth;xloop++)
							{
								srcdata=*(srcline+xloop);
								*(linebase1++)=srcdata&mask[0]; *(linebase1++)=srcdata&mask[0]; *(linebase1++)=srcdata&mask[0];
								*(linebase2++)=srcdata&mask[1]; *(linebase2++)=srcdata&mask[1]; *(linebase2++)=srcdata&mask[1];
								*(linebase3++)=srcdata&mask[2]; *(linebase3++)=srcdata&mask[2]; *(linebase3++)=srcdata&mask[2];
							}
							srcline+=mBackBufferPitch/sizeof(ULONG);
						}
					}
					break;
				case RENDER_ERROR:
					retval=FALSE;
					break;

			}

			// Unlock intermediate for blit
			hr=mIntermediateSurface->Unlock(NULL);

			// Calculate where the destination rectangle should land and
			// how large it should be
			CRect rect(dest_x,dest_y,dest_x+mWidth,dest_y+mHeight);
			(AfxGetMainWnd())->ClientToScreen(&rect);

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

		
		int	PixelFormat(void)
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
			else if(pixel.dwRBitMask==0x00ff0000 && pixel.dwGBitMask==0x0000ff00 && pixel.dwBBitMask==0x000000ff)
			{
				if(pixel.dwRGBBitCount==24)
				{
					format=RENDER_24BPP;
				}
				else if(pixel.dwRGBBitCount==32)
				{
					format=RENDER_32BPP;
				}
				else
				{
					format=RENDER_ERROR;
				}
			}
			else
			{
				format=RENDER_ERROR;
			}	
			return format;
		}


        IDirectDrawSurfacePtr GetFrontSurface(void ) { return mPrimarySurface; }
		UBYTE*	BackBuffer(void) { return mBackBuffer; }
		int		BufferPitch(void) { return mBackBufferPitch; }

    protected:
};

/////////////////////////////////////////////////////////////////////////////

#endif
