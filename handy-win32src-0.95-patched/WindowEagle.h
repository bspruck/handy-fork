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

#if !defined(_WINDOWEAGLE_H_)
#define _WINDOWEAGLE_H_

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

class CWindowDirectXEagle : public CLynxRender
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
		CWindowDirectXEagle()
		{
			mBackBuffer=NULL;
			mInitOK=false;
		}

		virtual ~CWindowDirectXEagle()
		{
			mInitOK=false;
		}

        bool Create(CWnd *pcwnd,int src_width, int src_height, int scrn_width, int scrn_height, int zoom)
		{
			// -- setup direct draw --
			_hresult hr;

			mSrcWidth=src_width;
			mSrcHeight=src_height;

			// Override ZOOM is always x2 or 4 for us
			mZoom=zoom;
			if(mZoom!=2 && mZoom!=4) mZoom=2;
			mWidth=mSrcWidth*mZoom;
			mHeight=mSrcHeight*mZoom;

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
				intDesc.dwWidth  = mSrcWidth*2;
				intDesc.dwHeight = mSrcHeight*2;
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

			switch(PixelFormat())
			{
				case RENDER_16BPP_555:
				case RENDER_16BPP_565:
					{
						ULONG plop;
						ULONG *dest;
						CPixelBlender16BPP pix(mSrcWidth,mSrcHeight,2,(ULONG*)mBackBuffer);
	
						dest=(ULONG*)IntermediateBuffer;
						for(yloop=0;yloop<mSrcHeight*2;yloop++)
						{
							pix.SetLinebase(yloop);
						for(xloop=0;xloop<mSrcWidth;xloop++)
							{
								plop=pix.GetEagle(2*xloop);
								plop|=pix.GetEagle(2*xloop+1)<<16;
								*dest++=plop;
							}
						}
					}
					break;
				case RENDER_24BPP:
					{
						ULONG plop;
						UBYTE *dest;
						CPixelBlender24BPP pix(mSrcWidth,mSrcHeight,2,(ULONG*)mBackBuffer);
						dest=IntermediateBuffer;
						for(yloop=0;yloop<mSrcHeight*2;yloop++)
						{
							pix.SetLinebase(yloop);
							for(xloop=0;xloop<mSrcWidth*2;xloop++)
							{
								plop=pix.GetEagle(xloop);
								*dest++=(UBYTE)(plop&0xff);
								*dest++=(UBYTE)((plop>>8)&0xff);
								*dest++=(UBYTE)((plop>>16)&0xff);
							}
						}
					}
					break;
				case RENDER_32BPP:
					{
						ULONG plop;
						ULONG *dest;
						CPixelBlender32BPP pix(mSrcWidth,mSrcHeight,2,(ULONG*)mBackBuffer);
						dest=(ULONG*)IntermediateBuffer;
						for(yloop=0;yloop<mSrcHeight*2;yloop++)
						{
							pix.SetLinebase(yloop);
							for(xloop=0;xloop<mSrcWidth*2;xloop++)
							{
								plop=pix.GetEagle(xloop);
								*dest++=plop;
							}
						}
					}
					break;
				default:
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
