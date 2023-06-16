; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#include "CommonConstants.iss"

#dim UnitsEx[16]
#define UnitsEx[ 0] "Agglomerator"
#define UnitsEx[ 1] "Bunker"
#define UnitsEx[ 2] "Crusher"
#define UnitsEx[ 3] "CrusherPBMTM"
#define UnitsEx[ 4] "CycloneMuschelknautz"
#define UnitsEx[ 5] "Granulator"
#define UnitsEx[ 6] "HeatExchanger"
#define UnitsEx[ 7] "Inlet"
#define UnitsEx[ 8] "Mixer"
#define UnitsEx[ 9] "Mixer3"
#define UnitsEx[10] "Outlet"
#define UnitsEx[11] "Screen"
#define UnitsEx[12] "Splitter"
#define UnitsEx[13] "Splitter3"
#define UnitsEx[14] "TimeDelay"
#define UnitsEx[15] "GranulatorSimpleBatch"
#define I

[Files]
#sub UnitsExFileEntry
Source: "..\..\Units\{#UnitsEx[I]}\*.cpp";     DestDir: "{app}\{code:DirExampleUnits}\{#UnitsEx[I]}"; Flags: ignoreversion
Source: "..\..\Units\{#UnitsEx[I]}\*.h";       DestDir: "{app}\{code:DirExampleUnits}\{#UnitsEx[I]}"; Flags: ignoreversion
Source: "..\..\Units\{#UnitsEx[I]}\*.vcxproj"; DestDir: "{app}\{code:DirExampleUnits}\{#UnitsEx[I]}"; Flags: ignoreversion
#endsub
#for {I = 0; I < DimOf(UnitsEx); I++} UnitsExFileEntry

[Dirs]
Name: "{app}\{code:DirExampleUnits}";               Flags: uninsalwaysuninstall
#sub UnitsExDirEntry
Name: "{app}\{code:DirExampleUnits}\{#UnitsEx[I]}"; Flags: uninsalwaysuninstall
#endsub
#for {I = 0; I < DimOf(UnitsEx); I++} UnitsExDirEntry
