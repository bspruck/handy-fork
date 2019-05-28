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
// Lynx hex dump window class                                               //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class provides a hex dump window, its only interface to the         //
// emulator is via the reference it is passed to the lynx object in the     //
// constructor. It passes on unused keypresses back to the main window.     //
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

#include "dumpwin.h"
#include "widget.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

#define DUMPWIN_FONT		"Fixedsys"
#define DUMPWIN_FONTPOINT	12
#define DUMPWIN_HEIGHT		24
#define DUMPWIN_WIDTH		16


/////////////////////////////////////////////////////////////////////////////
// CDumpWin

// IMPLEMENT_DYNCREATE(CDumpWin, CFrameWnd)

#ifdef _LYNXDBG

CDumpWin::CDumpWin(CSystem& pLynx, CObList &list, CString initstr)
	:mSystem(pLynx),
	mWindowList(list)
{
	CRect rect;

	// Set default vars

	if(initstr=="")
	{
		mMode=cpu;
		mAddress=0;
		rect=rectDefault;
	}
	else
	{
		CWidget widget(initstr);
		mAddress=widget.GetNext();
		mMode=(EMMODE)widget.GetNext();
		rect.bottom=widget.GetNext();
		rect.right=widget.GetNext();
		rect.top=widget.GetNext();
		rect.left=widget.GetNext();
	}

	// Create ourself

	Create(NULL,"XXX",WS_OVERLAPPEDWINDOW|WS_VSCROLL,rect,NULL,NULL);

	// Add ourselves onto the window list

	mWindowList.AddTail(this);

	// Define font

	mpDumpFont = new CFont;

	if(!mpDumpFont->CreatePointFont(120,DUMPWIN_FONT,NULL))
	{
		MessageBox("Font Select failed","ERROR",MB_ICONHAND);
	}

	// Load our icon

	HICON hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
	SetIcon(hIcon,TRUE);

	MoveWindow(UpdateWindowVars(TRUE,FALSE));
}


CDumpWin::~CDumpWin()
{
}

CString CDumpWin::ArchiveStr(void)
{
	CRect rect;
	GetWindowRect(&rect);

	CString archive;
	//              Left:Top-:Righ:Botm:Mode:Addr
	archive.Format("%08x:%08x:%08x:%08x:%08x:%08x",rect.left,rect.top,rect.right,rect.bottom,mMode,mAddress);
	return archive;
}


BEGIN_MESSAGE_MAP(CDumpWin, CFrameWnd)
	//{{AFX_MSG_MAP(CDumpWin)
	ON_WM_PAINT()
	ON_WM_CONTEXTMENU()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDumpWin message handlers

void CDumpWin::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CFont* pOldFont;

	// Select in our font

	pOldFont=dc.SelectObject(mpDumpFont);

	if(mMemorySize>1)
	{
		// Enter the drawing loop

		CString	line,sprot,dump;
		int	address,loop,page,offset;
		UBYTE data;

		// Now repaint

		address=mAddress;

		for(loop=0;loop<mLineCount;loop++)
		{
			if(mMode==ram || mMode==cpu)
			{
				line.Format("%06x\t",address);
			}
			else
			{
				page=address/(mMemorySize>>8);
				offset=address%(mMemorySize>>8);
				line.Format("%02x:%03x\t",page,offset);
			}

			dump="";
			for(int hexloop=0;hexloop<DUMPWIN_WIDTH;hexloop++)
			{
				// Fetch the data byte

				data=GetByte(address+hexloop);
				
				// Format the line
				
				sprot.Format("%02x\t",data);
				line+=sprot;
				sprot.Format("%c",isprint(data)?data:'.');
				dump+=sprot;
			}

			line.MakeUpper();
			line+=dump;
			dc.TabbedTextOut(0,loop*mCharHeight,line,17,mTabstops,0);
			address+=DUMPWIN_WIDTH;
		}
	}
	else
	{
		// We have selected an invalid memory range

		dc.TextOut(mCharWidth*12,(mLineCount/2-1)*mCharHeight,"CARTRIDGE BANK IS EMPTY");
	}

	// Select the old font back in

	dc.SelectObject(pOldFont);
}

void CDumpWin::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CDumpWinDlg dlg(this,mMode,mAddress);
	if(dlg.DoModal()==IDOK)
	{
		mAddress=dlg.mAddress;
		mMode=(EMMODE)dlg.mMode;
		UpdateWindowVars(FALSE,FALSE);
		Invalidate(TRUE);
	}
}

void CDumpWin::PostNcDestroy() 
{
	mWindowList.RemoveAt(mWindowList.Find(this));
	delete mpDumpFont;
	delete this;
}



void CDumpWin::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	int increment=0;
	BOOL redraw=FALSE;

	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);

	switch(nChar)
	{
		case VK_LEFT:
			mAddress--;
			redraw=TRUE;
			break;
		case VK_RIGHT:
			mAddress++;
			redraw=TRUE;
			break;
		case VK_UP:
			increment=-1;
			break;
		case VK_DOWN:
			increment=1;
			break;
		case VK_PRIOR:
			increment=-(mLineCount-1);
			break;
		case VK_NEXT:
			increment=mLineCount-1;
			break;
		case VK_END:
			increment=mMemorySize;
			break;
		case VK_HOME:
			increment=-mMemorySize;
			break;
		default:
			// Re-send the message to the main window
			AfxGetMainWnd()->PostMessage(WM_KEYDOWN,nChar,nRepCnt);
			break;
	}

	if(increment)
	{
		increment*=nRepCnt;
		increment*=DUMPWIN_WIDTH;
		mAddress+=increment;

		// Force a redraw
		redraw=TRUE;
	}

	// Perform bounds checking

	if(mAddress<0)mAddress=0;
	if(mAddress>(mMemorySize-(DUMPWIN_WIDTH*mLineCount))) mAddress=mMemorySize-(DUMPWIN_WIDTH*mLineCount);
	mAddress&=mMemorySize-1;

	if(redraw)
	{
		// Update the scrollbar

		SetScrollPos(SB_VERT,mAddress/DUMPWIN_WIDTH,TRUE);

		Invalidate(FALSE);
	}

}



void CDumpWin::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);

	switch(nChar)
	{
		case VK_DELETE:
			{
				int address=0;
				CString line,dump,sprot;
				UBYTE data;

				FILE* fout;
				fopen_s(&fout, "dump.txt", "w");

				for(int loop=0;loop<(65536/DUMPWIN_WIDTH);loop++)
				{
					line.Format("%06x\t",address);

					dump="";
					for(int hexloop=0;hexloop<DUMPWIN_WIDTH;hexloop++)
					{
						// Fetch the data byte
						data=GetByte(address+hexloop);				
						// Format the line
						sprot.Format("%02x ",data);
						line+=sprot;
						sprot.Format("%c",isprint(data)?data:'.');
						dump+=sprot;
					}

					line.MakeUpper();
					line+=dump;
					fprintf(fout, "%s\n", (LPCTSTR) line);
					address+=DUMPWIN_WIDTH;
				}
				fclose(fout);
			}
			break;
		case VK_LEFT:
		case VK_RIGHT:
		case VK_UP:
		case VK_DOWN:
		case VK_PRIOR:
		case VK_NEXT:
		case VK_END:
		case VK_HOME:
			break;
		default:
			// Re-send the message to the main window
			AfxGetMainWnd()->PostMessage(WM_KEYUP,nChar,nRepCnt);
			break;
	}
}



void CDumpWin::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	int increment=0;

	CFrameWnd::OnVScroll(nSBCode, nPos, pScrollBar);

	switch(nSBCode)
	{
		case SB_LINEUP:
			increment=-1;
			break;
		case SB_LINEDOWN:
			increment=1;
			break;
		case SB_PAGEUP:
			increment=-(mLineCount-1);
			break;
		case SB_PAGEDOWN:
			increment=mLineCount-1;
			break;
		case SB_BOTTOM:
			increment=mMemorySize;
			break;
		case SB_TOP:
			increment=-mMemorySize;
			break;
		case SB_THUMBPOSITION:
			increment=nPos-(mAddress/DUMPWIN_WIDTH);
			break;
		default:
			break;
	}

	if(increment)
	{
		increment*=DUMPWIN_WIDTH;
		mAddress+=increment;

		// Perform bounds checking

		if(mAddress<0) mAddress=0;
		if(mAddress>(mMemorySize-(DUMPWIN_WIDTH*mLineCount))) mAddress=mMemorySize-(DUMPWIN_WIDTH*mLineCount);

		// Update the scrollbar

		SetScrollPos(SB_VERT,mAddress/DUMPWIN_WIDTH,TRUE);

		// Force a redraw

		Invalidate(FALSE);
	}
}


void CDumpWin::OnSize(UINT nType, int cx, int cy) 
{
	// Call framework move

	CFrameWnd::OnSize(nType, cx, cy);

	MoveWindow(UpdateWindowVars(TRUE,FALSE));
	Invalidate(FALSE);
}

void CDumpWin::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	// TODO: Add your message handler code here and/or call default
	
	CFrameWnd::OnGetMinMaxInfo(lpMMI);

	lpMMI->ptMaxSize.x=mWindowMaxX;			// Stop the window oversizing
	lpMMI->ptMinTrackSize.x=mWindowMaxX;
	lpMMI->ptMaxTrackSize.x=mWindowMaxX;
}


//
// Private utility functions
//

UBYTE CDumpWin::GetByte(ULONG address)
{
	UBYTE retval=0;
	switch(mMode)
	{
		case ram:
			retval=mSystem.Peek_RAM(address);
			break;
		case cpu:
			retval=mSystem.Peek_CPU(address);
			break;
		case bank0:
		case bank1:
			retval=mSystem.Peek_CART(address);
			break;
		default:
			break;
	}
	return retval;
}

CRect* CDumpWin::UpdateWindowVars(int xfree,int yfree)
{
	static CRect rect; // Static for returns

	CPaintDC dc(this); // device context for painting
	CFont* pOldFont;
	TEXTMETRIC tm;

	// Select in our font

	pOldFont=dc.SelectObject(mpDumpFont);

	dc.GetTextMetrics(&tm);

	mCharHeight=tm.tmHeight;
	mCharWidth=tm.tmAveCharWidth;

	// Calculate the TAB stop arrays

	for(int loop=0;loop<DUMPWIN_WIDTH;loop++)
	{
		mTabstops[loop]=(mCharWidth*8);			// Offset numerical body from addr
		mTabstops[loop]+=mCharWidth*(loop*2);	// Space for number pair
		mTabstops[loop]+=(mCharWidth/2)*loop;	// Space between number pairs
	}

	// Offset text from numerical body

	mTabstops[DUMPWIN_WIDTH]=mTabstops[DUMPWIN_WIDTH-1]+2*mCharWidth;

	// Enumerate the memory size

	if(mMode==ram || mMode==cpu)
	{
		mMemorySize=SYSTEM_SIZE;
	}
	else
	{
		mSystem.CartBank(mMode);
		mMemorySize=mSystem.CartSize();
	}

	// Work out the number of lines to be drawn

	GetClientRect(&rect);
	mLineCount=rect.Height()/mCharHeight;

	// Work out the optimum window size

	GetWindowRect(&rect);

	if(xfree)
	{
		rect.right=rect.left;
		rect.right+=mTabstops[16];				// Start to text field
		rect.right+=mCharWidth*DUMPWIN_WIDTH;	// End of text field
		rect.right+=mCharWidth*2;				// Fudge factor
		
		rect.right+=GetSystemMetrics(SM_CXHSCROLL)+(2*GetSystemMetrics(SM_CXEDGE))+10;
		mWindowMaxX=rect.right-rect.left;
	}

	if(yfree)
	{
		rect.bottom=rect.top+mCharHeight*DUMPWIN_HEIGHT;
		rect.bottom+=GetSystemMetrics(SM_CYSIZE)+(2*GetSystemMetrics(SM_CYEDGE))+10;
	}

	// Use memory size to set scroll range

	SetScrollRange(SB_VERT,0,(mMemorySize/DUMPWIN_WIDTH)-mLineCount,TRUE);

	// Select the old font back in

	dc.SelectObject(pOldFont);

	// Set window title

	CString title;

	switch(mMode)
	{
		case bank0:
			title="Dump - Cartridge Bank 0";
			break;
		case bank1:
			title="Dump - Cartridge Bank 1";
			break;
		case ram:
			title="Dump - System RAM";
			break;
		case cpu:
			title="Dump - System Memory";
			break;
		default:
			title="Mission Implausible";
			break;
	}
	SetWindowText(title);

	return(&rect);
}



void CDumpWin::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	// CFrameWnd::OnLButtonDblClk(nFlags, point);

	CHexEdit editctrl;

	editctrl.Create(WS_CHILD|WS_VISIBLE|ES_LEFT,CRect(100,100,150,150),this,NULL);
	editctrl.SetWindowText("Hello World");
}

#endif