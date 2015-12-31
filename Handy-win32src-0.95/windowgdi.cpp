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

// windowgdi.cpp: implementation of the CWindowGDI class.
//
//////////////////////////////////////////////////////////////////////

#include "windowgdi.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWindowGDI::CWindowGDI()
{
//	mRenderFormat=RENDER_NORMAL;
	mBackBuffer=NULL;
	mInitOK=false;

	mpWinCDC=NULL;
	mpMemCDC=NULL;
	mOldBitmap=NULL;
	mOldPalette=NULL;
}

CWindowGDI::~CWindowGDI()
{
	mInitOK=false;
}

/////////////////////////////////////////////////////////////////////////////
// DoubleBufferedWindow message handlers

bool CWindowGDI::Create (CWnd *pcwnd,int src_width, int src_height, int scrn_width, int scrn_height,int zoom) 
{
	int bpp=0;

	// -- setup direct draw --
	mSrcWidth=src_width;
	mSrcHeight=src_height;
	mWidth=mSrcWidth*zoom;
	mHeight=mSrcHeight*zoom;
	mZoom=zoom;

	// Setup the GDI

	// Create a DC for us
	mpWinCDC= new CClientDC(pcwnd);		// Reference to our window
	mpMemCDC= new CDC;					// Memory device context we will paint from

	if(!mpMemCDC->CreateCompatibleDC(mpWinCDC))
	{
		return 0;
	}

	// Delete any existing bitmaps
	if(mBitmap.m_hObject!=NULL) mBitmap.DeleteObject();
	if(mPalette.m_hObject!=NULL) mPalette.DeleteObject();

	//
	// Create the DIBsection we will use for blitting
	//

	BITMAPINFO *bi;
	switch(PixelFormat())
	{
		case RENDER_16BPP_555:
		case RENDER_16BPP_565:
		case RENDER_24BPP:
		case RENDER_32BPP:
			bi= (BITMAPINFO*) new UBYTE[sizeof(BITMAPINFO)];
			ZeroMemory(bi, sizeof(BITMAPINFO));
			bi->bmiHeader.biBitCount = 16;
			bi->bmiHeader.biSizeImage = (mSrcWidth*mSrcHeight)*2;
			break;
		case RENDER_8BPP:
		default:
			bi= (BITMAPINFO*) new UBYTE[sizeof(BITMAPINFO)+(255*sizeof(RGBQUAD))];
			ZeroMemory(bi, sizeof(BITMAPINFO));
			bi->bmiHeader.biBitCount = 8;
			bi->bmiHeader.biSizeImage = (mSrcWidth*mSrcHeight)*1;
			for(int loop=0;loop<256;loop++)
			{
			    bi->bmiColors[loop].rgbRed=((loop&0x0e0)>>5)*(256/8);
				bi->bmiColors[loop].rgbGreen=((loop&0x01c)>>2)*(256/8);
		        bi->bmiColors[loop].rgbBlue=(loop&0x003)*(256/4);
		        bi->bmiColors[loop].rgbReserved=0;
			}
			break;
	}

	bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi->bmiHeader.biPlanes = 1;
	bi->bmiHeader.biWidth = mSrcWidth;
	bi->bmiHeader.biHeight = -mSrcHeight;
	bi->bmiHeader.biCompression = BI_RGB;

	HBITMAP hBitmap;
	if((hBitmap=CreateDIBSection(mpWinCDC->m_hDC,bi,DIB_RGB_COLORS,(void **)&mBackBuffer,NULL,0))==NULL)
	{
		return 0;
	}
	mBitmap.Attach(hBitmap);
	mOldBitmap=mpMemCDC->SelectObject(&mBitmap);

	// Set pitch correctly
	if(PixelFormat()==RENDER_8BPP)
	{
		mBackBufferPitch=(mSrcWidth+3)&0xfffc;
	}
	else
	{
		mBackBufferPitch=((mSrcWidth*2)+3)&0xfffc;
	}


	// Tidy up
	delete bi;

	// Allow operation
	mInitOK=true;

	return true;
}

bool CWindowGDI::Destroy (CWnd *pcwnd) 
{
	// Destroy bitmap and associated CDC's

	if(mpMemCDC!=NULL)
	{
        mpMemCDC->SelectObject(mOldBitmap);
        delete mpMemCDC;
        mpMemCDC=NULL;
        mOldBitmap=NULL;
	}

	if(mpWinCDC!=NULL)	
	{
		mpWinCDC->SelectObject(mOldPalette);
        delete mpWinCDC;
        mpWinCDC=NULL;
        mOldPalette=NULL;
	}

	if(mBitmap.m_hObject!=NULL) mBitmap.DeleteObject();
	if(mPalette.m_hObject!=NULL) mPalette.DeleteObject();

	mInitOK=false;
	return true;
}

int CWindowGDI::PixelFormat(void)
{
	int format;
	switch(mpWinCDC->GetDeviceCaps(BITSPIXEL))
	{
		case 32:
		case 24:
		case 16:
			format=RENDER_16BPP_555;
			break;
		case 8:
			format=RENDER_8BPP;
			break;
		default:
			format=RENDER_ERROR;
			break;
	}
	return format;
}

bool CWindowGDI::Render(int dest_x,int dest_y)
{
	if(!mInitOK) return FALSE;

	if(mZoom==1)
	{
		if(!mpWinCDC->BitBlt(dest_x,dest_y,mSrcWidth,mSrcHeight,mpMemCDC,0,0,SRCCOPY)) return 0;
	}
	else
	{
		if(!mpWinCDC->StretchBlt(dest_x,dest_y,mWidth,mHeight,mpMemCDC,0,0,mSrcWidth,mSrcHeight,SRCCOPY)) return 0;
	}

	return TRUE;
}
