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

// Debugger.h: interface for the Debugger class.
//
//////////////////////////////////////////////////////////////////////

#ifndef DEBUGGER_H
#define DEBUGGER_H


#define	MAX_DEBUGGER_WIN	1
#define	MAX_TRACE_WIN		1
#define MAX_DUMP_WIN		4
#define	MAX_CODE_WIN		4
#define	MAX_GRAPHICS_WIN	4

enum { DEBUG_WINUPD_NORMAL=0,DEBUG_WINUPD_FRAME, DEBUG_WINUPD_CYCLE };

class CDebugger
{
public:
	CDebugger(CSystem& mLynxSystem);
	virtual ~CDebugger();

	void Update();
	void Minimise();
	void Restore();

	void DebuggerDebugger();
	void DebuggerTrace();
	void DebuggerCode();
	void DebuggerDump();
	void DebuggerGFX();

	static void DebuggerMessage(ULONG objref,char *message);
	void DebuggerMessage(char *message);

	ULONG		mDebugWindowUpdateFreq;

private:
	CSystem		&mLynx;

	CObList		mDumpWindowList;
	CObList		mCodeWindowList;
	CObList		mTraceWindowList;
	CObList		mGraphicsWindowList;
	CObList		mDebuggerWindowList;

};

#endif
