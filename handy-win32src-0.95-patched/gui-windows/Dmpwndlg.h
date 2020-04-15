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
// Lynx debug window header file                                            //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition for the context       //
// dialog box used in the codewin and dumpwin classes                       //
//                                                                          //
//    K. Wilkins                                                            //
// August 1997                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 01Aug1997 KW Document header added & class documented.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef DMPWINDLG_H
#define DMPWINDLG_H

#include "resource.h"

class CDumpWinDlg : public CDialog
{
// Construction
public:
	CDumpWinDlg(CWnd* pParent, EMMODE mode=ram, int address=0, int followpc=-1, int zoom=-1 );
//	CDumpWinDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDumpWinDlg)
	enum { IDD = IDD_DUMPWIN_CONTEXT };
	int		mMode;
	CString	mAddrStr;
	int		mFollowPC;
	int		mZoom;
	//}}AFX_DATA

	int mAddress;	// Conversion of string address
	int mFontPoint;	// Conversion of font size

	int mEnableZoom;
	int mEnableFollowPC;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDumpWinDlg)
	public:
		virtual void OnOK();
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

public:
	void	SetAddress(int addr);
	void	SetMode(EMMODE mode);
	void	SetFollow(int follow);
	void	SetZoom(int zoom);
	void	EnableFollow();
	void	EnableZoom();
protected:

	// Generated message map functions
	//{{AFX_MSG(CDumpWinDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif