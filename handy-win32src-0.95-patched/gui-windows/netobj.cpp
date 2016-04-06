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

#define _NETOBJ_C

#include "netobj.h"


class CNetAddrDlg : public CDialog
{
	public:
		CNetAddrDlg(CString netadr)
			:CDialog(IDD_NETADDRESS,AfxGetMainWnd())
		{
			//{{AFX_DATA_INIT(CNetAddrDlg)
			mNetAddress = netadr;
			//}}AFX_DATA_INIT
		}


		BOOL OnInitDialog(void)
		{
			BOOL retval=CDialog::OnInitDialog();
			CEdit *tmp;
			tmp=(CEdit*)GetDlgItem(IDC_NETWORK_ADDRESS);
			tmp->SetWindowText(mNetAddress);
			return retval;
		}

		void DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CDumpWinDlg)
			DDX_Text(pDX, IDC_NETWORK_ADDRESS, mNetAddress);
			//}}AFX_DATA_MAP
		}

		//{{AFX_DATA(CNetAddrDlg)
		CString	mNetAddress;
		//}}AFX_DATA
};


CNetObj::CNetObj()
{
	int error;
	struct protoent *ppe;
	mState=NETOBJ_STOP;
	mRxSocket=0;
	mTxSocket=0;
	mTimerID=0;
	mSyncCounter=-1;
	mRXcount=0;
	mTXcount=0;
	mLastRxData=0;

	TRACE_NETOBJ0("CNetObj()");

	// Blank the dest socket
	memset(&mDestSockAddr, 0, sizeof(mDestSockAddr));

	// Initialise the dialog for later, its not visible by default
	mpWaitForSync= new CNetWaitDlg;	

	// Initialise WinSock
//	if((error=WSAStartup(MAKEWORD(2,0),&mWsadata)))
	if((error=WSAStartup(MAKEWORD(1,1),&mWsadata)))
	{
		CString errmsg;
		errmsg.Format("CNetObj::CNetObj() - Error %05d Unable to start winsock support",error);
		::MessageBox((AfxGetMainWnd())->m_hWnd,errmsg,NETOBJ_CAPTION,MB_ICONEXCLAMATION);
		TRACE_NETOBJ0("CNetObj() - Failed to start WinSock");
		mState=NETOBJ_DEAD;
	}
	else
	{
		// Lookup protocol
		if((ppe=getprotobyname("udp"))==0)
		{
			::MessageBox((AfxGetMainWnd())->m_hWnd,"CNetObj::CNetObj() - GetProtoByname() - Can't get UDP protocol entry.\nOnly TCP/IP(UDP) is supported by Handy.",NETOBJ_CAPTION,MB_ICONEXCLAMATION);
			mState=NETOBJ_DEAD;
		}
		else
		{
			TRACE_NETOBJ0("CNetObj() - WinSock started");
			// now open our socket
			if ((mRxSocket=socket(PF_INET, SOCK_DGRAM, ppe->p_proto))==INVALID_SOCKET || (mTxSocket=socket(PF_INET, SOCK_DGRAM, ppe->p_proto))==INVALID_SOCKET)
			{
				::MessageBox((AfxGetMainWnd())->m_hWnd,"CNetObj::CNetObj() - Request to create Rx/Tx sockets failed.\nOnly TCP/IP(UDP) is supported by Handy.",NETOBJ_CAPTION,MB_ICONEXCLAMATION);
			    mState=NETOBJ_DEAD;
			}
			else
			{
				TRACE_NETOBJ0("CNetObj() - Rx/Tx Sockets Created");
				// Success, now bind to the socket we will listen on
				struct sockaddr_in sin;
				memset(&sin, 0, sizeof(sin));
				sin.sin_family=AF_INET;
				sin.sin_addr.s_addr=htonl(INADDR_ANY);
				sin.sin_port=htons(HANDY_PORT_NUMBER);
				if(bind(mRxSocket,(struct sockaddr *)&sin, sizeof(sin))==SOCKET_ERROR)
				{
					::MessageBox((AfxGetMainWnd())->m_hWnd,"CNetObj::CNetObj() - Could not bind to socket.\nMay be already in use ??.",NETOBJ_CAPTION,MB_ICONEXCLAMATION);
				    mState=NETOBJ_DEAD;
				}
				else
				{
					TRACE_NETOBJ0("CNetObj() - Bound to socket");
					DWORD param=1;
					if(ioctlsocket(mRxSocket,FIONBIO,&param)==SOCKET_ERROR || ioctlsocket(mTxSocket,FIONBIO,&param)==SOCKET_ERROR)
					{
						::MessageBox((AfxGetMainWnd())->m_hWnd,"CNetObj::CNetObj() - Could set sockets to non-blocking.\nMay be already in use ??.",NETOBJ_CAPTION,MB_ICONEXCLAMATION);
 						mState=NETOBJ_DEAD;
					}
					else
					{
						param=1;
						if(setsockopt(mTxSocket,IPPROTO_UDP,TCP_NODELAY,(char*)&param,sizeof(param))==SOCKET_ERROR)
						{
//							int error=WSAGetLastError();
//							CString errmsg;
//							errmsg.Format("CNetObj::CNetObj() - Error %05d could not setsockopt to TCP_NODELAY.\nMay be already in use ??.",error);
//							::MessageBox((AfxGetMainWnd())->m_hWnd,errmsg,NETOBJ_CAPTION,MB_ICONEXCLAMATION);
// 							mState=NETOBJ_DEAD;
 							mState=NETOBJ_IDLE;
						}
						else
						{
							TRACE_NETOBJ0("CNetObj() - Socket set to TCP_NODELAY");
 							mState=NETOBJ_IDLE;
						}
					}
				}
			}
		}
	}
}

CNetObj::~CNetObj(void)
{
	// Kill the timer if its active
	if(mTimerID!=NULL) timeKillEvent(mTimerID);
	// Destroy the wait dialog
	if(mpWaitForSync!=NULL)
	{
		mpWaitForSync->DestroyWindow();
		delete mpWaitForSync;
	}
	mpWaitForSync=NULL;
	// Close any open sockets
	if(mRxSocket) closesocket(mRxSocket);
	if(mTxSocket) closesocket(mTxSocket);
}

void CALLBACK CNetObj::SyncCallback(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2)
{
	(AfxGetMainWnd())->PostMessage(WM_NETOBJ_UPDATE);
}

void CNetObj::Update()
{
	static int twiddle=0x00000001;

	// Refreference the objects as this is a static member
//	CNetObj *netobj = (CNetObj*) dwUser;

	// Only do anything if in the sync state
	TRACE_NETOBJ0("SyncCallback()");
	if(mState==NETOBJ_SYNC)
	{
		// We're uninitialised
		if(mSyncCounter==-1)
		{
			// Setup the count
			mSyncCounter=HANDY_SYNCUP_COUNT;
		}
		// Check for an abort or a sync packet
		if(mpWaitForSync->mAbort)
		{
			TRACE_NETOBJ0("SyncCallback() - Synchronisation aborted");
			Stop();
		}
		else
		{
			// Once we get the first Sync message we start to auto decrement
			// the counter, the first sync tells that both machines are in
			// Sync mode, we then do HANDY_SYNCUP_COUNT transmits, one every 100ms.
			// when count reaches 0 we assume SYNCHRONISATION.
			//
			// This isnt perfect but it IS simple.
			//
			if(mSyncCounter==HANDY_SYNCUP_COUNT)
			{
				if(Receive()==HANDY_SYNC_MESSAGE)
				{
					mpWaitForSync->SetString("Sync complete, please wait...");
					TRACE_NETOBJ0("SyncCallback() - Received a HANDY_SYNC_MESSAGE, starting countdown.");
					mSyncCounter--;
				}
				else
				{
					// Do the night rider bit...
					CString waitstr;
					waitstr.Format("Waiting for sync message... %07x",twiddle&0x7ffffff);
					mpWaitForSync->SetString(waitstr);
					if(twiddle==0x01000000) twiddle=0x11000000;
					if(twiddle==0x10000001) twiddle=0x00000001;
					if(twiddle&0x10000000) twiddle=0x10000000+((twiddle&0x7ffffff)>>4); else twiddle=0x00000000+((twiddle&0x7ffffff)<<4);
				}
			}
			else if(!mSyncCounter)
			{
				TRACE_NETOBJ0("SyncCallback() - Synchronisation complete, flushing net buffers, mState=NETOBJ_ACTIVE");
				mpWaitForSync->ShowWindow(SW_HIDE);
				mState=NETOBJ_ACTIVE;
				// Halt the multimedia timer
				if(mTimerID!=NULL)
				{
					timeKillEvent(mTimerID);
					mTimerID=NULL;
				}
				// Flush any incoming SYNC messages
				while(Receive()==HANDY_SYNC_MESSAGE);

				// Make winsock send messages to the main window
				if(WSAAsyncSelect(mRxSocket,(AfxGetMainWnd())->m_hWnd,WM_NETOBJ_SELECT,FD_READ)==SOCKET_ERROR)
				{
					// A bad thing has occured, what to do...
					Stop();
					TRACE_NETOBJ0("SyncCallback() - WSAAsyncSelect failed - Set mState=NETOBJ_DEAD");
					mState=NETOBJ_DEAD;
				}
				// Reset our internal state
				mSyncCounter=-1;
				mRXcount=0;
				mTXcount=0;
			}
			else
			{
				mSyncCounter--;
			}

			// Transmit the Sync to the destination
			Transmit(HANDY_SYNC_MESSAGE);
		}
	}
	else
	{
		// Usually net connection has stopped and we're just displaying a message
		// We're uninitialised
		if(mSyncCounter==-1) mSyncCounter=20;

		if(!mSyncCounter)
		{
				TRACE_NETOBJ0("SyncCallback() - Shutdown complete, mState=NETOBJ_IDLE");
				mpWaitForSync->ShowWindow(SW_HIDE);
				// Halt the multimedia timer
				if(mTimerID!=NULL)
				{
					timeKillEvent(mTimerID);
					mTimerID=NULL;
				}
				// Reset our internal state
				mSyncCounter=-1;
		}
		else
		{
			mSyncCounter--;
		}
	}
//	else
//	{
//		Stop();
//		TRACE_NETOBJ1("SyncCallback() - Illegal mState value %d",mState);
//		mState=NETOBJ_DEAD;
//	}
}

int CNetObj::Start(CString netadr)
{
	mRXcount=0;
	mTXcount=0;
	if(mState==NETOBJ_IDLE)
	{
		// Fetch the network address
		struct hostent *hostentry;
		CString tmpstr;
		CNetAddrDlg *netdlg= new CNetAddrDlg(netadr);
		int result=netdlg->DoModal();
		tmpstr=netdlg->mNetAddress;
		delete netdlg;

		if(result==IDOK && !tmpstr.IsEmpty())
		{
			ULONG inetaddr;
			mNetAddress=tmpstr;
			TRACE_NETOBJ1("Start() - Got Address : %s",mNetAddress);
			mNetAddress.TrimLeft();
			mNetAddress.MakeLower();
			TRACE_NETOBJ1("Start() - Processed to: %s",mNetAddress);

			inetaddr=INADDR_NONE;
			// Lets try to resolve the hostname/address to an IP addr
			if((inetaddr=inet_addr(mNetAddress))==INADDR_NONE)
			{
				// Wasnt a raw IP address, try resolution
				if((hostentry=gethostbyname(mNetAddress))!=NULL)
				{
					memset(&mDestSockAddr, 0, sizeof(mDestSockAddr));
					memcpy((char*)&mDestSockAddr.sin_addr,hostentry->h_addr,hostentry->h_length);
					mDestSockAddr.sin_family=AF_INET;
					mDestSockAddr.sin_port=htons(HANDY_PORT_NUMBER);
					// Go to sync mode
					TRACE_NETOBJ0("Start() - Set mDestSockAddr, mState=NETOBJ_SYNC");
					mState=NETOBJ_SYNC;
				}
				else
				{
					// Failed to resolve
					CString message;
					message.Format("CNetObj::Start() - Failed to resolve the network name you provided ( %s ), connect failed.",mNetAddress);
					::MessageBox((AfxGetMainWnd())->m_hWnd,message,NETOBJ_CAPTION,MB_ICONEXCLAMATION);
				}
			}
			else
			{
				memset(&mDestSockAddr, 0, sizeof(mDestSockAddr));
				mDestSockAddr.sin_family=AF_INET;
				mDestSockAddr.sin_addr.s_addr=inetaddr;
				mDestSockAddr.sin_port=htons(HANDY_PORT_NUMBER);
				// Go to sync mode
				TRACE_NETOBJ0("Start() - Set mDestSockAddr, mState=NETOBJ_SYNC");
				mState=NETOBJ_SYNC;
			}

			// Now we've entered SYNC stage start up the callback to perform SYNC
			if(mState==NETOBJ_SYNC)
			{
				// Reset the sync engine state
				mSyncCounter=-1;
				mpWaitForSync->mAbort=FALSE;
				// Display the synchro dialog
				mpWaitForSync->ShowWindow(SW_SHOW);
				// If the timer is not running then start it
				if(mTimerID==NULL)
				{
					// Initialise the timer callback
					if((mTimerID=timeSetEvent(100,0,(LPTIMECALLBACK)SyncCallback,(DWORD)this,TIME_PERIODIC))==NULL)
					{
						TRACE_NETOBJ0("Start() - Multimedia timer failed");
						::MessageBox((AfxGetMainWnd())->m_hWnd,"CNetObj::Start() - Couldn't initialise Multimedia timer, aborting ???",NETOBJ_CAPTION,MB_ICONEXCLAMATION);
						Stop();
						TRACE_NETOBJ0("Start() - Set mState=NETOBJ_DEAD");
						mState=NETOBJ_DEAD;
					}
				}
			}
		}
		else
		{
			TRACE_NETOBJ0("Start() - NetAddress Dialog, cancelled or empty.");
		}
	}
	else if(mState==NETOBJ_SYNC)
	{
		// Dont allow re-start if we are already in Sync mode
		::MessageBox((AfxGetMainWnd())->m_hWnd,"CNetObj::Start() - Already in network sync mode.",NETOBJ_CAPTION,MB_ICONEXCLAMATION);
	}
	else
	{
		if(mState!=NETOBJ_DEAD) TRACE_NETOBJ1("Start() - mState!=NETOBJ_IDLE/SYNC, mState=%d",mState);
		TRACE_NETOBJ0("Start() - Set mState=NETOBJ_DEAD");
		mState=NETOBJ_DEAD;
	}

	// Return sucess indicator
	if(mState==NETOBJ_SYNC) return TRUE; else return FALSE;
}

int CNetObj::Stop(void)
{
	if(mState==NETOBJ_ACTIVE || mState==NETOBJ_SYNC)
	{
		// Alert user that network is stopping
		mpWaitForSync->SetString("Closing connection, please wait...");
		mpWaitForSync->ShowWindow(SW_SHOW);
		// Reset the update counter index
		mSyncCounter=-1;

		// If the timer is not running then start it
		if(mTimerID==NULL)
		{
			// Initialise the timer callback
			if((mTimerID=timeSetEvent(100,0,(LPTIMECALLBACK)SyncCallback,(DWORD)this,TIME_PERIODIC))==NULL)
			{
				TRACE_NETOBJ0("Start() - Multimedia timer failed");
				::MessageBox((AfxGetMainWnd())->m_hWnd,"CNetObj::Stop() - Couldn't initialise Multimedia timer, aborting ???",NETOBJ_CAPTION,MB_ICONEXCLAMATION);
				TRACE_NETOBJ0("Start() - Set mState=NETOBJ_DEAD");
				mState=NETOBJ_DEAD;
			}
		}

 		// Make winsock send messages to the main window
		if(WSAAsyncSelect(mRxSocket,(AfxGetMainWnd())->m_hWnd,0,0)==SOCKET_ERROR)
		{
			// A bad thing has occured, what to do...
			::MessageBox((AfxGetMainWnd())->m_hWnd,"CNetObj::Stop() - WSAASyncSelect failes",NETOBJ_CAPTION,MB_ICONEXCLAMATION);
			TRACE_NETOBJ0("Start() - WSAAsyncSelect failed - Set mState=NETOBJ_DEAD");
			mState=NETOBJ_DEAD;
		}
		else
		{
			TRACE_NETOBJ0("Stop() - Set mState=NETOBJ_IDLE");
			mState=NETOBJ_IDLE;
		}

		// Send a message back to force the window to close
	}
	else
	{
		if(mState!=NETOBJ_DEAD) TRACE_NETOBJ1("Stop() - mState!=NETOBJ_ACTIVE/SYNC, mState=%d",mState);
		TRACE_NETOBJ0("Stop() - Set mState=NETOBJ_DEAD");
		mState=NETOBJ_DEAD;
	}

	if(mState==NETOBJ_IDLE) return TRUE; else return FALSE;
}

int CNetObj::Receive(void)
{
	int retval=0;
	if(mState==NETOBJ_ACTIVE || mState==NETOBJ_SYNC)
	{
		unsigned char rxbuf[10];
		int rxcount;

		// If there is any to get then get it
		rxcount=recv(mRxSocket,(char*)rxbuf,2,0);

		if(rxcount==2)
		{
			retval=rxbuf[0]+(rxbuf[1]<<8);
			TRACE_NETOBJ1("Reveive() - Data received = %04x",retval);
			mRXcount++;
			mLastRxData=retval;
		}
		else if(rxcount==-1)
		{
			TRACE_NETOBJ0("Reveive() - No Data Waiting");
			retval=HANDY_NODATA_MESSAGE;
		}
		else
		{
			TRACE_NETOBJ1("Reveive() - Wrong number of bytes waiting = %d",rxcount);
			retval=HANDY_NODATA_MESSAGE;
		}
	}
	else
	{
		if(mState!=NETOBJ_DEAD) TRACE_NETOBJ1("Reveive() - mState!=NETOBJ_ACTIVE/SYNC, mState=%d",mState);
		TRACE_NETOBJ0("Receive() - Set mState=NETOBJ_DEAD");
		mState=NETOBJ_DEAD;
		retval=HANDY_NODATA_MESSAGE;
	}
	return retval;
}

void CNetObj::Transmit(int data)
{
	// Only sync messages can be sent in sync state
	if(mState==NETOBJ_SYNC && data!=HANDY_SYNC_MESSAGE)
	{
		TRACE_NETOBJ0("Transmit() - mState=NETOBJ_SYNC, Lynx data discarded");
		return;
	}

	mTXcount++;
	if(mState==NETOBJ_ACTIVE || mState==NETOBJ_SYNC)
	{
		unsigned char txbuf[10];
		int txcount;

		txbuf[0]=(unsigned char) data&0xff;
		txbuf[1]=(unsigned char) (data>>8)&0xff;

		if((txcount=sendto(mTxSocket,(char*)txbuf,2,0,(struct sockaddr *)&mDestSockAddr,sizeof(mDestSockAddr)))!=SOCKET_ERROR)
		{
			TRACE_NETOBJ1("Transmit() - Data = %04x",data);
		}
		else
		{
			TRACE_NETOBJ2("Transmit() - Failed sendto call (Error=%d, txcount=%d)",WSAGetLastError(),txcount);

			// Process error in a sensible manner
			switch(WSAGetLastError())
			{
				case WSAENETUNREACH:
				case WSAEHOSTUNREACH:
					{
						CString message;
						Stop();
						message.Format("CNetObj::Transmit() - Host \"%s\" is unreachable.",mNetAddress);
						::MessageBox((AfxGetMainWnd())->m_hWnd,message,NETOBJ_CAPTION,MB_ICONEXCLAMATION);
					}
					break;
				default:
					Stop();
					::MessageBox((AfxGetMainWnd())->m_hWnd,"CNetObj::Transmit() - Winsock sendto call has failed, disabling networking.",NETOBJ_CAPTION,MB_ICONEXCLAMATION);
					TRACE_NETOBJ0("Transmit() - Set mState=NETOBJ_DEAD");
					mState=NETOBJ_DEAD;
					break;
			}
		}
	}
	else if(mState==NETOBJ_IDLE)
	{
		TRACE_NETOBJ0("Transmit() - mState=NETOBJ_IDLE, your wasting your time");
	}
	else
	{
		if(mState!=NETOBJ_DEAD) TRACE_NETOBJ1("Transmit() - mState!=NETOBJ_ACTIVE/SYNC, mState=%d",mState);
		TRACE_NETOBJ0("Transmit() - Set mState=NETOBJ_DEAD");
		mState=NETOBJ_DEAD;
	}
}

