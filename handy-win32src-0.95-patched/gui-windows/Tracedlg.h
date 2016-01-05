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
// Lynx trace window header file                                            //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition afor the trace        //
// window that is used for displaying emulator status when debugging        //
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

#ifndef TRACEDLG_H
#define TRACEDLG_H

class CSystem;
class CRam;
class CCart;

class CTraceDlg : public CDialog
{
//	DECLARE_DYNCREATE(CTraceDlg)
public:
	CTraceDlg(CWnd *parent, CSystem& pLynx, ULONG *winupdfreq);

// Operations
public:
	int mCharHeight;
	int mCharWidth;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTraceDlg)
protected:
	virtual BOOL PreTranslateMessage(MSG*);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTraceDlg)
	afx_msg void OnPaint();
	afx_msg void OnSpriteDebugSelect();
	afx_msg void OnWinUpdFrameSelect();
	afx_msg void OnWinUpdCycleSelect();
	afx_msg void OnWinUpdNormSelect();
	afx_msg void OnRunStatRunSelect();
	afx_msg void OnRunStatHaltSelect();
	afx_msg void OnRunStatStepSelect();
	afx_msg void OnPauseSelect();
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// Reference back to the lynx system object we interrogate
	CSystem& mSystem;
	ULONG    *mpWindowUpdateFreq;

	HACCEL	mAccelerator;
};

#endif