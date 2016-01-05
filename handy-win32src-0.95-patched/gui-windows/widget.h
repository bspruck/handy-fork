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

// Small widget to do text decoding
//
//////////////////////////////////////////////////////////////////////


class CWidget
{
	public:
		CWidget(CString string) {mString=string; mIndex=mString.GetLength()-1; mString.MakeUpper(); };
		~CWidget() {};

		int	GetNext(void)
		{
			// Sanity Check
			if(mIndex<=0)
			{
				mIndex=0;
				return 0;
			}

			// Decode
			int temp=0,loop=0,radix=1;
			for(loop=0;loop<8;loop++)
			{
				int ch=mString.GetAt(mIndex);
				if(ch>='0' && ch<='9') ch-='0'; else if (ch>='A' && ch<='F') ch=0x0a+(ch-'A'); else ch=0;
				temp+=radix*ch;
				radix*=16;
				mIndex--;
			}
			mIndex--;		// Skip the separator
			return temp;
		}

	public:
		CString mString;
		int mIndex;
};


