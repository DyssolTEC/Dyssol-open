; Copyright (c) 2020, Dyssol Development Team. 
; Copyright (c) 2024, DyssolTEC GmbH. 
; All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#include "CommonConstants.iss"

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
Source: "{#QtPath}\bin\{#QtLibs[I]}.dll";  DestDir: "{app}";                                            Flags: ignoreversion
#ifdef IsWithSDK
Source: "{#QtPath}\bin\{#QtLibs[I]}d.dll"; DestDir: "{app}\{code:DirModelsCreator}\{code:DirDebugExe}"; Flags: ignoreversion
#endif
#endsub
#for {I = 0; I < DimOf(QtLibs); I++} QtLibsFileEntry

#sub QtLibsPlatformsFileEntry
Source: "{#QtPath}\plugins\platforms\{#QtLibsPlatforms[I]}.dll";  DestDir: "{app}\{code:DirQtPlatforms}";                                            Flags: ignoreversion
#ifdef IsWithSDK
Source: "{#QtPath}\plugins\platforms\{#QtLibsPlatforms[I]}d.dll"; DestDir: "{app}\{code:DirModelsCreator}\{code:DirDebugExe}\{code:DirQtPlatforms}"; Flags: ignoreversion
#endif
#endsub
#for {I = 0; I < DimOf(QtLibsPlatforms); I++} QtLibsPlatformsFileEntry

#sub QtLibsStylesFileEntry
Source: "{#QtPath}\plugins\styles\{#QtLibsStyles[I]}.dll";  DestDir: "{app}\{code:DirQtStyles}";                                            Flags: ignoreversion
#ifdef IsWithSDK
Source: "{#QtPath}\plugins\styles\{#QtLibsStyles[I]}d.dll"; DestDir: "{app}\{code:DirModelsCreator}\{code:DirDebugExe}\{code:DirQtStyles}"; Flags: ignoreversion
#endif
#endsub
#for {I = 0; I < DimOf(QtLibsStyles); I++} QtLibsStylesFileEntry

[Dirs]
Name: "{app}\{code:DirQtPlatforms}";                                            Flags: uninsalwaysuninstall
Name: "{app}\{code:DirQtStyles}";                                               Flags: uninsalwaysuninstall
#ifdef IsWithSDK
Name: "{app}\{code:DirModelsCreator}\{code:DirDebugExe}";                       Flags: uninsalwaysuninstall
Name: "{app}\{code:DirModelsCreator}\{code:DirDebugExe}\{code:DirQtPlatforms}"; Flags: uninsalwaysuninstall
Name: "{app}\{code:DirModelsCreator}\{code:DirDebugExe}\{code:DirQtStyles}";    Flags: uninsalwaysuninstall
#endif
