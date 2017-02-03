# Microsoft Developer Studio Project File - Name="PathFinder2D" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PathFinder2D - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PathFinder2D.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PathFinder2D.mak" CFG="PathFinder2D - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PathFinder2D - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PathFinder2D - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "PathFinder2D - Win32 Profile" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PathFinder2D - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
HCW=hcw.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /Gr /MT /W3 /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FAs /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:windows /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=hcw /c /m /e pathfinder2d.hpj	copy release\pathfinder2d.exe pathfinder2d.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PathFinder2D - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
HCW=hcw.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "PathFinder2D - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PathFinder2D___Win32_Profile"
# PROP BASE Intermediate_Dir "PathFinder2D___Win32_Profile"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
HCW=hcw.exe
# ADD BASE CPP /nologo /G6 /Gr /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FAs /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /O2 /Ob0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FAs /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:windows /verbose /profile /map /debug /debugtype:both /machine:I386

!ENDIF 

# Begin Target

# Name "PathFinder2D - Win32 Release"
# Name "PathFinder2D - Win32 Debug"
# Name "PathFinder2D - Win32 Profile"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Astar.cpp
# End Source File
# Begin Source File

SOURCE=.\AStarArray.cpp
# End Source File
# Begin Source File

SOURCE=.\AStarComplete.cpp
# End Source File
# Begin Source File

SOURCE=.\AStarHeap.cpp
# End Source File
# Begin Source File

SOURCE=.\AStarHeapInteger.cpp
# End Source File
# Begin Source File

SOURCE=.\AStarLinkedList.cpp
# End Source File
# Begin Source File

SOURCE=.\BestFirst.cpp

!IF  "$(CFG)" == "PathFinder2D - Win32 Release"

!ELSEIF  "$(CFG)" == "PathFinder2D - Win32 Debug"

!ELSEIF  "$(CFG)" == "PathFinder2D - Win32 Profile"

# ADD CPP /W3

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\BreadthFirst.cpp
# End Source File
# Begin Source File

SOURCE=.\DepthFirst.cpp
# End Source File
# Begin Source File

SOURCE=.\Development.cpp
# End Source File
# Begin Source File

SOURCE=.\Dijkstra.cpp
# End Source File
# Begin Source File

SOURCE=.\DStar.cpp
# End Source File
# Begin Source File

SOURCE=.\Generic.cpp
# End Source File
# Begin Source File

SOURCE=.\PathFinder2D.cpp
# End Source File
# Begin Source File

SOURCE=.\PathFinder2D.rc
# End Source File
# Begin Source File

SOURCE=.\RightHandRule.cpp
# End Source File
# Begin Source File

SOURCE=.\Setup.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AStar.h
# End Source File
# Begin Source File

SOURCE=.\AStarArray.h
# End Source File
# Begin Source File

SOURCE=.\AStarComplete.h
# End Source File
# Begin Source File

SOURCE=.\AStarHeap.h
# End Source File
# Begin Source File

SOURCE=.\AStarHeapInteger.h
# End Source File
# Begin Source File

SOURCE=.\AStarLinkedList.h
# End Source File
# Begin Source File

SOURCE=.\BestFirst.h
# End Source File
# Begin Source File

SOURCE=.\BreadthFirst.h
# End Source File
# Begin Source File

SOURCE=.\DepthFirst.h
# End Source File
# Begin Source File

SOURCE=.\Development.h
# End Source File
# Begin Source File

SOURCE=.\Dijkstra.h
# End Source File
# Begin Source File

SOURCE=.\DStar.h
# End Source File
# Begin Source File

SOURCE=.\Generic.h
# End Source File
# Begin Source File

SOURCE=.\PathFinder2D.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\RightHandRule.h
# End Source File
# Begin Source File

SOURCE=.\Setup.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\arrow.cur
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\PathFinder2D.ico
# End Source File
# Begin Source File

SOURCE=.\small.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\PathFinder2D.dib
# End Source File
# Begin Source File

SOURCE=.\PathFinder2D_about.wav
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
