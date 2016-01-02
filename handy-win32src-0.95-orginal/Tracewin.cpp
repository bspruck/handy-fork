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
// Lynx trace window class                                                  //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class provides a window with with to control and display the status //
// of the emulator when debugging.                                          //
//                                                                          //
//    K. Wilkins                                                            //
// August 1997                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 01Aug1997 KW Document header added & class documented.                   //
// 02Nov1998 KW Removed CLynxWindow dependancy                              //                                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "tracewin.h"
#include "widget.h"


//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

enum { DEBUG_WINUPD_NORMAL=0,DEBUG_WINUPD_FRAME, DEBUG_WINUPD_CYCLE };

#ifdef _LYNXDBG

/////////////////////////////////////////////////////////////////////////////
// CTraceWin

// IMPLEMENT_DYNCREATE(CTraceWin, CFrameWnd)

CTraceWin::CTraceWin(CSystem& pLynx, CObList &list, ULONG *winupdfreq, CString initstr)
	:mSystem(pLynx),
	mWindowList(list),
	mpWindowUpdateFreq(winupdfreq)
{
	CRect rect;

	if(initstr=="")
	{
		rect.top=CW_USEDEFAULT;
		rect.left=CW_USEDEFAULT;
	}
	else
	{
		CWidget widget(initstr);
		rect.top=widget.GetNext();
		rect.left=widget.GetNext();
	}

	// Create ourself
	CString strWndClass=AfxRegisterWndClass(NULL, 0, (HBRUSH) COLOR_APPWORKSPACE+1, AfxGetApp()->LoadIcon(IDI_ICON1));
	CreateEx(0,strWndClass,"Trace Window",WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,rect.left,rect.top,CW_USEDEFAULT,CW_USEDEFAULT,NULL,NULL);

	// Add ourselves onto the window list
	mWindowList.AddTail(this);
}


CTraceWin::~CTraceWin()
{
}


CString CTraceWin::ArchiveStr(void)
{
	CRect rect;
	GetWindowRect(&rect);

	CString archive;
	//              Left:Top-
	archive.Format("%08x:%08x",rect.left,rect.top);
	return archive;
}


BEGIN_MESSAGE_MAP(CTraceWin, CWnd)
	//{{AFX_MSG_MAP(CTraceWin)
	ON_WM_CREATE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDumpWin message handlers

int CTraceWin::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// Create Dialog frame an attach to us/
	mDialog = new CTraceDlg(this,mSystem,mpWindowUpdateFreq);

	// Resize ourselves to fit the dialog
	CRect rect,wrect;
	mDialog->GetClientRect(&rect);
	CalcWindowRect(&rect);
	rect.OffsetRect(-rect.left,-rect.top);

	// Now offset the window to the current position
	GetWindowRect(&wrect);
	rect.OffsetRect(wrect.left,wrect.top);

	// Finally do the move
	MoveWindow(rect,FALSE);

	// Show the dialog & ourselves
	mDialog->ShowWindow(SW_SHOW);
	ShowWindow(SW_SHOW);

	// Return
	return CWnd::OnCreate(lpCreateStruct);
}

void CTraceWin::OnPaint()
{
	mDialog->Invalidate(FALSE);
	CWnd::OnPaint();
	return;
}

void CTraceWin::PostNcDestroy() 
{
	mWindowList.RemoveAt(mWindowList.Find(this));
	delete mDialog;
	delete this;
}

#endif