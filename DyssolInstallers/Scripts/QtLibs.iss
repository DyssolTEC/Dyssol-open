; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#include "CommonConstants.iss"

#dim QtLibs[3]
#define QtLibs[0] "Qt5Core"
#define QtLibs[1] "Qt5Gui"
#define QtLibs[2] "Qt5Widgets"

#dim QtLibsPlatforms[3]
#define QtLibsPlatforms[0] "qminimal"
#define QtLibsPlatforms[1] "qoffscreen"
#define QtLibsPlatforms[2] "qwindows"

#define I

[Files]
#sub QtLibsFileEntry
#ifdef IsIncludeX32
Source: "..\Data\QtLibs2015\{#QtLibs[I]}.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "..\Data\QtLibs2015\{#QtLibs[I]}d.dll"; DestDir: "{app}\{code:DirCppTemplate}\{code:DirDebugExe}"; Flags: ignoreversion; Check: not Is64BitInstallMode
#endif
#ifdef IsIncludeX64
Source: "..\Data\QtLibs2015_64\{#QtLibs[I]}.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "..\Data\QtLibs2015_64\{#QtLibs[I]}d.dll"; DestDir: "{app}\{code:DirCppTemplate}\{code:DirDebugExe}"; Flags: ignoreversion; Check: Is64BitInstallMode
#endif
#endsub
#for {I = 0; I < DimOf(QtLibs); I++} QtLibsFileEntry

#sub QtLibsPlatformsFileEntry
#ifdef IsIncludeX32
Source: "..\Data\QtLibs2015\platforms\{#QtLibsPlatforms[I]}.dll"; DestDir: "{app}\{code:DirQtPlatforms}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "..\Data\QtLibs2015\platforms\{#QtLibsPlatforms[I]}d.dll"; DestDir: "{app}\{code:DirCppTemplate}\{code:DirDebugExe}\{code:DirQtPlatforms}"; Flags: ignoreversion; Check: not Is64BitInstallMode
#endif
#ifdef IsIncludeX64
Source: "..\Data\QtLibs2015_64\platforms\{#QtLibsPlatforms[I]}.dll"; DestDir: "{app}\{code:DirQtPlatforms}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "..\Data\QtLibs2015_64\platforms\{#QtLibsPlatforms[I]}d.dll"; DestDir: "{app}\{code:DirCppTemplate}\{code:DirDebugExe}\{code:DirQtPlatforms}"; Flags: ignoreversion; Check: Is64BitInstallMode
#endif
#endsub
#for {I = 0; I < DimOf(QtLibsPlatforms); I++} QtLibsPlatformsFileEntry

[Dirs]
Name: "{app}\{code:DirQtPlatforms}"; Flags: uninsalwaysuninstall
Name: "{app}\{code:DirCppTemplate}\{code:DirDebugExe}"; Flags: uninsalwaysuninstall
Name: "{app}\{code:DirCppTemplate}\{code:DirDebugExe}\{code:DirQtPlatforms}"; Flags: uninsalwaysuninstall
