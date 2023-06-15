; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#include "CommonConstants.iss"

#dim SolversDll[3]
#define SolversDll[0] "AgglomerationCellAverage"
#define SolversDll[1] "AgglomerationFFT"
#define SolversDll[2] "AgglomerationFixedPivot"
#define I

[Files]
#sub SolversDllFileEntry
#ifdef IsIncludeX32
Source: "..\..\Win32\Release\Solver_{#SolversDll[I]}.dll"; DestDir: "{app}\{code:DirSolversDll}";                              Flags: ignoreversion; Check: not Is64BitInstallMode
  #ifdef IsWithSDK
Source: "..\..\Win32\Debug\Solver_{#SolversDll[I]}.dll";   DestDir: "{app}\{code:DirModelsCreator}\{code:DirSolversDebugDll}"; Flags: ignoreversion; Check: not Is64BitInstallMode
  #endif
#endif
#ifdef IsIncludeX64
Source: "..\..\x64\Release\Solver_{#SolversDll[I]}.dll";   DestDir: "{app}\{code:DirSolversDll}";                              Flags: ignoreversion; Check: Is64BitInstallMode
  #ifdef IsWithSDK
Source: "..\..\x64\Debug\Solver_{#SolversDll[I]}.dll";     DestDir: "{app}\{code:DirModelsCreator}\{code:DirSolversDebugDll}"; Flags: ignoreversion; Check: Is64BitInstallMode
  #endif
#endif
#endsub
#for {I = 0; I < DimOf(SolversDll); I++} SolversDllFileEntry

[Dirs]
Name: "{app}\{code:DirSolversDll}";                              Flags: uninsalwaysuninstall
#ifdef IsWithSDK
Name: "{app}\{code:DirModelsCreator}\{code:DirSolversDebugDll}"; Flags: uninsalwaysuninstall
#endif
