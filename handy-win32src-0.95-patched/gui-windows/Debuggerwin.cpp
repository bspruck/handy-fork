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
// Lynx code debugger window class                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class provides a debugger    window, its only interface to the      //
// emulator is via the reference it is passed to the lynx object in the     //
// constructor. It passes on unused keypresses back to the main window.     //
//                                                                          //
//    K. Wilkins                                                            //
// March 2002                                                               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 10Mar2002 KW Document header added & class documented.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <afxwin.h>
#include <process.h>
#include "color.h"
#include "colorstatic.h"
#include "debuggerwin.h"
#include "editline.h"
#include "displine.h"
#include "widget.h"
#include "resource.h"


//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

#define DEBUGGERWIN_FONT		"Fixedsys"
#define DEBUGGERWIN_FONTPOINT	12
#define DEBUGGERWIN_HEIGHT		24
#define DEBUGGERWIN_WIDTH		36


struct command_list
{
//	bool (*function)(int argc, char **argv);
	int  command_id;
	char *command;
	char *help;
	char *usage;
};


/////////////////////////////////////////////////////////////////////////////
// CDebuggerWin

// IMPLEMENT_DYNCREATE(CDebuggerWin, CFrameWnd)

#ifdef _LYNXDBG

CDebuggerWin::CDebuggerWin(CSystem& pLynx,CObList &list, CString initstr)
	:mSystem(pLynx),
	mWindowList(list)
{
	CRect rect;
	CFont *pOldFont;
	TEXTMETRIC tm;

	// Setup the line buffers
	for(int loop=0;loop<MAX_LINE_BUFFERS;loop++)
	{
		mLineBuffer[loop]=NULL;
	}
	mLineInput=0;
	mCharHeight=0;
	mCharWidth=0;
	mWinHeight;
	mLastAddr=0;
	mpLogFile=NULL;

	// Set default vars

	if(initstr=="")
	{
//		rect=rectDefault;
		rect.top=20;
		rect.left=20;
		rect.bottom=rect.top+100;
		rect.right=rect.left+100;
	}
	else
	{
		CWidget widget(initstr);
		rect.bottom=widget.GetNext();
		rect.right=widget.GetNext();
		rect.top=widget.GetNext();
		rect.left=widget.GetNext();
	}

	// Create ourself

	CWinApp	*pLynxApp=AfxGetApp();

	Create(NULL,"Lynx Debugger",WS_OVERLAPPEDWINDOW,rect,NULL,NULL);

	// Add ourselves onto the window list

	mWindowList.AddTail(this);

	// Define font

	mpFont = new CFont;

	if(!mpFont->CreatePointFont(120,DEBUGGERWIN_FONT,NULL))
	{
		MessageBox("Font Select failed","ERROR",MB_ICONHAND);
	}

	// Load our icon

	HICON hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
	SetIcon(hIcon,TRUE);

	// Fetch the font metrics for later usage
	CPaintDC dc(this);
	pOldFont=dc.SelectObject(mpFont);
	dc.GetTextMetrics(&tm);
	mCharHeight=tm.tmHeight;
	mCharWidth=tm.tmAveCharWidth;
	dc.SelectObject(pOldFont);

	GetClientRect(&rect);
	mWinHeight=rect.Height()/mCharHeight;

	// Lets create an edit control and attach to the window
	CRect disp_rect, cmd_rect, static_rect;

	GetClientRect(&rect);
	disp_rect=rect;
	disp_rect.bottom=disp_rect.top+((mWinHeight-1)*mCharHeight);

	cmd_rect=disp_rect;
	cmd_rect.top=cmd_rect.bottom;
	cmd_rect.bottom=cmd_rect.top+(mCharHeight*1);
	cmd_rect.left+=mCharWidth;

	static_rect=cmd_rect;
	static_rect.left-=mCharWidth;
	static_rect.right=static_rect.left+mCharWidth;

	mpEditLine = new CEditLine();
	mpEditLine->Create(WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,cmd_rect,this,IDC_EDIT_DBG2);
	mpEditLine->SetFont(mpFont);
	mpEditLine->SetBkColor(WHITE);
	mpEditLine->SetTextColor(BLACK);

	mpEditDisplay = new CDispLine();
	mpEditDisplay->Create(WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_READONLY|ES_MULTILINE|ES_AUTOVSCROLL|ES_AUTOHSCROLL ,disp_rect,this,IDC_EDIT_DBG1);
	mpEditDisplay->SetFont(mpFont);
	mpEditDisplay->SetBkColor(WHITE);
	mpEditDisplay->SetTextColor(BLACK);

	mpEditPrompt = new CColorStatic();
	mpEditPrompt->Create(">",WS_CHILD|WS_VISIBLE,static_rect,this,IDC_EDIT_DBG3);
	mpEditPrompt->SetFont(mpFont);
	mpEditPrompt->SetBkColor(WHITE);
	mpEditPrompt->SetTextColor(BLACK);

	// Fill the editbox lines to the bottom so when the first command
	// comes in it is in the right place
	for(int loop=0;loop<mWinHeight;loop++) LineOutput("");

	// Send ourselves a welcome message
	LineOutput("HANDY Lynx Emulator - Command Line Debugger");
	LineOutput("(Type \"Help\" for command list)");
	LineOutput("");

	mpEditLine->SetFocus();
}


CDebuggerWin::~CDebuggerWin()
{
}

CString CDebuggerWin::ArchiveStr(void)
{
	CRect rect;
	GetWindowRect(&rect);

	CString archive;
	//              Left:Top-:Righ:Botm
	archive.Format("%08x:%08x:%08x:%08x",rect.left,rect.top,rect.right,rect.bottom);
	return archive;
}

BEGIN_MESSAGE_MAP(CDebuggerWin, CFrameWnd)
	//{{AFX_MSG_MAP(CDebuggerWin)
	ON_WM_SIZE()
	ON_COMMAND(IDM_EDITLINE_INPUTWAITING,OnEditLine)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDumpWin message handlers

void CDebuggerWin::PostNcDestroy() 
{
	if(mpLogFile)
	{
		fclose(mpLogFile);
		mpLogFile=NULL;
	}
	mWindowList.RemoveAt(mWindowList.Find(this));
	delete mpFont;
	delete mpEditLine;
	delete mpEditDisplay;
	delete mpEditPrompt;

	for(int loop=0;loop<mLineInput;loop++)
	{
		delete mLineBuffer[loop];
	}

	delete this;
}

void CDebuggerWin::OnSize(UINT nType, int cx, int cy) 
{
	CRect rect;

	// Work out the new window height
	mWinHeight=cy/mCharHeight;

	// Make size a modular of mCharHeight
	cy=mWinHeight*mCharHeight;

	rect.top=0;rect.bottom=cy;
	rect.left=0;rect.right=cx;
	rect.bottom=rect.top+((mWinHeight-1)*mCharHeight);
	mpEditDisplay->MoveWindow(rect.left,rect.top,rect.Width(),rect.Height());

	rect.top=rect.bottom+1;
	rect.bottom=rect.top+(mCharHeight*1)+1;
	rect.left+=mCharWidth;
	mpEditLine->MoveWindow(rect.left,rect.top,rect.Width(),rect.Height());

	rect.left-=mCharWidth;
	rect.right=rect.left+mCharWidth;
	mpEditPrompt->MoveWindow(rect.left,rect.top,rect.Width(),rect.Height());

	// Call the base function
	CFrameWnd::OnSize(nType, cx, cy);

	// Redraw the main window
	Invalidate(FALSE);
}

void CDebuggerWin::OnEditLine(void) 
{
	int length;
	char linetext[MAX_LINE_SIZE+1];

	// Copy the line to the output window
	length=mpEditLine->GetLine(0,linetext,MAX_LINE_SIZE);
	// Terminate the unterminated string
	linetext[length]=0;

	// Execute the command
	ExecCommand(linetext);
}


bool CDebuggerWin::ExecCommand(char *linetext) 
{
	char emptystring[]=" ";
	char tmptext[MAX_LINE_SIZE+1];
	char *arglist[MAX_COMMAND_ARGS+1];
	int argcount=0,loop=0;
	bool cmd_ok=TRUE;

	enum {
		COMMAND_QUIT,
		COMMAND_RUN,
		COMMAND_STOP,
		COMMAND_STEP,
		COMMAND_BPOINT,
		COMMAND_BPOINTC,
		COMMAND_RESET,
		COMMAND_REGISTERS,
		COMMAND_MEM_SET,
		COMMAND_MEM_DUMP,
		COMMAND_MEM_LOAD,
		COMMAND_MEM_SAVE,
		COMMAND_CART_SET,
		COMMAND_CART_DUMP,
		COMMAND_CART_LOAD,
		COMMAND_CART_SAVE,
		COMMAND_HELP,
		COMMAND_SCRIPT,
		COMMAND_SEARCH,
		COMMAND_REM,
		COMMAND_EXECUTE,
		COMMAND_LOG,
		COMMAND_UNKNOWN
	};

	const struct command_list commands[]=
	{
		{ COMMAND_QUIT,      "QUIT"  ,"QUIT                           - Immediate exit from the emulator back to windows"        ,"" },
		{ COMMAND_RUN,       "RUN"   ,"RUN [xxxx]                     - Run from xxxx (optional)"                                ,"Usage: RUN [addr]" },
		{ COMMAND_STOP,      "STOP"  ,"STOP                           - Halt CPU"                                                ,"" },
		{ COMMAND_STEP,	     "STEP"  ,"STEP [xxxx]                    - Single Step the CPU from addr xxxx (optional)"           ,"" },
		{ COMMAND_SCRIPT,    "SCRIPT","SCRIPT <file>                  - Run a script file"                                       ,"Usage: SCRIPT <file>" },
		{ COMMAND_BPOINT,    "BP"    ,"BP [n] [xxxx]/CLR              - Set/List/Clear breakpoint n to xxxx"                     ,"Usage: BP [n] [xxxx]/CLR" },
		{ COMMAND_RESET,     "RESET" ,"RESET                          - Reset CPU, also halts"                                   ,"" },
		{ COMMAND_REGISTERS, "REG"   ,"REG [PC/SP/PS/X/Y/A] [Value]   - Display/Set registers"                                   ,"Usage: REG [PC/SP/PS/X/Y/A] [Value]" },

		{ COMMAND_MEM_SET,   "MS"    ,"MS <xxxx> <aa> [bb]..[ff]      - Set RAM xxxx to aa.bb,cc,etc"                            ,"Usage: MS <xxxx> <aa> [bb]..[ff]" },
		{ COMMAND_MEM_DUMP,  "MD"    ,"MD [xxxx] [yyyy]               - Dump RAM from xxxx to yyyy"                              ,"Usage: MD <xxxx> [yyyy]" },
		{ COMMAND_MEM_LOAD,  "MLOAD" ,"MLOAD <xxxx> <file>            - Load RAM from file starting at xxxx"                     ,"Usage: MLOAD <xxxx> <file>" },
		{ COMMAND_MEM_SAVE,  "MSAVE" ,"MSAVE <xxxx> <yyyy> <file>     - Save RAM to file from xxxx-yyyy"                         ,"Usage: MSAVE <xxxx> <yyyy> <file>" },
		{ COMMAND_SEARCH,    "MSRCH" ,"MSRCH <xxxx> <yyyy> \"param\"    - Search RAM for string"                                 ,"Usage: MSRCH <xxxx> <yyyy> \"param\"\\<aa bb cc ...>" },
		{ COMMAND_SEARCH,    "MSRCH" ,"MSRCH <xxxx> <yyyy> <aa bb ..> - Search RAM for bytes"                                    ,"Usage: MSRCH <xxxx> <yyyy> \"param\"\\<aa bb cc ...>" },

		{ COMMAND_CART_SET,  "CS"    ,"CS <xxxx> <aa> [bb]..[ff]      - Set cartridge memory xxxx to aa.bb,cc,etc"               ,"Usage: CS <xxxx> <aa> [bb]..[ff]" },
		{ COMMAND_CART_DUMP, "CD"    ,"CD [xxxxxx] [yyyyyy]           - Dump cartridge memory from xxxxxx to yyyyyy"             ,"Usage: CD <xxxxxx> [yyyyyy]" },
		{ COMMAND_CART_LOAD, "CLOAD" ,"CLOAD <xxxxxx> <file>          - Load cartridge from file starting at xxxxxx"             ,"Usage: CLOAD <xxxxxx> <file>" },
		{ COMMAND_CART_SAVE, "CSAVE" ,"CSAVE <xxxxxx> <yyyyyy> <file> - Save cartridge to file from xxxxxx-yyyyyy"               ,"Usage: CSAVE <xxxxxx> <yyyyyy> <file>" },
		{ COMMAND_SEARCH,    "CSRCH" ,"CSRCH <xxxx> <yyyy> \"param\"    - Search cartridge for string"                           ,"Usage: CSRCH <xxxx> <yyyy> \"param\"\\<aa bb cc ...>" },
		{ COMMAND_SEARCH,    "CSRCH" ,"CSRCH <xxxx> <yyyy> <aa bb ..> - Search cartridge for bytes"                              ,"Usage: CSRCH <xxxx> <yyyy> \"param\"\\<aa bb cc ...>" },

		{ COMMAND_REM,       "REM"   ,"REM - Some comments            - Script Remark/Comments, will be ignored "                ,"" },
		{ COMMAND_EXECUTE,   "EXEC"  ,"EXEC ....                      - Execute/Spawn external command"                          ,"Usage: EXEC (args....)" },
		{ COMMAND_LOG,       "LOG"   ,"LOG [filename] ....            - Log all output to a file"                                ,"Usage: LOG [filename]" },
		{ COMMAND_HELP,      "HELP"  ,"HELP                           - Prints this text"                                        ,"" },
		{ COMMAND_UNKNOWN,   ""      ,""                                                                                         ,"" }
	};

	// Dump back to the edit window
	strcpy(tmptext,">");
	strncat(tmptext,linetext,MAX_LINE_SIZE-1);
	tmptext[MAX_LINE_SIZE]=0;
	LineOutput(tmptext);

	// Splat remaining arg array to null
	for(loop=0;loop<MAX_COMMAND_ARGS;loop++) arglist[loop]=&emptystring[0];
	arglist[MAX_COMMAND_ARGS]=NULL;

	loop=0;
	// Put all to upper case
	while(linetext[loop]!=0x00)
	{
		linetext[loop]=toupper(linetext[loop]);
		loop++;
	}

	if((arglist[0]=Tokenize(linetext))!=NULL)
	{
		argcount++;
		while(argcount<MAX_COMMAND_ARGS)
		{
			if((arglist[argcount]=Tokenize(NULL))==NULL)
			{
				// Fix up the null string on the end
				arglist[argcount]=emptystring;
				break;
			}
			argcount++;
		}
	}

	// Process into argc/argv
	if(argcount)
	{
		loop=0;
		for(;;)
		{
			// Check for the end of the list
			if(strcmp(commands[loop].command,"")==0) break;
			// Check our command
			if(strcmp(commands[loop].command,arglist[0])==0) break;
			loop++;
		}


		switch(commands[loop].command_id)
		{
			case COMMAND_REM:
				cmd_ok=TRUE;
				break;
			case COMMAND_QUIT:
				cmd_ok=CommandQuit(argcount,arglist);
				break;
			case COMMAND_RUN:
				cmd_ok=CommandRun(argcount,arglist);
				break;
			case COMMAND_STOP:
				cmd_ok=CommandStop(argcount,arglist);
				break;
			case COMMAND_STEP:
				cmd_ok=CommandStep(argcount,arglist);
				break;
			case COMMAND_BPOINT:
				cmd_ok=CommandBpoint(argcount,arglist);
				break;
			case COMMAND_SCRIPT:
				cmd_ok=CommandScript(argcount,arglist);
				break;
			case COMMAND_SEARCH:
				cmd_ok=CommandSearch(argcount,arglist);
				break;
			case COMMAND_RESET:
				cmd_ok=CommandReset(argcount,arglist);
				break;
			case COMMAND_REGISTERS:
				cmd_ok=CommandRegisters(argcount,arglist);
				break;
			case COMMAND_MEM_SET:
				cmd_ok=CommandMemSet(argcount,arglist);
				break;
			case COMMAND_MEM_DUMP:
				cmd_ok=CommandMemDump(argcount,arglist);
				break;
			case COMMAND_MEM_LOAD:
				cmd_ok=CommandMemLoad(argcount,arglist);
				break;
			case COMMAND_MEM_SAVE:
				cmd_ok=CommandMemSave(argcount,arglist);
				break;
			case COMMAND_CART_SET:
				cmd_ok=CommandCartSet(argcount,arglist);
				break;
			case COMMAND_CART_DUMP:
				cmd_ok=CommandCartDump(argcount,arglist);
				break;
			case COMMAND_CART_LOAD:
				cmd_ok=CommandCartLoad(argcount,arglist);
				break;
			case COMMAND_CART_SAVE:
				cmd_ok=CommandCartSave(argcount,arglist);
				break;
			case COMMAND_EXECUTE:
				cmd_ok=CommandExecute(argcount,arglist);
				break;
			case COMMAND_LOG:
				cmd_ok=CommandLog(argcount,arglist);
				break;
			case COMMAND_HELP:
				{
					int loop=0;
					LineOutput("Supported Command Line Functions are:");
					do
					{
						if(strcmp(commands[loop].help,"")!=0) LineOutput(commands[loop].help);
						loop++;
					}
					while(commands[loop].command_id!=COMMAND_UNKNOWN);
					LineOutput("  ( <xx> - Mandatory agrument, [xx] - Optional argument )");
					LineOutput("");
					cmd_ok=TRUE;
				}
				break;
			default:
			case COMMAND_UNKNOWN:
				cmd_ok=CommandNotFound(argcount,arglist);
				break;
		}

		// Redraw all debug windows incase global data change
		AfxGetMainWnd()->PostMessage(WM_COMMAND,IDM_DRAW_DEBUG_WINDOWS);

		if(cmd_ok)	
		{
			// Clear the command line
			mpEditLine->SetWindowText("");
		}
		else
		{
			// Provide usage text if available
			if(strcmp(commands[loop].help,"")!=0) LineOutput(commands[loop].usage);
		}
	}
	return cmd_ok;
}


void CDebuggerWin::LineOutput(char *text)
{
	char wintext[MAX_LINE_BUFFERS*(MAX_LINE_SIZE+1)];
	char winline[MAX_LINE_SIZE+1];

	if(mpLogFile)
	{
		fprintf(mpLogFile,"%s\n",text);
	}

	if(mLineInput>=MAX_LINE_BUFFERS)
	{
		// Scroll all the buffers up one
		char *tmpline=mLineBuffer[0];
		for(int loop=0;loop<MAX_LINE_BUFFERS-1;loop++)
		{
			mLineBuffer[loop]=mLineBuffer[loop+1];
		}
		mLineBuffer[MAX_LINE_BUFFERS-1]=tmpline;
		strncpy(mLineBuffer[MAX_LINE_BUFFERS-1],text,MAX_LINE_SIZE);
		mLineBuffer[MAX_LINE_BUFFERS-1][MAX_LINE_SIZE]=0;
	}
	else
	{
		char *newbuf;
		newbuf = new char[MAX_LINE_SIZE+1];
		strncpy(newbuf,text,MAX_LINE_SIZE);
		newbuf[MAX_LINE_SIZE]=0;
		mLineBuffer[mLineInput++]=newbuf;
	}


	// Dump the line buffers into the window text

	wintext[0]=0;
    int loop;
	for(loop=0;loop<mLineInput-1;loop++)
	{
		sprintf(winline,"%s%c%c",mLineBuffer[loop],0x0d,0x0a);
		strcat(wintext,winline);
	}
	sprintf(winline,"%s",mLineBuffer[loop],0x0d,0x0a);
	strcat(wintext,winline);

	mpEditDisplay->SetWindowText(wintext);

	// Set the display window to show the last line entered
//	mpEditDisplay->LineScroll(MAX_LINE_BUFFERS-mWinHeight)+1);
	mpEditDisplay->LineScroll(MAX_LINE_BUFFERS);
}



bool CDebuggerWin::CommandNotFound(int argc, char **argv)
{
	LineOutput("Command not recognised.");
	return false;
}

bool CDebuggerWin::CommandLog(int argc, char **argv)
{
	if(argc!=1 && argc!=2)
	{
		LineOutput("Invalid number of arguments");
		return false;
	}

	if(argc==1)
	{
		// Close logging
		LineOutput("Logging Stopped.");
		if(mpLogFile) fclose(mpLogFile);
		mpLogFile=NULL;
	}
	else
	{
		// Start logging
		if((mpLogFile=fopen(argv[1],"wt"))==NULL)
		{
			LineOutput("Error Creating Log file");
			return true;
		}
		LineOutput("Logging Started.");
	}
	return true;
}

bool CDebuggerWin::CommandStep(int argc, char **argv)
{
	if(argc>1)
	{
		ULONG newpc;
		if((newpc=GetLongFromString(argv[1],0xffff))!=0xffffffff)
		{
			C6502_REGS regs;
			mSystem.GetRegs(regs);
			regs.PC=(UWORD)newpc;
			mSystem.SetRegs(regs);

			gSystemHalt=FALSE;		// Make sure we run always
			gSingleStepMode=TRUE;	// Go for it....
		}
		else
		{
			LineOutput("Invalid PC, must be in range 0-ffff");
			return false;
		}
	}
	else
	{
		gSystemHalt=FALSE;		// Make sure we run always
		gSingleStepMode=TRUE;	// Go for it....
	}
	return true;
}

bool CDebuggerWin::CommandBpoint(int argc, char **argv)
{
	char tmptext[MAX_LINE_SIZE+1];
	switch(argc)
	{
		case 1:		// List all BPs
			{
				C6502_REGS regs;
				mSystem.GetRegs(regs);
				for(int loop=0;loop<MAX_CPU_BREAKPOINTS;loop++)
				{
					if(regs.cpuBreakpoints[loop]>0xffff)
					{
						sprintf(tmptext,"BP%02x - Unset",loop);
						LineOutput(tmptext);
					}
					else
					{
						sprintf(tmptext,"BP%02x - $%04x",loop,regs.cpuBreakpoints[loop]);
						LineOutput(tmptext);
					}
				}
			}
			break;
		case 2:		// List individual BP
			{
				int bpn;
				C6502_REGS regs;
				mSystem.GetRegs(regs);
				if((bpn=GetLongFromString(argv[1],0xff))!=0xffffffff && bpn<MAX_CPU_BREAKPOINTS)
				{
					if(regs.cpuBreakpoints[bpn]>0xffff)
					{
						sprintf(tmptext,"BP%02x - Unset",bpn);
						LineOutput(tmptext);
					}
					else
					{
						sprintf(tmptext,"BP%02x - $%04x",bpn,regs.cpuBreakpoints[bpn]);
						LineOutput(tmptext);
					}
				}
				else
				{
					LineOutput("Invalid breakpoint number");
					return false;
				}
			}
			break;
		case 3:		// Set individual BP
			{
				int bpn,bpa;
				C6502_REGS regs;
				mSystem.GetRegs(regs);
				if((bpn=GetLongFromString(argv[1],0xff))!=0xffffffff && bpn<MAX_CPU_BREAKPOINTS)
				{
					if((bpa=GetLongFromString(argv[2],0xffff))!=0xffffffff)
					{
						regs.cpuBreakpoints[bpn]=bpa;
						mSystem.SetRegs(regs);
					}
					else if(strcmp(argv[2],"CLEAR")==0 ||strcmp(argv[2],"CLR")==0)
					{
						regs.cpuBreakpoints[bpn]=0xffffff;
						mSystem.SetRegs(regs);
					}
					else
					{
						LineOutput("Invalid breakpoint address");
						return false;
					}
				}
				else
				{
					LineOutput("Invalid breakpoint number");
					return false;
				}
			}
			break;
		default:
			break;
	}
	return true;
}

bool CDebuggerWin::CommandScript(int argc, char **argv)
{
	UBYTE data;
	FILE *fp;

	if(argc!=2)
	{
		LineOutput("Invalid number of arguments");
		return false;
	}
	else
	{
		if((fp=fopen(argv[1],"rt"))!=NULL)
		{
			while(1)
			{
				char inbuf[MAX_LINE_SIZE+1];
				int counter;
				bool endoffile;

				endoffile=FALSE;
				counter=0;
				inbuf[counter]=0;

				// Read line and exec until either script error or EOF
				while(1)
				{
					if(fread(&data,1,1,fp)!=1)
					{
						endoffile=TRUE;
						break;
					}
					else
					{
						if(data!='\n' && counter<MAX_LINE_SIZE) inbuf[counter++]=data; else break;
					}
				}
				// Correctly terminate the line
				inbuf[counter]=0;

				// Exec the line, stop on an error
				if(ExecCommand(inbuf)==FALSE) break;
				if(endoffile==TRUE) break;
			}
			fclose(fp);
		}
		else
		{
			LineOutput("Could not open file");
			return false;
		}
	}
	return true;
}

bool CDebuggerWin::CommandSearch(int argc, char **argv)
{
#define MAX_SEARCH_LENGTH	8
#define MAX_SEARCH_MATCHES	20
	bool rambased=false;
	bool flag=false;
	bool string=false;
	ULONG	addr_start,addr_end,addr,loop,tmp;
	ULONG	search_data[MAX_SEARCH_LENGTH];
	ULONG	search_length=0;
	ULONG	search_matches=0;

	// First thing lets check for min arg size
	if(argc<4 || argc>(3+MAX_SEARCH_LENGTH))
	{
		LineOutput("Invalid number of arguments");
		return false;
	}
	else
	{
		// Check the 1st letter of ARG(0) to see if we are MEM or CART
		if(argv[0][0]=='M') rambased=true;

		// Fetch the start & end
		if(rambased)
		{
			addr_start=GetLongFromString(argv[1],0xffff);
			addr_end=GetLongFromString(argv[2],0xffff);
		}
		else
		{
			addr_start=GetLongFromString(argv[1],0xffffff);
			addr_end=GetLongFromString(argv[2],0xffffff);
		}

		if(addr_start==0xffffffff || addr_end==0xffffffff)
		{
			LineOutput("Invalid/Out of range start/end address argument");
			return false;
		}

		if(addr_start>addr_end)
		{
			tmp=addr_start;
			addr_start=addr_end;
			addr_end=tmp;
		}

		// Now check for search type and convert everything to array of LONG for search
		if(GetLongFromString(argv[3],0xff)==0xffffffff)
		{
			// Couldnt decode the first byte so we'll assume its a string
			string=true;
		
			search_length=strlen(argv[3]);
			for(loop=0;loop<search_length;loop++)
			{
				search_data[loop]=(ULONG)argv[3][loop];
			}
		}
		else
		{
			search_length=argc-3;
			for(loop=0;loop<(ULONG)argc-3;loop++)
			{
				// Decode each byte into the search array
				tmp=GetLongFromString(argv[3+loop],0xff);
				if(tmp==0xffffffff)
				{
					LineOutput("Invalid byte in search data");
					return false;
				}
				else
				{
					search_data[loop]=tmp;
				}
			}
		}

		// Now we can actually do the search
		for(addr=addr_start;addr<=addr_end;addr++)
		{
			flag=true;
			// Sub loop for local array search
			for(loop=0;loop<search_length;loop++)
			{
				// Read the data
				if(rambased) tmp=mSystem.Peek_RAM(addr+loop); else tmp=mSystem.Peek_CART(addr+loop);
				// Check for NOT match, string check is different
				if(string)
				{
					if((ULONG)toupper(tmp)!=search_data[loop] && tmp!=search_data[loop])
					{
						flag=false;
						break;
					}
				}
				else
				{
					if(tmp!=search_data[loop])
					{
						flag=false;
						break;
					}
				}
			}

			if(flag)
			{
				char tmptext[MAX_LINE_SIZE+1];

				//We found something
				sprintf(tmptext,"Match at 0x%04x",addr);
				LineOutput(tmptext);

				search_matches++;
				if(search_matches>MAX_SEARCH_MATCHES)
				{
					LineOutput("Too many matches, aborting");
					break;
				}
			}
		}
	}
	return true;
}

bool CDebuggerWin::CommandMemSet(int argc, char **argv)
{
	ULONG addr,data;
	int loop;

	if(argc<3)
	{
		LineOutput("Invalid number of arguments");
		return false;
	}
	else
	{
		if((addr=GetLongFromString(argv[1],0xffff))!=0xffffffff)
		{
			for(loop=2;loop<argc;loop++)
			{
				if((data=GetLongFromString(argv[loop],0xff))!=0xffffffff)
				{
					mSystem.Poke_CPU(addr++,(UBYTE)data);
				}
				else
				{
					LineOutput("Invalid data argument must be in range 0-ff");
					return false;
				}
			}
		}
		else
		{
			LineOutput("Invalid address argument must be in range 0-ffff");
			return false;
		}
	}

	return true;
}


bool CDebuggerWin::CommandMemDump(int argc, char **argv)
{
	ULONG addr,count,tmp;
	ULONG loop,loop2;

	if(argc!=1 && argc!=2 && argc!=3)
	{
		LineOutput("Invalid number of arguments");
		return false;
	}
	else
	{
		if(argc>1) addr=GetLongFromString(argv[1],0xffff); else addr=mLastAddr;
		if(argc==3) tmp=GetLongFromString(argv[2],0xffff); else tmp=addr+64;

		if(addr==0xffffffff || tmp==0xffffffff)
		{
			LineOutput("Invalid address argument must be in range 0-ffff");
			return false;
		}
		else
		{
			// Number of 16 byte lines
			count=((tmp-addr)&0xffff)>>4;
			addr&=0xffff;

			for(loop=0;loop<count;loop++)
			{
				char linetext[MAX_LINE_SIZE+1];
				sprintf(linetext,"%04x ",addr);

				for(loop2=0;loop2<16;loop2++)
				{
					char cliptext[MAX_LINE_SIZE+1];
					sprintf(cliptext," %02x",mSystem.Peek_RAM(addr+loop2));
					strcat(linetext,cliptext);
				}
				addr+=16;
				LineOutput(linetext);
			}
			mLastAddr=addr;
		}
	}

	return true;
}

bool CDebuggerWin::CommandMemLoad(int argc, char **argv)
{
	ULONG addr;
	UBYTE data;
	FILE *fp;

	if(argc!=3)
	{
		LineOutput("Invalid number of arguments");
		return false;
	}
	else
	{
		if((addr=GetLongFromString(argv[1],0xffff))!=0xffffffff)
		{
			if((fp=fopen(argv[2],"rb"))!=NULL)
			{
				// Read bytes until the end
				while(fread(&data,1,1,fp)==1 && addr<=0xffff)
				{
					mSystem.Poke_RAM(addr++,(UBYTE)data);
				}
				fclose(fp);
			}
			else
			{
				LineOutput("Could not open file");
				return false;
			}
		}
		else
		{
			LineOutput("Invalid address must be in range 0-ffff");
			return false;
		}
	}
	return true;
}

bool CDebuggerWin::CommandMemSave(int argc, char **argv)
{
	ULONG saddr,eaddr,addr;
	UBYTE data;
	FILE *fp;

	if(argc!=4)
	{
		LineOutput("Invalid number of arguments");
		return false;
	}
	else
	{
		if((saddr=GetLongFromString(argv[1],0xffff))!=0xffffffff && (eaddr=GetLongFromString(argv[2],0xffff))!=0xffffffff)
		{
			if((fp=fopen(argv[3],"wb"))!=NULL)
			{
				// Write loop
				for(addr=saddr;addr<=eaddr;addr++)
				{
					data=mSystem.Peek_RAM(addr);
					fwrite(&data,1,1,fp);
				}
				fclose(fp);
			}
			else
			{
				LineOutput("Could not open file");
				return false;
			}
		}
		else
		{
			LineOutput("Invalid address must be in range 0-ffff");
			return false;
		}
	}

	return true;
}

bool CDebuggerWin::CommandCartSet(int argc, char **argv)
{
	ULONG addr,data;
	int loop;

	if(argc<3)
	{
		LineOutput("Invalid number of arguments");
	}
	else
	{
		if((addr=GetLongFromString(argv[1],0xffffff))!=0xffffffff)
		{
			mSystem.mCart->BankSelect(bank0);
			mSystem.mCart->mWriteEnableBank0=TRUE;
			for(loop=2;loop<argc;loop++)
			{
				if((data=GetLongFromString(argv[loop],0xff))!=0xffffffff)
				{
					mSystem.Poke_CART(addr++,(UBYTE)data);
				}
				else
				{
					LineOutput("Invalid data argument must be hex bytes");
					return false;
				}
			}
			mSystem.mCart->mWriteEnableBank0=FALSE;
		}
		else
		{
			LineOutput("Invalid address argument must be in range 0-ffffff");
			return false;
		}
	}

	return true;
}

bool CDebuggerWin::CommandCartDump(int argc, char **argv)
{
	ULONG addr,count,tmp;
	ULONG loop,loop2;

	if(argc!=1 && argc!=2 && argc!=3)
	{
		LineOutput("Invalid number of arguments");
		return false;
	}
	else
	{
		if(argc>1) addr=GetLongFromString(argv[1],0xffffff); else addr=mLastAddr;
		if(argc==3) tmp=GetLongFromString(argv[2],0xffffff); else tmp=addr+64;

		if(addr==0xffffffff || tmp==0xffffffff)
		{
			LineOutput("Invalid address argument must be in range 0-ffffff");
			return false;
		}
		else
		{
			// Number of 16 byte lines
			count=((tmp-addr)&0xfffff)>>4;

			mSystem.mCart->BankSelect(bank0);

			for(loop=0;loop<count;loop++)
			{
				char linetext[MAX_LINE_SIZE+1];
				int page,offset;
				page=addr/(mSystem.CartSize()>>8);
				offset=addr%(mSystem.CartSize()>>8);
				sprintf(linetext,"%02x:%03x ",page,offset);

				for(loop2=0;loop2<16;loop2++)
				{
					char cliptext[MAX_LINE_SIZE+1];
					sprintf(cliptext," %02x",mSystem.Peek_CART(addr+loop2));
					strcat(linetext,cliptext);
				}
				addr+=16;
				LineOutput(linetext);
			}
			mLastAddr=addr;
		}
	}
	return true;
}

bool CDebuggerWin::CommandCartLoad(int argc, char **argv)
{
	ULONG addr;
	UBYTE data;
	FILE *fp;

	if(argc!=3)
	{
		LineOutput("Invalid number of arguments");
		return false;
	}
	else
	{
		if((addr=GetLongFromString(argv[1],0xffffff))!=0xffffffff)
		{
			if((fp=fopen(argv[2],"rb"))!=NULL)
			{
				mSystem.mCart->BankSelect(bank0);
				mSystem.mCart->mWriteEnableBank0=TRUE;
				// Read bytes until the end
				while(fread(&data,1,1,fp)==1 && addr<=0xffffff)
				{
					mSystem.Poke_CART(addr++,(UBYTE)data);
				}
				fclose(fp);
				mSystem.mCart->mWriteEnableBank0=FALSE;
			}
			else
			{
				LineOutput("Could not open file");
				return false;
			}
		}
		else
		{
			LineOutput("Invalid address must be in range 0-ffffff");
			return false;
		}
	}
	return true;
}

bool CDebuggerWin::CommandCartSave(int argc, char **argv)
{
	ULONG saddr,eaddr,addr;
	UBYTE data;
	FILE *fp;

	if(argc!=4)
	{
		LineOutput("Invalid number of arguments");
		return false;
	}
	else
	{
		if((saddr=GetLongFromString(argv[1],0xffffff))!=0xffffffff && (eaddr=GetLongFromString(argv[2],0xffffff))!=0xffffffff)
		{
			if((fp=fopen(argv[3],"wb"))!=NULL)
			{
				// Write loop
				for(addr=saddr;addr<=eaddr;addr++)
				{
					data=mSystem.Peek_CART(addr);
					fwrite(&data,1,1,fp);
				}
				fclose(fp);
			}
			else
			{
				LineOutput("Could not open file");
				return false;
			}
		}
		else
		{
			LineOutput("Invalid address must be in range 0-ffff");
			return false;
		}
	}

	return true;
}

bool CDebuggerWin::CommandQuit(int argc, char **argv)
{
	AfxGetMainWnd()->PostMessage(WM_CLOSE);
	return true;
}

bool CDebuggerWin::CommandRun(int argc, char **argv)
{
	if(argc>1)
	{
		ULONG newpc;
		if((newpc=GetLongFromString(argv[1],0xffff))!=0xffffffff)
		{
			C6502_REGS regs;
			mSystem.GetRegs(regs);
			regs.PC=(UWORD)newpc;
			mSystem.SetRegs(regs);

			gSystemHalt=FALSE;		// Make sure we run always
			gSingleStepMode=FALSE;	// Go for it....
		}
		else
		{
			LineOutput("Invalid PC given, must be in range 0-ffff");
			return false;
		}
	}
	else
	{
		gSystemHalt=FALSE;		// Make sure we run always
		gSingleStepMode=FALSE;	// Go for it....
	}
	return true;
}

bool CDebuggerWin::CommandStop(int argc, char **argv)
{
	gBreakpointHit=TRUE;	// This will force a window redraw in the debug version
	gSystemHalt=TRUE;		// This will stop the system
	return true;
}

bool CDebuggerWin::CommandReset(int argc, char **argv)
{
	mSystem.Reset();
	return true;
}

bool CDebuggerWin::CommandExecute(int argc, char **argv)
{
	if(argc<2)
	{
		LineOutput("Invalid number of arguments");
		return false;
	}
	else
	{
		spawnvp(_P_WAIT,argv[1],&argv[1]);
	}
	return true;
}

bool CDebuggerWin::CommandRegisters(int argc, char **argv)
{
	C6502_REGS regs;
	ULONG newreg;

	if(argc!=3 && argc!=1)
	{
		LineOutput("Invalid number of arguments");
		return false;
	}
	else if(argc==1)
	{
		char linetext[MAX_LINE_SIZE+1];
		mSystem.GetRegs(regs);
		sprintf(linetext,"PC=$%04x SP=$%02x PS=0x%02x A=0x%02x X=0x%02x Y=0x%02x",regs.PC,regs.SP, regs.PS,regs.A,regs.X,regs.Y);
		LineOutput(linetext);
	}
	else
	{
		mSystem.GetRegs(regs);
		if(strcmp(argv[1],"PC")==0 && (newreg=GetLongFromString(argv[2],0xffff))!=0xffffffff)
		{
			regs.PC=(UWORD)newreg;
		}
		else if(strcmp(argv[1],"SP")==0 && (newreg=GetLongFromString(argv[2],0xff))!=0xffffffff)
		{
			regs.SP=(UWORD)newreg;
		}
		else if(strcmp(argv[1],"PS")==0 && (newreg=GetLongFromString(argv[2],0xff))!=0xffffffff)
		{
			regs.PS=(UWORD)newreg;
		}
		else if(strcmp(argv[1],"X")==0 && (newreg=GetLongFromString(argv[2],0xff))!=0xffffffff)
		{
			regs.X=(UWORD)newreg;
		}
		else if(strcmp(argv[1],"Y")==0 && (newreg=GetLongFromString(argv[2],0xff))!=0xffffffff)
		{
			regs.Y=(UWORD)newreg;
		}
		else if(strcmp(argv[1],"A")==0 && (newreg=GetLongFromString(argv[2],0xff))!=0xffffffff)
		{
			regs.A=(UWORD)newreg;
		}
		else
		{
			LineOutput("Invalid register name or hex value");
			return false;
		}
		mSystem.SetRegs(regs);
	}
	return true;
}

//
// Convert string to tokens
//
char* CDebuggerWin::Tokenize(char *input_line)
{
	static char *workline=NULL;
	static int count=0;
	int quote=0,token_start=0,token_end=0;
	char *retval=NULL;

	if(input_line!=NULL)
	{
		workline=input_line;
		count=0;
	}

	// Exit if there is no line to process
	if(workline==NULL) return NULL;
	if(workline[count]=='\0') return NULL;

	// Munge out the first token

	while(!token_end)
	{
		switch(workline[count])
		{
			case 0x00:
				token_end=1;
				break;
			case ' ':
			case '\t':
			case '\n':
				if(token_start && !quote)
				{
					token_end=1;
					workline[count]=0;
				}
				count++;
				break;
			case '"':
				if(token_start) token_end=1;
				workline[count]=0;
				quote=(quote)?0:1;
				count++;
				break;
			default:
				if(!token_start)
				{
					retval=&workline[count];
					token_start=1;
				}
				count++;
				break;
		}
	}
	return retval;
}

ULONG CharToLong(char text)
{
	int loop=0;
	char convert[18]={"0123456789ABCDEFX"};
	while(convert[loop]!='X')
	{
		if(convert[loop]==toupper(text)) break;
		loop++;
	}
	return loop;
}

ULONG CDebuggerWin::GetLongFromString(char *instr,ULONG mask)
{
	char convert[18]={"0123456789ABCDEFX"};
	ULONG retval,tmp,radix;
	int loop=0,count=0;

	// Check max length
	count=strlen(instr);
	if(count>6) return 0xffffffff;

	radix=1;
	retval=0;
	// Process in reverse
	for(loop=count-1;loop>=0;loop--)
	{
		tmp=CharToLong(instr[loop]);
		if(tmp==0x10) return 0xffffffff;
		retval+=tmp*radix;
		radix*=16;
	}

	if(retval>mask) return 0xffffffff;

	return retval;
}


#endif

