; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#include "CommonConstants.iss"

#define QtConfig "msvc2019"
#define QtPath32 QtPath + "\" + QtConfig
#define QtPath64 QtPath + "\" + QtConfig + "_64"

#dim QtLibs[3]
#define QtLibs[0] "Qt5Core"
#define QtLibs[1] "Qt5Gui"
#define QtLibs[2] "Qt5Widgets"

#dim QtLibsPlatforms[1]
#define QtLibsPlatforms[0] "qwindows"

#dim QtLibsStyles[1]
#define QtLibsStyles[0] "qwindowsvistastyle"

#define I

[Files]
#sub QtLibsFileEntry
#ifdef IsIncludeX32
Source: "{#QtPath32}\bin\{#QtLibs[I]}.dll";  DestDir: "{app}";                                          Flags: ignoreversion; Check: not Is64BitInstallMode
  #ifdef IsWithSDK
Source: "{#QtPath32}\bin\{#QtLibs[I]}d.dll"; DestDir: "{app}\{code:DirCppTemplate}\{code:DirDebugExe}"; Flags: ignoreversion; Check: not Is64BitInstallMode
  #endif
#endif
#ifdef IsIncludeX64
Source: "{#QtPath64}\bin\{#QtLibs[I]}.dll";  DestDir: "{app}";                                          Flags: ignoreversion; Check: Is64BitInstallMode
  #ifdef IsWithSDK
Source: "{#QtPath64}\bin\{#QtLibs[I]}d.dll"; DestDir: "{app}\{code:DirCppTemplate}\{code:DirDebugExe}"; Flags: ignoreversion; Check: Is64BitInstallMode
  #endif
#endif
#endsub
#for {I = 0; I < DimOf(QtLibs); I++} QtLibsFileEntry

#sub QtLibsPlatformsFileEntry
#ifdef IsIncludeX32
Source: "{#QtPath32}\plugins\platforms\{#QtLibsPlatforms[I]}.dll";  DestDir: "{app}\{code:DirQtPlatforms}";                                          Flags: ignoreversion; Check: not Is64BitInstallMode
  #ifdef IsWithSDK
Source: "{#QtPath32}\plugins\platforms\{#QtLibsPlatforms[I]}d.dll"; DestDir: "{app}\{code:DirCppTemplate}\{code:DirDebugExe}\{code:DirQtPlatforms}"; Flags: ignoreversion; Check: not Is64BitInstallMode
  #endif
#endif
#ifdef IsIncludeX64
Source: "{#QtPath64}\plugins\platforms\{#QtLibsPlatforms[I]}.dll";  DestDir: "{app}\{code:DirQtPlatforms}";                                          Flags: ignoreversion; Check: Is64BitInstallMode
  #ifdef IsWithSDK
Source: "{#QtPath64}\plugins\platforms\{#QtLibsPlatforms[I]}d.dll"; DestDir: "{app}\{code:DirCppTemplate}\{code:DirDebugExe}\{code:DirQtPlatforms}"; Flags: ignoreversion; Check: Is64BitInstallMode
  #endif
#endif
#endsub
#for {I = 0; I < DimOf(QtLibsPlatforms); I++} QtLibsPlatformsFileEntry

#sub QtLibsStylesFileEntry
#ifdef IsIncludeX32
Source: "{#QtPath32}\plugins\styles\{#QtLibsStyles[I]}.dll";  DestDir: "{app}\{code:DirQtStyles}";                                          Flags: ignoreversion; Check: not Is64BitInstallMode
  #ifdef IsWithSDK
Source: "{#QtPath32}\plugins\styles\{#QtLibsStyles[I]}d.dll"; DestDir: "{app}\{code:DirCppTemplate}\{code:DirDebugExe}\{code:DirQtStyles}"; Flags: ignoreversion; Check: not Is64BitInstallMode
  #endif
#endif
#ifdef IsIncludeX64
Source: "{#QtPath64}\plugins\styles\{#QtLibsStyles[I]}.dll";  DestDir: "{app}\{code:DirQtStyles}";                                          Flags: ignoreversion; Check: Is64BitInstallMode
  #ifdef IsWithSDK
Source: "{#QtPath64}\plugins\styles\{#QtLibsStyles[I]}d.dll"; DestDir: "{app}\{code:DirCppTemplate}\{code:DirDebugExe}\{code:DirQtStyles}"; Flags: ignoreversion; Check: Is64BitInstallMode
  #endif
#endif
#endsub
#for {I = 0; I < DimOf(QtLibsStyles); I++} QtLibsStylesFileEntry

[Dirs]
Name: "{app}\{code:DirQtPlatforms}";                                          Flags: uninsalwaysuninstall
Name: "{app}\{code:DirQtStyles}";                                             Flags: uninsalwaysuninstall
#ifdef IsWithSDK
Name: "{app}\{code:DirCppTemplate}\{code:DirDebugExe}";                       Flags: uninsalwaysuninstall
Name: "{app}\{code:DirCppTemplate}\{code:DirDebugExe}\{code:DirQtPlatforms}"; Flags: uninsalwaysuninstall
Name: "{app}\{code:DirCppTemplate}\{code:DirDebugExe}\{code:DirQtStyles}";    Flags: uninsalwaysuninstall
#endif
