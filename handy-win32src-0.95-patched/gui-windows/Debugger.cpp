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


// ErrorHandler.cpp: implementation of the ErrorHandler class.
//
//////////////////////////////////////////////////////////////////////

#include <afxwin.h>
#include <afxcmn.h>
#include <afxdlgs.h>

#include "debugger.h"

#include "../core/system.h"
#include "../core/error.h"

#include "debuggerwin.h"
#include "tracedlg.h"
#include "tracewin.h"
#include "dumpwin.h"
#include "codewin.h"
#include "gfxwin.h"

//#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
//#endif

#define REGISTRY_VERSION	"Version 1.0"

#ifdef _LYNXDBG

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDebugger::CDebugger(CSystem &mLynxSystem)
	:mLynx(mLynxSystem)
{
	// Find the handle to our calling application
	CWinApp	*pLynxApp=AfxGetApp();

	int loop;
	CString regentry,debugstr;

	mDebugWindowUpdateFreq=pLynxApp->GetProfileInt(REGISTRY_VERSION,"DebugWindowUpdateFreq",DEBUG_WINUPD_NORMAL);
	
	for(loop=0;loop<MAX_DEBUGGER_WIN;loop++)
	{
		regentry.Format("DebugDebuggerWin%d",loop);
		debugstr=pLynxApp->GetProfileString(REGISTRY_VERSION,regentry,"");
		if(debugstr!="")
		{
			// Now Create window from string - unarchive
			CDebuggerWin *tmp = new CDebuggerWin(mLynx,mDebuggerWindowList,debugstr);
			tmp->ShowWindow(SW_SHOW);
			tmp->UpdateWindow();
		}
	}
	for(loop=0;loop<MAX_TRACE_WIN;loop++)
	{
		regentry.Format("DebugTraceWin%d",loop);
		debugstr=pLynxApp->GetProfileString(REGISTRY_VERSION,regentry,"");
		if(debugstr!="")
		{
			// Now Create window from string - unarchive
			CTraceWin *tmp = new CTraceWin(mLynx,mTraceWindowList,&mDebugWindowUpdateFreq,debugstr);
			tmp->ShowWindow(SW_SHOW);
			tmp->UpdateWindow();
		}
	}
	for(loop=0;loop<MAX_DUMP_WIN;loop++)
	{
		regentry.Format("DebugDumpWin%d",loop);
		debugstr=pLynxApp->GetProfileString(REGISTRY_VERSION,regentry,"");
		if(debugstr!="")
		{
			// Now Create window from string - unarchive
			CDumpWin *tmp = new CDumpWin(mLynx,mDumpWindowList,debugstr);
			tmp->ShowWindow(SW_SHOW);
			tmp->UpdateWindow();
		}
	}
	for(loop=0;loop<MAX_CODE_WIN;loop++)
	{
		regentry.Format("DebugCodeWin%d",loop);
		debugstr=pLynxApp->GetProfileString(REGISTRY_VERSION,regentry,"");
		if(debugstr!="")
		{
			// Now Create window from string - unarchive
			CCodeWin *tmp = new CCodeWin(mLynx,mCodeWindowList,debugstr);
			tmp->ShowWindow(SW_SHOW);
			tmp->UpdateWindow();
		}
	}
	for(loop=0;loop<MAX_GRAPHICS_WIN;loop++)
	{
		regentry.Format("DebugGraphicsWin%d",loop);
		debugstr=pLynxApp->GetProfileString(REGISTRY_VERSION,regentry,"");
		if(debugstr!="")
		{
			// Now Create window from string - unarchive
			CGraphicsWin *tmp = new CGraphicsWin(mLynx,mGraphicsWindowList,debugstr);
			tmp->ShowWindow(SW_SHOW);
			tmp->UpdateWindow();
		}
	}

}

CDebugger::~CDebugger()
{
	int winnum=0;
	CString regentry;
	// Find the handle to our calling application
	CWinApp	*pLynxApp=AfxGetApp();

	pLynxApp->WriteProfileInt(REGISTRY_VERSION,"DebugWindowUpdateFreq",mDebugWindowUpdateFreq);
	
	winnum=0;
	// Archive windows to the registry
	while(mDebuggerWindowList.GetCount())
	{
		// Registry entry
		regentry.Format("DebugDebuggerWin%d",winnum++);
		pLynxApp->WriteProfileString(REGISTRY_VERSION,regentry,((CDebuggerWin*)mDebuggerWindowList.GetHead())->ArchiveStr());
		// Scratch one window
		((CDebuggerWin*)mDebuggerWindowList.GetHead())->DestroyWindow();
	}
	// Blank unused windows
	for(;winnum<MAX_DEBUGGER_WIN;)
	{
		regentry.Format("DebugDebuggerWin%d",winnum++);
		pLynxApp->WriteProfileString(REGISTRY_VERSION,regentry,"");
	}

	winnum=0;
	// Archive windows to the registry
	while(mTraceWindowList.GetCount())
	{
		// Registry entry
		regentry.Format("DebugTraceWin%d",winnum++);
		pLynxApp->WriteProfileString(REGISTRY_VERSION,regentry,((CTraceWin*)mTraceWindowList.GetHead())->ArchiveStr());
		// Scratch one window
		((CTraceWin*)mTraceWindowList.GetHead())->DestroyWindow();
	}
	// Blank unused windows
	for(;winnum<MAX_TRACE_WIN;)
	{
		regentry.Format("DebugTraceWin%d",winnum++);
		pLynxApp->WriteProfileString(REGISTRY_VERSION,regentry,"");
	}

	winnum=0;
	// Archive windows to the registry
	while(mDumpWindowList.GetCount())
	{
		// Registry entry
		regentry.Format("DebugDumpWin%d",winnum++);
		pLynxApp->WriteProfileString(REGISTRY_VERSION,regentry,((CDumpWin*)mDumpWindowList.GetHead())->ArchiveStr());
		// Scratch one window
		((CDumpWin*)mDumpWindowList.GetHead())->DestroyWindow();
	}
	// Blank unused windows
	for(;winnum<MAX_DUMP_WIN;)
	{
		regentry.Format("DebugDumpWin%d",winnum++);
		pLynxApp->WriteProfileString(REGISTRY_VERSION,regentry,"");
	}

	winnum=0;
	// Archive windows to the registry
	while(mCodeWindowList.GetCount())
	{
		// Registry entry
		regentry.Format("DebugCodeWin%d",winnum++);
		pLynxApp->WriteProfileString(REGISTRY_VERSION,regentry,((CCodeWin*)mCodeWindowList.GetHead())->ArchiveStr());
		// Scratch one window
		((CCodeWin*)mCodeWindowList.GetHead())->DestroyWindow();
	}
	// Blank unused windows
	for(;winnum<MAX_CODE_WIN;)
	{
		regentry.Format("DebugCodeWin%d",winnum++);
		pLynxApp->WriteProfileString(REGISTRY_VERSION,regentry,"");
	}

	winnum=0;
	// Archive windows to the registry
	while(mGraphicsWindowList.GetCount())
	{
		// Registry entry
		regentry.Format("DebugGraphicsWin%d",winnum++);
		pLynxApp->WriteProfileString(REGISTRY_VERSION,regentry,((CGraphicsWin*)mGraphicsWindowList.GetHead())->ArchiveStr());
		// Scratch one window
		((CGraphicsWin*)mGraphicsWindowList.GetHead())->DestroyWindow();
	}
	// Blank unused windows
	for(;winnum<MAX_GRAPHICS_WIN;)
	{
		regentry.Format("DebugGraphicsWin%d",winnum++);
		pLynxApp->WriteProfileString(REGISTRY_VERSION,regentry,"");
	}


}

void CDebugger::Update(void)
{
	POSITION pos;

	for(pos=mDebuggerWindowList.GetHeadPosition();pos!=NULL;)
	{
		((CDebuggerWin*)mDebuggerWindowList.GetNext(pos))->Invalidate(FALSE);
	}

	for(pos=mTraceWindowList.GetHeadPosition();pos!=NULL;)
	{
		((CTraceWin*)mTraceWindowList.GetNext(pos))->Invalidate(FALSE);
	}

	for(pos=mDumpWindowList.GetHeadPosition();pos!=NULL;)
	{
		((CDumpWin*)mDumpWindowList.GetNext(pos))->Invalidate(FALSE);
	}

	for(pos=mCodeWindowList.GetHeadPosition();pos!=NULL;)
	{
		((CCodeWin*)mCodeWindowList.GetNext(pos))->Invalidate(FALSE);
	}

	for(pos=mGraphicsWindowList.GetHeadPosition();pos!=NULL;)
	{
		((CGraphicsWin*)mGraphicsWindowList.GetNext(pos))->Invalidate(FALSE);
	}
}


void CDebugger::Minimise(void)
{
	POSITION pos;
	for(pos=mDebuggerWindowList.GetHeadPosition();pos!=NULL;)
		((CDebuggerWin*)mDebuggerWindowList.GetNext(pos))->ShowWindow(SW_MINIMIZE);

	for(pos=mTraceWindowList.GetHeadPosition();pos!=NULL;)
		((CTraceWin*)mTraceWindowList.GetNext(pos))->ShowWindow(SW_MINIMIZE);

	for(pos=mDumpWindowList.GetHeadPosition();pos!=NULL;)
		((CDumpWin*)mDumpWindowList.GetNext(pos))->ShowWindow(SW_MINIMIZE);

	for(pos=mCodeWindowList.GetHeadPosition();pos!=NULL;)
		((CCodeWin*)mCodeWindowList.GetNext(pos))->ShowWindow(SW_MINIMIZE);
	
	for(pos=mGraphicsWindowList.GetHeadPosition();pos!=NULL;)
		((CGraphicsWin*)mGraphicsWindowList.GetNext(pos))->ShowWindow(SW_MINIMIZE);
}

void CDebugger::Restore(void)
{
	POSITION pos;
	for(pos=mDebuggerWindowList.GetHeadPosition();pos!=NULL;)
		((CDebuggerWin*)mDebuggerWindowList.GetNext(pos))->ShowWindow(SW_RESTORE);

	for(pos=mTraceWindowList.GetHeadPosition();pos!=NULL;)
		((CTraceWin*)mTraceWindowList.GetNext(pos))->ShowWindow(SW_RESTORE);

	for(pos=mDumpWindowList.GetHeadPosition();pos!=NULL;)
		((CDumpWin*)mDumpWindowList.GetNext(pos))->ShowWindow(SW_RESTORE);

	for(pos=mCodeWindowList.GetHeadPosition();pos!=NULL;)
		((CCodeWin*)mCodeWindowList.GetNext(pos))->ShowWindow(SW_RESTORE);
	
	for(pos=mGraphicsWindowList.GetHeadPosition();pos!=NULL;)
		((CGraphicsWin*)mGraphicsWindowList.GetNext(pos))->ShowWindow(SW_RESTORE);
}


void CDebugger::DebuggerDebugger()
{
	if(mDebuggerWindowList.GetCount()<MAX_DEBUGGER_WIN)
	{
		// Window will add itself to the window list
		CDebuggerWin *tmp = new CDebuggerWin(mLynx,mDebuggerWindowList);
		tmp->ShowWindow(SW_SHOW);
		tmp->UpdateWindow();
	}
}

void CDebugger::DebuggerTrace()
{
	if(mTraceWindowList.GetCount()<MAX_TRACE_WIN)
	{
		// Window will add itself to the window list
		CTraceWin *tmp = new CTraceWin(mLynx,mTraceWindowList,&mDebugWindowUpdateFreq);
		tmp->ShowWindow(SW_SHOW);
		tmp->UpdateWindow();
	}
}

void CDebugger::DebuggerCode()
{
	if(mCodeWindowList.GetCount()<MAX_CODE_WIN)
	{
		// Window will add itself to the window list
		CCodeWin *tmp = new CCodeWin(mLynx,mCodeWindowList);
		tmp->ShowWindow(SW_SHOW);
		tmp->UpdateWindow();
	}
}

void CDebugger::DebuggerDump()
{
	if(mDumpWindowList.GetCount()<MAX_DUMP_WIN)
	{
		// Window will add itself to the window list
		CDumpWin *tmp = new CDumpWin(mLynx,mDumpWindowList);
		tmp->ShowWindow(SW_SHOW);
		tmp->UpdateWindow();
	}
}

void CDebugger::DebuggerGFX()
{
	if(mGraphicsWindowList.GetCount()<MAX_GRAPHICS_WIN)
	{
		// Window will add itself to the window list
		CGraphicsWin *tmp = new CGraphicsWin(mLynx,mGraphicsWindowList);
		tmp->ShowWindow(SW_SHOW);
		tmp->UpdateWindow();
	}
}

void CDebugger::DebuggerMessage(ULONG objref, char* message)
{
	POSITION pos;
	CDebugger *debugger = (CDebugger*)objref;

	//Dump to all debugger windows
	for(pos=debugger->mDebuggerWindowList.GetHeadPosition();pos!=NULL;)
		((CDebuggerWin*)debugger->mDebuggerWindowList.GetNext(pos))->LineOutput(message);

	//Dump to log file if opened
}

void CDebugger::DebuggerMessage(char* message)
{
	POSITION pos;

	//Dump to all debugger windows
	for(pos=mDebuggerWindowList.GetHeadPosition();pos!=NULL;)
		((CDebuggerWin*)mDebuggerWindowList.GetNext(pos))->LineOutput(message);

	//Dump to log file if opened
}

#endif