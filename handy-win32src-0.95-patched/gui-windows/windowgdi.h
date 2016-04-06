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

// windowgdi.h: interface for the CWindowGDI class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINDOWGDI_H__B6374667_2339_4329_90E7_286AB772F820__INCLUDED_)
#define AFX_WINDOWGDI_H__B6374667_2339_4329_90E7_286AB772F820__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxwin.h>
#include <afxcmn.h>
#include <afxdlgs.h>
#include <mmsystem.h>
#include "../core/machine.h"
#include "../core/pixblend.h"
#include "lynxrender.h"

class CWindowGDI : public CLynxRender  
{
	private:
		int		mInitOK;
//		int		mRenderFormat;
		UBYTE*	mBackBuffer;
		int		mBackBufferPitch;

		CDC			*mpWinCDC;
		CDC			*mpMemCDC;

		CBitmap		mBitmap;
		CBitmap		*mOldBitmap;
		CPalette	mPalette;
		CPalette	*mOldPalette;

	public:
		CWindowGDI();
		virtual ~CWindowGDI();

        bool	Create (CWnd *pcwnd,int src_width, int src_height, int scrn_width, int scrn_height, int zoom); 
        bool	Destroy (CWnd *pcwnd); 
		bool	Render(int dest_x,int dest_y);
		int		PixelFormat(void);

		UBYTE*	BackBuffer(void)	{ return mBackBuffer; };
		int		BufferPitch(void) { return mBackBufferPitch; };

};

#endif // !defined(AFX_WINDOWGDI_H__B6374667_2339_4329_90E7_286AB772F820__INCLUDED_)
