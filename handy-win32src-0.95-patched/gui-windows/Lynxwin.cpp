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
// Lynx window class                                                        //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class is the main window control class, it creates and manages the  //
// lynx object (CSystem - mLynx). The Update() function does most of the    //
// work and is in the header file for inline purposes. It also handles the  //
// speed throttling via the MM timer callback.                              //
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

#include "lynxwin.h"
#include "../core/error.h"
#include "../core/lynxdef.h"
#include "errorhandler.h"
#include "debugger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define HANDY_VERSION		"Version 0.98beta"
#define HANDY_BUILD			"Build ("__DATE__")"

#define REGISTRY_VERSION	"Version 1.0"

#define TRACE_LYNXWIN

/////////////////////////////////////////////////////////////////////////////
// CLynxWindown

CLynxWindow::CLynxWindow(CString gamefile)
	:mpLynx(NULL)
{
#ifdef _LYNXDBG
	_CrtSetReportFile(_CRT_WARN,_CRTDBG_FILE_STDOUT);
#endif

	// Initialise local member variables
	mInitOK=FALSE;

	// Find the handle to our calling application
	mpLynxApp=AfxGetApp();

	// First build an error handler
	gError = new CErrorHandler(this);

	// Continue initialisation
	mWindowInFocus=TRUE;
	mEmulationSpeed=0;
	mFramesPerSecond=0;
	mFrameCount=0;
	mFrameSkip=0;

	// Init display stuff so it will create correctly

	mDisplayRender=NULL;
	mDisplayNoPainting=TRUE;
	mDisplayMode=DISPLAY_WINDOWED|DISPLAY_X1|DISPLAY_NO_ROTATE;
	mDisplayOffsetX=0;
	mDisplayOffsetY=0;

	// Load up the default background bitmap
	mDisplayBackgroundType=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"DisplayBackgroundType", IDB_BITMAP_BACKGROUND3);
	if(mDisplayBackgroundType!=IDB_BITMAP_BACKGROUND1 && mDisplayBackgroundType!=IDB_BITMAP_BACKGROUND2 && mDisplayBackgroundType!=IDB_BITMAP_BACKGROUND3) mDisplayBackgroundType=IDB_BITMAP_BACKGROUND1;
	if(!mDisplayBackground.LoadBitmap(mDisplayBackgroundType)) return;

	// Setup default networker state
	mpNetLynx=NULL;

	// Read the default window position from the registry

	CRect rect=rectDefault;

	rect.left=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"MainWindowX1", rectDefault.left);
	rect.top=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"MainWindowY1", rectDefault.top);

	// If less than 50 pixels is onscreen in any axis then reset screen coords
	if(rect.left<0 || rect.right<0 || (rect.left+50)>GetSystemMetrics(SM_CXSCREEN) || (rect.top+50)>GetSystemMetrics(SM_CYSCREEN))
	{
		rect.left=0; rect.top=0;
	}
	
	// Create our window OVERLAPPEDWINDOW - THICKFRAME (No resize)
	Create(NULL,"Handy",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,rect,NULL,MAKEINTRESOURCE(IDR_MAINFRAME));

	//
	// Create an invisible dialog for emulator info
	//
	mInfoDialogEnable=0;
	mInfoDialog.Create(IDD_INFO_BOX,this);
	mInfoDialog.ShowWindow(SW_HIDE);
	if(mpLynxApp->GetProfileInt(REGISTRY_VERSION,"InfoDialogEnable",FALSE)) OnInfoSelect();
	gThrottleMaxPercentage=100;
	CSpinButtonCtrl& ctlspeed=*(CSpinButtonCtrl*)mInfoDialog.GetDlgItem(IDC_STATUS_SPIN1);
	ctlspeed.SetRange(1,500);
	ctlspeed.SetPos(gThrottleMaxPercentage);

	//
	// Initialise/Hide the debugging menu if debugging is disabled
	//
#ifdef _LYNXDBG
	mpDebugger=NULL;
	gSingleStepModeSprites=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"DebugSpriteStep",FALSE);
#else
	GetMenu()->RemoveMenu(2,MF_BYPOSITION);
	DrawMenuBar();
#endif

	// Load our icon

	HICON hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
	SetIcon(hIcon,TRUE);

	// Load the keyboard accelerator table

	LoadAccelTable(MAKEINTRESOURCE(IDR_ACCELERATOR1));

	// Setup the root directory path

	GetCurrentDirectory(256,mRootPath.GetBuffer(256));
	mRootPath.ReleaseBuffer();
	mRootPath+="\\";

	//
	// Finally make the lynx emulator object now all the windoze shit is done
	//

	mCommandLineMode=(strcmp(gamefile,""))?TRUE:FALSE;

	if((mpLynx=CreateLynx(gamefile))==NULL)
	{
		return;
	}

//
// Set the video mode, create device contexts
//
	ULONG render=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"DisplayModeRender",DISPLAY_WINDOWED)&DISPLAY_RENDER_MASK;
	ULONG magnify=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"DisplayModeMagnification",DISPLAY_X1)&DISPLAY_X_MASK;
	ULONG rotate=DISPLAY_NO_ROTATE;
	ULONG bkgnd=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"DisplayModeBackground",DISPLAY_BKGND);
	if(mpLynx->CartGetRotate()==CART_ROTATE_LEFT) rotate=DISPLAY_ROTATE_LEFT;
	if(mpLynx->CartGetRotate()==CART_ROTATE_RIGHT) rotate=DISPLAY_ROTATE_RIGHT;
// Allow boot into fullscreen -	if(render==DISPLAY_FULLSCREEN || render==DISPLAY_EAGLE_FULLSCREEN || render==DISPLAY_LYNXLCD_FULLSCREEN) render=DISPLAY_WINDOWED;
	mDisplayMode=DisplayModeSet(render,bkgnd,magnify,rotate);

//
// Create a Multimedia timer for speed throttling/calculation
//
	if((mTimerID=timeSetEvent(1000/HANDY_TIMER_FREQ,0,(LPTIMECALLBACK)fTimerEventHandler,(DWORD)this,TIME_PERIODIC))==NULL)
	{
		gError->Fatal("CLynxWindow::CLynxWindow() - Couldn't initialise Multimedia timer ???");
	}

	// Create a sound object

	mDirectSoundPlayer.Create(this,gAudioBuffer,&gAudioBufferPointer,HANDY_AUDIO_BUFFER_SIZE,HANDY_AUDIO_SAMPLE_FREQ);
	if(mpLynxApp->GetProfileInt(REGISTRY_VERSION,"AudioEnabled",FALSE)) gAudioEnabled=mDirectSoundPlayer.Start();

	// Enable the joystick if required

	mJoystickEnable=FALSE;
	if(mpLynxApp->GetProfileInt(REGISTRY_VERSION,"JoystickEnabled",FALSE)) OnJoystickMenuSelect();

	// Read in the key definitions
	mKeyDefs.key_up=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"KeyDefs_UP",VK_UP);
	mKeyDefs.key_down=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"KeyDefs_DOWN",VK_DOWN);
	mKeyDefs.key_left=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"KeyDefs_LEFT",VK_LEFT);
	mKeyDefs.key_right=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"KeyDefs_RIGHT",VK_RIGHT);
	mKeyDefs.key_a=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"KeyDefs_A",VK_X);
	mKeyDefs.key_b=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"KeyDefs_B",VK_Z);
	mKeyDefs.key_opt1=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"KeyDefs_OPT1",VK_1);
	mKeyDefs.key_opt2=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"KeyDefs_OPT2",VK_2);
	mKeyDefs.key_pause=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"KeyDefs_PAUSE",VK_Q);

	mInitOK=TRUE;
	mDisplayNoPainting=FALSE;
}


CLynxWindow::~CLynxWindow()
{
}



BOOL CLynxWindow::DestroyWindow() 
{

	// If we have a net link up then destroy it
	if(mpNetLynx)
	{
		delete mpNetLynx;
		mpNetLynx=NULL;
	}

	// Delete the sound player
	mDirectSoundPlayer.Destroy(this);

	// Write the status to the registry only if NOT an abort

	if(mInitOK)
	{
		CRect rect;
		GetWindowRect(&rect);

		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"MainWindowX1", rect.left);
		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"MainWindowY1", rect.top);

		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"AudioEnabled",(int)mDirectSoundPlayer.GetStatus());
		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"JoystickEnabled",(mJoystickEnable)?1:0);

		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"KeyDefs_UP",mKeyDefs.key_up);
		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"KeyDefs_DOWN",mKeyDefs.key_down);
		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"KeyDefs_LEFT",mKeyDefs.key_left);
		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"KeyDefs_RIGHT",mKeyDefs.key_right);
		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"KeyDefs_A",mKeyDefs.key_a);
		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"KeyDefs_B",mKeyDefs.key_b);
		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"KeyDefs_OPT1",mKeyDefs.key_opt1);
		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"KeyDefs_OPT2",mKeyDefs.key_opt2);
		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"KeyDefs_PAUSE",mKeyDefs.key_pause);

		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"DisplayModeMagnification",DisplayModeMagnification());
		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"DisplayModeBackground",DisplayModeBkgnd());
		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"DisplayBackgroundType",mDisplayBackgroundType);
		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"DisplayModeRender",DisplayModeRender());

		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"InfoDialogEnable",(mInfoDialogEnable)?1:0);

		mInfoDialog.GetWindowRect(&rect);
		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"InfoWindowX1", rect.left);
		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"InfoWindowY1", rect.top);
	}

	// Destroy the previous renderer
	if(mDisplayRender!=NULL)
	{
		mDisplayRender->Destroy(this);
		delete mDisplayRender;
		mDisplayRender=NULL;
	}

	// Destroy any dump or code windows that are open

#ifdef _LYNXDBG
	mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"DebugSpriteStep",gSingleStepModeSprites);
	delete mpDebugger;
#endif

	// Squash our timer event

	if(mTimerID!=NULL) timeKillEvent(mTimerID);

	// Destroy the infodialog window
	if(mInfoDialogEnable) OnInfoSelect();
	mInfoDialog.DestroyWindow();

	// Destroy the emulation
	delete mpLynx;

	// Destroy the error handler
	delete gError;

	// Return stat of main window destruction
	return CFrameWnd::DestroyWindow();
}


//
// Multimedia timer handler
//

void CALLBACK CLynxWindow::fTimerEventHandler(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2)
{
	static ULONG count=0;
	static ULONG fcount=0;
	static ULONG last_cycle_count=0;
	static ULONG last_frame_count=0;

	// Refreference the objects as this is a static member
	CLynxWindow *lwin = (CLynxWindow*) dwUser;

	//
	// Only function when in focus
	//
	if(!lwin->mWindowInFocus) return;

	//
	// Calculate emulation speed
	//
	count++;
	fcount++;

	if(count==HANDY_TIMER_FREQ/2)
	{
		count=0;
		if(last_cycle_count>gSystemCycleCount)
		{
			lwin->mEmulationSpeed=0;
		}
		else
		{
			// Add .5% correction factor for round down error
			lwin->mEmulationSpeed=(gSystemCycleCount-last_cycle_count)*100;
			lwin->mEmulationSpeed+=(gSystemCycleCount-last_cycle_count)/2;
			lwin->mEmulationSpeed/=HANDY_SYSTEM_FREQ/2;
		}
		last_cycle_count=gSystemCycleCount;
	}

	if(fcount==HANDY_TIMER_FREQ)
	{
		fcount=0;
		if(last_frame_count>lwin->mFrameCount)
		{
			lwin->mFramesPerSecond=0;
		}
		else
		{
			lwin->mFramesPerSecond=(lwin->mFrameCount-last_frame_count);
		}
		last_frame_count=lwin->mFrameCount;
	}

	//
	// Increment system counter
	//
	gTimerCount++;


	// Send ourselves a message, the main idle loop will
	// put the app to sleep if waiting for the next timer
	// tick, this will bring things back to life
	lwin->PostMessage(WM_LYNXWIN_WAKEUP);

}


CSystem* CLynxWindow::CreateLynx(CString gamefile)
{
	CSystem *newsystem=NULL;

	// Remove leading and trailing " that windows kindly adds as this chokes the
	// CFile open function for some reason
	int	fileprovided=0;
	if(gamefile!="")
	{
		fileprovided=1;
		gamefile.Remove('\"');
	}

	// First thing is to locate a Lynxboot image, see if the registry has one

	CString romfile=mRootPath+"lynxboot.img";
	romfile=mpLynxApp->GetProfileString(REGISTRY_VERSION,"BootImageFilename",romfile);

	// Try to open it, if not then fail over to message then file dialog

	romfile = "dontcare";
#if 0 // no need for bios image anymore...
	CFile file;
	
	if(!file.Open(romfile,CFile::modeRead))
	{
		// Warn the user that the file couldn't be opened

		MessageBox("The Lynx boot image could not be found, please select the location of the image in the browser","Warning", MB_OK | MB_ICONERROR);

		// Check if gamefile exists if so then load, else dialog

		CFileDialog	dlg(TRUE,"img","lynxboot.img",OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,"BootRom Image (*.img;*.rom)|*.img;*.rom|All Files (*.*)|*.*||",NULL);

		// Do the dialog and abort if no image given

		if(dlg.DoModal()==IDCANCEL) return NULL;

		romfile=dlg.GetPathName();

		// Save the rom image path to the registry
		mpLynxApp->WriteProfileString(REGISTRY_VERSION,"BootImageFilename",romfile);
	}
	else
	{
		file.Close();
	}
#endif

	// Loop around the file open menu until we have cart or the user
	// has cancelled the operation
	do
	{
		try
		{
			if(!fileprovided)
			{
				CString filter="Handy Filetypes (*.zip;*.lnx;*.com;*.o)|*.zip;*.lnx;*.com;*.o|Lynx Images (*.zip;*.lnx)|*.zip;*.lnx|Homebrew Images (*.zip;*.com;*.o)|*.zip;*.com;*.o|ZIP Files (*.zip)|*.zip|All Files (*.*)|*.*||";
				gamefile=mpLynxApp->GetProfileString(REGISTRY_VERSION,"DefaultGameFile","");
				// Check if gamefile exists if so then load, else dialog
				CFileDialog	dlg(TRUE,"lnx",gamefile,OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,filter,NULL);
				// Retrieve file from dialog
				if(dlg.DoModal()==IDCANCEL)
				{
					gamefile="";
				}
				else
				{
					gamefile=dlg.GetPathName();
					mpLynxApp->WriteProfileString(REGISTRY_VERSION,"DefaultGameFile",gamefile);
				}
			}

			// Create the system object
			newsystem = new CSystem((char*)LPCTSTR(gamefile),(char*)LPCTSTR(romfile));

#ifdef _LYNXDBG
			if(mpDebugger)
			{
				delete mpDebugger;
				mpDebugger=NULL;
			}
			mpDebugger = new CDebugger(*newsystem);
			newsystem->DebugSetCallback(mpDebugger->DebuggerMessage,(ULONG)mpDebugger);
#endif
		}
		catch(CLynxException &err)
		{
			MessageBox(err.mDesc, err.mMsg, MB_OK | MB_ICONERROR);
			if(newsystem!=NULL)
			{
				delete newsystem;
				newsystem=NULL;
			}
			gamefile="";
		}
	}
	while(newsystem==NULL);

	return newsystem;
}


void CLynxWindow::CalcWindowSize(CRect *rect)
{
	int win_height,win_width;
	int img_height,img_width;

	//
	// Derive the image size, simple eh!
	//
	img_width=mDisplayRender->mSrcWidth*DisplayModeMagnification();
	img_height=mDisplayRender->mSrcHeight*DisplayModeMagnification();

	//
	// Derive the basic window dimensions
	//
	if(DisplayModeWindowed() && DisplayModeBkgnd()==DISPLAY_BKGND && DisplayModeRotate()==DISPLAY_NO_ROTATE)
	{
		BITMAP bkgnd;
		mDisplayBackground.GetBitmap(&bkgnd);
		win_height=bkgnd.bmHeight*DisplayModeMagnification();
		win_width=bkgnd.bmWidth*DisplayModeMagnification();
	}
	else
	{
		win_width=mDisplayRender->mWidth;
		win_height=mDisplayRender->mHeight;
	}

	// Ensure minimum width
	if(win_width<HANDY_SCREEN_WIDTH) win_width=HANDY_SCREEN_WIDTH;

	// Calculate display offsets
	mDisplayOffsetX=(win_width/2)-(img_width/2);
	mDisplayOffsetY=(win_height/2)-(img_height/2);

	// Move into the output structure
	rect->left=0;
	rect->top=0;
	rect->right=rect->left+win_width+GetSystemMetrics(SM_CXFRAME);
	rect->bottom=rect->top+win_height+GetSystemMetrics(SM_CYMENU)+GetSystemMetrics(SM_CYFRAME);
	CalcWindowRect(rect,0);
}


ULONG CLynxWindow::DisplayModeSet(ULONG mode)
{
	CRect rect;

	// Disable painting during a GFX mode change
	mDisplayNoPainting=TRUE;

	// Update the global mode value
	mDisplayMode=mode;

	// Calculate zoom value to pass to renderer
	int zoom=mDisplayMode&DISPLAY_X_MASK;

	// Initialisation state
	int sourceX=(DisplayModeRotate()==DISPLAY_NO_ROTATE)?HANDY_SCREEN_WIDTH:HANDY_SCREEN_HEIGHT;
	int sourceY=(DisplayModeRotate()==DISPLAY_NO_ROTATE)?HANDY_SCREEN_HEIGHT:HANDY_SCREEN_WIDTH;

	// Destroy the previous renderer
	if(mDisplayRender!=NULL)
	{
		mDisplayRender->Destroy(this);
		delete mDisplayRender;
		mDisplayRender=NULL;
	}

	// Check full screen status currently and create/destroy mode appropriately

	switch(mode&DISPLAY_RENDER_MASK)
	{
		case DISPLAY_WINDOWED:
			mDisplayRender = new CWindowDirectX;
			break;
		case DISPLAY_FULLSCREEN:
			mDisplayRender = new CFullScreenDirectX;
			break;
		case DISPLAY_LYNXLCD_WINDOWED:
			mDisplayRender= new CWindowDirectXLynxLCD;
			break;
		case DISPLAY_LYNXLCD_FULLSCREEN:
			mDisplayRender= new CFullScreenDirectXLynxLCD;
			break;
		case DISPLAY_EAGLE_WINDOWED:
			mDisplayRender= new CWindowDirectXEagle;
			break;
		case DISPLAY_EAGLE_FULLSCREEN:
			mDisplayRender= new CFullScreenDirectXEagle;
			break;
		case DISPLAY_GDI_WINDOWED:
			mDisplayRender= new CWindowGDI;
			break;
		default:
			mDisplayRender = new CWindowDirectX;
			break;
	}


	// Now create the window as required

	if(mDisplayRender->Windowed())
	{
		if(mDisplayRender->Create(this,sourceX,sourceY,0,0,zoom)==FALSE)
		{
			gError->Warning("DisplayModeSet() - Couldnt initialise the DirectX session for windowed mode");
		}
	}
	else
	{
		int scr_x[10]={ 320, 320, 400, 512, 640,1024, 800,1024,1280,-1};
		int scr_y[10]={ 200, 240, 300, 384, 480, 480, 600, 768,1024,-1};
		int index=0;
		int result=0;

		while(scr_x[index]!=-1)
		{
			// Its doesnt matter if the zoom is not set correctly as the creation
			// handler will reject the creation if the size is wrong.
			if((result=mDisplayRender->Create(this,sourceX,sourceY,scr_x[index],scr_y[index],zoom))==FALSE)
			{
				// Try the next res up, this one isnt supported
				index++;
			}
			else
			{
				// Success, leave exit the search loop
				break;
			}
		}

		if(!result)
		{
			// Clear the fullscreen bit
			mode&=(DISPLAY_RENDER_MASK^0xffff);
			mode|=DISPLAY_WINDOWED;
			mDisplayMode=mode;
			gError->Warning("DisplayModeSet() - It looks like your video driver will not support any requested mode, tried 7 different resolutions");
		}
	}

	// Calculate the new window/offset parameters, its also important
	// for full screen mode as it calulates bitmap blit offsets
	CalcWindowSize(&rect);

	// Resize if in windowed mode
	if(DisplayModeWindowed())
	{
		SetWindowPos(&wndTop,0,0,rect.Width(),rect.Height(),SWP_NOMOVE|SWP_SHOWWINDOW);
		ShowWindow(SW_SHOWNORMAL);
//		Invalidate();
//		UpdateWindow();
//		ModifyStyle(0,0,SWP_SHOWWINDOW);
	}

	//
	// Set window based attributes in the Lynx model
	//

	if(mpLynx)
	{
		int rotate=MIKIE_NO_ROTATE;
		int format=MIKIE_BAD_MODE;
		int pitch=0;

		pitch=mDisplayRender->BufferPitch();

		switch(mDisplayRender->PixelFormat())
		{
			case RENDER_8BPP:
				format=MIKIE_PIXEL_FORMAT_8BPP;
				break;
			case RENDER_16BPP_555:
				format=MIKIE_PIXEL_FORMAT_16BPP_555;
				break;
			case RENDER_16BPP_565:
				format=MIKIE_PIXEL_FORMAT_16BPP_565;
				break;
			case RENDER_24BPP:
				format=MIKIE_PIXEL_FORMAT_24BPP;
				break;
			case RENDER_32BPP:
				format=MIKIE_PIXEL_FORMAT_32BPP;
				break;
			default:
				format=MIKIE_BAD_MODE;
				mDisplayRender->Destroy(this);
				delete mDisplayRender;
				mDisplayRender=NULL;
				gError->Warning("DisplayModeSet() - Display rendering device returned unknown pixel format, Handy cannot display an image.");
				break;
		}

		switch(DisplayModeRotate())
		{
			case DISPLAY_ROTATE_LEFT:
				rotate=MIKIE_ROTATE_L;
				break;
			case DISPLAY_ROTATE_RIGHT:
				rotate=MIKIE_ROTATE_R;
				break;
			case DISPLAY_NO_ROTATE:
			default:
				rotate=MIKIE_NO_ROTATE;
				break;
		}

		// Now set the details in the lynx object
		mpLynx->DisplaySetAttributes(rotate,format,pitch,DisplayCallback,(ULONG)this);
	}

	// Reenable painting again
	mDisplayNoPainting=FALSE;

	Invalidate(TRUE);

	return mDisplayMode;
}


ULONG CLynxWindow::DisplayModeSet(ULONG render,ULONG background,ULONG magnification,ULONG rotate)
{

	//
	//
	// Select a sensible video mode (Check registry for last settings)
	//
	//
	if(background==DISPLAY_PRESERVE) background=DisplayModeBkgnd();

	if(render==DISPLAY_PRESERVE) render=DisplayModeRender();

	switch(render&DISPLAY_RENDER_MASK)
	{
		case DISPLAY_WINDOWED:
		case DISPLAY_LYNXLCD_WINDOWED:
		case DISPLAY_EAGLE_WINDOWED:
			{
				// If in 8 bit mode then use GDI rendering
				CDC *winCDC= new CClientDC(this);
				if(winCDC->GetDeviceCaps(BITSPIXEL)==8)
				{
					static int onceonly=0;
					if(!onceonly) gError->Warning("DisplayModeSet() - 256 Colour mode is not supported by under DirectX with Handy, reverting to GDI rendering (slower)");
					onceonly=1;
					render=DISPLAY_GDI_WINDOWED;
				}
				delete winCDC;
			}
			break;
		case DISPLAY_GDI_WINDOWED:
		case DISPLAY_FULLSCREEN:
		case DISPLAY_LYNXLCD_FULLSCREEN:
		case DISPLAY_EAGLE_FULLSCREEN:
		default:
			break;
	}

#ifdef _LYNXDBG
	if(render==DISPLAY_FULLSCREEN)
	{
		gError->Warning("Full screen mode is not allowed with Handy debugging version.");
		render=DISPLAY_WINDOWED;
	}
#endif

	//
	// Handle magnification issues
	//
	if(magnification==DISPLAY_PRESERVE)
	{
		magnification=DisplayModeMagnification();
	}
	else if(magnification!=DISPLAY_X1 && magnification!=DISPLAY_X2 && magnification!=DISPLAY_X3 && magnification!=DISPLAY_X4)
	{
		magnification=DISPLAY_X_DEFAULT;
	}
	// Background is only available for x1 & x2, x3
	if(magnification==DISPLAY_X4) background=DISPLAY_NO_BKGND;

	//
	// Handle Rotation issues
	//
	if(rotate==DISPLAY_PRESERVE)
	{
		rotate=DisplayModeRotate();
	}
	else if(rotate!=DISPLAY_ROTATE_LEFT && rotate!=DISPLAY_ROTATE_RIGHT && rotate!=DISPLAY_NO_ROTATE)
	{
		rotate=DISPLAY_ROTATE_DEFAULT;
	}

	// Not allowed more than x1 in rotated mode
	if(rotate!=DISPLAY_NO_ROTATE && magnification!=DISPLAY_X1) background=DISPLAY_NO_BKGND;
	// Not allowed x3 x4 in fullscreen
	if(render==DISPLAY_FULLSCREEN && (magnification==DISPLAY_X3 || magnification==DISPLAY_X4)) magnification=DISPLAY_X2;

	//
	// Calculate the video mode
	//
	ULONG newmode=magnification+rotate+render+background;

	//
	// Set the video mode
	//
	DisplayModeSet(newmode);

	return mDisplayMode;
}

void CLynxWindow::NetworkTxCallback(int data,ULONG objref)
{
	// Refreference the objects as this is a static member
	CLynxWindow *lwin = (CLynxWindow*)objref;
	if(lwin->mpNetLynx)
	{
		lwin->mpNetLynx->Transmit(data);
		TRACE_LYNXWIN1("NetworkTxCallback() - Transmitted %02x to CNetObj",data);
	}
	else
	{
//		gError->Warning("CLynxWindow::NetworkTxCallback() - NULL mpNetLynx!!! Must be ghosts...");
		TRACE_LYNXWIN0("NetworkTxCallback() - No valid net object");
	}
}


UBYTE* CLynxWindow::DisplayCallback(ULONG objref)
{
	static int frameskipcount=0;

	// Refreference the objects as this is a static member
	CLynxWindow *lwin = (CLynxWindow*)objref;

	// Skip frames if required
	if(frameskipcount>0)
	{
		frameskipcount--;
	}
	else
	{
		// Reset the frameskip counter
		frameskipcount=lwin->mFrameSkip;

		// A screen update is required so lets get on with it
		lwin->mFrameCount++;
		lwin->OnPaint();
	}

#ifdef _LYNXDBG
	// If in debug mode then update all open windows
	if(lwin->mpDebugger->mDebugWindowUpdateFreq==DEBUG_WINUPD_FRAME) lwin->OnDebuggerUpdate();
#endif

	// Now return a pointer to the painting buffer for the next frame
	if(lwin->mDisplayRender)
	{
		return lwin->mDisplayRender->BackBuffer();
	}
	else
	{
		return NULL;
	}
}

BEGIN_MESSAGE_MAP(CLynxWindow,CFrameWnd)
	//{{AFX_MSG_MAP(CLynxWindow)
#ifdef _LYNXDBG
	ON_COMMAND(IDM_DEBUG_DUMP,OnDumpMenuSelect)
	ON_COMMAND(IDM_DEBUG_CODE,OnCodeMenuSelect)
	ON_COMMAND(IDM_DEBUG_TRACE,OnTraceMenuSelect)
	ON_COMMAND(IDM_DEBUG_GRAPHICS,OnGraphicsMenuSelect)
	ON_COMMAND(IDM_DEBUG_RAMDUMP,OnRAMDumpMenuSelect)
	ON_COMMAND(IDM_DEBUG_STEP, OnStepMenuSelect)
	ON_COMMAND(IDM_DEBUG_DEBUGGER, OnDebuggerMenuSelect)
	// Dont delete me some of the debug windows send me messages
	ON_COMMAND(IDM_DRAW_DEBUG_WINDOWS,OnDebuggerUpdate)
#endif
	ON_COMMAND(IDM_OPTIONS_PAUSE, OnPauseMenuSelect)
	ON_UPDATE_COMMAND_UI(IDM_OPTIONS_PAUSE, OnPauseMenuUpdate)
	ON_COMMAND(IDM_OPTIONS_JOYSTICK, OnJoystickMenuSelect)
	ON_UPDATE_COMMAND_UI(IDM_OPTIONS_JOYSTICK, OnJoystickMenuUpdate)
	ON_COMMAND(IDM_OPTIONS_SOUND, OnSoundMenuSelect)
	ON_UPDATE_COMMAND_UI(IDM_OPTIONS_SOUND, OnSoundMenuUpdate)
	ON_COMMAND(IDM_HELP_INFO, OnInfoSelect)
	ON_UPDATE_COMMAND_UI(IDM_HELP_INFO,OnInfoMenuUpdate)
	ON_COMMAND(IDM_HELP_ABOUT, OnAboutBoxSelect)
	ON_COMMAND(IDM_OPTIONS_RESET, OnResetMenuSelect)
	ON_COMMAND(IDM_OPTIONS_BACKGROUND, OnBkgndMenuSelect)
	ON_UPDATE_COMMAND_UI(IDM_OPTIONS_BACKGROUND, OnBkgndMenuUpdate)
	ON_COMMAND(IDM_OPTIONS_KEYDEFS,OnDefineKeysSelect)
	//ON_MESSAGE(WM_NETOBJ_SELECT, OnNetworkDataWaiting)
	//ON_MESSAGE(WM_NETOBJ_UPDATE, OnNetworkUpdate)
	ON_COMMAND(IDM_OPTIONS_NETWORK, OnNetworkMenuSelect)
	ON_UPDATE_COMMAND_UI(IDM_OPTIONS_NETWORK, OnNetworkMenuUpdate)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(IDM_FILE_EXIT, OnFileExit)
	ON_COMMAND(IDM_FILE_LOAD, OnFileLoad)
	ON_COMMAND(IDM_FILE_SNAPSHOT_GAME, OnFileSnapshotGame)
	ON_COMMAND(IDM_FILE_SNAPSHOT_LOAD, OnFileSnapshotLoad)
	ON_COMMAND(IDM_FILE_SNAPSHOT_BMP, OnFileSnapshotBMP)
	ON_COMMAND(IDM_FILE_SNAPSHOT_RAW, OnFileSnapshotRAW)
	ON_COMMAND(IDM_ESCAPE_FULLSCREEN, OnDisplayEscapeFullScreen)
	ON_COMMAND(IDM_OPTIONS_DISPLAY_FULLSCN, OnDisplayToggleFullScreen)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_ACTIVATEAPP()
	ON_WM_INITMENU()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(IDM_OPTIONS_DISPLAY_MODE_1,IDM_OPTIONS_DISPLAY_MODE_7,OnDisplayModeSelect)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_OPTIONS_DISPLAY_MODE_1,IDM_OPTIONS_DISPLAY_MODE_7,OnDisplayModeUpdate)
	ON_COMMAND_RANGE(IDM_OPTIONS_SIZE_NORMAL,IDM_OPTIONS_SIZE_QUAD, OnDisplayZoomSelect)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_OPTIONS_SIZE_NORMAL,IDM_OPTIONS_SIZE_QUAD,OnDisplayZoomUpdate)
	ON_COMMAND_RANGE(IDM_OPTIONS_ROTATE_LEFT,IDM_OPTIONS_ROTATE_RIGHT,OnDisplayRotateSelect)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_OPTIONS_ROTATE_LEFT,IDM_OPTIONS_ROTATE_RIGHT,OnDisplayRotateUpdate)
	ON_COMMAND_RANGE(IDM_OPTIONS_BACKGROUNDTYPE1,IDM_OPTIONS_BACKGROUNDTYPE3,OnDisplayBackgroundTypeSelect)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_OPTIONS_BACKGROUNDTYPE1,IDM_OPTIONS_BACKGROUNDTYPE3,OnDisplayBackgroundTypeUpdate)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLynxWindow message handlers


inline void CLynxWindow::OnPaint()
{
	CPaintDC dc (this);
//	BeginPaint(NULL);

	// Dont attempt to paint if the Lynx object is not constructed, this
	// can happen on return from full screen with Ctrl-O to open a new file
	// caused a repaint between the delete and create calls.
	if(mDisplayNoPainting==TRUE || mDisplayRender==NULL || mpLynx==NULL) return;

	try
	{
		mDisplayRender->Render(mDisplayOffsetX,mDisplayOffsetY);
	}
	catch (_com_error& ex)
	{
		CString message ("CLynxWindow::OnPaint() - Rendering Object Error\n");
	    message += DXAppErrorToString (ex.Error());
		gError->Fatal(message);
	}
//	EndPaint(NULL);
}

void CLynxWindow::OnTimer(UINT nIDEvent)
{
	// Abort if no valid Lynx or Display
	if(mpLynx==NULL || mDisplayRender==NULL) return;

	if(nIDEvent==HANDY_INFO_TIMER)
	{
		// TEST TEST TEST
//		if(mpNetLynx!=NULL)
//		{
//			char info[40];
//			CDC* fullscnCDC;
//			HDC fullscnHDC;
//			IDirectDrawSurfacePtr surfDesc;
//			surfDesc=((CFullScreenDirectX*)mDisplayRender)->GetFrontSurface();
//			if(surfDesc)
//			{
//				surfDesc->GetDC(&fullscnHDC);
//				fullscnCDC = CDC::FromHandle (fullscnHDC);
//				sprintf(info,"RX=%08d TX=%08d ST=%d DAT=%04x",mpNetLynx->mRXcount,mpNetLynx->mTXcount,mpNetLynx->mState,mpNetLynx->mLastRxData);
//				TextOut(fullscnHDC,0,0,info,strlen(info));
//				surfDesc->ReleaseDC(fullscnHDC);
//			}
//		}
		// TEST TEST TEST

		//
		// Display speed information if enabled
		//
		if(DisplayModeWindowed())
		{
			CString text;
			text.Format("%d",mFramesPerSecond);
			CEdit&	ctlfps=*(CEdit*)mInfoDialog.GetDlgItem(IDC_STATUS_FPS);
			ctlfps.SetWindowText(text);

			text.Format("%d",mEmulationSpeed);
			CEdit&	ctleff=*(CEdit*)mInfoDialog.GetDlgItem(IDC_STATUS_EFF);
			ctleff.SetWindowText(text);

			text.Format("%d",mFrameSkip);
			CEdit&	ctlskip=*(CEdit*)mInfoDialog.GetDlgItem(IDC_STATUS_SKIP);
			ctlskip.SetWindowText(text);

			CSpinButtonCtrl& ctlspeed=*(CSpinButtonCtrl*)mInfoDialog.GetDlgItem(IDC_STATUS_SPIN1);
			gThrottleMaxPercentage=ctlspeed.GetPos();
		}
		else
		{
			char info[40];
			CDC* fullscnCDC;
			HDC fullscnHDC;
			IDirectDrawSurfacePtr surfDesc;
			surfDesc=((CFullScreenDirectX*)mDisplayRender)->GetFrontSurface();
			if(surfDesc)
			{
				surfDesc->GetDC(&fullscnHDC);
				fullscnCDC = CDC::FromHandle (fullscnHDC);
				sprintf(info,"%%=%03d F=%03d FSkip=%01d",mEmulationSpeed,mFramesPerSecond,mFrameSkip);
				TextOut(fullscnHDC,0,0,info,strlen(info));
				surfDesc->ReleaseDC(fullscnHDC);
			}
		}
	}
	else if(nIDEvent==HANDY_JOYSTICK_TIMER)
	{
		//
		// Check joystick buttons in sync with the timer
		//
		static ULONG		lynx_buttons;
		static JOYINFOEX	joyinfo;

		// Check buttons
		joyinfo.dwSize=sizeof(JOYINFOEX);
		joyinfo.dwFlags=JOY_RETURNBUTTONS|JOY_RETURNX|JOY_RETURNY;
		if(joyGetPosEx(JOYSTICKID1,&joyinfo)!=JOYERR_NOERROR)
		{
			// This will disable the joystick
			OnJoystickMenuSelect();
			// Warn the user
			gError->Warning("Joystick Error:\nDirectInput error reading status, joystick will be disabled.");
		}
		else
		{
			// Get current button state
			lynx_buttons=mpLynx->GetButtonData();

			// Button A
			if(joyinfo.dwButtons&JOY_BUTTON1) lynx_buttons|=BUTTON_B; else lynx_buttons&=BUTTON_B^0xffffffff;
			// Button B
			if(joyinfo.dwButtons&JOY_BUTTON2) lynx_buttons|=BUTTON_A; else lynx_buttons&=BUTTON_A^0xffffffff;
			// Option 1
			if(joyinfo.dwButtons&JOY_BUTTON3) lynx_buttons|=BUTTON_OPT1; else lynx_buttons&=BUTTON_OPT1^0xffffffff;
			// Option 2
			if(joyinfo.dwButtons&JOY_BUTTON4) lynx_buttons|=BUTTON_OPT2; else lynx_buttons&=BUTTON_OPT2^0xffffffff;

			// Read axis's
			if(joyinfo.dwYpos<mJoystickYDown)
			{
				switch(DisplayModeRotate())
				{
					case DISPLAY_ROTATE_LEFT:
						lynx_buttons|=BUTTON_LEFT;
						lynx_buttons&=BUTTON_RIGHT^0xffffffff;
						break;
					case DISPLAY_ROTATE_RIGHT:
						lynx_buttons|=BUTTON_RIGHT;
						lynx_buttons&=BUTTON_LEFT^0xffffffff;
						break;
					case DISPLAY_NO_ROTATE:
					default:
						lynx_buttons|=BUTTON_UP;
						lynx_buttons&=BUTTON_DOWN^0xffffffff;
						break;
				}
			}
			else if(joyinfo.dwYpos>mJoystickYUp)
			{
				switch(DisplayModeRotate())
				{
					case DISPLAY_ROTATE_LEFT:
						lynx_buttons|=BUTTON_RIGHT;
						lynx_buttons&=BUTTON_LEFT^0xffffffff;
						break;
					case DISPLAY_ROTATE_RIGHT:
						lynx_buttons|=BUTTON_LEFT;
						lynx_buttons&=BUTTON_RIGHT^0xffffffff;
						break;
					case DISPLAY_NO_ROTATE:
					default:
						lynx_buttons|=BUTTON_DOWN;
						lynx_buttons&=BUTTON_UP^0xffffffff;
						break;
				}
			}
			else
			{
				switch(DisplayModeRotate())
				{
					case DISPLAY_ROTATE_LEFT:
					case DISPLAY_ROTATE_RIGHT:
						lynx_buttons&=BUTTON_LEFT^0xffffffff;
						lynx_buttons&=BUTTON_RIGHT^0xffffffff;
						break;
					case DISPLAY_NO_ROTATE:
					default:
						lynx_buttons&=BUTTON_UP^0xffffffff;
						lynx_buttons&=BUTTON_DOWN^0xffffffff;
						break;
				}
			}

			if(joyinfo.dwXpos>mJoystickXUp)
			{
				switch(DisplayModeRotate())
				{
					case DISPLAY_ROTATE_LEFT:
						lynx_buttons|=BUTTON_UP;
						lynx_buttons&=BUTTON_DOWN^0xffffffff;
						break;
					case DISPLAY_ROTATE_RIGHT:
						lynx_buttons|=BUTTON_DOWN;
						lynx_buttons&=BUTTON_UP^0xffffffff;
						break;
					case DISPLAY_NO_ROTATE:
					default:
						lynx_buttons|=BUTTON_RIGHT;
						lynx_buttons&=BUTTON_LEFT^0xffffffff;
						break;
				}
			}
			else if(joyinfo.dwXpos<mJoystickXDown)
			{
				switch(DisplayModeRotate())
				{
					case DISPLAY_ROTATE_LEFT:
						lynx_buttons|=BUTTON_DOWN;
						lynx_buttons&=BUTTON_UP^0xffffffff;
						break;
					case DISPLAY_ROTATE_RIGHT:
						lynx_buttons|=BUTTON_UP;
						lynx_buttons&=BUTTON_DOWN^0xffffffff;
						break;
					case DISPLAY_NO_ROTATE:
					default:
						lynx_buttons|=BUTTON_LEFT;
						lynx_buttons&=BUTTON_RIGHT^0xffffffff;
						break;
				}
			}
			else
			{
				switch(DisplayModeRotate())
				{
					case DISPLAY_ROTATE_LEFT:
					case DISPLAY_ROTATE_RIGHT:
						lynx_buttons&=BUTTON_UP^0xffffffff;
						lynx_buttons&=BUTTON_DOWN^0xffffffff;
						break;
					case DISPLAY_NO_ROTATE:
					default:
						lynx_buttons&=BUTTON_LEFT^0xffffffff;
						lynx_buttons&=BUTTON_RIGHT^0xffffffff;
						break;
				}
			}
			// Send the button data back again
			mpLynx->SetButtonData(lynx_buttons);
		}
	}
}

void CLynxWindow::OnBkgndMenuSelect()
{
	if(DisplayModeWindowed())
	{
		if(DisplayModeBkgnd()==DISPLAY_BKGND)
		{
			DisplayModeSet(DISPLAY_PRESERVE,DISPLAY_NO_BKGND,DISPLAY_PRESERVE,DISPLAY_PRESERVE);
		}
		else
		{
			DisplayModeSet(DISPLAY_PRESERVE,DISPLAY_BKGND,DISPLAY_PRESERVE,DISPLAY_PRESERVE);
		}
	}
}

void CLynxWindow::OnBkgndMenuUpdate(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck((DisplayModeBkgnd()==DISPLAY_BKGND)?TRUE:FALSE);
}

void CLynxWindow::OnNetworkUpdate(WPARAM wparam,LPARAM lparam)
{
	if(mpNetLynx) mpNetLynx->Update();
}

void CLynxWindow::OnNetworkDataWaiting(WPARAM wparam,LPARAM lparam)
{
	if(mpNetLynx)
	{
		// Winsock has signalled there to be a byte waiting to be received
		if(lparam==FD_READ && mpNetLynx->Active())
		{
			int rxbyte=mpNetLynx->Receive();
			// Data bytes have the upper byte==0, otherwise its a command
			if(!(rxbyte&0xff00) || rxbyte==HANDY_BREAK_MESSAGE) mpLynx->ComLynxRxData(rxbyte);
			TRACE_LYNXWIN1("OnNetworkDataWaiting() - Received %04x from CNetObj, given to Lynx",rxbyte);
		}
		else
		{
			gError->Warning("CLynxWindow::OnNetworkDataWaiting() - Network not active or lparam!=FD_READ");
		}
	}
	else
	{
//		gError->Warning("CLynxWindow::OnNetworkDataWaiting() - NULL mpNetLynx!!! Must be ghosts...");
		TRACE_LYNXWIN0("OnNetworkDataWaiting() - No valid net object");
	}
}

void CLynxWindow::OnNetworkMenuSelect()
{
	static bool firsttime=0;

	if(!firsttime)
	{
		gError->Warning("NetLynx code is currently not functional for commercial games, it will work in "
						"so long as both sides dont transmit at the same time. The code needs to be rewritten"
						"to solve the problem\n\nShould be fixed for the next release...");
		firsttime=1;
	}

	if(!mpNetLynx)
	{
		TRACE_LYNXWIN0("OnNetworkMenuSelect() - Created CNetObj");
		mpNetLynx= new CNetObj;
		// Fix the state of the Lynx
		mpLynx->ComLynxCable(FALSE);
		mpLynx->ComLynxTxCallback(NULL,NULL);
	}

	// Fetch the network address for the user

	if(mpNetLynx->InitOK()==FALSE)
	{
		TRACE_LYNXWIN0("OnNetworkMenuSelect() - CNetObj InitOK failed, deleting");
		delete mpNetLynx;
		mpNetLynx=NULL;
		gError->Warning("CNetObj failed to initialise - Deleted it.");
	}
	else
	{
		if(mpNetLynx->Active())
		{
			TRACE_LYNXWIN0("OnNetworkMenuSelect() - CNetObj Active - Calling STOP");
			mpNetLynx->Stop();
			// Fix the state of the Lynx
			mpLynx->ComLynxCable(FALSE);
			mpLynx->ComLynxTxCallback(NULL,NULL);
		}
		else
		{
			TRACE_LYNXWIN0("OnNetworkMenuSelect() - CNetObj InActive - Calling START");
			if(mpNetLynx->Start(mpLynxApp->GetProfileString(REGISTRY_VERSION,"NetLynxAddress",""))==TRUE)
			{
				TRACE_LYNXWIN0("OnNetworkMenuSelect() - CNetObj Start suceeded");
				mpLynxApp->WriteProfileString(REGISTRY_VERSION,"NetLynxAddress",mpNetLynx->mNetAddress);
				// Fix the state of the Lynx
				mpLynx->ComLynxCable(TRUE);
				mpLynx->ComLynxTxCallback(NetworkTxCallback,(ULONG)this);
			}
			else
			{
				TRACE_LYNXWIN0("OnNetworkMenuSelect() - CNetObj Start failed");
			}
		}
	}
}

void CLynxWindow::OnNetworkMenuUpdate(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck((mpNetLynx && mpNetLynx->mState==NETOBJ_ACTIVE)?TRUE:FALSE);
}

void CLynxWindow::OnFileExit()
{
	SendMessage(WM_CLOSE,0,0);
}

void CLynxWindow::OnFileLoad()
{
	if(mCommandLineMode) return;

	// If full screen then make windowed
	if(!DisplayModeWindowed())
	{
		OnDisplayToggleFullScreen();
	}

	// If networking is active then disable it
	if(mpNetLynx && mpNetLynx->Active())
	{
		TRACE_LYNXWIN0("OnFileLoad() - CNetObj Active - Calling STOP");
		mpNetLynx->Stop();
		// Fix the state of the Lynx
		mpLynx->ComLynxCable(FALSE);
		mpLynx->ComLynxTxCallback(NULL,NULL);
	}

	// Delete the lynx object
	if(mpLynx!=NULL)
	{
		delete mpLynx;
		mpLynx=NULL;
	}

	// Try and create a new lynx object

	if((mpLynx=CreateLynx(""))==NULL)
	{
		gError->Fatal("CLynxWindow::CreateLynx() returned failure message, aborting");
	}
	else
	{
		// Now the object has been sucessfully recreated then we must re-initialise
		// the graphics subsystem, while we're doing it we can set the mode level
		// up accordingly.

		ULONG rotate=DISPLAY_NO_ROTATE;
		if(mpLynx->CartGetRotate()==CART_ROTATE_LEFT) rotate=DISPLAY_ROTATE_LEFT;
		if(mpLynx->CartGetRotate()==CART_ROTATE_RIGHT) rotate=DISPLAY_ROTATE_RIGHT;

		// Refresh the video mode to set memory pointers
		DisplayModeSet(DISPLAY_PRESERVE,DISPLAY_PRESERVE,DISPLAY_PRESERVE,rotate);
		OnDebuggerUpdate();
	}
}

void CLynxWindow::OnDropFiles(HDROP hDrop)
{
	CString newgame;
	char temp[255];

	// Retrive the filename from the handle
	if(!DragQueryFile(hDrop,0,temp,255)) return;
	newgame=temp;

	if(mCommandLineMode) return;

	// If full screen then make windowed
	if(!DisplayModeWindowed())
	{
		OnDisplayToggleFullScreen();
	}

	// If networking is active then disable it
	if(mpNetLynx && mpNetLynx->Active())
	{
		TRACE_LYNXWIN0("OnFileLoad() - CNetObj Active - Calling STOP");
		mpNetLynx->Stop();
		// Fix the state of the Lynx
		mpLynx->ComLynxCable(FALSE);
		mpLynx->ComLynxTxCallback(NULL,NULL);
	}

	// Delete the lynx object
	if(mpLynx!=NULL)
	{
		delete mpLynx;
		mpLynx=NULL;
	}

	// Try and create a new lynx object

	if((mpLynx=CreateLynx(newgame))==NULL)
	{
		gError->Fatal("CLynxWindow::CreateLynx() returned failure message, aborting");
	}
	else
	{
		// Now the object has been sucessfully recreated then we must re-initialise
		// the graphics subsystem, while we're doing it we can set the mode level
		// up accordingly.

		ULONG rotate=DISPLAY_NO_ROTATE;
		if(mpLynx->CartGetRotate()==CART_ROTATE_LEFT) rotate=DISPLAY_ROTATE_LEFT;
		if(mpLynx->CartGetRotate()==CART_ROTATE_RIGHT) rotate=DISPLAY_ROTATE_RIGHT;

		// Refresh the video mode to set memory pointers
		DisplayModeSet(DISPLAY_PRESERVE,DISPLAY_PRESERVE,DISPLAY_PRESERVE,rotate);
		OnDebuggerUpdate();
	}
}

void CLynxWindow::OnFileSnapshotGame()
{
	if(mCommandLineMode) return;

	// If full screen then make windowed
	if(!DisplayModeWindowed())
	{
		OnDisplayToggleFullScreen();
	}

	// Open a save dialog

	CString context;
	CString filter="Lynx SnapShots (*.lss)|*.lss|All Files (*.*)|*.*||";
	CFileDialog	dlg(FALSE,"lss",NULL,OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,filter,NULL);
	if(dlg.DoModal()==IDCANCEL) return;
	context=dlg.GetPathName();

	// Save the context
	if(!mpLynx->ContextSave((char*)LPCTSTR(context)))
	{
		gError->Warning("An error occured during save, please delete the snapshot.");
	}
}

void CLynxWindow::OnFileSnapshotLoad()
{
	if(mCommandLineMode) return;

	// If full screen then make windowed
	if(!DisplayModeWindowed())
	{
		OnDisplayToggleFullScreen();
	}

	// Open a save dialog

	CString context;
	CString filter="Lynx SnapShots (*.lss;*.zip)|*.lss;*.zip|All Files (*.*)|*.*||";
	CFileDialog	dlg(TRUE,"lss",NULL,OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,filter,NULL);
	if(dlg.DoModal()==IDCANCEL) return;
	context=dlg.GetPathName();

	// Save the context
	if(context!="")
	{
		if(!mpLynx->ContextLoad((char*)LPCTSTR(context)))
		{
			gError->Warning("An error occured whilst trying to load the snapshot");
		}
	}
}

void CLynxWindow::OnFileSnapshotBMP()
{
	static int	snapnumber=0;
	CFile file;
	CString snapname;

	while(snapnumber<100)
	{
		snapname.Format("snap%02d.bmp",snapnumber++);
		snapname=mRootPath+snapname;

		if(!file.Open(snapname,CFile::modeRead))
		{
			try
			{
				BITMAPFILEHEADER header;
				BITMAPINFOHEADER info;
				BITMAP bmap;
				CBitmap client_bitmap;
				CRect clrect;

				// Create relevant device contexts
				CDC *wincdc = new CClientDC(this);
				CDC *memcdc= new CDC;
				memcdc->CreateCompatibleDC(wincdc);

				// Get bitmap size details
				GetClientRect(&clrect);

				BITMAPINFO *bi;
				bi= (BITMAPINFO*) new UBYTE[sizeof(BITMAPINFO)];
				ZeroMemory(bi, sizeof(BITMAPINFO));
				bi->bmiHeader.biBitCount = 16;
				bi->bmiHeader.biSizeImage = (clrect.Width()*clrect.Height())*2;
				bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				bi->bmiHeader.biPlanes = 1;
				bi->bmiHeader.biWidth = clrect.Width();
				bi->bmiHeader.biHeight = clrect.Height();
				bi->bmiHeader.biCompression = BI_RGB;

				HBITMAP hBitmap;
				if((hBitmap=CreateDIBSection(wincdc->m_hDC,bi,DIB_RGB_COLORS,NULL,NULL,0))==NULL)
				{
					gError->Warning("OnFileSnapshotBMP() - CreateDIBSection() failed");
					delete wincdc;
					delete memcdc;
					delete bi;
					return;
				}

				// Attach the DIB to the bitmap
				client_bitmap.Attach(hBitmap);

				// Select it into the CDC
				memcdc->SelectObject(&client_bitmap);
			
				// Blit the client window into the memory CDC i.e our dibsection
				if(!memcdc->BitBlt(0,0,clrect.Width(),clrect.Height(),wincdc,0,0,SRCCOPY))
				{
					gError->Warning("OnFileSnapshotBMP() - BitBlt() failed");
					delete memcdc;
					delete wincdc;
					delete bi;
					return;
				}
 
				// Grab the details back again
				if(!client_bitmap.GetBitmap(&bmap))
				{
					gError->Warning("OnFileSnapshotBMP() - GetBitmap() failed");
					delete memcdc;
					delete wincdc;
					delete bi;
					return;
				}
	
				// Now fill in the header details
				header.bfType=0x4d42;
				header.bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+(bmap.bmWidthBytes*bmap.bmHeight);
				header.bfReserved1=0;
				header.bfReserved2=0;
				header.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);

				info.biSize=sizeof(BITMAPINFOHEADER); 
				info.biWidth=bmap.bmWidth; 
				info.biHeight=bmap.bmHeight; 
				info.biPlanes=bmap.bmPlanes; 
				info.biBitCount=bmap.bmBitsPixel;
				info.biCompression=BI_RGB; 
				info.biSizeImage=0;
				info.biXPelsPerMeter=0; 
				info.biYPelsPerMeter=0; 
				info.biClrUsed=0; 
				info.biClrImportant=0;

				file.Open(snapname,CFile::modeCreate|CFile::shareExclusive|CFile::modeWrite);
				file.Write((void*)&header,sizeof(BITMAPFILEHEADER));
				file.Write((void*)&info,sizeof(BITMAPINFOHEADER));
				file.Write((void*)bmap.bmBits,+(bmap.bmWidthBytes*bmap.bmHeight));
				file.Close();

				// Tidy up and delete any created objects
				delete memcdc;
				delete wincdc;
				delete bi;
			}
			catch(CFileException *filerr)
			{
				filerr->Delete();
				gError->Warning("OnFileSnapshotBMP() - CFileException, Snapshot could not be written ???");
			}
			break;
		}
	}
	if(snapnumber==100)	gError->Warning("Maximum snapshot number reached, please delete some");
}

void CLynxWindow::OnFileSnapshotRAW()
{
	static int	snapnumber=0;
	CFile file;
	CString snapname;

	while(snapnumber<100)
	{
		snapname.Format("snap%02d.raw",snapnumber++);
		snapname=mRootPath+snapname;

		if(!file.Open(snapname,CFile::modeRead))
		{
			try
			{
				UBYTE memmap;
				UBYTE image[8192];
				ULONG index=0;
                ULONG loop = 0;

				file.Open(snapname,CFile::modeCreate|CFile::shareExclusive|CFile::modeWrite);

				// Page mikey in and save memory map status
				memmap=mpLynx->Peek_CPU(0xfff9);
				mpLynx->Poke_CPU(0xfff9,0);
				// Dump the palette to the index
				for(loop=GREEN0;loop<=BLUEREDF;loop++)
				{
					image[index++]=(UBYTE)mpLynx->Peek_CPU(loop);
				}
				// Read the location of the screen buffer
				ULONG start=mpLynx->PeekW_CPU(DISPADR);
				ULONG end=mpLynx->PeekW_CPU(DISPADR)+8160;
				// Dump the buffer to the image
				for(loop=start;loop<end;loop++)
				{
					image[index++]=(UBYTE)mpLynx->Peek_RAM(loop);
				}
				// Restore memory map
				mpLynx->Poke_CPU(0xfff9,memmap);
				// Write the memory image in one swoop
				file.Write((void*)image,8192);
				// Finished, close the file
				file.Close();
			}
			catch(CFileException *filerr)
			{
				filerr->Delete();
				gError->Warning("Snapshot could not be written ???");
			}
			break;
		}
	}
	if(snapnumber==100)	gError->Warning("Maximum snapshot number reached, please delete some");
}

void CLynxWindow::OnContextMenu(CWnd *pWnd, CPoint point)
{
	if(DisplayModeWindowed())
	{
		CRect rect;
		GetClientRect(&rect);
		ClientToScreen(&rect);

		if(rect.PtInRect(point))
		{
			CMenu menu;
			menu.LoadMenu(IDR_MAINFRAME_CONTEXT_MENU);
			CMenu *pContextMenu=menu.GetSubMenu(0);
			pContextMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,point.x,point.y,this);
			return;
		}
		CFrameWnd::OnContextMenu(pWnd,point);
	}
}

void CLynxWindow::OnDefineKeysSelect()
{
	CKeyDefs keydefdlg(this,&mKeyDefs);
	keydefdlg.DoModal();
}

void CLynxWindow::OnJoystickMenuSelect()
{
	if(mJoystickEnable)
	{
		KillTimer(mJoystickEnable);
		mJoystickEnable=0;
	}
	else
	{
		JOYINFOEX	joyinfo;
		MMRESULT	mr;

		memset(&joyinfo,0,sizeof(JOYINFOEX));
		joyinfo.dwFlags=JOY_RETURNALL;
		joyinfo.dwSize=sizeof(JOYINFOEX);
		mr=joyGetPosEx(JOYSTICKID1,&joyinfo);
		
		if(mr!=JOYERR_NOERROR)
		{
			mJoystickEnable=FALSE;

			switch(mr)
			{
				case MMSYSERR_NODRIVER:
					gError->Warning("Joystick Error:\nThe joystick driver is not present.");
					break;
				case MMSYSERR_INVALPARAM:
				case JOYERR_PARMS:
					gError->Warning("Joystick Error:\nAn invalid parameter was passed. Most likely no joystick is installed.");
					break;
				case MMSYSERR_BADDEVICEID:
					gError->Warning("Joystick Error:\nThe specified joystick identifier (JOYSTICKID1) is invalid.");
					break;
				case JOYERR_UNPLUGGED:
					gError->Warning("Joystick Error:\nThe specified joystick (JOYSTICKID1) is not connected to the system.");
					break;
				case JOYERR_NOCANDO:
					gError->Warning("Joystick Error:\nThe specified joystick request could not be completed.");
					break;
				default:
					gError->Warning("Joystick Error:\nUnknown joystick error returned");
					break;
			}
		}
		else
		{
			// Check the joystick caps
			JOYCAPS jcaps;

			if(joyGetDevCaps(JOYSTICKID1,&jcaps,sizeof(JOYCAPS))!=JOYERR_NOERROR)
			{
				gError->Warning("Joystick Error:\nCouldn't read joystick capabilities ?? Joystick disabled.");
			}
			else
			{
				ULONG joymidx=jcaps.wXmin+((jcaps.wXmax-jcaps.wXmin)/2);
				ULONG joyrangex=jcaps.wXmax-jcaps.wXmin;
				ULONG joymidy=jcaps.wYmin+((jcaps.wYmax-jcaps.wYmin)/2);
				ULONG joyrangey=jcaps.wYmax-jcaps.wYmin;

				// Set joystick trigger thresholds 25%
				mJoystickXUp=joymidx+(joyrangex/4);
				mJoystickXDown=joymidx-(joyrangex/4);
				mJoystickYUp=joymidy+(joyrangey/4);
				mJoystickYDown=joymidy-(joyrangey/4);

				// Enable joystick timer messages
				mJoystickEnable=SetTimer(HANDY_JOYSTICK_TIMER,HANDY_JOYSTICK_TIMER_PERIOD,NULL);
			}
		}
	}
}

void CLynxWindow::OnJoystickMenuUpdate(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(mJoystickEnable);
}

void CLynxWindow::OnSoundMenuSelect()
{
	if(mDirectSoundPlayer.GetStatus())
	{
		gAudioEnabled=mDirectSoundPlayer.Stop();
	}
	else
	{
		gAudioEnabled=mDirectSoundPlayer.Start();
	}
}

void CLynxWindow::OnSoundMenuUpdate(CCmdUI *pCmdUI)
{
	gAudioEnabled=mDirectSoundPlayer.GetStatus();
	pCmdUI->SetCheck(gAudioEnabled);
}

void CLynxWindow::OnResetMenuSelect()
{
	mpLynx->Reset();
	OnDebuggerUpdate();
	Invalidate(FALSE);
}

void CLynxWindow::OnPauseMenuSelect()
{
	if(!gSystemHalt)
	{
		gBreakpointHit=TRUE;	// This will force a window redraw in the debug version
		gSystemHalt=TRUE;		// This will stop the system
	}
	else
	{
		gSystemHalt=FALSE;		// Make sure we run always
		gSingleStepMode=FALSE;	// Go for it....
	}
	OnDebuggerUpdate();
	Invalidate(FALSE);
}

void CLynxWindow::OnPauseMenuUpdate(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(gSystemHalt);
}

void CLynxWindow::OnDisplayRotateSelect(UINT nID)
{
	ULONG rotate=DISPLAY_NO_ROTATE;

	switch(nID)
	{
		case IDM_OPTIONS_ROTATE_LEFT:
			switch(DisplayModeRotate())
			{
				case DISPLAY_NO_ROTATE:
					rotate=DISPLAY_ROTATE_LEFT;
					break;
				case DISPLAY_ROTATE_LEFT:
					rotate=DISPLAY_NO_ROTATE;
					break;
				case DISPLAY_ROTATE_RIGHT:
					rotate=DISPLAY_ROTATE_LEFT;
					break;
				default:
					break;
			}
			break;
		case IDM_OPTIONS_ROTATE_RIGHT:
			switch(DisplayModeRotate())
			{
				case DISPLAY_NO_ROTATE:
					rotate=DISPLAY_ROTATE_RIGHT;
					break;
				case DISPLAY_ROTATE_LEFT:
					rotate=DISPLAY_ROTATE_RIGHT;
					break;
				case DISPLAY_ROTATE_RIGHT:
					rotate=DISPLAY_NO_ROTATE;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	DisplayModeSet(DISPLAY_PRESERVE,DISPLAY_PRESERVE,DISPLAY_PRESERVE,rotate);
}

void CLynxWindow::OnDisplayRotateUpdate(CCmdUI *pCmdUI)
{
	ULONG check;
	switch(pCmdUI->m_nID)
	{
		case IDM_OPTIONS_ROTATE_LEFT:
			check=DISPLAY_ROTATE_LEFT;
			break;
		case IDM_OPTIONS_ROTATE_RIGHT:
			check=DISPLAY_ROTATE_RIGHT;
			break;
		default:
			check=DISPLAY_NO_ROTATE;
			break;
	}
	pCmdUI->SetCheck(DisplayModeRotate()==check);
}

void CLynxWindow::OnDisplayModeUpdate(CCmdUI *pCmdUI)
{
	ULONG check;
	switch(pCmdUI->m_nID)
	{
		case IDM_OPTIONS_DISPLAY_MODE_1:
			check=DISPLAY_WINDOWED;
			break;
		case IDM_OPTIONS_DISPLAY_MODE_2:
			check=DISPLAY_FULLSCREEN;
			break;
		case IDM_OPTIONS_DISPLAY_MODE_3:
			check=DISPLAY_LYNXLCD_WINDOWED;
			break;
		case IDM_OPTIONS_DISPLAY_MODE_4:
			check=DISPLAY_LYNXLCD_FULLSCREEN;
			break;
		case IDM_OPTIONS_DISPLAY_MODE_5:
			check=DISPLAY_EAGLE_WINDOWED;
			break;
		case IDM_OPTIONS_DISPLAY_MODE_6:
			check=DISPLAY_EAGLE_FULLSCREEN;
			break;
		case IDM_OPTIONS_DISPLAY_MODE_7:
			check=DISPLAY_GDI_WINDOWED;
			break;
		default:
			check=DISPLAY_RENDER_DEFAULT;
			break;
	}

	pCmdUI->SetCheck(DisplayModeRender()==check);
}

void CLynxWindow::OnDisplayModeSelect(UINT nID)
{
	int mode=DISPLAY_RENDER_DEFAULT;

	switch(nID)
	{
		case IDM_OPTIONS_DISPLAY_MODE_1:
			mode=DISPLAY_WINDOWED;
			break;
		case IDM_OPTIONS_DISPLAY_MODE_2:
			mode=DISPLAY_FULLSCREEN;
			break;
		case IDM_OPTIONS_DISPLAY_MODE_3:
			mode=DISPLAY_LYNXLCD_WINDOWED;
			break;
		case IDM_OPTIONS_DISPLAY_MODE_4:
			mode=DISPLAY_LYNXLCD_FULLSCREEN;
			break;
		case IDM_OPTIONS_DISPLAY_MODE_5:
			mode=DISPLAY_EAGLE_WINDOWED;
			break;
		case IDM_OPTIONS_DISPLAY_MODE_6:
			mode=DISPLAY_EAGLE_FULLSCREEN;
			break;
		case IDM_OPTIONS_DISPLAY_MODE_7:
			mode=DISPLAY_GDI_WINDOWED;
			break;
		default:
			mode=DISPLAY_RENDER_DEFAULT;
			break;
	}

	DisplayModeSet(mode,DISPLAY_PRESERVE,DISPLAY_PRESERVE,DISPLAY_PRESERVE);
}

void CLynxWindow::OnDisplayEscapeFullScreen()
{
	// If we are in command line mode ESC will exit regardless
	if(mCommandLineMode)
	{
		SendMessage(WM_CLOSE);
	}
	else if(!DisplayModeWindowed())
	{
		// Exit full screen by inverting the rendering type
		int newrender;
		switch(DisplayModeRender())
		{
			case DISPLAY_LYNXLCD_FULLSCREEN:
				newrender=DISPLAY_LYNXLCD_WINDOWED;
				break;
			case DISPLAY_EAGLE_FULLSCREEN:
				newrender=DISPLAY_EAGLE_WINDOWED;
				break;
			case DISPLAY_FULLSCREEN:
			default:
				newrender=DISPLAY_WINDOWED;
				break;
		}
		DisplayModeSet(newrender,DISPLAY_PRESERVE,DISPLAY_PRESERVE,DISPLAY_PRESERVE);
	}


}

void CLynxWindow::OnDisplayToggleFullScreen()
{
	if(mCommandLineMode) return;

	if(DisplayModeWindowed())
	{
		// Enter full screen mode
		int newrender;
		switch(DisplayModeRender())
		{
			case DISPLAY_LYNXLCD_WINDOWED:
				newrender=DISPLAY_LYNXLCD_FULLSCREEN;
				break;
			case DISPLAY_EAGLE_WINDOWED:
				newrender=DISPLAY_EAGLE_FULLSCREEN;
				break;
			case DISPLAY_GDI_WINDOWED:
			case DISPLAY_WINDOWED:
			default:
				newrender=DISPLAY_FULLSCREEN;
				break;
		}
		DisplayModeSet(newrender,DISPLAY_PRESERVE,DISPLAY_PRESERVE,DISPLAY_PRESERVE);
	}
	else
	{
		// Exit full screen by inverting the rendering type
		int newrender;
		switch(DisplayModeRender())
		{
			case DISPLAY_LYNXLCD_FULLSCREEN:
				newrender=DISPLAY_LYNXLCD_WINDOWED;
				break;
			case DISPLAY_EAGLE_FULLSCREEN:
				newrender=DISPLAY_EAGLE_WINDOWED;
				break;
			case DISPLAY_FULLSCREEN:
			default:
				newrender=DISPLAY_WINDOWED;
				break;
		}
		DisplayModeSet(newrender,DISPLAY_PRESERVE,DISPLAY_PRESERVE,DISPLAY_PRESERVE);
	}

}

void CLynxWindow::OnDisplayZoomSelect(UINT nID)
{
	switch(nID)
	{
		case IDM_OPTIONS_SIZE_QUAD:
			DisplayModeSet(DISPLAY_PRESERVE,DISPLAY_PRESERVE,DISPLAY_X4,DISPLAY_PRESERVE);
			break;
		case IDM_OPTIONS_SIZE_TRIPLE:
			DisplayModeSet(DISPLAY_PRESERVE,DISPLAY_PRESERVE,DISPLAY_X3,DISPLAY_PRESERVE);
			break;
		case IDM_OPTIONS_SIZE_DOUBLE:
			DisplayModeSet(DISPLAY_PRESERVE,DISPLAY_PRESERVE,DISPLAY_X2,DISPLAY_PRESERVE);
			break;
		case IDM_OPTIONS_SIZE_NORMAL:
		default:
			DisplayModeSet(DISPLAY_PRESERVE,DISPLAY_PRESERVE,DISPLAY_X1,DISPLAY_PRESERVE);
			break;
	}
}

void CLynxWindow::OnDisplayZoomUpdate(CCmdUI *pCmdUI)
{
	ULONG check;
	switch(pCmdUI->m_nID)
	{
		case IDM_OPTIONS_SIZE_QUAD:
			check=DISPLAY_X4;
			break;
		case IDM_OPTIONS_SIZE_TRIPLE:
			check=DISPLAY_X3;
			break;
		case IDM_OPTIONS_SIZE_DOUBLE:
			check=DISPLAY_X2;
			break;
		case IDM_OPTIONS_SIZE_NORMAL:
		default:
			check=DISPLAY_X1;
			break;
	}
	pCmdUI->SetCheck(DisplayModeMagnification()==check);
}

void CLynxWindow::OnDisplayBackgroundTypeSelect(UINT nID)
{
	switch(nID)
	{
		case IDM_OPTIONS_BACKGROUNDTYPE3:
			mDisplayBackgroundType=IDB_BITMAP_BACKGROUND3;
			break;
		case IDM_OPTIONS_BACKGROUNDTYPE2:
			mDisplayBackgroundType=IDB_BITMAP_BACKGROUND2;
			break;
		case IDM_OPTIONS_BACKGROUNDTYPE1:
		default:
			mDisplayBackgroundType=IDB_BITMAP_BACKGROUND1;
			break;
	}
	// Attempt to destroy the old bitmap
	if(!mDisplayBackground.DeleteObject())
	{
		gError->Warning("CLynxWindow::OnDisplayBackgroundTypeSelect() - Failed to DeleteObject() on old bitmap");
	}

	// Load up the background bitmap
	if(!mDisplayBackground.LoadBitmap(mDisplayBackgroundType))
	{
		gError->Fatal("CLynxWindow::OnDisplayBackgroundTypeSelect() - Failed to LoadBitmap() on new background");
	}

	// Force a display update with the display background set
	DisplayModeSet(DISPLAY_PRESERVE,DISPLAY_BKGND,DISPLAY_PRESERVE,DISPLAY_PRESERVE);
}

void CLynxWindow::OnDisplayBackgroundTypeUpdate(CCmdUI *pCmdUI)
{
	unsigned short check;

	switch(pCmdUI->m_nID)
	{
		case IDM_OPTIONS_BACKGROUNDTYPE3:
			check=IDB_BITMAP_BACKGROUND3;
			break;
		case IDM_OPTIONS_BACKGROUNDTYPE2:
			check=IDB_BITMAP_BACKGROUND2;
			break;
		case IDM_OPTIONS_BACKGROUNDTYPE1:
		default:
			check=IDB_BITMAP_BACKGROUND1;
			break;
	}
	pCmdUI->SetCheck(mDisplayBackgroundType==check);
}


void CLynxWindow::OnDebuggerUpdate()
{
#ifdef _LYNXDBG
	mpDebugger->Update();
#endif
}

#ifdef _LYNXDBG

void CLynxWindow::OnStepMenuSelect()
{
	gSingleStepMode=TRUE;
	gSystemHalt=FALSE;
	OnDebuggerUpdate();
	Invalidate(FALSE);
}

void CLynxWindow::OnTraceMenuSelect()
{
	mpDebugger->DebuggerTrace();
}

void CLynxWindow::OnDumpMenuSelect()
{
	mpDebugger->DebuggerDump();
}

void CLynxWindow::OnCodeMenuSelect()
{
	mpDebugger->DebuggerCode();
}

void CLynxWindow::OnDebuggerMenuSelect()
{
	mpDebugger->DebuggerDebugger();
}

void CLynxWindow::OnGraphicsMenuSelect()
{
	mpDebugger->DebuggerGFX();
}

void CLynxWindow::OnRAMDumpMenuSelect()
{
	FILE *fp;

	if((fp=fopen("lynxram.bin","wb"))!=NULL)
	{
		int loop;
		for(loop=0;loop<65536;loop++)
		{
			UBYTE data;
			data=mpLynx->Peek_RAM(loop);
			fwrite(&data,1,1,fp);
		}
	}
	else
	{
		gError->Warning("Failed to open RAM dump file (lynxram.bin)");
	}
}
#endif

void CLynxWindow::OnAboutBoxSelect()
{
	CAboutDialog dlg(IDD_ABOUT_BOX,this,mpLynx->CartGetName(),mpLynx->CartGetManufacturer(),HANDY_VERSION,HANDY_BUILD);
	dlg.DoModal();
}


void CLynxWindow::OnInfoMenuUpdate(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(mInfoDialogEnable!=0);
}

void CLynxWindow::OnInfoSelect()
{
	if(!mInfoDialogEnable)
	{
		CRect rect;
		GetWindowRect(&rect);

		int xpos=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"InfoWindowX1",rect.right+4);
		int ypos=mpLynxApp->GetProfileInt(REGISTRY_VERSION,"InfoWindowY1",rect.top);

		mInfoDialog.SetWindowPos(&wndTop,xpos,ypos,0,0,SWP_NOSIZE);

		mInfoDialogEnable=SetTimer(HANDY_INFO_TIMER,HANDY_INFO_TIMER_PERIOD,NULL);
		mInfoDialog.ShowWindow(SW_SHOW);
	}
	else
	{
		CRect rect;
		mInfoDialog.GetWindowRect(&rect);
		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"InfoWindowX1", rect.left);
		mpLynxApp->WriteProfileInt(REGISTRY_VERSION,"InfoWindowY1", rect.top);

		KillTimer(mInfoDialogEnable);
		mInfoDialogEnable=0;
		mInfoDialog.ShowWindow(SW_HIDE);
	}

	// Get focus back for ourselves

	SetFocus();
}


void CLynxWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	ULONG mask=0;

	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);

	if(nChar==mKeyDefs.key_up)
	{
		switch(DisplayModeRotate())
		{
			case DISPLAY_ROTATE_LEFT:
				mask=BUTTON_LEFT;
				break;
			case DISPLAY_ROTATE_RIGHT:
				mask=BUTTON_RIGHT;
				break;
			case DISPLAY_NO_ROTATE:
			default:
				mask=BUTTON_UP;
				break;
		}
	}

	if(nChar==mKeyDefs.key_down)
	{
		switch(DisplayModeRotate())
		{
			case DISPLAY_ROTATE_LEFT:
				mask=BUTTON_RIGHT;
				break;
			case DISPLAY_ROTATE_RIGHT:
				mask=BUTTON_LEFT;
				break;
			case DISPLAY_NO_ROTATE:
			default:
				mask=BUTTON_DOWN;
				break;
		}
	}

	if(nChar==mKeyDefs.key_left)
	{
		switch(DisplayModeRotate())
		{
			case DISPLAY_ROTATE_LEFT:
				mask=BUTTON_DOWN;
				break;
			case DISPLAY_ROTATE_RIGHT:
				mask=BUTTON_UP;
				break;
			case DISPLAY_NO_ROTATE:
			default:
				mask=BUTTON_LEFT;
				break;
		}
	}

	if(nChar==mKeyDefs.key_right)
	{
		switch(DisplayModeRotate())
		{
			case DISPLAY_ROTATE_LEFT:
				mask=BUTTON_UP;
				break;
			case DISPLAY_ROTATE_RIGHT:
				mask=BUTTON_DOWN;
				break;
			case DISPLAY_NO_ROTATE:
			default:
				mask=BUTTON_RIGHT;
				break;
		}
	}

	if(nChar==mKeyDefs.key_a)
	{
		mask=BUTTON_A;
	}

	if(nChar==mKeyDefs.key_b)
	{
		mask=BUTTON_B;
	}

	if(nChar==mKeyDefs.key_opt1)
	{
		mask=BUTTON_OPT1;
	}

	if(nChar==mKeyDefs.key_opt2)
	{
		mask=BUTTON_OPT2;
	}

	if(nChar==mKeyDefs.key_pause)
	{
		mask=BUTTON_PAUSE;
	}

	if(mask)
	{
		ULONG buttons=mpLynx->GetButtonData();
		buttons|=mask;
		mpLynx->SetButtonData(buttons);
	}
}

void CLynxWindow::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{	
	ULONG mask=0;

	CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);

	if(nChar==mKeyDefs.key_up)
	{
		switch(DisplayModeRotate())
		{
			case DISPLAY_ROTATE_LEFT:
				mask=BUTTON_LEFT^0xffffffff;
				break;
			case DISPLAY_ROTATE_RIGHT:
				mask=BUTTON_RIGHT^0xffffffff;
				break;
			case DISPLAY_NO_ROTATE:
			default:
				mask=BUTTON_UP^0xffffffff;
				break;
		}
	}

	if(nChar==mKeyDefs.key_down)
	{
		switch(DisplayModeRotate())
		{
			case DISPLAY_ROTATE_LEFT:
				mask=BUTTON_RIGHT^0xffffffff;
				break;
			case DISPLAY_ROTATE_RIGHT:
				mask=BUTTON_LEFT^0xffffffff;
				break;
			case DISPLAY_NO_ROTATE:
			default:
				mask=BUTTON_DOWN^0xffffffff;
				break;
		}
	}

	if(nChar==mKeyDefs.key_left)
	{
		switch(DisplayModeRotate())
		{
			case DISPLAY_ROTATE_LEFT:
				mask=BUTTON_DOWN^0xffffffff;
				break;
			case DISPLAY_ROTATE_RIGHT:
				mask=BUTTON_UP^0xffffffff;
				break;
			case DISPLAY_NO_ROTATE:
			default:
				mask=BUTTON_LEFT^0xffffffff;
				break;
		}
	}

	if(nChar==mKeyDefs.key_right)
	{
		switch(DisplayModeRotate())
		{
			case DISPLAY_ROTATE_LEFT:
				mask=BUTTON_UP^0xffffffff;
				break;
			case DISPLAY_ROTATE_RIGHT:
				mask=BUTTON_DOWN^0xffffffff;
				break;
			case DISPLAY_NO_ROTATE:
			default:
				mask=BUTTON_RIGHT^0xffffffff;
				break;
		}
	}

	if(nChar==mKeyDefs.key_a)
	{
		mask=BUTTON_A^0xffffffff;
	}

	if(nChar==mKeyDefs.key_b)
	{
		mask=BUTTON_B^0xffffffff;
	}

	if(nChar==mKeyDefs.key_opt1)
	{
		mask=BUTTON_OPT1^0xffffffff;
	}

	if(nChar==mKeyDefs.key_opt2)
	{
		mask=BUTTON_OPT2^0xffffffff;
	}

	if(nChar==mKeyDefs.key_pause)
	{
		mask=BUTTON_PAUSE^0xffffffff;
	}

	if(mask)
	{
		ULONG buttons=mpLynx->GetButtonData();
		buttons&=mask;
		mpLynx->SetButtonData(buttons);
	}
}


void CLynxWindow::OnActivateApp(BOOL bActive, DWORD hTask) 
{
	CFrameWnd::OnActivateApp(bActive, hTask);
	
// This isnt required now as direct sound controls the focus automagically
//	// Reset sound status
//	if(bActive && gAudioEnabled) mSoundPlayer->SoundSetStatus(bActive);

	// If fullscreen then make windowed
	if(!bActive && DisplayModeWindowed()==DISPLAY_FULLSCREEN)
	{
		DisplayModeSet(DISPLAY_WINDOWED,DISPLAY_PRESERVE,DISPLAY_PRESERVE,DISPLAY_PRESERVE);
	}
	mWindowInFocus=bActive;
}

void CLynxWindow::OnInitMenu(CMenu* pMenu) 
{
	CFrameWnd::OnInitMenu(pMenu);
	
	// No lynx activity can occur here so we'll wipe the buffers clean
	// its a bit of a cheat but the simplest way !!!

	mDirectSoundPlayer.Flush();

}

BOOL CLynxWindow::OnEraseBkgnd(CDC* pDC) 
{
	if(DisplayModeRotate()!=DISPLAY_NO_ROTATE)
	{
		// This is the only case when the bitmap doesnt fill the picture
		CRect rect;
		CGdiObject *oldobj;
		GetClientRect(&rect);
		oldobj=pDC->SelectStockObject(BLACK_BRUSH);
		pDC->Rectangle(rect);
		pDC->SelectObject(oldobj);
	}
	else if(DisplayModeBkgnd()==DISPLAY_BKGND && DisplayModeWindowed())
	{
		// Change the elseif back to the line below to add backgrounds in fullscreen
		// it looks crap though.
		//		else if(DisplayModeBkgnd()==DISPLAY_BKGND)

		// Create a background and display
		BITMAP bkgnd;
		int win_height,win_width;
		// Fill the backgound in
		CDC *memcdc = new CDC;
		if(!memcdc->CreateCompatibleDC(pDC))
		{
			gError->Fatal("CLynxWindow::OnEraseBkgnd() - Failed CreateCompatibleDC()");
			return TRUE;
		}
		memcdc->SelectObject(&mDisplayBackground);
		mDisplayBackground.GetBitmap(&bkgnd);
		win_height=bkgnd.bmHeight*DisplayModeMagnification();
		win_width=bkgnd.bmWidth*DisplayModeMagnification();
		switch(DisplayModeRotate())
		{
			case DISPLAY_NO_ROTATE:
				if(!pDC->StretchBlt(0,0,win_width,win_height,memcdc,0,0,bkgnd.bmWidth,bkgnd.bmHeight,SRCCOPY)) gError->Fatal("CLynxWindow::OnEraseBkgnd() - Failed StretchBlt");
				break;
//			case DISPLAY_ROTATE_LEFT:
//				if(!pDC->StretchBlt(0,0,win_width,win_height,mpMemCDC,0,0,bkgnd.bmWidth,bkgnd.bmHeight,SRCCOPY)) gError->Fatal("CLynxWindow::OnEraseBkgnd() - Failed StretchBlt");
//				break;
//			case DISPLAY_ROTATE_RIGHT:
//				if(!pDC->StretchBlt(0,0,win_width,win_height,mpMemCDC,0,0,bkgnd.bmWidth,bkgnd.bmHeight,SRCCOPY)) gError->Fatal("CLynxWindow::OnEraseBkgnd() - Failed StretchBlt");
//				break;
			default:
				break;
		}
		delete memcdc;
	}
	return TRUE;	

// Dont call the base class, we've finished
//	return CFrameWnd::OnEraseBkgnd(pDC);
}


void CLynxWindow::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType,cx,cy);
#ifdef _LYNXDBG
	// If hiding then hide all debug else show all debug
	if(nType==SIZE_MINIMIZED)
	{
		mpDebugger->Minimise();
	}
	else if(nType==SIZE_RESTORED)
	{
		mpDebugger->Restore();
	}
#endif
}
