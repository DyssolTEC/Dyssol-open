; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#include "CommonConstants.iss"

#dim SolversEx[3]       ; solvers to include in version with sources (full)
#define SolversEx[0] "AgglomerationCellAverage"
#define SolversEx[1] "AgglomerationFFT"
#define SolversEx[2] "AgglomerationFixedPivot"
#define I

#dim SolversExNoSrc[1]  ; solvers to include in version without sources
#define SolversExNoSrc[0] "AgglomerationFixedPivot"

[Files]
#ifdef IsWithSrc
  #sub SolversExFileEntry
Source: "..\..\Solvers\{#SolversEx[I]}\*.cpp"; DestDir: "{app}\{code:DirExampleSolvers}\{#SolversEx[I]}"; Flags: ignoreversion
Source: "..\..\Solvers\{#SolversEx[I]}\*.h"; DestDir: "{app}\{code:DirExampleSolvers}\{#SolversEx[I]}"; Flags: ignoreversion
Source: "..\..\Solvers\{#SolversEx[I]}\*.vcxproj"; DestDir: "{app}\{code:DirExampleSolvers}\{#SolversEx[I]}"; Flags: ignoreversion
  #endsub
  #for {I = 0; I < DimOf(SolversEx); I++} SolversExFileEntry
#else
  #sub SolversExNoSrcFileEntry
Source: "..\..\Solvers\{#SolversExNoSrc[I]}\*.cpp"; DestDir: "{app}\{code:DirExampleSolvers}\{#SolversExNoSrc[I]}"; Flags: ignoreversion
Source: "..\..\Solvers\{#SolversExNoSrc[I]}\*.h"; DestDir: "{app}\{code:DirExampleSolvers}\{#SolversExNoSrc[I]}"; Flags: ignoreversion
Source: "..\..\Solvers\{#SolversExNoSrc[I]}\*.vcxproj"; DestDir: "{app}\{code:DirExampleSolvers}\{#SolversExNoSrc[I]}"; Flags: ignoreversion
  #endsub
  #for {I = 0; I < DimOf(SolversExNoSrc); I++} SolversExNoSrcFileEntry
#endif

[Dirs]
Name: "{app}\{code:DirExampleSolvers}"; Flags: uninsalwaysuninstall
#ifdef IsWithSrc
  #sub SolversExDirEntry
Name: "{app}\{code:DirExampleSolvers}\{#SolversEx[I]}"; Flags: uninsalwaysuninstall
  #endsub
  #for {I = 0; I < DimOf(SolversEx); I++} SolversExDirEntry
#else
  #sub SolversExNoSrcDirEntry
Name: "{app}\{code:DirExampleSolvers}\{#SolversExNoSrc[I]}"; Flags: uninsalwaysuninstall
  #endsub
  #for {I = 0; I < DimOf(SolversExNoSrc); I++} SolversExNoSrcDirEntry
#endif
