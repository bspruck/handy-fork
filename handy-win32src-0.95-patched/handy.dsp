# Microsoft Developer Studio Project File - Name="handy" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=handy - Win32 Debug Dbg
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "handy.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "handy.mak" CFG="handy - Win32 Debug Dbg"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "handy - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "handy - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "handy - Win32 Release Dbg" (based on "Win32 (x86) Application")
!MESSAGE "handy - Win32 Debug Dbg" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "handy - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FAs /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 nafxcw.lib libcmt.lib winmm.lib wsock32.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"nafxcw.lib libcmt.lib"
# SUBTRACT LINK32 /map

!ELSEIF  "$(CFG)" == "handy - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /X /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Nafxcwd.lib Libcmtd.lib winmm.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"Nafxcwd.lib Libcmtd.lib" /pdbtype:sept
# SUBTRACT LINK32 /incremental:no /nodefaultlib

!ELSEIF  "$(CFG)" == "handy - Win32 Release Dbg"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_Dbg"
# PROP BASE Intermediate_Dir "Release_Dbg"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Dbg"
# PROP Intermediate_Dir "Release_Dbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_LYNXDBG" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 nafxcw.lib libcmt.lib winmm.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"nafxcw.lib libcmt.lib"
# ADD LINK32 nafxcw.lib libcmt.lib winmm.lib wsock32.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"nafxcw.lib libcmt.lib" /out:"Release_Dbg/handybug.exe"

!ELSEIF  "$(CFG)" == "handy - Win32 Debug Dbg"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug_Dbg"
# PROP BASE Intermediate_Dir "Debug_Dbg"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Dbg"
# PROP Intermediate_Dir "Debug_Dbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_LYNXDBG" /FR /FD /GZ /c
# SUBTRACT BASE CPP /X /YX
# ADD CPP /nologo /G5 /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_LYNXDBG" /FR /FD /GZ /c
# SUBTRACT CPP /X /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 Nafxcwd.lib Libcmtd.lib winmm.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"Nafxcwd.lib Libcmtd.lib" /pdbtype:sept
# SUBTRACT BASE LINK32 /incremental:no /nodefaultlib
# ADD LINK32 Nafxcwd.lib Libcmtd.lib winmm.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"Nafxcwd.lib Libcmtd.lib" /out:"Debug_Dbg/handybug.exe" /pdbtype:sept
# SUBTRACT LINK32 /incremental:no /nodefaultlib

!ENDIF 

# Begin Target

# Name "handy - Win32 Release"
# Name "handy - Win32 Debug"
# Name "handy - Win32 Release Dbg"
# Name "handy - Win32 Debug Dbg"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\zlib-113\adler32.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\compress.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\crc32.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\deflate.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\deflate.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\gzio.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\infblock.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\infblock.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\infcodes.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\infcodes.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\inffast.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\inffast.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\inffixed.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\inflate.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\inftrees.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\inftrees.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\infutil.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\infutil.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\trees.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\trees.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\uncompr.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\unzip.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\unzip.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\zconf.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\zlib.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\zutil.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-113\zutil.h"
# End Source File
# End Group
# Begin Source File

SOURCE=.\Cart.cpp
# End Source File
# Begin Source File

SOURCE=.\Codewin.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorStatic.cpp
# End Source File
# Begin Source File

SOURCE=.\Debugger.cpp
# End Source File
# Begin Source File

SOURCE=.\Debuggerwin.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectSoundPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\DispLine.cpp
# End Source File
# Begin Source File

SOURCE=.\Dmpwndlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Dumpwin.cpp
# End Source File
# Begin Source File

SOURCE=.\DxError.cpp
# End Source File
# Begin Source File

SOURCE=.\EditLine.cpp
# End Source File
# Begin Source File

SOURCE=.\ErrorHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\gfxwin.cpp
# End Source File
# Begin Source File

SOURCE=.\HexEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\keydefs.cpp
# End Source File
# Begin Source File

SOURCE=.\Lynxapp.cpp
# End Source File
# Begin Source File

SOURCE=.\Lynxwin.cpp
# End Source File
# Begin Source File

SOURCE=.\Memmap.cpp
# End Source File
# Begin Source File

SOURCE=.\Mikie.cpp
# End Source File
# Begin Source File

SOURCE=.\netobj.cpp
# End Source File
# Begin Source File

SOURCE=.\Ram.cpp
# End Source File
# Begin Source File

SOURCE=.\Rom.cpp
# End Source File
# Begin Source File

SOURCE=.\Susie.cpp
# End Source File
# Begin Source File

SOURCE=.\System.cpp
# End Source File
# Begin Source File

SOURCE=.\Tracedlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Tracewin.cpp
# End Source File
# Begin Source File

SOURCE=.\windowgdi.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Aboutdlg.h
# End Source File
# Begin Source File

SOURCE=.\C6502mak.h
# End Source File
# Begin Source File

SOURCE=.\C65c02.h
# End Source File
# Begin Source File

SOURCE=.\Cart.h
# End Source File
# Begin Source File

SOURCE=.\CodeWin.h
# End Source File
# Begin Source File

SOURCE=.\Color.h
# End Source File
# Begin Source File

SOURCE=.\ColorStatic.h
# End Source File
# Begin Source File

SOURCE=.\Debugger.h
# End Source File
# Begin Source File

SOURCE=.\Debuggerwin.h
# End Source File
# Begin Source File

SOURCE=.\DirectSoundPlayer.h
# End Source File
# Begin Source File

SOURCE=.\DirectX.h
# End Source File
# Begin Source File

SOURCE=.\Dis6502.h
# End Source File
# Begin Source File

SOURCE=.\DispLine.h
# End Source File
# Begin Source File

SOURCE=.\Dmpwndlg.h
# End Source File
# Begin Source File

SOURCE=.\DumpWin.h
# End Source File
# Begin Source File

SOURCE=.\EditLine.h
# End Source File
# Begin Source File

SOURCE=.\Error.h
# End Source File
# Begin Source File

SOURCE=.\ErrorHandler.h
# End Source File
# Begin Source File

SOURCE=.\ErrorInterface.h
# End Source File
# Begin Source File

SOURCE=.\FullScreenDirectX.h
# End Source File
# Begin Source File

SOURCE=.\FullScreenEagle.h
# End Source File
# Begin Source File

SOURCE=.\fullscreenlcd.h
# End Source File
# Begin Source File

SOURCE=.\gfxwin.h
# End Source File
# Begin Source File

SOURCE=.\HexEdit.h
# End Source File
# Begin Source File

SOURCE=.\keydefs.h
# End Source File
# Begin Source File

SOURCE=.\Lynxapp.h
# End Source File
# Begin Source File

SOURCE=.\lynxbase.h
# End Source File
# Begin Source File

SOURCE=.\Lynxdef.h
# End Source File
# Begin Source File

SOURCE=.\lynxrender.h
# End Source File
# Begin Source File

SOURCE=.\Lynxwin.h
# End Source File
# Begin Source File

SOURCE=.\Machine.h
# End Source File
# Begin Source File

SOURCE=.\Memfault.h
# End Source File
# Begin Source File

SOURCE=.\Memmap.h
# End Source File
# Begin Source File

SOURCE=.\Mikie.h
# End Source File
# Begin Source File

SOURCE=.\netobj.h
# End Source File
# Begin Source File

SOURCE=.\pixblend.h
# End Source File
# Begin Source File

SOURCE=.\Ram.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\Rom.h
# End Source File
# Begin Source File

SOURCE=.\Susie.h
# End Source File
# Begin Source File

SOURCE=.\Sysbase.h
# End Source File
# Begin Source File

SOURCE=.\System.h
# End Source File
# Begin Source File

SOURCE=.\Tracedlg.h
# End Source File
# Begin Source File

SOURCE=.\Tracewin.h
# End Source File
# Begin Source File

SOURCE=.\warndlg.h
# End Source File
# Begin Source File

SOURCE=.\widget.h
# End Source File
# Begin Source File

SOURCE=.\WindowDirectX.h
# End Source File
# Begin Source File

SOURCE=.\WindowEagle.h
# End Source File
# Begin Source File

SOURCE=.\windowgdi.h
# End Source File
# Begin Source File

SOURCE=.\windowlcd.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\ICON1.ICO
# End Source File
# Begin Source File

SOURCE=.\LynxBackground.bmp
# End Source File
# Begin Source File

SOURCE=.\LynxBackground1.bmp
# End Source File
# Begin Source File

SOURCE=.\lynxbackground2.bmp
# End Source File
# Begin Source File

SOURCE=.\lynxbackground3.bmp
# End Source File
# Begin Source File

SOURCE=.\lynxmenu.rc
# End Source File
# End Group
# Begin Group "Support Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\public\Contents.txt
# End Source File
# Begin Source File

SOURCE=.\public\handybug\dvreadme.txt
# End Source File
# Begin Source File

SOURCE=.\public\license.txt
# End Source File
# Begin Source File

SOURCE=.\public\Readme.txt
# End Source File
# Begin Source File

SOURCE=.\public\whatsnew.txt
# End Source File
# End Group
# End Target
# End Project
