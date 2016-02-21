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


// DirectSoundPlayer.cpp: implementation of the CDirectSoundPlayer class.
//
//////////////////////////////////////////////////////////////////////

#include <afxwin.h>
#include <afxcmn.h>
#include <afxdlgs.h>
#include <mmsystem.h>
#include "directx.h"
#include "../core/machine.h"
#include "DirectSoundPlayer.h"

//#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
//#endif

#define TIMER_PERIOD		50		// 25mS per tick - 40Hz
#define TIMER_FREQUENCY		(1000/TIMER_PERIOD)

extern CErrorInterface *gError;

//CFile dbg;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDirectSoundPlayer::CDirectSoundPlayer()
{
	m_TimerID=NULL;
	m_AudioEnabled=FALSE;
	m_InService=FALSE;
}

CDirectSoundPlayer::~CDirectSoundPlayer()
{
	if(m_AudioEnabled) Stop();
	m_InitOK=FALSE;
}

bool CDirectSoundPlayer::Create(CWnd *pcwnd, UBYTE *lynxbuff, ULONG *lynxpoint, ULONG lynxbuflen, ULONG lynxfreq)
{
    _hresult hr;

	// Store lynx related trivia

	m_LynxBufferBasePtr=lynxbuff;
	m_LynxBufferIndexPtr=lynxpoint;
	m_LynxBufferLength=lynxbuflen;
	m_LynxSampleFreq=lynxfreq;
	m_InitOK=TRUE;

    // -- setup direct sound --

    try 
    {
        // -- construct a direct draw object and set to windowed mode --
        m_ids = IDirectSoundPtr (CLSID_DirectSound);
		hr = m_ids->Initialize (NULL);
		hr = m_ids->SetCooperativeLevel (pcwnd->m_hWnd, DSSCL_PRIORITY);

		// Allocate ourselves a buffer, according to direct-x the buffer defaults
		// to 22KHz 8 bit samples, just fine by me.

        DXStruct<DSBUFFERDESC> dsbdesc;
		dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
		hr= m_ids->CreateSoundBuffer(&dsbdesc, &m_BufferPrimary, NULL);

		// Set the primary buffer format

		WAVEFORMATEX wfx;
		memset(&wfx, 0, sizeof(WAVEFORMATEX)); 
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = 1; 
		wfx.nSamplesPerSec = m_LynxSampleFreq;
		wfx.wBitsPerSample = 8; 
		wfx.nBlockAlign = wfx.wBitsPerSample / 8 * wfx.nChannels;
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign; 
		hr = m_BufferPrimary->SetFormat(&wfx); 

		// Create the secondary buffer, use the WFX from before

		dsbdesc.reset();
		dsbdesc.dwBufferBytes = m_LynxBufferLength;
		dsbdesc.lpwfxFormat = &wfx;
		hr = m_ids->CreateSoundBuffer(&dsbdesc, &m_BufferSecondary, NULL);
	}

    catch (_com_error& ex)
    {
		CString message ("CDirectSoundPlayer::Create - Error: \n");
//		message += ex.ErrorMessage ();
		message += DXAppErrorToString (ex.Error());
		gError->Warning(message);

		// Return failure
        m_InitOK=FALSE;
    }

    return m_InitOK;
}

void CDirectSoundPlayer::Destroy(CWnd *pcwnd)
{
//  Dont release as the IPTR macros will do this for us automatically
//	if (m_ids) m_ids->Release();
}

bool CDirectSoundPlayer::Start()
{
    HRESULT hr;

	if(m_InitOK && !m_AudioEnabled)
	{
		// Flush Buffers
		Flush();

		m_AudioEnabled=TRUE;

		// Now create the multimedia timer object to play sounds without polling

		if((m_TimerID=timeSetEvent(25,0,TimerCallBack,(DWORD)this ,TIME_PERIODIC))==NULL)
		{
			gError->Warning("CDirectSoundPlayer::Start - Error: \n timeSetEvent returned an error");
			m_AudioEnabled=FALSE;
		}

		hr=m_BufferSecondary->SetCurrentPosition(0);
		hr=m_BufferSecondary->Play(0,0,DSBPLAY_LOOPING);

		if(hr==DSERR_BUFFERLOST)
		{
			// Attempt to restore lost buffer
			hr=m_BufferPrimary->Restore();
		}

		if(hr!=DS_OK)
		{
			CString message ("CDirectSoundPlayer::Start - Error: \n");
			message += DXAppErrorToString (hr);
			gError->Warning(message);
			m_AudioEnabled=FALSE;
		}
//		dbg.Open("c:\\develop\\handy\\soundlog.txt",CFile::modeCreate|CFile::modeWrite);
	}
	return m_AudioEnabled;
}

bool CDirectSoundPlayer::Stop()
{
    HRESULT hr;
	if(m_InitOK && m_AudioEnabled)
	{
		if(m_TimerID!=NULL) timeKillEvent(m_TimerID);
		hr=m_BufferSecondary->Stop();
		if(hr!=DS_OK)
		{
			CString message ("CDirectSoundPlayer::Stop - Error: \n");
			message += DXAppErrorToString (hr);
			gError->Warning(message);
		}

		hr=m_BufferSecondary->SetCurrentPosition(0);
		if(hr!=DS_OK)
		{
			CString message ("CDirectSoundPlayer::Stop - Error: \n");
			message += DXAppErrorToString (hr);
			gError->Warning(message);
		}
//		dbg.Close();
	}
	m_AudioEnabled=FALSE;
	return m_AudioEnabled;
}

//void CDirectSoundPlayer::Update()
//{
	// This function polls the sound buffer and updates the contents of the
	// secondard buffer if required
	//
	//  If(DS(play-write)>TRIGGER)
	//     Work out num samples required by DS
	//     Copy avail lynx samples to DS buffer (repeat data if under/truncate if over)
	//     Zero lynx buffer pointer
//}

void CALLBACK CDirectSoundPlayer::TimerCallBack(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	HRESULT	hr;
	LPVOID lpbuf1 = NULL;
	LPVOID lpbuf2 = NULL;
	LPBYTE lynxpointer = NULL;
	LPBYTE tmppointer = NULL;
	DWORD lynxbytesavail =0;
	DWORD dwsize1 = 0;
	DWORD dwsize2 = 0;
	DWORD dwbyteswritten1 = 0;
	DWORD dwbyteswritten2 = 0;
	CDirectSoundPlayer *dsp = (CDirectSoundPlayer*) dwUser;

	// Check for reentrance
	if (InterlockedExchange (&(dsp->m_InService), TRUE) == FALSE)
	{
		// Not reentered, proceed normally

		DWORD writelen=dsp->CalcWriteSize();


		if(writelen)
		{
			// Lock required amount of buffer

			hr = dsp->m_BufferSecondary->Lock (dsp->m_WriteCursor, writelen, &lpbuf1, &dwsize1, &lpbuf2, &dwsize2,0);

			if (hr == DS_OK && writelen>0)
			{

				// We only write the available amount of lynx data into the sound buffers.
				// If there is underrun then the last byte is repeated.
				// If there is overrun then it is truncated
	
				// Fetch temp lynx pointer
				lynxpointer=dsp->m_LynxBufferBasePtr;
				lynxbytesavail=*dsp->m_LynxBufferIndexPtr;
			    dwbyteswritten1 = dwsize1;
				tmppointer=(LPBYTE)lpbuf1;
				for(DWORD loop=0;loop<dwsize1;loop++)
				{
					*(tmppointer++)=*lynxpointer;
					if(lynxbytesavail)
					{
						lynxbytesavail--;
						lynxpointer++;
					}
					else
					{
//						if(*dsp->m_LynxBufferIndexPtr>2) lynxpointer-=2; else lynxpointer-=*dsp->m_LynxBufferIndexPtr;
//						if(*dsp->m_LynxBufferIndexPtr>2) lynxbytesavail=2; else lynxbytesavail=*dsp->m_LynxBufferIndexPtr;
					}
				}

			    // Second write required?
			    if (lpbuf2)
			    {
			        dwbyteswritten2 = dwsize2;
					tmppointer=(LPBYTE)lpbuf2;
					for(DWORD loop=0;loop<dwsize2;loop++)
					{
						*(tmppointer++)=*lynxpointer;
						if(lynxbytesavail)
						{
							lynxbytesavail--;
							lynxpointer++;
						}
						else
						{
//							if(*dsp->m_LynxBufferIndexPtr>2) lynxpointer-=2; else lynxpointer-=*dsp->m_LynxBufferIndexPtr;
//							if(*dsp->m_LynxBufferIndexPtr>2) lynxbytesavail=2; else lynxbytesavail=*dsp->m_LynxBufferIndexPtr;
						}
					}
			    }

			    // Update our buffer offset and unlock sound buffer
			    dsp->m_WriteCursor = (dsp->m_WriteCursor + dwbyteswritten1 + dwbyteswritten2) % dsp->m_LynxBufferLength;
				dsp->m_BufferSecondary->Unlock (lpbuf1, dwbyteswritten1, lpbuf2, dwbyteswritten2);

				// Now update the lynx variables
				*dsp->m_LynxBufferIndexPtr=0;
			}
			else
			{
				CString message ("CDirectSoundPlayer::TimerCallBack - Lock Error: \n");
				message += DXAppErrorToString (hr);
				AfxMessageBox (message);
				dsp->Stop();
			}
		}

		// Reset reentrancy semaphore
		InterlockedExchange (&(dsp->m_InService), FALSE);
	}
}

void CDirectSoundPlayer::Flush()
{
	HRESULT hr;
	LPVOID	lpbuf1=NULL;
//	LPVOID	lpbuf2=NULL;
	DWORD	dwsize1=0;
//	DWORD	dwsize2=0;
	bool	status;

	if(m_InitOK)
	{
		status=m_AudioEnabled;

		if(status) Stop();

		hr = m_BufferSecondary->Lock(0,m_LynxBufferLength,&lpbuf1,&dwsize1,NULL,NULL,DSBLOCK_ENTIREBUFFER);
		if(hr==DS_OK)
		{
			memset(lpbuf1,128,dwsize1);
			m_BufferSecondary->Unlock(lpbuf1,dwsize1,NULL,NULL);
			m_WriteCursor=0;
		}
		else
		{
// Commented out due to WinNT problems with Lock ALWAYS failing
//			CString warn("CDirectSoundPlayer::Flush - Could not lock sound buffer\n");
//			warn += DXAppErrorToString (hr);
//			gError->Warning(warn);
		}

		if(status) Start();
	}
}

bool CDirectSoundPlayer::GetStatus()
{
	return m_AudioEnabled;
}

bool CDirectSoundPlayer::SetStatus(bool sound)
{
	if(sound && !m_AudioEnabled) return Start();
	if(!sound && m_AudioEnabled) return Stop();
	return m_AudioEnabled;
}

DWORD CDirectSoundPlayer::CalcWriteSize()
{
	// Calculate how many bytes to write to the secondary buffer bearing
	// in mind that we always want to stay 2 buffers ahead of the play
	// pointer = 50mS

	long lAveQueueLen=(m_LynxSampleFreq/TIMER_FREQUENCY)*2;
	long lQueueMaxDelta=lAveQueueLen/4;


	HRESULT hr;
	DWORD dwWriteCursor, dwPlayCursor;
	long  lQueueLength=0;
	long  lBytesInLynxQueue=(long)*m_LynxBufferIndexPtr;
	long  lSamplesForAveQueue=0;
	DWORD result;

	// Get current play position
	hr=m_BufferSecondary->GetCurrentPosition(&dwPlayCursor, &dwWriteCursor);

	if(hr==DS_OK)
	{
		if (m_WriteCursor <= dwPlayCursor)
	    {
		    // Our write position trails play cursor, i.e data to be played
			// wraps around the end of the bufer.
			//
			//  start                                      end
			//    |XXXXXXXXXXXXX                XXXXXXXXXXXX|
			//    -------------+----------------+------------
			//                 |                |
			//              write              play
			//
			//   X = Date to be played i.e lookahead
			//
			//   lookahead = m_LynxBufferLength - play + write

			lQueueLength = m_LynxBufferLength - dwPlayCursor + m_WriteCursor;
	    }
	    else // (m_cbBufOffset > dwPlayCursor)
	    {
		    // Our write position leads the play cursor.
			//
			//  start                                      end
			//    |             XXXXXXXXXXXXXXXX            |
			//    -------------+----------------+------------
			//                 |                |
			//               play             write
			//
			//   X = Date to be played i.e lookahead
			//
			//   lookahead = write - play

			lQueueLength = m_WriteCursor-dwPlayCursor;
		}

	}
	else
	{
			CString message ("CDirectSoundPlayer::CalcWriteSize - Error: \n");
			message += DXAppErrorToString (hr);
			gError->Warning(message);
			return 0;
	}

	lSamplesForAveQueue=lAveQueueLen-lQueueLength;

	// Stop -ve values in the case that the queue is already underrun/overrun
	if(lSamplesForAveQueue<0) lSamplesForAveQueue=0;

	if(lSamplesForAveQueue<lBytesInLynxQueue)
	{
		// Queue is overunning, check if it is allowable
		long overrun = lBytesInLynxQueue-lSamplesForAveQueue;

		// Make the result decision, truncate or allow
		if(overrun>lQueueMaxDelta) result=lSamplesForAveQueue; else result=lBytesInLynxQueue;

	}
	else if(lSamplesForAveQueue>lBytesInLynxQueue)
	{
		// Queue is underrunning, check if allowable
		long underrun = lSamplesForAveQueue-lBytesInLynxQueue;

		// Make the result decision, truncate or allow
		if(underrun>lQueueMaxDelta) result=lSamplesForAveQueue; else result=lBytesInLynxQueue;
	}
	else
	{
		// The ideal in==out
		result=lBytesInLynxQueue;
	}

	// 

//	CString dbgmsg;
//	dbgmsg.Format("AVEQ=%05d  QLEN=%05d  SFAQ=%05d  LYNX=%05d  RES=%05d\n",lAveQueueLen,lQueueLength,lSamplesForAveQueue,lBytesInLynxQueue,result);
//	dbg.Write(dbgmsg,dbgmsg.GetLength());

	return result;
}
