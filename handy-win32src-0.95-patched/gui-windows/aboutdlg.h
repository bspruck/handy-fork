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
// About box class                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Custom dialog class for displaying the Handy about box                   //
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

class CAboutDialog : public CDialog
{
	public:
		CAboutDialog(UINT id,CWnd* pParent=NULL, CString name="", CString manuf="", CString version="", CString build="")
			:CDialog(id, pParent)
		{
			mName=name;
			mManufacturer=manuf;
			mVersion=version;
			mBuild=build;
		}

		void SetName(CString name) {mName=name;};
		void SetManufacturer(CString manuf) {mManufacturer=manuf;};
	public:
		CString mName;
		CString mManufacturer;
		CString mVersion;
		CString mBuild;

	public:
		virtual BOOL OnInitDialog(void)
		{
			CDialog::OnInitDialog();

			mManufacturer="Manufacturer: "+mManufacturer;
			mName="Cartridge: "+mName;

			CEdit& ctlmanuf=*(CEdit*)GetDlgItem(IDC_ABOUT_MANUFACTURER);
			ctlmanuf.SetWindowText(mManufacturer);

			CEdit& ctlname=*(CEdit*)GetDlgItem(IDC_ABOUT_CARTRIDGE);
			ctlname.SetWindowText(mName);

			CStatic& ctlver=*(CStatic*)GetDlgItem(IDC_ABOUT_VERSION);
			ctlver.SetWindowText(mVersion);

			CStatic& ctlbuild=*(CStatic*)GetDlgItem(IDC_ABOUT_BUILD);
			ctlbuild.SetWindowText(mBuild);

			return TRUE;
		}
};
