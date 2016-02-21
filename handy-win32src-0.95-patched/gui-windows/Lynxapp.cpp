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
// Lynx application class                                                   //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// The bare minimum needed to make a viable windows program. It creates a   //
// window object which is then constantly updated by the OnIdle handler     //
// which is in the header file for now real reason other than I cant be     //
// bothered to move it back in here.                                        //
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

#include <afxdisp.h>
#include "lynxwin.h"
#include "lynxapp.h"
#include "../core/error.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

CLynxApplication LynxEmulator;

BOOL CLynxApplication::InitInstance()
{
	AfxOleInit();

	// Setup the registry

	LynxEmulator.SetRegistryKey("Irwell Expert Systems");

	// Pass down the command line
	CString gamefile(m_lpCmdLine);
	m_pMainWnd = new CLynxWindow(gamefile);
	
	// Check for abort during creation == ERROR
	CLynxWindow* pWnd=(CLynxWindow*)m_pMainWnd;

	// Allow file drop acceptance
	pWnd->DragAcceptFiles(TRUE);

	// Check for correct initialisation
	if(pWnd->mInitOK==FALSE)
	{
		delete m_pMainWnd;
		return FALSE;
	}

	m_pMainWnd->ShowWindow(m_nCmdShow);
	m_pMainWnd->UpdateWindow();
	return TRUE;
}




