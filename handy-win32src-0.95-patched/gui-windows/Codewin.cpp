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
// Lynx code disassembly window class                                       //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class provides a dissasembly window, its only interface to the      //
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

#include "codewin.h"
#include "../core/dis6502.h"
#include "widget.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

#define CODEWIN_FONT		"Fixedsys"
#define CODEWIN_FONTPOINT	12
#define CODEWIN_HEIGHT		24
#define CODEWIN_WIDTH		36


/////////////////////////////////////////////////////////////////////////////
// CCodeWin

// IMPLEMENT_DYNCREATE(CCodeWin, CFrameWnd)

#ifdef _LYNXDBG

CCodeWin::CCodeWin(CSystem& pLynx,CObList &list, CString initstr)
	:mSystem(pLynx),
	mWindowList(list)
{
	CRect rect;

	// Set default vars

	if(initstr=="")
	{
		mMode=cpu;
		mFollowPC=FALSE;

		C6502_REGS	regs;
		mSystem.GetRegs(regs);
		mAddress=regs.PC;
		rect=rectDefault;
	}
	else
	{
		CWidget widget(initstr);
		mAddress=widget.GetNext();
		mFollowPC=widget.GetNext();
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

	mpCodeFont = new CFont;

	if(!mpCodeFont->CreatePointFont(120,CODEWIN_FONT,NULL))
	{
		MessageBox("Font Select failed","ERROR",MB_ICONHAND);
	}

	// Load our icon

	HICON hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
	SetIcon(hIcon,TRUE);

	MoveWindow(UpdateWindowVars(TRUE,FALSE));
}


CCodeWin::~CCodeWin()
{
}

CString CCodeWin::ArchiveStr(void)
{
	CRect rect;
	GetWindowRect(&rect);

	CString archive;
	//              Left:Top-:Righ:Botm:Mode:Fllw:Addr
	archive.Format("%08x:%08x:%08x:%08x:%08x:%08x:%08x",rect.left,rect.top,rect.right,rect.bottom,mMode,mFollowPC,mAddress);
	return archive;
}

BEGIN_MESSAGE_MAP(CCodeWin, CFrameWnd)
	//{{AFX_MSG_MAP(CCodeWin)
	ON_WM_PAINT()
	ON_WM_CONTEXTMENU()
	ON_WM_KEYDOWN()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDumpWin message handlers

void CCodeWin::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CFont* pOldFont;
	C6502_REGS	regs;
	bool foundpc=false;

	// Select in our font

	pOldFont=dc.SelectObject(mpCodeFont);

	// Find out the PC for highlighting

	mSystem.GetRegs(regs);

	if(mMemorySize>1)
	{
		CString	line;
		int	current_address,address,loop;

		// If in follow mode then check PC will be draw else adjust mAddress
		// we don't follow if the CPU is not running

		if(mFollowPC)
		{
			bool test=false;
			address=mAddress;
			for(loop=0;loop<mLineCount;loop++)
			{
				if(address==regs.PC) test=true;
				address=NextAddress(address);
			}
			if(!test) mAddress=regs.PC;
		}

		// Now repaint

		address=mAddress;

		for(loop=0;loop<mLineCount;loop++)
		{
			line="";
			current_address=address;
			address=Disassemble(line,address);

			
			for(int bps=0;bps<MAX_CPU_BREAKPOINTS;bps++)
			{		
				if(current_address==regs.cpuBreakpoints[bps])
				{
					line.SetAt(0,'*');
				}
			}

			if(current_address!=regs.PC)
			{
				dc.TabbedTextOut(0,loop*mCharHeight,line,7,mTabstops,0);
			}
			else
			{
				COLORREF back,front;
				front=dc.GetTextColor();
				back=dc.GetBkColor();
				dc.SetTextColor(back);
				dc.SetBkColor(front);
				dc.TabbedTextOut(0,loop*mCharHeight,line,7,mTabstops,0);
				dc.SetTextColor(front);
				dc.SetBkColor(back);
			}
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

void CCodeWin::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CDumpWinDlg dlg(this,mMode,mAddress,mFollowPC,-1);
	if(dlg.DoModal()==IDOK)
	{
		mAddress=dlg.mAddress;
		mMode=(EMMODE)dlg.mMode;
		mFollowPC=(dlg.mFollowPC)?1:0;
		UpdateWindowVars(FALSE,FALSE);
		Invalidate(TRUE);
	}
}

void CCodeWin::PostNcDestroy() 
{
	mWindowList.RemoveAt(mWindowList.Find(this));
	delete mpCodeFont;
	delete this;
}


void CCodeWin::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	int increment=0;
	bool redraw=false;

	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);

	switch(nChar)
	{
		case VK_LEFT:
			if(mAddress) mAddress--;
			redraw=true;
			break;
		case VK_RIGHT:
			mAddress++;
			redraw=true;
			break;
		case VK_UP:
			increment=-1;
			break;
		case VK_DOWN:
			increment=1;
			break;
		case VK_PRIOR:
			increment=-((int)mLineCount-1);
			break;
		case VK_NEXT:
			increment=(int)mLineCount-1;
			break;
		case VK_END:
			increment=(int)mMemorySize;
			break;
		case VK_HOME:
			mAddress=0;
			redraw=true;
//			Invalidate(FALSE);
			break;
		default:
			// Re-send the message to the main window
			AfxGetMainWnd()->PostMessage(WM_KEYDOWN,nChar,nRepCnt);
			break;
	}

	if(increment)
	{
		increment*=nRepCnt;

		if(increment>0)
		{
			while(increment--) mAddress=NextAddress(mAddress);
		}
		else
		{
			while(increment++) mAddress=PrevAddress(mAddress);
		}

		// Force a redraw

		redraw=true;
	}

	// Perform bounds checking

	if(mAddress<0)mAddress=0;
	if(mAddress>(mMemorySize-mLineCount)) mAddress=mMemorySize-mLineCount;
	mAddress&=mMemorySize-1;
		
	if(redraw)
	{
		// Update the scrollbar

		SetScrollPos(SB_VERT,mAddress/CODEWIN_WIDTH,TRUE);

		Invalidate(FALSE);
	}
}


void CCodeWin::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);

	// Do not send back things we handle here

	switch(nChar)
	{
		case VK_DELETE:
			{
				int	current_address,address;
				CString line,dump,sprot;
				FILE *fout;

				fout=fopen("code.txt","w");

				for(address=0;address<65536;)
				{
					line="";
					current_address=address;
					address=Disassemble2(line,address);
					fprintf(fout,"%s\n",line);
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


void CCodeWin::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
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
			mAddress=0;
			Invalidate();
			break;
		case SB_THUMBPOSITION:
			increment=nPos-(mAddress/CODEWIN_WIDTH);
			break;
		default:
			break;
	}

	if(increment)
	{
		if(increment>0)
		{
			while(increment--) mAddress=NextAddress(mAddress);
		}
		else
		{
			while(increment++) mAddress=PrevAddress(mAddress);
		}

		// Perform bounds checking

		mAddress&=0xffff;
		
		// Update the scrollbar

		SetScrollPos(SB_VERT,mAddress/CODEWIN_WIDTH,TRUE);

		// Force a redraw

		Invalidate(false);
	}
}


void CCodeWin::OnSize(UINT nType, int cx, int cy) 
{
	// Call framework move

	CFrameWnd::OnSize(nType, cx, cy);

	MoveWindow(UpdateWindowVars(TRUE,FALSE));
	Invalidate(FALSE);
}

void CCodeWin::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	// TODO: Add your message handler code here and/or call default
	
	CFrameWnd::OnGetMinMaxInfo(lpMMI);

	lpMMI->ptMaxSize.x=mWindowMaxX;		// Stop the window oversizing
	lpMMI->ptMinTrackSize.x=mWindowMaxX;
	lpMMI->ptMaxTrackSize.x=mWindowMaxX;
}

void CCodeWin::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CFrameWnd::OnLButtonDblClk(nFlags, point);

	// Work out on which line it occured

	int line=point.y/mCharHeight;
	int address=mAddress;

	for(;line;line--)
	{
		address=NextAddress(address);
	}

	C6502_REGS regs;
	mSystem.GetRegs(regs);

	int loop;
	for(loop=0;loop<MAX_CPU_BREAKPOINTS;loop++)
	{
		// Unset if already set
		if(address==regs.cpuBreakpoints[loop]) break;
	}
	if(loop>=MAX_CPU_BREAKPOINTS)
	{
		// New breakpoint at next free
		for(loop=0;loop<MAX_CPU_BREAKPOINTS;loop++)
		{
			// Unset if already set
			if(regs.cpuBreakpoints[loop]>0xffff) break;
		}
		if(loop>=MAX_CPU_BREAKPOINTS)
		{
			MessageBox("All CPU breakpoints are in use, please unset some.","Warning",MB_ICONWARNING);
		}
		else
		{
			regs.cpuBreakpoints[loop]=address;
		}

	}
	else
	{
		// Remove old breakpoint by fudging address
		regs.cpuBreakpoints[loop]=0xffffff;
	}


	mSystem.SetRegs(regs);

	// Post the message to the main window to redraw debug
	AfxGetMainWnd()->PostMessage(WM_COMMAND,IDM_DRAW_DEBUG_WINDOWS);

//	Invalidate(FALSE);
}


//
//  Disassembly functions
//

int	CCodeWin::NextAddress(int address)
{
	if(address>0xffff) address=0xffff;

	UBYTE data=GetByte(address);

	int	operand=mLookupTable[data].mode;

	address=(address+mOperandSizes[operand]);

	if(address>0xffff) address=0xffff;

	return address;
}


int	CCodeWin::PrevAddress(int address)
{
	UBYTE data;
	int	operand,size;
	
	if(address>0xffff) address=0xffff;
	if(address<4) return address;

	// Check previous byte operand size 1==OK

	for(int loop=1;loop<4;loop++)
	{

		data=GetByte(address-loop);
		operand=mLookupTable[data].mode;
		size=mOperandSizes[operand];

		if(size==loop) return((address-loop)&0xffff);
	}

	return ((address-1)&0xffff);
}


int CCodeWin::Disassemble(CString &str,int address)
{
	CString tmpstr;
	int opcode,operand;
	int	count=NextAddress(address)-address;

	// Add the label

//	str+="Lxxxxx\t";
	str+="\t";

	// Add the address field

	tmpstr.Format("%05x\t",address);
	str+=tmpstr;

	// Dump the data fields

	for(int loop=0;loop<3;loop++)
	{
		if(count-->0)
		{
			tmpstr.Format("%02x\t",GetByte(address+loop));
		}
		else
		{
			tmpstr="  \t";
		}
		str+=tmpstr;
	}

	opcode=GetByte(address);

	// Add the disassembled code

	str+=mLookupTable[opcode].opcode;
	str+="\t";
	tmpstr="";

	// Read the operand
	
	switch(NextAddress(address)-address)
	{
		case 3:
			address++;
			operand=GetByte(address++);
			operand+=(GetByte(address++))<<8;
			break;
		case 2:
			address++;
			operand=GetByte(address++);
			break;
		case 1:
		default:
			address++;
			break;
	}

	// Decode the data

	switch(mLookupTable[opcode].mode)
	{
		case accu:
			tmpstr="A";
			break;
		case imm:
			tmpstr.Format("#$%02x",operand);
			break;
		case absl:
			tmpstr.Format("$%04x",operand);
			break;
		case rel:
			int scrap;
			if(operand>128) scrap=-128+(operand&0x7f); else scrap=operand;
			tmpstr.Format("$%04x",address+scrap);
			break;
		case iabs:
			tmpstr.Format("($%04x)",operand);
			break;			
		case ind:
			tmpstr.Format("($%02x)",operand);
			break;
		case zp:
			tmpstr.Format("$%02x",operand);
			break;
		case zpx:
			tmpstr.Format("$%02x,X",operand);
			break;
		case zpy:
			tmpstr.Format("$%02x,Y",operand);
			break;
		case absx:
			tmpstr.Format("$%04x,X",operand);
			break;
		case absy:
			tmpstr.Format("$%04x,Y",operand);
			break;
		case iabsx:
			tmpstr.Format("($%04x,X)",operand);
			break;
		case zrel:
			scrap=operand>>8;
			if(scrap>128) scrap=-128+(scrap&0x7f);
			tmpstr.Format("$%02x,$%04x",operand&0xff,address+scrap);
			break;
		case indx:
			tmpstr.Format("($%02x,X)",operand);
			break;
		case indy:
			tmpstr.Format("($%02x),Y",operand);
			break;
		case impl:
		case illegal:
		default:
			break;
	}

	str+=tmpstr;
	str+="\t ";

	return address;
}

int CCodeWin::Disassemble2(CString &str,int address)
{
	CString tmpstr;
	int opcode,operand;
	int	count=NextAddress(address)-address;

	// Add the address field

	tmpstr.Format("%05x\t",address);
	str+=tmpstr;

	// Dump the data fields

	for(int loop=0;loop<3;loop++)
	{
		if(count-->0)
		{
			tmpstr.Format("%02x ",GetByte(address+loop));
		}
		else
		{
			tmpstr="   ";
		}
		str+=tmpstr;
	}

	str+="    ";

	opcode=GetByte(address);

	// Add the disassembled code

	str+=mLookupTable[opcode].opcode;
	str+=" ";
	tmpstr="";

	// Read the operand
	
	switch(NextAddress(address)-address)
	{
		case 3:
			address++;
			operand=GetByte(address++);
			operand+=(GetByte(address++))<<8;
			break;
		case 2:
			address++;
			operand=GetByte(address++);
			break;
		case 1:
		default:
			address++;
			break;
	}

	// Decode the data

	switch(mLookupTable[opcode].mode)
	{
		case accu:
			tmpstr="A";
			break;
		case imm:
			tmpstr.Format("#$%02x",operand);
			break;
		case absl:
			tmpstr.Format("$%04x",operand);
			break;
		case rel:
			int scrap;
			if(operand>128) scrap=-128+(operand&0x7f); else scrap=operand;
			tmpstr.Format("$%04x",address+scrap);
			break;
		case iabs:
			tmpstr.Format("($%04x)",operand);
			break;			
		case ind:
			tmpstr.Format("($%02x)",operand);
			break;
		case zp:
			tmpstr.Format("$%02x",operand);
			break;
		case zpx:
			tmpstr.Format("$%02x,X",operand);
			break;
		case zpy:
			tmpstr.Format("$%02x,Y",operand);
			break;
		case absx:
			tmpstr.Format("$%04x,X",operand);
			break;
		case absy:
			tmpstr.Format("$%04x,Y",operand);
			break;
		case iabsx:
			tmpstr.Format("($%04x,X)",operand);
			break;
		case zrel:
			scrap=operand>>8;
			if(scrap>128) scrap=-128+(scrap&0x7f);
			tmpstr.Format("$%02x,$%04x",operand&0xff,address+scrap);
			break;
		case indx:
			tmpstr.Format("($%02x,X)",operand);
			break;
		case indy:
			tmpstr.Format("($%02x),Y",operand);
			break;
		case impl:
		case illegal:
		default:
			break;
	}

	str+=tmpstr;

	return address;
}

//
// Private utility functions
//

int CCodeWin::GetByte(int address)
{
	int retval=0;
	switch(mMode)
	{
		case ram:
			address&=0xffff;
			retval=mSystem.Peek_RAM(address);
			break;
		case cpu:
			address&=0xffff;
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


CRect* CCodeWin::UpdateWindowVars(int xfree,int yfree)
{
	static CRect rect; // Static for returns

	CPaintDC dc(this); // device context for painting
	CFont* pOldFont;
	TEXTMETRIC tm;

	// Select in our font

	pOldFont=dc.SelectObject(mpCodeFont);

	dc.GetTextMetrics(&tm);

	mCharHeight=tm.tmHeight;
	mCharWidth=tm.tmAveCharWidth;

	// Calculate the TAB stop arrays
	// Lxxxx  aaaaa nn nn nn  inc xxx

//	mTabstops[0]=mCharWidth*8;					// Label
	mTabstops[0]=mCharWidth;
	mTabstops[1]=mTabstops[0]+mCharWidth*6;		// Address
	mTabstops[2]=mTabstops[1]+mCharWidth*3;		// OP1
	mTabstops[3]=mTabstops[2]+mCharWidth*3;		// OP2
	mTabstops[4]=mTabstops[3]+mCharWidth*4;		// OP3
	mTabstops[5]=mTabstops[4]+mCharWidth*5;		// Operands
	mTabstops[6]=mTabstops[5]+mCharWidth*20;	// Blank

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
		rect.right+=mCharWidth*CODEWIN_WIDTH;	// End of text field
		rect.right+=mCharWidth*2;				// Fudge factor
		
		rect.right+=GetSystemMetrics(SM_CXHSCROLL)+(2*GetSystemMetrics(SM_CXEDGE))+10;
		mWindowMaxX=rect.right-rect.left;
	}

	if(yfree)
	{
		rect.bottom=rect.top+mCharHeight*CODEWIN_HEIGHT;
		rect.bottom+=GetSystemMetrics(SM_CYSIZE)+(2*GetSystemMetrics(SM_CYEDGE))+10;
	}

	// Use memory size to set scroll range

	SetScrollRange(SB_VERT,0,(mMemorySize/CODEWIN_WIDTH)-mLineCount,TRUE);

	// Select the old font back in

	dc.SelectObject(pOldFont);

	// Set window title

	CString title;

	switch(mMode)
	{
		case bank0:
			title="Code - Cartridge Bank 0";
			break;
		case bank1:
			title="Code - Cartridge Bank 1";
			break;
		case ram:
			title="Code - System RAM";
			break;
		case cpu:
			title="Code - System Memory";
			break;
		default:
			title="Mission Implausible";
			break;
	}
	SetWindowText(title);

	return(&rect);
}

#endif

