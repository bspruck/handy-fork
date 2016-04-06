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


// KeyDefs.cpp : implementation file
//

#include <afxwin.h>
#include <afxcmn.h>
#include <afxdlgs.h>
#include "resource.h"
#include "keydefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static char *charnames[256]=
{
// 0x00
	"<$00>","<$01>","<$02>","<$03>","<$04>","<$05>","<$06>","<$07>",
	"<$08>","<Tab>","<$0a>","<$0b>","<$0c>","<Retn>","<$0e>","<$0f>",
// 0x10
	"<Shft>","<Ctrl>","<$12>","<$13>","<Cplk>","<$15>","<$16>","<$17>",
	"<$18>","<$19>","<$1a>","<$1b>","<$1c>","<$1d>","<$1e>","<$1f>",
// 0x20
	"<Spc>","<PgUp>","<PgDn>","<End>","<Home>","<Left>","<Up>" ,"<Rght>",
	"<Down>","<$29>","<$2a>","<$2b>","<$2c>","<Ins>","<Del>","<$2f>",
// 0x30
	" < 0 >"," < 1 >"," < 2 >"," < 3 >"," < 4 >"," < 5 >"," < 6 >"," < 7 >",
	" < 8 >"," < 9 >","<$3a>","<$3b>","<$3c>","<$3d>","<$3e>","<$3f>",
// 0x40
	"<$40>" ," < A >"," < B >"," < C >"," < D >"," < E >"," < F >"," < G >",
	" < H >"," < I >"," < J >"," < K >"," < L >"," < M >"," < N >"," < O >",
// 0x50
	" < P >"," < Q >"," < R >"," < S >"," < T >"," < U >"," < V >"," < W >",
	" < X >"," < Y >"," < Z >","<$5b>","<$5c>","<$5d>","<$5e>","<$5f>",
// 0x60
	"<$60>","<$61>","<$62>","<$63>","<$64>","<$65>","<$66>","<$67>",
	"<$68>","<$69>","<$6a>","<$6b>","<$6c>","<$6d>","<$6e>","<$6f>",
// 0x70
	"<F1>" ,"<F2>" ,"<F3>" ,"<F4>" ,"<F5>" ,"<F6>" ,"<F7>" ,"<F8>" ,
	"<F9>" ,"<$79>","<$7a>","<$7b>","<$7c>","<$7d>","<$7e>","<$7f>",
// 0x80
	"<$80>","<$81>","<$82>","<$83>","<$84>","<$85>","<$86>","<$87>",
	"<$88>","<$89>","<$8a>","<$8b>","<$8c>","<$8d>","<$8e>","<$8f>",
// 0x90
	"<$90>","<$91>","<$92>","<$93>","<$94>","<$95>","<$96>","<$97>",
	"<$98>","<$99>","<$9a>","<$9b>","<$9c>","<$9d>","<$9e>","<$9f>",
// 0xA0
	"<$a0>","<$a1>","<$a2>","<$a3>","<$a4>","<$a5>","<$a6>","<$a7>",
	"<$a8>","<$a9>","<$aa>","<$ab>","<$ac>","<$ad>","<$ae>","<$af>",
// 0xB0
	"<$b0>","<$b1>","<$b2>","<$b3>","<$b4>","<$b5>","<$b6>","<$b7>",
	"<$b8>","<$b9>"," < ; >"," < = >"," < , >"," < - >"," < . >"," < / >",
// 0xC0
	"<$c0>","<$c1>","<$c2>","<$c3>","<$c4>","<$c5>","<$c6>","<$c7>",
	"<$c8>","<$c9>","<$ca>","<$cb>","<$cc>","<$cd>","<$ce>","<$cf>",
// 0xD0
	"<$d0>","<$d1>","<$d2>","<$d3>","<$d4>","<$d5>","<$d6>","<$d7>",
	"<$d8>","<$d9>","<$da>"," < [ >"," < \\ >"," < ] >"," < # >","<$df>",
// 0xE0
	"<$e0>","<$e1>","<$e2>","<$e3>","<$e4>","<$e5>","<$e6>","<$e7>",
	"<$e8>","<$e9>","<$ea>","<$eb>","<$ec>","<$ed>","<$ee>","<$ef>",
// 0xF0
	"<$f0>","<$f1>","<$f2>","<$f3>","<$f4>","<$f5>","<$f6>","<$f7>",
	"<$f8>","<$f9>","<$fa>","<$fb>","<$fc>","<$fd>","<$fe>","<$ff>",

};


/////////////////////////////////////////////////////////////////////////////
// CKeyDefs dialog


CKeyDefs::CKeyDefs(CWnd* pParent, KEYCONF *keyconf)
	: CDialog(CKeyDefs::IDD, pParent)
{
	//{{AFX_DATA_INIT(CKeyDefs)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	mNewKeys=keyconf;
	mGrabNext=NULL;
}


void CKeyDefs::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKeyDefs)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CKeyDefs, CDialog)
	//{{AFX_MSG_MAP(CKeyDefs)
	ON_BN_CLICKED(IDC_KEYDEF_A, OnKeydefA)
	ON_BN_CLICKED(IDC_KEYDEF_B, OnKeydefB)
	ON_BN_CLICKED(IDC_KEYDEF_DOWN, OnKeydefDown)
	ON_BN_CLICKED(IDC_KEYDEF_LEFT, OnKeydefLeft)
	ON_BN_CLICKED(IDC_KEYDEF_OPT1, OnKeydefOpt1)
	ON_BN_CLICKED(IDC_KEYDEF_OPT2, OnKeydefOpt2)
	ON_BN_CLICKED(IDC_KEYDEF_PAUSE, OnKeydefPause)
	ON_BN_CLICKED(IDC_KEYDEF_RIGHT, OnKeydefRight)
	ON_BN_CLICKED(IDC_KEYDEF_UP, OnKeydefUp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyDefs message handlers


void CKeyDefs::OnKeydefA() 
{
	GetKey(&mNewKeys->key_a);
}

void CKeyDefs::OnKeydefB() 
{
	GetKey(&mNewKeys->key_b);
}

void CKeyDefs::OnKeydefDown() 
{
	GetKey(&mNewKeys->key_down);
}

void CKeyDefs::OnKeydefLeft() 
{
	GetKey(&mNewKeys->key_left);
}

void CKeyDefs::OnKeydefOpt1() 
{
	GetKey(&mNewKeys->key_opt1);
}

void CKeyDefs::OnKeydefOpt2() 
{
	GetKey(&mNewKeys->key_opt2);
}

void CKeyDefs::OnKeydefPause() 
{
	GetKey(&mNewKeys->key_pause);
}

void CKeyDefs::OnKeydefRight() 
{
	GetKey(&mNewKeys->key_right);
}

void CKeyDefs::OnKeydefUp() 
{
	GetKey(&mNewKeys->key_up);
}

void CKeyDefs::UpdateButtons()
{
	CString tmpstr;
	CButton *button_tmp;

	tmpstr.Format("  A  \n%s",charnames[mNewKeys->key_a&0x00ff]);
	button_tmp=(CButton*)GetDlgItem(IDC_KEYDEF_A);
	button_tmp->SetWindowText(tmpstr);

	tmpstr.Format("  B  \n%s",charnames[mNewKeys->key_b&0x00ff]);
	button_tmp=(CButton*)GetDlgItem(IDC_KEYDEF_B);
	button_tmp->SetWindowText(tmpstr);

	tmpstr.Format("  UP \n%s",charnames[mNewKeys->key_up&0x00ff]);
	button_tmp=(CButton*)GetDlgItem(IDC_KEYDEF_UP);
	button_tmp->SetWindowText(tmpstr);

	tmpstr.Format(" DOWN\n%s",charnames[mNewKeys->key_down&0x00ff]);
	button_tmp=(CButton*)GetDlgItem(IDC_KEYDEF_DOWN);
	button_tmp->SetWindowText(tmpstr);

	tmpstr.Format(" LEFT\n%s",charnames[mNewKeys->key_left&0x00ff]);
	button_tmp=(CButton*)GetDlgItem(IDC_KEYDEF_LEFT);
	button_tmp->SetWindowText(tmpstr);

	tmpstr.Format("RIGHT\n%s",charnames[mNewKeys->key_right&0x00ff]);
	button_tmp=(CButton*)GetDlgItem(IDC_KEYDEF_RIGHT);
	button_tmp->SetWindowText(tmpstr);

	tmpstr.Format(" OPT1\n%s",charnames[mNewKeys->key_opt1&0x00ff]);
	button_tmp=(CButton*)GetDlgItem(IDC_KEYDEF_OPT1);
	button_tmp->SetWindowText(tmpstr);

	tmpstr.Format(" OPT2\n%s",charnames[mNewKeys->key_opt2&0x00ff]);
	button_tmp=(CButton*)GetDlgItem(IDC_KEYDEF_OPT2);
	button_tmp->SetWindowText(tmpstr);

	tmpstr.Format("PAUSE\n%s",charnames[mNewKeys->key_pause&0x00ff]);
	button_tmp=(CButton*)GetDlgItem(IDC_KEYDEF_PAUSE);
	button_tmp->SetWindowText(tmpstr);
}


BOOL CKeyDefs::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UpdateButtons();

	CString tmpstr="";
	CStatic *static_tmp;
	static_tmp=(CStatic*)GetDlgItem(IDC_KEYGET);
	static_tmp->SetWindowText(tmpstr);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CKeyDefs::GetKey(UINT *key)
{
	CString tmpstr="Next keypress will be taken as the button assignment, press escape to cancel";
	CStatic *static_tmp;
	static_tmp=(CStatic*)GetDlgItem(IDC_KEYGET);
	static_tmp->SetWindowText(tmpstr);

	mGrabNext=key;
}

BOOL CKeyDefs::PreTranslateMessage(MSG *pMsg)
{
	if(pMsg->message==WM_KEYDOWN)
	{
		if(mGrabNext)
		{
			CString tmpstr="";
			CStatic *static_tmp;
			static_tmp=(CStatic*)GetDlgItem(IDC_KEYGET);
			static_tmp->SetWindowText(tmpstr);

			if(pMsg->wParam!=VK_ESCAPE)	*mGrabNext=pMsg->wParam;

			mGrabNext=NULL;

			UpdateButtons();

			return 1;
		}
	}

//	if(pMsg->message==WM_KEYDOWN)
//	{
//		CString tmpstr="`";
//		CStatic *static_tmp;
//		tmpstr.Format("0x%04x\n0x%08x",pMsg->wParam,pMsg->lParam);
//		static_tmp=(CStatic*)GetDlgItem(IDC_KEYGET);
//		static_tmp->SetWindowText(tmpstr);
//	}

	return CDialog::PreTranslateMessage(pMsg);
}
