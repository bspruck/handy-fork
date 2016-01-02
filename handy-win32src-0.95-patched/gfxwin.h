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

//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                                 K. Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Lynx graphics debug display window header file                           //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition and code for the      //
// graphics window used in the debug version of the emulator                //
//                                                                          //
//    K. Wilkins                                                            //
// August 1997                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 02Nov1998 KW Document header added & class created                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _GFXWIN_H
#define _GFXWIN_H

#include <afxwin.h>
#include "resource.h"
#include "system.h"
#include "dmpwndlg.h"

#ifdef _LYNXDBG

class CGraphicsWin : public CFrameWnd
{
//	DECLARE_DYNCREATE(CGraphicsWin)
public:
	CGraphicsWin(CSystem& pLynx, CObList &list, CString initstr="");
	CString ArchiveStr(void);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGraphicsWin)
protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CGraphicsWin();

	// Generated message map functions
	//{{AFX_MSG(CGraphicsWin)
	afx_msg void OnPaint();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


private:
	// Reference back to the system object we interrogate
	CSystem& mSystem;
	CObList& mWindowList;

	// Bitmap objects
	CDC			*mWinCDC;
	CDC			*mMemCDC;
	CPalette	mPalette;
	CBitmap		mBitmap;

	CBitmap		*mOldBitmap;
	CPalette	*mOldPalette;

	UBYTE		*mBitmapBits;

	TPALETTE	mLynxPalette[16];
	ULONG		mColourMap[4096];

	// Memory address
	SLONG	mAddress;

	// Memory mode
	EMMODE	mMode;					// Defines RAM/CART0/CART1

	// Memory mask
	int		mMemorySize;			// Memory area size

	// Window drawing controls
	int		mLineCount;				// Number of lines within the window display
	int		mCharHeight;
	int		mCharWidth;
	int		mTabstops[32];
	int		mWindowMaxX;			// Maximum window width
	int		mZoomMode;

	// Utility functions

	CRect*	UpdateWindowVars(int xfree,int yfree);
	UBYTE	GetByte(ULONG address);
};

/////////////////////////////////////////////////////////////////////////////

#endif
#endif