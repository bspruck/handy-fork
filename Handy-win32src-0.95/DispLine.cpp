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
// Code class for overloaded variant of CEdit for Debugger usage            //
//////////////////////////////////////////////////////////////////////////////

#include <afxwin.h>
#include "displine.h"
#include "resource.h"
#include "debuggerwin.h"

#ifdef _LYNXDBG

CDispLine::CDispLine()
{
	m_crBkColor = ::GetSysColor(COLOR_3DFACE); // Initializing background color to the system face color.
	m_crTextColor = BLACK; // Initializing text color to black
	m_brBkgnd.CreateSolidBrush(m_crBkColor); // Creating the Brush Color For the Edit Box Background

	mIgnore=false;
}

CDispLine::~CDispLine()
{
}

BEGIN_MESSAGE_MAP(CDispLine, CEdit)
	//{{AFX_MSG_MAP(CDispLine)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CDispLine::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// If the control key is not held down then pass control, this will 
	// allow the edit keys to function normally
	if(mIgnore)
	{
		CEdit::OnChar(nChar,nRepCnt,nFlags);
	}
	else
	{
		((CDebuggerWin*)(GetParent()))->mpEditLine->SendMessage(WM_CHAR,nChar,nRepCnt|(nFlags<<16));
		((CDebuggerWin*)(GetParent()))->mpEditLine->SetFocus();
		mIgnore=false;
	}
}

void CDispLine::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar==VK_CONTROL) mIgnore=true;
	CEdit::OnKeyDown(nChar,nRepCnt,nFlags);
}

void CDispLine::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar==VK_CONTROL)
	{
		((CDebuggerWin*)(GetParent()))->mpEditLine->SetFocus();
		mIgnore=false;
	}

	CEdit::OnKeyUp(nChar,nRepCnt,nFlags);
}


/////////////////////////////////////////////////////////////////////////////
// CColorEdit message handlers

void CDispLine::SetTextColor(COLORREF crColor)
{
	m_crTextColor = crColor; // Passing the value passed by the dialog to the member varaible for Text Color
	RedrawWindow();
}

void CDispLine::SetBkColor(COLORREF crColor)
{
	m_crBkColor = crColor; // Passing the value passed by the dialog to the member varaible for Backgound Color
	m_brBkgnd.DeleteObject(); // Deleting any Previous Brush Colors if any existed.
	m_brBkgnd.CreateSolidBrush(crColor); // Creating the Brush Color For the Edit Box Background
	RedrawWindow();
}



HBRUSH CDispLine::CtlColor(CDC* pDC, UINT nCtlColor)
{
	HBRUSH hbr;
	hbr = (HBRUSH)m_brBkgnd; // Passing a Handle to the Brush
	pDC->SetBkColor(m_crBkColor); // Setting the Color of the Text Background to the one passed by the Dialog
	pDC->SetTextColor(m_crTextColor); // Setting the Text Color to the one Passed by the Dialog

	if (nCtlColor)       // To get rid of compiler warning
      nCtlColor += 0;

	return hbr;
}

BOOL CDispLine::SetReadOnly(BOOL flag)
{
   if (flag == TRUE)
      SetBkColor(m_crBkColor);
   else
      SetBkColor(WHITE);

   return CEdit::SetReadOnly(flag);
}

#endif