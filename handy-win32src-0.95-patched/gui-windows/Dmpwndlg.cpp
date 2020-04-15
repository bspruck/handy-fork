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
// Lynx context window object for dump/code windows                         //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class provides a context dialog and is used by the dumpwin and      //
// codewin classes.                                                         //
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

#include "afxwin.h"
#include "../core/machine.h"
#include "dmpwndlg.h"
#include "dumpwin.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

/////////////////////////////////////////////////////////////////////////////
// CDumpWinDlg dialog


CDumpWinDlg::CDumpWinDlg(CWnd* pParent, EMMODE mode, int address, int followpc, int zoom )
	: CDialog(CDumpWinDlg::IDD, pParent)
{
	mEnableZoom=(zoom==-1)?FALSE:TRUE;
	mEnableFollowPC=(followpc==-1)?FALSE:TRUE;
	mAddress=address;

	//{{AFX_DATA_INIT(CDumpWinDlg)
	mMode = mode;
	mAddrStr = _T("");
	mFollowPC = followpc;
	mZoom = zoom;
	//}}AFX_DATA_INIT


}


void CDumpWinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDumpWinDlg)
	DDX_Radio(pDX, IDC_RADIO1, mMode);
	DDX_Text(pDX, IDC_EDIT1, mAddrStr);
	DDV_MaxChars(pDX, mAddrStr, 10);
	DDX_Check(pDX, IDC_CHECK1, (int&)mFollowPC);
	DDX_Check(pDX, IDC_CHECK2, (int&)mZoom);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDumpWinDlg, CDialog)
	//{{AFX_MSG_MAP(CDumpWinDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDumpWinDlg message handlers

BOOL CDumpWinDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
// Initialise the address window

	CString addrtext;
	addrtext.Format("0x%06x",mAddress);
	CEdit&	ctladdr=*(CEdit*)GetDlgItem(IDC_EDIT1);
	ctladdr.SetWindowText(addrtext);

	if(mEnableFollowPC)
	{
		CButton& button=*(CButton*)GetDlgItem(IDC_CHECK1);
		button.EnableWindow(TRUE);
	}

	if(mEnableZoom)
	{
		CButton& button=*(CButton*)GetDlgItem(IDC_CHECK2);
		button.EnableWindow(TRUE);
	}

// Initialise the radio boxes

	if(mMode==bank0)
	{
		CheckDlgButton(IDC_RADIO1,1);
	}
	else if(mMode==bank1)
	{
		CheckDlgButton(IDC_RADIO2,1);
	}
	else if(mMode==ram)
	{
		CheckDlgButton(IDC_RADIO3,1);
	}
	else
	{
		CheckDlgButton(IDC_RADIO4,1);
	}

	CheckDlgButton(IDC_CHECK1,mFollowPC);
	CheckDlgButton(IDC_CHECK2,mZoom);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CDumpWinDlg::OnOK() 
{
// Perform the default functions first
	
	CDialog::OnOK();

// Perform address conversion from string to int

	int radix;

	// Do a little preparation
	mAddrStr.MakeLower();
	mAddrStr.TrimRight();
	mAddrStr.TrimLeft();
	
	// 0x Signifies a hex number

	if(mAddrStr.Left(2)=="0x"){
		mAddrStr=mAddrStr.Mid(2);
		radix=16;
		mAddrStr=mAddrStr.SpanIncluding("0123456789abcdef");
	}
	else{
		radix=10;
		mAddrStr=mAddrStr.SpanIncluding("0123456789");
	}

	int index=mAddrStr.GetLength();
	int digit=1;
	CString	lookup = "0123456789abcdef";
	mAddress=0;

	while(index){
		mAddress+=digit * lookup.Find(mAddrStr.GetAt(index-1));
		digit*=radix;
		index--;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CDumpWinDlg function interface

void CDumpWinDlg::SetMode(EMMODE mode)
{
	mMode=(int)mode;
}

void CDumpWinDlg::SetAddress(int addr)
{
	mAddress=addr;
}

void CDumpWinDlg::SetFollow(int follow)
{
	mFollowPC=follow;
}

void CDumpWinDlg::SetZoom(int zoom)
{
	mZoom=zoom;
}

void CDumpWinDlg::EnableFollow()
{
	mEnableFollowPC=TRUE;
}

void CDumpWinDlg::EnableZoom()
{
	mEnableZoom=TRUE;
}
