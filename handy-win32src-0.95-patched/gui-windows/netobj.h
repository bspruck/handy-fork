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
// Network Object Class                                                     //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class provides a minimal UDP connection service between two PCs     //
// to allow Lynx networking to be implemented.                              //
//                                                                          //
//    K. Wilkins                                                            //
// March 2000                                                               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 21Mar2000 KW Document header added & class created                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _NETOBJ_H
#define _NETOBJ_H

#include <afxwin.h>
//#include <winsock2.h>
#include <winsock.h>
#include <mmsystem.h>
#include "resource.h"
#include "system.h"

// Define the message we will send to 
// Use the ON_MESSAGE() macro to decode it
#define WM_NETOBJ_SELECT (WM_USER+0x1a01)
#define WM_NETOBJ_UPDATE (WM_USER+0x1a02)

// Network Message Types
//

#define HANDY_NODATA_MESSAGE	-1
#define HANDY_SYNC_MESSAGE		0x5aa5
#define HANDY_DATA_MESSAGE		0x0000
// NOTE THIS MUST, REPEAT MUST BE DEFINED THE SAME AS THE UART_BREAK_CODE in mikie.h
#define HANDY_BREAK_MESSAGE		0x9000

#define HANDY_SYNCUP_COUNT		0x14

#define HANDY_PORT_NUMBER		55573

// Define NetObj states & other info
#define NETOBJ_CAPTION		"Handy - Network Error"
enum { NETOBJ_STOP,NETOBJ_IDLE,NETOBJ_SYNC,NETOBJ_ACTIVE,NETOBJ_DEAD };

#if 1
#define TRACE_NETOBJ0(msg)					_RPT1(_CRT_WARN,"CNetObj::"msg" (Time=%012d)\n",gSystemCycleCount)
#define TRACE_NETOBJ1(msg,arg1)				_RPT2(_CRT_WARN,"CNetObj::"msg" (Time=%012d)\n",arg1,gSystemCycleCount)
#define TRACE_NETOBJ2(msg,arg1,arg2)		_RPT3(_CRT_WARN,"CNetObj::"msg" (Time=%012d)\n",arg1,arg2,gSystemCycleCount)
#define TRACE_NETOBJ3(msg,arg1,arg2,arg3)	_RPT4(_CRT_WARN,"CNetObj::"msg" (Time=%012d)\n",arg1,arg2,arg3,gSystemCycleCount)
#else
#define TRACE_NETOBJ0(msg)
#define TRACE_NETOBJ1(msg,arg1)
#define TRACE_NETOBJ2(msg,arg1,arg2)
#define TRACE_NETOBJ3(msg,arg1,arg2,arg3)
#endif

class CNetWaitDlg : public CDialog
{
	public:
		CNetWaitDlg()
			:CDialog()
		{
			Create(IDD_NETWAIT,NULL);
			mAbort=FALSE;
		}

		void SetString(CString tmpstr)
		{
			CStatic *tmp;
			tmp=(CStatic*)GetDlgItem(IDC_NETWAIT_TEXT);
			tmp->SetWindowText(tmpstr);
			ShowWindow(SW_SHOW);
		}
	private:
		afx_msg void OnCancel()
		{
			CDialog::OnCancel();
			mAbort=TRUE;
		}

	public:
		int mAbort;
};

class CNetObj
{
	public:
		CNetObj();
		~CNetObj();

		int  Start(CString netadr);
		int  Stop(void);
		int  Receive(void);
		void Transmit(int data);
		void Update(void);
		int  InitOK(void) { return ((mState==NETOBJ_DEAD)?FALSE:TRUE); };
		int  Active(void) { return ((mState==NETOBJ_SYNC || mState==NETOBJ_ACTIVE)?TRUE:FALSE); };

	private:
		static void CALLBACK CNetObj::SyncCallback(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2);

	public:
		int		mState;
		CString	mNetAddress;
		int			mRXcount;
		int			mTXcount;
		int			mLastRxData;

	private:
		WSADATA		mWsadata;
		CNetWaitDlg *mpWaitForSync;
		SOCKET		mRxSocket;
		SOCKET		mTxSocket;
		struct sockaddr_in mDestSockAddr;
		ULONG		mTimerID;
		int			mSyncCounter;
};

#endif