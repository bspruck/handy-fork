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
// Lynx main window header file                                             //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition and code for the      //
// class that provides the normal window interface on Handy.                //
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

#ifndef _LYNXWIN_H
#define _LYNXWIN_H

#include <afxwin.h>
#include <afxcmn.h>
#include <afxdlgs.h>
#include <mmsystem.h>
#include "directx.h"
#include "fullscreendirectx.h"
#include "fullscreenlcd.h"
#include "fullscreeneagle.h"
#include "windowdirectx.h"
#include "windowgdi.h"
#include "windowlcd.h"
#include "windoweagle.h"
#include "directsoundplayer.h"
#include "resource.h"
#include "aboutdlg.h"
#include "keydefs.h"
#include "../core/system.h"
#include "netobj.h"
#include "debugger.h"

#ifdef TRACE_LYNXWIN

#define TRACE_LYNXWIN0(msg)					_RPT1(_CRT_WARN,"CLynxWin::"msg" (Time=%012d)\n",gSystemCycleCount)
#define TRACE_LYNXWIN1(msg,arg1)			_RPT2(_CRT_WARN,"CLynxWin::"msg" (Time=%012d)\n",arg1,gSystemCycleCount)
#define TRACE_LYNXWIN2(msg,arg1,arg2)		_RPT3(_CRT_WARN,"CLynxWin::"msg" (Time=%012d)\n",arg1,arg2,gSystemCycleCount)
#define TRACE_LYNXWIN3(msg,arg1,arg2,arg3)	_RPT4(_CRT_WARN,"CLynxWin::"msg" (Time=%012d)\n",arg1,arg2,arg3,gSystemCycleCount)

#else

#define TRACE_LYNXWIN0(msg)
#define TRACE_LYNXWIN1(msg,arg1)
#define TRACE_LYNXWIN2(msg,arg1,arg2)
#define TRACE_LYNXWIN3(msg,arg1,arg2,arg3)

#endif


// Dummy message we send to ourselves to prime the message pump
#define WM_LYNXWIN_WAKEUP (WM_USER+0x0231)

// Define timer IDs used
#define HANDY_INFO_TIMER			9741
#define HANDY_INFO_TIMER_PERIOD		2000

#define HANDY_JOYSTICK_TIMER		9742
#define HANDY_JOYSTICK_TIMER_PERIOD	50

//
// Defined type for the window mode
//
//
//	Bits	Function
//-------------------
//	0-1		Magnification (==Value+1)
//  2-3		Rotation (0=No rotate,)
//  8-11 	Display Type (1=Windowed, 0=Direct_x)
//  14      Background (1=Background)
//  15      Preserve

#define DISPLAY_X_MASK				0x0007
#define DISPLAY_ROTATE_MASK			0x0030
#define DISPLAY_RENDER_MASK			0x0f00
#define DISPLAY_BKGND_MASK			0x4000

#define DISPLAY_X_DEFAULT			0x0001
#define	DISPLAY_X1					0x0001
#define DISPLAY_X2					0x0002
#define	DISPLAY_X3					0x0003
#define	DISPLAY_X4					0x0004

#define DISPLAY_ROTATE_DEFAULT		0x0000
#define	DISPLAY_NO_ROTATE			0x0000
#define	DISPLAY_ROTATE_LEFT			0x0010
#define	DISPLAY_ROTATE_RIGHT		0x0020

#define DISPLAY_RENDER_DEFAULT		0x0000
#define	DISPLAY_WINDOWED			0x0000
#define	DISPLAY_FULLSCREEN			0x0100
#define DISPLAY_LYNXLCD_WINDOWED	0x0200
#define DISPLAY_LYNXLCD_FULLSCREEN	0x0300
#define DISPLAY_EAGLE_WINDOWED		0x0400
#define DISPLAY_EAGLE_FULLSCREEN	0x0500
#define DISPLAY_GDI_WINDOWED		0x0600

#define DISPLAY_BKGND_DEFAULT		0x4000
#define DISPLAY_NO_BKGND			0x0000
#define	DISPLAY_BKGND				0x4000

#define	DISPLAY_PRESERVE		0x8000


class CLynxWindow : public CFrameWnd
{
public:
	CLynxWindow(CString gamefile);
	~CLynxWindow();

	ULONG		DisplayModeSet(ULONG windowed,ULONG background,ULONG magnification,ULONG rotate);
	ULONG		DisplayModeSet(ULONG mode);
	ULONG		DisplayModeBkgnd(void) {return (mDisplayMode&DISPLAY_BKGND_MASK);}
	ULONG		DisplayModeRotate(void) {return (mDisplayMode&DISPLAY_ROTATE_MASK);}
	ULONG		DisplayModeMagnification(void) {if (mDisplayRender==NULL) return 1; else return mDisplayRender->mZoom;}

	ULONG		DisplayModeRender(void) {return (mDisplayMode&DISPLAY_RENDER_MASK);}
	ULONG		DisplayModeWindowed(void) {if (mDisplayRender==NULL) return TRUE; else return mDisplayRender->Windowed();}

	// Update all active windows, no Lynx update though

	inline void UpdateWindows(void)
	{
			OnPaint();
			OnDebuggerUpdate();
	}


	// Perform a cycle update on the complete system

	inline BOOL Update(void)
	{
		// Leave here if we are not in focus
		// if(!mWindowInFocus) return 0;
		// We dont need this now as the timer routine wont tick
		// if we are not in focus and so the throttling code
		// will put us to sleep at the next checkpoint.

		//
		// Throttling code
		//
		if(gSystemCycleCount>gThrottleNextCycleCheckpoint)
		{
			static int limiter=0;
			static int flipflop=0;
			int overrun=gSystemCycleCount-gThrottleNextCycleCheckpoint;
			int nextstep=(((HANDY_SYSTEM_FREQ/HANDY_TIMER_FREQ)*gThrottleMaxPercentage)/100);
			
			// We've gone thru the checkpoint, so therefore the
			// we must have reached the next timer tick, if the
			// timer hasnt ticked then we've got here early. If
			// so then put the system to sleep by saying there
			// is no more idle work to be done in the idle loop

			if(gThrottleLastTimerCount==gTimerCount)
			{
				// All we know is that we got here earlier than expected as the
				// counter has not yet rolled over
				if(limiter<0) limiter=0; else limiter++;
				if(limiter>40 && mFrameSkip>0)
				{
					mFrameSkip--;
					limiter=0;
				}
				flipflop=1;
				return 0;
			}

			// Frame Skip adjustment
			if(!flipflop)
			{
				if(limiter>0) limiter=0; else limiter--;
				if(limiter<-7 && mFrameSkip<10)
				{
					mFrameSkip++;
					limiter=0;
				}
			}

			flipflop=0;

			//Set the next control point
			gThrottleNextCycleCheckpoint+=nextstep;

			// Set next timer checkpoint
			gThrottleLastTimerCount=gTimerCount;

			// Check if we've overstepped the speed limit
			if(overrun>nextstep)
			{
				// We've exceeded the next timepoint, going way too
				// fast (sprite drawing) so reschedule.
				return 0;
			}

		}


#ifdef _LYNXDBG

		if(!gSystemHalt)
		{
			mpLynx->Update();
		}

		// If in debug mode then update all open windows if cycle mode
		if(mpDebugger->mDebugWindowUpdateFreq==DEBUG_WINUPD_CYCLE && !gSingleStepMode && !gSystemHalt) OnDebuggerUpdate();

		// Check breakpoints & status, repaint if system halted

		if(gBreakpointHit)
		{
			gSystemHalt=TRUE;
			gBreakpointHit=FALSE;

			// Force a window update
	
			OnDebuggerUpdate();
		}

#else

		// Step the model N times if no debugging required, its faster as there is a lower
		// windoze overhead.

		if(!gSystemHalt)
		{
			for(ULONG loop=1024;loop;loop--)
			{
				mpLynx->Update();
			}
		}

#endif

		return 1;
	}


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLynxWindow)
public:
	virtual BOOL DestroyWindow();
	//}}AFX_VIRTUAL

// Implementation

protected:	

	// Generated message map functions
	//{{AFX_MSG(CLynxWindow)
#ifdef _LYNXDBG
	afx_msg void OnDumpMenuSelect();
	afx_msg void OnCodeMenuSelect();
	afx_msg void OnTraceMenuSelect();
	afx_msg void OnGraphicsMenuSelect();
	afx_msg void OnRAMDumpMenuSelect();
	afx_msg void OnStepMenuSelect();
	afx_msg void OnDebuggerMenuSelect();
#endif
	afx_msg void OnDebuggerUpdate();
	afx_msg void OnPauseMenuSelect();
	afx_msg void OnPauseMenuUpdate(CCmdUI *pCmdUI);
	afx_msg void OnJoystickMenuSelect();
	afx_msg void OnJoystickMenuUpdate(CCmdUI *pCmdUI);
	afx_msg void OnDefineKeysSelect();
	afx_msg void OnSoundMenuSelect();
	afx_msg void OnSoundMenuUpdate(CCmdUI *pCmdUI);
	afx_msg void OnBkgndMenuSelect();
	afx_msg void OnBkgndMenuUpdate(CCmdUI *pCmdUI);
	afx_msg void OnBootromMenuSelect();
	afx_msg void OnBootromMenuUpdate(CCmdUI *pCmdUI);
	afx_msg void OnNetworkUpdate(WPARAM,LPARAM);
	afx_msg void OnNetworkDataWaiting(WPARAM,LPARAM);
	afx_msg void OnNetworkMenuSelect();
	afx_msg void OnNetworkMenuUpdate(CCmdUI *pCmdUI);
	afx_msg void OnInfoSelect();
	afx_msg void OnInfoMenuUpdate(CCmdUI *pCmdUI);
	afx_msg void OnAboutBoxSelect();
	afx_msg void OnResetMenuSelect();
	afx_msg void OnContextMenu(CWnd *pWnd, CPoint point);
	afx_msg void OnFileExit();
	afx_msg void OnFileLoad();
	afx_msg void OnFileSnapshotGame();
	afx_msg void OnFileSnapshotLoad();
	afx_msg void OnFileSnapshotBMP();
	afx_msg void OnFileSnapshotRAW();
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnActivateApp(BOOL bActive, DWORD hTask);
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy );
	afx_msg void OnDisplayEscapeFullScreen();
	afx_msg void OnDisplayToggleFullScreen();
	afx_msg void OnDropFiles(HDROP hDrop);
	//}}AFX_MSG
	afx_msg void OnDisplayModeSelect(UINT nID);
	afx_msg void OnDisplayModeUpdate(CCmdUI *pCmdUI);
	afx_msg void OnDisplayZoomSelect(UINT nID);
	afx_msg void OnDisplayZoomUpdate(CCmdUI *pCmdUI);
	afx_msg void OnDisplayRotateSelect(UINT nID);
	afx_msg void OnDisplayRotateUpdate(CCmdUI *pCmdUI);
	afx_msg void OnDisplayBackgroundTypeSelect(UINT nID);
	afx_msg void OnDisplayBackgroundTypeUpdate(CCmdUI *pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	void		CalcWindowSize(CRect *rect);
	CSystem*	CreateLynx(CString gamefile);

	static void		CALLBACK CLynxWindow::fTimerEventHandler(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2);
	static void		CLynxWindow::NetworkTxCallback(int data,ULONG objref);
	static UBYTE*	CLynxWindow::DisplayCallback(ULONG objref);
	void            CLynxWindow::CheckForBootRom(CString &romfile);

private:

	// Debug window object storage
#ifdef _LYNXDBG
	CDebugger	*mpDebugger;
#endif

	int			mJoystickEnable;
	ULONG		mJoystickXUp;
	ULONG		mJoystickXDown;
	ULONG		mJoystickYUp;
	ULONG		mJoystickYDown;

	KEYCONF		mKeyDefs;

	CLynxRender	*mDisplayRender;
	CBitmap		mDisplayBackground;
	unsigned short	mDisplayBackgroundType;
	BOOL        mUseBootRom;
	int			mDisplayOffsetX;
	int			mDisplayOffsetY;
	ULONG		mDisplayMode;
	BOOL		mDisplayNoPainting;

	BOOL		mCommandLineMode;

	ULONG		mTimerID;
	CDialog		mInfoDialog;
	int			mInfoDialogEnable;

	CString		mRootPath;

	CDirectSoundPlayer	mDirectSoundPlayer;

	CNetObj		*mpNetLynx;
	CWinApp		*mpLynxApp;

	volatile ULONG mEmulationSpeed;
	volatile ULONG mFramesPerSecond;
	volatile ULONG mFrameCount;
	ULONG	mFrameSkip;

public:
	// System object

	CSystem		*mpLynx;

	BOOL		mInitOK;
	BOOL		mWindowInFocus;

};


/////////////////////////////////////////////////////////////////////////////

#endif