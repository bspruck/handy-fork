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
#include "editline.h"
#include "resource.h"

CEditLine::CEditLine()
{
	m_crBkColor = ::GetSysColor(COLOR_3DFACE); // Initializing background color to the system face color.
	m_crTextColor = BLACK; // Initializing text color to black
	m_brBkgnd.CreateSolidBrush(m_crBkColor); // Creating the Brush Color For the Edit Box Background

	mHistoryLast=-1;
	mHistoryMax=0;
	for(int loop=0;loop<EL_MAX_LINES;loop++)
	{
		strcpy(mHistory[loop],"");
	}
}

CEditLine::~CEditLine()
{
}

BEGIN_MESSAGE_MAP(CEditLine, CEdit)
	//{{AFX_MSG_MAP(CEditLine)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_CHAR()
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CEditLine::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
		case VK_UP:
			if(mHistoryMax>0)
			{
				// Print the current selection
				if(strcmp(mHistory[mHistoryLast],"")!=0) SetWindowText(mHistory[mHistoryLast]);
				SetSel(LineLength(-1),LineLength(-1));

				// Move up one in the list
				mHistoryLast--;
				if(mHistoryLast<0) mHistoryLast=mHistoryMax-1;
			}
			break;
		case VK_DOWN:
			if(mHistoryMax>0)
			{
				// Print the current selection
				if(strcmp(mHistory[mHistoryLast],"")!=0) SetWindowText(mHistory[mHistoryLast]);
				SetSel(LineLength(-1),LineLength(-1));

				// Move up one in the list
				mHistoryLast++;
				if(mHistoryLast>=mHistoryMax) mHistoryLast=0;
			}
			break;
		case VK_RETURN:
			{
				if(LineLength())
				{
					// Push the pointer
					mHistoryLast++;

					// Expand the history buffer to the max size
					if(mHistoryLast==mHistoryMax && mHistoryMax<EL_MAX_LINES) mHistoryMax++;

					// Wrap the buffer to the beginning
					if(mHistoryLast>=mHistoryMax) mHistoryLast=0;

					// Store the line
					int length=GetLine(0,mHistory[mHistoryLast],EL_MAX_LINE_LENGTH);
					mHistory[mHistoryLast][length]=0;

				}
				// Post a message to our parent, what shall we say ?
				(GetParent())->PostMessage(WM_COMMAND,IDM_EDITLINE_INPUTWAITING);
			}
			break;
		default:
			CEdit::OnKeyDown(nChar,nRepCnt,nFlags);
			break;
	}
}

void CEditLine::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar!=VK_RETURN) CEdit::OnKeyUp(nChar,nRepCnt,nFlags);
}

void CEditLine::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar!=VK_RETURN) CEdit::OnChar(nChar,nRepCnt,nFlags);
}

/////////////////////////////////////////////////////////////////////////////
// CColorEdit message handlers

void CEditLine::SetTextColor(COLORREF crColor)
{
	m_crTextColor = crColor; // Passing the value passed by the dialog to the member varaible for Text Color
	RedrawWindow();
}

void CEditLine::SetBkColor(COLORREF crColor)
{
	m_crBkColor = crColor; // Passing the value passed by the dialog to the member varaible for Backgound Color
	m_brBkgnd.DeleteObject(); // Deleting any Previous Brush Colors if any existed.
	m_brBkgnd.CreateSolidBrush(crColor); // Creating the Brush Color For the Edit Box Background
	RedrawWindow();
}



HBRUSH CEditLine::CtlColor(CDC* pDC, UINT nCtlColor)
{
	HBRUSH hbr;
	hbr = (HBRUSH)m_brBkgnd; // Passing a Handle to the Brush
	pDC->SetBkColor(m_crBkColor); // Setting the Color of the Text Background to the one passed by the Dialog
	pDC->SetTextColor(m_crTextColor); // Setting the Text Color to the one Passed by the Dialog

	if (nCtlColor)       // To get rid of compiler warning
      nCtlColor += 0;

	return hbr;
}

BOOL CEditLine::SetReadOnly(BOOL flag)
{
   if (flag == TRUE)
      SetBkColor(m_crBkColor);
   else
      SetBkColor(WHITE);

   return CEdit::SetReadOnly(flag);
}
