; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#include "CommonConstants.iss"

#dim SolversEx[3]
#define SolversEx[0] "AgglomerationCellAverage"
#define SolversEx[1] "AgglomerationFFT"
#define SolversEx[2] "AgglomerationFixedPivot"
#define I

[Files]
#sub SolversExFileEntry
Source: "..\..\Solvers\{#SolversEx[I]}\*.cpp";     DestDir: "{app}\{code:DirExampleSolvers}\{#SolversEx[I]}"; Flags: ignoreversion
Source: "..\..\Solvers\{#SolversEx[I]}\*.h";       DestDir: "{app}\{code:DirExampleSolvers}\{#SolversEx[I]}"; Flags: ignoreversion
Source: "..\..\Solvers\{#SolversEx[I]}\*.vcxproj"; DestDir: "{app}\{code:DirExampleSolvers}\{#SolversEx[I]}"; Flags: ignoreversion
#endsub
#for {I = 0; I < DimOf(SolversEx); I++} SolversExFileEntry

[Dirs]
Name: "{app}\{code:DirExampleSolvers}";                 Flags: uninsalwaysuninstall
#sub SolversExDirEntry
Name: "{app}\{code:DirExampleSolvers}\{#SolversEx[I]}"; Flags: uninsalwaysuninstall
#endsub
#for {I = 0; I < DimOf(SolversEx); I++} SolversExDirEntry
