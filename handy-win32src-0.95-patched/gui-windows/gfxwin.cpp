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
// Lynx graphics debug display window class file                            //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class provides a graphics window, its only interface to the         //
// emulator is via the reference it is passed to the lynx object in the     //
// constructor. It passes on unused keypresses back to the main window.     //
//                                                                          //
//    K. Wilkins                                                            //
// November 1998                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 02Nov1998 KW Document header added & class documented.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "gfxwin.h"
#include "../core/lynxdef.h"
#include "widget.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

#define GFXWIN_WIDTH		160
#define GFXWIN_HEIGHT		320


/////////////////////////////////////////////////////////////////////////////
// CGraphicsWin

// IMPLEMENT_DYNCREATE(CGraphicsWin, CFrameWnd)

#ifdef _LYNXDBG

CGraphicsWin::CGraphicsWin(CSystem& pLynx,CObList &list, CString initstr)
	:mSystem(pLynx),
	mWindowList(list)
{
	CRect rect;

	// Set default vars

	if(initstr=="")
	{
		mMode=ram;
		mZoomMode=TRUE;
		mAddress=0;
		rect=rectDefault;
	}
	else
	{
		CWidget widget(initstr);
		mAddress=widget.GetNext();
		mZoomMode=widget.GetNext();
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

	// Load our icon

	HICON hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
	SetIcon(hIcon,TRUE);

	mWinCDC=NULL;
	mMemCDC=NULL;
	mOldBitmap=NULL;
	mOldPalette=NULL;
	mBitmapBits=NULL;

	// Initialise the local colour map
	TPALETTE Spot;

	for(Spot.Index=0;Spot.Index<4096;Spot.Index++)
	{
		mColourMap[Spot.Index]=(Spot.Colours.Red<<11)&0x7c00;
		mColourMap[Spot.Index]|=(Spot.Colours.Green<<6)&0x03e0;
		mColourMap[Spot.Index]|=(Spot.Colours.Blue<<1)&0x001f;
	}

	MoveWindow(UpdateWindowVars(TRUE,FALSE));
}


CGraphicsWin::~CGraphicsWin()
{
}

CString CGraphicsWin::ArchiveStr(void)
{
	CRect rect;
	GetWindowRect(&rect);

	CString archive;
	//              Left:Top-:Righ:Botm:Mode:Zoom:Addr
	archive.Format("%08x:%08x:%08x:%08x:%08x:%08x:%08x",rect.left,rect.top,rect.right,rect.bottom,mMode,mZoomMode,mAddress);
	return archive;
}


BEGIN_MESSAGE_MAP(CGraphicsWin, CFrameWnd)
	//{{AFX_MSG_MAP(CGraphicsWin)
	ON_WM_PAINT()
	ON_WM_CONTEXTMENU()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_VSCROLL()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGraphicsWin message handlers

void CGraphicsWin::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CBitmap* pOldBitmap;
	UBYTE *dest;
	int address,data;

	// Build pallette
	for(address=0;address<16;address++)
	{
		data=mSystem.Peek_CPU(GREEN0+address);
		mLynxPalette[address].Index=0;
		mLynxPalette[address].Colours.Green=data&0x0f;
	}

	for(address=0;address<16;address++)
	{
		data=mSystem.Peek_CPU(BLUERED0+address);
		mLynxPalette[address].Colours.Blue=(data&0xf0)>>4;
		mLynxPalette[address].Colours.Red=data&0x0f;
	}

	// Select in our font

	pOldBitmap=dc.SelectObject(&mBitmap);

	// Enter the rendering loop

	address=mAddress+((mLineCount-1)*(GFXWIN_WIDTH/2));
	dest=mBitmapBits;

	if(mMemorySize>1)
	{
		for(int loop=0;loop<mLineCount;loop++)
		{
			for(int hexloop=0;hexloop<GFXWIN_WIDTH/2;hexloop++)
			{
				// Fetch the data byte

				data=GetByte(address+hexloop);
				*((UWORD*)(dest))=(UWORD)mColourMap[mLynxPalette[data>>4].Index];
				dest+=2;
				*((UWORD*)(dest))=(UWORD)mColourMap[mLynxPalette[data&0x0f].Index];
				dest+=2;
				
				// Paint the pixels into the bitmap
			}
	
			address-=GFXWIN_WIDTH/2;
		}
	}
	else
	{
		// We have selected an invalid memory range

		dc.TextOut(mCharWidth*12,(mLineCount/2-1)*mCharHeight,"CARTRIDGE BANK IS EMPTY");
	}

	// Now Blit to the window

	if(mZoomMode)
	{
		if(!mWinCDC->StretchBlt(0,0,GFXWIN_WIDTH*2,mLineCount*2,mMemCDC,0,0,160,mLineCount,SRCCOPY)) gError->Fatal("CGraphicsWin::OnPaint() - Failed StretchBlt");
	}
	else
	{
		if(!mWinCDC->BitBlt(0,0,GFXWIN_WIDTH,mLineCount,mMemCDC,0,0,SRCCOPY)) gError->Fatal("CGraphicsWin::OnPaint() - Failed BitBlt");
	}

	// Select the old font back in

	dc.SelectObject(pOldBitmap);
}

void CGraphicsWin::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CDumpWinDlg dlg(this,mMode,mAddress,-1,mZoomMode);
	if(dlg.DoModal()==IDOK)
	{
		mAddress=dlg.mAddress;
		mMode=(EMMODE)dlg.mMode;
		mZoomMode=dlg.mZoom;
		MoveWindow(UpdateWindowVars(TRUE,TRUE));
		Invalidate(TRUE);
	}
}

void CGraphicsWin::PostNcDestroy() 
{
	mWindowList.RemoveAt(mWindowList.Find(this));

	// Destroy bitmap and associated CDC's

	if(mMemCDC!=NULL)
	{
        mMemCDC->SelectObject(mOldBitmap);
        delete mMemCDC;
        mMemCDC=NULL;
        mOldBitmap=NULL;
	}

	if(mWinCDC!=NULL)	
	{
		mWinCDC->SelectObject(mOldPalette);
        delete mWinCDC;
        mWinCDC=NULL;
        mOldPalette=NULL;
	}

	if(mBitmap.m_hObject!=NULL) mBitmap.DeleteObject();
	if(mPalette.m_hObject!=NULL) mPalette.DeleteObject();

	delete this;
}



void CGraphicsWin::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
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
		increment*=GFXWIN_WIDTH/2;		// 1 Byte = 2 pixels
		mAddress+=increment;

		// Force a redraw
		redraw=TRUE;
	}

	// Perform bounds checking

	if(mAddress<0)mAddress=0;
	if(mAddress>(mMemorySize-((GFXWIN_WIDTH/2)*mLineCount))) mAddress=mMemorySize-((GFXWIN_WIDTH/2)*mLineCount);
	mAddress&=mMemorySize-1;

	if(redraw)
	{
		// Update the scrollbar

		SetScrollPos(SB_VERT,mAddress/(GFXWIN_WIDTH/2),TRUE);

		Invalidate(FALSE);
	}

}


void CGraphicsWin::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	int increment=0;
	BOOL redraw=FALSE;

	CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);

	switch(nChar)
	{
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



void CGraphicsWin::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
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
			increment=nPos-(mAddress/(GFXWIN_WIDTH/2));
			break;
		default:
			break;
	}

	if(increment)
	{
		increment*=(GFXWIN_WIDTH/2);
		mAddress+=increment;

		// Perform bounds checking

		if(mAddress<0) mAddress=0;
		if(mAddress>(mMemorySize-((GFXWIN_WIDTH/2)*mLineCount))) mAddress=mMemorySize-((GFXWIN_WIDTH/2)*mLineCount);

		// Update the scrollbar

		SetScrollPos(SB_VERT,mAddress/(GFXWIN_WIDTH/2),TRUE);

		// Force a redraw

		Invalidate(FALSE);
	}
}


void CGraphicsWin::OnSize(UINT nType, int cx, int cy) 
{
	// Call framework move

	CFrameWnd::OnSize(nType, cx, cy);

	if(nType!=SIZE_MINIMIZED)
	{
		MoveWindow(UpdateWindowVars(TRUE,FALSE));
		Invalidate(FALSE);
	}
}

void CGraphicsWin::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
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

UBYTE CGraphicsWin::GetByte(ULONG address)
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

CRect* CGraphicsWin::UpdateWindowVars(int xfree,int yfree)
{
	static CRect rect; // Static for returns

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
	mLineCount=(mZoomMode)?rect.Height()/2:rect.Height();

	// Work out the optimum window size

	GetWindowRect(&rect);

	if(xfree)
	{
		rect.right=rect.left;
		rect.right+=GFXWIN_WIDTH;
		if(mZoomMode) rect.right+=GFXWIN_WIDTH;
		rect.right+=GetSystemMetrics(SM_CXHSCROLL)+(2*GetSystemMetrics(SM_CXEDGE))+10;
		mWindowMaxX=rect.right-rect.left;
	}

	if(yfree)
	{
		rect.bottom=rect.top+GFXWIN_HEIGHT;
		rect.bottom+=GetSystemMetrics(SM_CYSIZE)+(2*GetSystemMetrics(SM_CYEDGE))+10;
	}

	// Use memory size to set scroll range

	SetScrollRange(SB_VERT,0,(mMemorySize/(GFXWIN_WIDTH/2))-(mLineCount/(GFXWIN_WIDTH/2)),TRUE);

	// Set window title

	CString title;

	switch(mMode)
	{
		case bank0:
			title="GFX - Cart 0";
			break;
		case bank1:
			title="GFX - Cart 1";
			break;
		case ram:
			title="GFX - SysRAM";
			break;
		case cpu:
			title="GFX - SysMem";
			break;
		default:
			title="Mission Implausible";
			break;
	}
	SetWindowText(title);

	// Now create a bitmap we can select and write to

	//
	// Delete before we create
	//

	if(mWinCDC!=NULL)	
	{
		mWinCDC->SelectObject(mOldPalette);
        delete mWinCDC;
        mWinCDC=NULL;
        mOldPalette=NULL;
	}

	if(mMemCDC!=NULL)
	{
        mMemCDC->SelectObject(mOldBitmap);
        delete mMemCDC;
        mMemCDC=NULL;
        mOldBitmap=NULL;
	}

	if(mBitmap.m_hObject!=NULL) mBitmap.DeleteObject();
	if(mPalette.m_hObject!=NULL) mPalette.DeleteObject();

	//
	// Create a DC for us
	//

	mWinCDC= new CClientDC(this);

	//
	// Create the DIBsection we will use for blitting
	//

	BITMAPINFO *bi;

	bi= (BITMAPINFO*) new UBYTE[sizeof(BITMAPINFO)];
	ZeroMemory(bi, sizeof(BITMAPINFO));

	// Set up a BITMAPINFO structure for use with CreateDIBSection()

	bi->bmiHeader.biBitCount = 16;
	bi->bmiHeader.biSizeImage = (GFXWIN_WIDTH*mLineCount)*2;
	bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi->bmiHeader.biPlanes = 1;
	bi->bmiHeader.biWidth = GFXWIN_WIDTH;
	bi->bmiHeader.biHeight = mLineCount;
	bi->bmiHeader.biCompression = BI_RGB;

	HBITMAP hBitmap;
	if((hBitmap=CreateDIBSection(mWinCDC->m_hDC,bi,DIB_RGB_COLORS,(void **)&mBitmapBits,NULL,0))==NULL)
	{
		gError->Fatal("CGraphicsWin::UpdateWindowVars() - Failed CreateDIBSection for bitmap");
		return FALSE;
	}
	mBitmap.Attach(hBitmap);

	mMemCDC = new CDC;

	if(!mMemCDC->CreateCompatibleDC(mWinCDC))
	{
		gError->Fatal("CGraphiscWin::UpdateWindowVars() - Failed CreateCompatibleDC");
		return FALSE;
	}

	mOldBitmap=mMemCDC->SelectObject(&mBitmap);

	delete bi;

	return(&rect);
}


BOOL CGraphicsWin::OnEraseBkgnd(CDC* pDC) 
{
	CRect rect;
	CGdiObject *oldobj;
	GetClientRect(&rect);
	oldobj=pDC->SelectStockObject(BLACK_BRUSH);
	pDC->Rectangle(rect);
	pDC->SelectObject(oldobj);

	return TRUE;	

// Dont call the base class, we've finished
//	return CFrameWnd::OnEraseBkgnd(pDC);
}

#endif