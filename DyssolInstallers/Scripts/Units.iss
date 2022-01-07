; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#include "CommonConstants.iss"

#dim UnitsDll[16]
#define UnitsDll[ 0] "Agglomerator"
#define UnitsDll[ 1] "Bunker"
#define UnitsDll[ 2] "Crusher"
#define UnitsDll[ 3] "CrusherPBMTM"
#define UnitsDll[ 4] "Cyclone2"
#define UnitsDll[ 5] "Granulator"
#define UnitsDll[ 6] "HeatExchanger"
#define UnitsDll[ 7] "Inlet"
#define UnitsDll[ 8] "Mixer"
#define UnitsDll[ 9] "Mixer3"
#define UnitsDll[10] "Outlet"
#define UnitsDll[11] "Screen"
#define UnitsDll[12] "ScreenMultideck"
#define UnitsDll[13] "Splitter"
#define UnitsDll[14] "Splitter3"
#define UnitsDll[15] "TimeDelay"
#define I

[Files]
#sub UnitsDllFileEntry
#ifdef IsIncludeX32
Source: "..\..\Win32\Release\Unit_{#UnitsDll[I]}.dll"; DestDir: "{app}\{code:DirUnitsDll}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "..\..\Win32\Debug\Unit_{#UnitsDll[I]}.dll"; DestDir: "{app}\{code:DirCppTemplate}\{code:DirUnitsDebugDll}"; Flags: ignoreversion; Check: not Is64BitInstallMode
#endif
#ifdef IsIncludeX64
Source: "..\..\x64\Release\Unit_{#UnitsDll[I]}.dll"; DestDir: "{app}\{code:DirUnitsDll}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "..\..\x64\Debug\Unit_{#UnitsDll[I]}.dll"; DestDir: "{app}\{code:DirCppTemplate}\{code:DirUnitsDebugDll}"; Flags: ignoreversion; Check: Is64BitInstallMode
#endif
#endsub
#for {I = 0; I < DimOf(UnitsDll); I++} UnitsDllFileEntry

[Dirs]
Name: "{app}\{code:DirUnitsDll}"; Flags: uninsalwaysuninstall
Name: "{app}\{code:DirCppTemplate}\{code:DirUnitsDebugDll}"; Flags: uninsalwaysuninstall
