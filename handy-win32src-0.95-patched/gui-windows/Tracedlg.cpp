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

#include <afxwin.h>
#include "../core/system.h"
#include "dmpwndlg.h"
#include "resource.h"
#include "tracedlg.h"


//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

enum { DEBUG_WINUPD_NORMAL=0,DEBUG_WINUPD_FRAME, DEBUG_WINUPD_CYCLE };

/////////////////////////////////////////////////////////////////////////////
// CTraceDlg

// IMPLEMENT_DYNCREATE(CTraceDlg, CDialog)

#ifdef _LYNXDBG

CTraceDlg::CTraceDlg(CWnd *parent,CSystem& pLynx, ULONG *winupdfreq)
	:mSystem(pLynx),
	mpWindowUpdateFreq(winupdfreq)
{
	// Create Dialog
	CDialog::Create(IDD_TRACE,parent);

	// Load the keyboard accelerator table
//	mAccelerator=::LoadAccelerators(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_ACCELERATOR1));
}


BEGIN_MESSAGE_MAP(CTraceDlg, CDialog)
	//{{AFX_MSG_MAP(CTraceDlg)
	ON_COMMAND(IDC_SPRITE_DEBUG, OnSpriteDebugSelect)
	ON_COMMAND(IDC_WINUPD_FRAME, OnWinUpdFrameSelect)
	ON_COMMAND(IDC_WINUPD_CYCLE, OnWinUpdCycleSelect)
	ON_COMMAND(IDC_WINUPD_NORM,  OnWinUpdNormSelect)
	ON_COMMAND(IDC_RUNSTAT_RUN,  OnRunStatRunSelect)
	ON_COMMAND(IDC_RUNSTAT_HALT, OnRunStatHaltSelect)
	ON_COMMAND(IDC_RUNSTAT_STEP, OnRunStatStepSelect)
	ON_COMMAND(IDM_DEBUG_STEP,   OnRunStatStepSelect)
	ON_COMMAND(IDM_OPTIONS_PAUSE,OnPauseSelect)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDumpWin message handlers

void CTraceDlg::OnSpriteDebugSelect()
{
	if(gSingleStepModeSprites) gSingleStepModeSprites=FALSE; else gSingleStepModeSprites=TRUE;
//	Invalidate(FALSE);
	// Post the message to the main window to redraw debug windows
	AfxGetMainWnd()->PostMessage(WM_COMMAND,IDM_DRAW_DEBUG_WINDOWS);
}

void CTraceDlg::OnWinUpdFrameSelect()
{
	*mpWindowUpdateFreq=DEBUG_WINUPD_FRAME;
//	Invalidate(FALSE);
	// Post the message to the main window to redraw debug windows
	AfxGetMainWnd()->PostMessage(WM_COMMAND,IDM_DRAW_DEBUG_WINDOWS);
}

void CTraceDlg::OnWinUpdCycleSelect()
{
	*mpWindowUpdateFreq=DEBUG_WINUPD_CYCLE;
//	Invalidate(FALSE);
	// Post the message to the main window to redraw debug windows
	AfxGetMainWnd()->PostMessage(WM_COMMAND,IDM_DRAW_DEBUG_WINDOWS);
}

void CTraceDlg::OnWinUpdNormSelect()
{
	*mpWindowUpdateFreq=DEBUG_WINUPD_NORMAL;
//	Invalidate(FALSE);
	// Post the message to the main window to redraw debug windows
	AfxGetMainWnd()->PostMessage(WM_COMMAND,IDM_DRAW_DEBUG_WINDOWS);
}

void CTraceDlg::OnRunStatRunSelect()
{
	gSingleStepMode=FALSE;
	gSystemHalt=FALSE;
//	Invalidate(FALSE);
	// Post the message to the main window to redraw debug windows
	AfxGetMainWnd()->PostMessage(WM_COMMAND,IDM_DRAW_DEBUG_WINDOWS);
}

void CTraceDlg::OnRunStatHaltSelect()
{
	gSingleStepMode=FALSE;
	gSystemHalt=TRUE;
//	Invalidate(FALSE);
	// Post the message to the main window to redraw debug windows
	AfxGetMainWnd()->PostMessage(WM_COMMAND,IDM_DRAW_DEBUG_WINDOWS);
}

void CTraceDlg::OnRunStatStepSelect()
{
	gSingleStepMode=TRUE;
	gSystemHalt=FALSE;
//	Invalidate(FALSE);
	// Post the message to the main window to redraw debug windows
	AfxGetMainWnd()->PostMessage(WM_COMMAND,IDM_DRAW_DEBUG_WINDOWS);
}

void CTraceDlg::OnPauseSelect()
{
	if(!gSystemHalt)
	{
		gBreakpointHit=TRUE;	// This will force a window redraw in the debug version
		gSystemHalt=TRUE;		// This will stop the system
	}
	else
	{
		gSystemHalt=FALSE;		// Make sure we run always
		gSingleStepMode=FALSE;	// Go for it....
	}
//	Invalidate(FALSE);
	// Post the message to the main window to redraw debug windows
	AfxGetMainWnd()->PostMessage(WM_COMMAND,IDM_DRAW_DEBUG_WINDOWS);
}


void CTraceDlg::OnPaint() 
{
	C6502_REGS	regs;
	CStatic	*static_control_tmp;
	CString	tmpstr;
	int tmp;

	// Fetch our new register data
	mSystem.GetRegs(regs);

	// Only repaint if there has been a change, stops things flickering.

	// Initialise the Breakpoint list
	UINT dlgids[8]={IDC_BPOINT0,IDC_BPOINT1,IDC_BPOINT2,IDC_BPOINT3,IDC_BPOINT4,IDC_BPOINT5,IDC_BPOINT6,IDC_BPOINT7};
	for(int loop=0;loop<8;loop++)
	{
		if(regs.cpuBreakpoints[loop]>0xffff) tmpstr="Empty"; else tmpstr.Format("0x%04x",regs.cpuBreakpoints[loop]);
		static_control_tmp=(CStatic*)GetDlgItem(dlgids[loop]);
		static_control_tmp->SetWindowText(tmpstr);
	}

	// Print Cycle Count
	tmpstr.Format("%08x",gSystemCycleCount);
	static_control_tmp=(CStatic*)GetDlgItem(IDC_CYCLE_COUNT);
	static_control_tmp->SetWindowText(tmpstr);

	// Initialise the radio buttons for frame
	switch(*mpWindowUpdateFreq)
	{
		case DEBUG_WINUPD_NORMAL:
			tmp=IDC_WINUPD_NORM;
			break;
		case DEBUG_WINUPD_FRAME:
			tmp=IDC_WINUPD_FRAME;
			break;
		case DEBUG_WINUPD_CYCLE:
		default:
			tmp=IDC_WINUPD_CYCLE;
			break;
	}
	CheckRadioButton(IDC_WINUPD_NORM,IDC_WINUPD_CYCLE,tmp);

	// Initialise the radio buttons for run mode

	if(gSystemHalt)
	{
		if(gSingleStepMode) tmp=IDC_RUNSTAT_STEP; else tmp=IDC_RUNSTAT_HALT;
	}	
	else
	{	
		tmp=IDC_RUNSTAT_RUN;
	}
	CheckRadioButton(IDC_RUNSTAT_HALT,IDC_RUNSTAT_RUN,tmp);

	// Flag if sprite stepping
	CheckDlgButton(IDC_SPRITE_DEBUG,gSingleStepModeSprites);

	// Sort out the PC registers
	tmpstr.Format("0x%04x",regs.PC);
	static_control_tmp=(CStatic*)GetDlgItem(IDC_REG_PC);
	static_control_tmp->SetWindowText(tmpstr);

	tmpstr.Format("0x%02x",regs.SP);
	static_control_tmp=(CStatic*)GetDlgItem(IDC_REG_SP);
	static_control_tmp->SetWindowText(tmpstr);

	tmpstr.Format("0x%02x",regs.PS);
	static_control_tmp=(CStatic*)GetDlgItem(IDC_REG_PS);
	static_control_tmp->SetWindowText(tmpstr);

	tmpstr.Format("0x%02x",regs.A);
	static_control_tmp=(CStatic*)GetDlgItem(IDC_REG_A);
	static_control_tmp->SetWindowText(tmpstr);

	tmpstr.Format("0x%02x",regs.X);
	static_control_tmp=(CStatic*)GetDlgItem(IDC_REG_X);
	static_control_tmp->SetWindowText(tmpstr);

	tmpstr.Format("0x%02x",regs.Y);
	static_control_tmp=(CStatic*)GetDlgItem(IDC_REG_Y);
	static_control_tmp->SetWindowText(tmpstr);

	tmpstr.Format("%d",regs.NMI);
	static_control_tmp=(CStatic*)GetDlgItem(IDC_REG_NMI);
	static_control_tmp->SetWindowText(tmpstr);

	tmpstr.Format("%d",regs.IRQ);
	static_control_tmp=(CStatic*)GetDlgItem(IDC_REG_IRQ);
	static_control_tmp->SetWindowText(tmpstr);

	tmpstr.Format("%d",regs.WAIT);
	static_control_tmp=(CStatic*)GetDlgItem(IDC_REG_WAI);
	static_control_tmp->SetWindowText(tmpstr);

	tmpstr.Format("%d",(regs.PS&0x80)?1:0);
	static_control_tmp=(CStatic*)GetDlgItem(IDC_REG_N);
	static_control_tmp->SetWindowText(tmpstr);

	tmpstr.Format("%d",(regs.PS&0x40)?1:0);
	static_control_tmp=(CStatic*)GetDlgItem(IDC_REG_V);
	static_control_tmp->SetWindowText(tmpstr);

	tmpstr.Format("%d",(regs.PS&0x10)?1:0);
	static_control_tmp=(CStatic*)GetDlgItem(IDC_REG_B);
	static_control_tmp->SetWindowText(tmpstr);

	tmpstr.Format("%d",(regs.PS&0x08)?1:0);
	static_control_tmp=(CStatic*)GetDlgItem(IDC_REG_D);
	static_control_tmp->SetWindowText(tmpstr);

	tmpstr.Format("%d",(regs.PS&0x04)?1:0);
	static_control_tmp=(CStatic*)GetDlgItem(IDC_REG_I);
	static_control_tmp->SetWindowText(tmpstr);

	tmpstr.Format("%d",(regs.PS&0x02)?1:0);
	static_control_tmp=(CStatic*)GetDlgItem(IDC_REG_Z);
	static_control_tmp->SetWindowText(tmpstr);

	tmpstr.Format("%d",(regs.PS&0x01)?1:0);
	static_control_tmp=(CStatic*)GetDlgItem(IDC_REG_C);
	static_control_tmp->SetWindowText(tmpstr);

	// Run the baseclass OnPaint
	CDialog::OnPaint();
}


BOOL CTraceDlg::OnEraseBkgnd(CDC *pDC)
{
//	return CDialog::OnEraseBkgnd(pDC);

	// Dont allow the background to be redrawn
	return 1;
}


BOOL CTraceDlg::PreTranslateMessage(MSG *pMsg)
{
//	_RPT3(_CRT_WARN,"CTraceDlg::PreTranslateMessage( Message:%08d   wP:%08d  lP:%08d\n",pMsg->message,pMsg->wParam,pMsg->lParam);
	if(mAccelerator!=NULL)
		if(::TranslateAccelerator(m_hWnd,mAccelerator,pMsg)) return TRUE;

	return CDialog::PreTranslateMessage(pMsg);
}



#endif