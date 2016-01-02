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

#if !defined(_INC_DIRECTX_)
#define _INC_DIRECTX_

// -- make sure the guids are linked in --
#pragma comment (lib, "dxguid")
#pragma comment (lib, "ddraw")
#pragma comment (lib, "dsound")

// -- inlcude all direct x headers here --
#include <ddraw.h>
#include <d3d.h>
#include <dsound.h>

#include <afxcom_.h>
#include <comdef.h>

// -- interface pointer typedefs --

DEFINE_IPTR (IDirectDraw2);
DEFINE_IPTR (IDirectDrawSurface);
DEFINE_IPTR (IDirectDrawClipper);
DEFINE_IPTR (IDirectSound);
DEFINE_IPTR (IDirectSoundBuffer);

template <class T> class DXStruct : public T
{
    public:
        DXStruct ()
        {
            reset ();
        }

        void reset ()
        {
            memset(this, 0, sizeof(T));
            dwSize = sizeof(T);
        }
};


class _hresult
{
    public:
	    inline void operator=(HRESULT hr)
        {
            if (FAILED(hr)) throw _com_error (hr);
        }
};


extern char* DXAppErrorToString(HRESULT error);


#endif