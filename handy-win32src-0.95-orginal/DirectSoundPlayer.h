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

// DirectSoundPlayer.h: interface for the CDirectSoundPlayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIRECTSOUNDPLAYER_H__EF893E01_6C53_11D2_8E90_9EDB8A2AF51F__INCLUDED_)
#define AFX_DIRECTSOUNDPLAYER_H__EF893E01_6C53_11D2_8E90_9EDB8A2AF51F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "machine.h"
#include "directx.h"

class CDirectSoundPlayer  
{
	public:
		void Flush();
		CDirectSoundPlayer();
		virtual ~CDirectSoundPlayer();
		bool Create(CWnd *pcwnd, UBYTE *lynxbuff, ULONG *lynxpoint ,ULONG lynxbuflen, ULONG lynxfreq);
		void Destroy(CWnd *pcwnd);
		bool Stop();
		bool Start();
		bool SetStatus(bool sound);
		bool GetStatus();

	private:
		DWORD CalcWriteSize();
		static void CALLBACK TimerCallBack(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

		bool m_InitOK;
		IDirectSoundPtr       m_ids;     
        IDirectSoundBufferPtr m_BufferPrimary;
        IDirectSoundBufferPtr m_BufferSecondary;
		MMRESULT m_TimerID;

		UBYTE *m_LynxBufferBasePtr;
		ULONG *m_LynxBufferIndexPtr;
		ULONG m_LynxBufferLength;
		ULONG m_LynxSampleFreq;
		bool m_AudioEnabled;

		DWORD m_WriteCursor;	// Our private write cursor as the directsound one is not useable
		LONG m_InService;		// Re-entrance flag for timer callback
};

#endif // !defined(AFX_DIRECTSOUNDPLAYER_H__EF893E01_6C53_11D2_8E90_9EDB8A2AF51F__INCLUDED_)
