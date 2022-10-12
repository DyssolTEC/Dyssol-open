; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#include "CommonConstants.iss"

#dim UnitsEx[16]            ; units to include in version with sources (full)
#define UnitsEx[ 0] "Agglomerator"
#define UnitsEx[ 1] "Bunker"
#define UnitsEx[ 2] "Crusher"
#define UnitsEx[ 3] "CrusherPBMTM"
#define UnitsEx[ 4] "Cyclone2"
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

#dim UnitsExNoSrc[16]       ; units to include in version without sources
#define UnitsExNoSrc[ 0] "Agglomerator"
#define UnitsExNoSrc[ 1] "Bunker"
#define UnitsExNoSrc[ 2] "Crusher"
#define UnitsExNoSrc[ 3] "CrusherPBMTM"
#define UnitsExNoSrc[ 4] "Cyclone2"
#define UnitsExNoSrc[ 5] "Granulator"
#define UnitsExNoSrc[ 6] "HeatExchanger"
#define UnitsExNoSrc[ 7] "Inlet"
#define UnitsExNoSrc[ 8] "Mixer"
#define UnitsExNoSrc[ 9] "Mixer3"
#define UnitsExNoSrc[10] "Outlet"
#define UnitsExNoSrc[11] "Screen"
#define UnitsExNoSrc[12] "Splitter"
#define UnitsExNoSrc[13] "Splitter3"
#define UnitsExNoSrc[14] "TimeDelay"
#define UnitsExNoSrc[15] "GranulatorSimpleBatch"

#define I

[Files]
#ifdef IsWithSrc
  #sub UnitsExFileEntry
Source: "..\..\Units\{#UnitsEx[I]}\*.cpp"; DestDir: "{app}\{code:DirExampleUnits}\{#UnitsEx[I]}"; Flags: ignoreversion
Source: "..\..\Units\{#UnitsEx[I]}\*.h"; DestDir: "{app}\{code:DirExampleUnits}\{#UnitsEx[I]}"; Flags: ignoreversion
Source: "..\..\Units\{#UnitsEx[I]}\*.vcxproj"; DestDir: "{app}\{code:DirExampleUnits}\{#UnitsEx[I]}"; Flags: ignoreversion
  #endsub
  #for {I = 0; I < DimOf(UnitsEx); I++} UnitsExFileEntry
#else
  #sub UnitsExNoSrcFileEntry
Source: "..\..\Units\{#UnitsExNoSrc[I]}\*.cpp"; DestDir: "{app}\{code:DirExampleUnits}\{#UnitsExNoSrc[I]}"; Flags: ignoreversion
Source: "..\..\Units\{#UnitsExNoSrc[I]}\*.h"; DestDir: "{app}\{code:DirExampleUnits}\{#UnitsExNoSrc[I]}"; Flags: ignoreversion
Source: "..\..\Units\{#UnitsExNoSrc[I]}\*.vcxproj"; DestDir: "{app}\{code:DirExampleUnits}\{#UnitsExNoSrc[I]}"; Flags: ignoreversion
  #endsub
  #for {I = 0; I < DimOf(UnitsExNoSrc); I++} UnitsExNoSrcFileEntry
#endif

[Dirs]
Name: "{app}\{code:DirExampleUnits}"; Flags: uninsalwaysuninstall
#ifdef IsWithSrc
  #sub UnitsExDirEntry
Name: "{app}\{code:DirExampleUnits}\{#UnitsEx[I]}"; Flags: uninsalwaysuninstall
  #endsub
  #for {I = 0; I < DimOf(UnitsEx); I++} UnitsExDirEntry
#else
  #sub UnitsExNoSrcDirEntry
Name: "{app}\{code:DirExampleUnits}\{#UnitsExNoSrc[I]}"; Flags: uninsalwaysuninstall
  #endsub
  #for {I = 0; I < DimOf(UnitsExNoSrc); I++} UnitsExNoSrcDirEntry
#endif
