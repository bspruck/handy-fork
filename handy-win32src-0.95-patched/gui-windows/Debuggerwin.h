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
// Lynx code debugger window header file                                    //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition and code for some the //
// debugger     window used in the debug version of the emulator            //
//                                                                          //
//    K. Wilkins                                                            //
// August 2002                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 10Mar2002 KW Document header added & class documented.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef DEBUGGERWIN_H
#define DEBUGGERWIN_H


#include <afxwin.h>
#include "../core/system.h"
#include "color.h"
#include "colorstatic.h"
#include "displine.h"
#include "dmpwndlg.h"
#include "editline.h"
#include "resource.h"

#define	MAX_LINE_BUFFERS 500
#define MAX_LINE_SIZE	 120
#define MAX_COMMAND_ARGS 20

#ifdef _LYNXDBG

class CDebuggerWin : public CFrameWnd
{
//	DECLARE_DYNCREATE(CDebuggerWin)
public:
	CDebuggerWin(CSystem& pLynx, CObList &list, CString initstr="");
	CString ArchiveStr(void);

// Operations
public:
	void LineOutput(char *text);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDebuggerWin)
protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CDebuggerWin();

	// Generated message map functions
	//{{AFX_MSG(CDebuggerWin)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEditLine();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool ExecCommand(char *linetext);

	bool CommandNotFound(int argc, char **argv);
	bool CommandQuit(int argc, char **argv);
	bool CommandRun(int argc, char **argv);
	bool CommandStop(int argc, char **argv);
	bool CommandReset(int argc, char **argv);
	bool CommandRegisters(int argc, char **argv);
	bool CommandMemSet(int argc, char **argv);
	bool CommandMemDump(int argc, char **argv);
	bool CommandMemLoad(int argc, char **argv);
	bool CommandMemSave(int argc, char **argv);
	bool CommandCartSet(int argc, char **argv);
	bool CommandCartDump(int argc, char **argv);
	bool CommandCartLoad(int argc, char **argv);
	bool CommandCartSave(int argc, char **argv);
	bool CommandStep(int argc, char **argv);
	bool CommandBpoint(int argc, char **argv);
	bool CommandScript(int argc, char **argv);
	bool CommandSearch(int argc, char **argv);
	bool CommandExecute(int argc, char **argv);
	bool CommandLog(int argc, char **argv);

	ULONG mLastAddr;
	char* Tokenize(char *input_line);
	ULONG GetLongFromString(char *instr,ULONG mask);

public:
	// Reference back to the system object we interrogate
	CSystem&	mSystem;
	CObList&	mWindowList;
	CFont		*mpFont;
	CEditLine	*mpEditLine;
	CDispLine	*mpEditDisplay;
	CColorStatic *mpEditPrompt;

	int	mCharHeight;
	int	mCharWidth;

	int mWinHeight;		// Height of window in char lines

	int	mLineInput;
	FILE *mpLogFile;

	char *mLineBuffer[MAX_LINE_BUFFERS];
};

#endif
#endif