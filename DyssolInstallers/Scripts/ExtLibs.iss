; Copyright (c) 2023, DyssolTEC GmbH. All rights reserved. This file is part of Dyssol. See LICENSE file for license information.

#include "CommonConstants.iss"

#dim ExternalLibs[3]
#define ExternalLibs[0] "hdf5"
#define ExternalLibs[1] "sundials"
#define ExternalLibs[2] "zlib"

#define I

[Files]
; External libraries
#sub ExternalLibsFileEntry
Source: "..\..\ExternalLibraries\{#ExternalLibs[I]}\include\*"; DestDir: "{app}\{code:DirModelsCreator}\{code:DirExternalLibs}\{#ExternalLibs[I]}\include"; Flags: ignoreversion createallsubdirs recursesubdirs
  #ifdef IsIncludeX32
Source: "..\..\ExternalLibraries\{#ExternalLibs[I]}\lib32\*";   DestDir: "{app}\{code:DirModelsCreator}\{code:DirExternalLibs}\{#ExternalLibs[I]}\lib32";   Flags: ignoreversion createallsubdirs recursesubdirs; Check: not Is64BitInstallMode
  #endif
  #ifdef IsIncludeX64
Source: "..\..\ExternalLibraries\{#ExternalLibs[I]}\lib64\*";   DestDir: "{app}\{code:DirModelsCreator}\{code:DirExternalLibs}\{#ExternalLibs[I]}\lib64";   Flags: ignoreversion createallsubdirs recursesubdirs; Check: Is64BitInstallMode
  #endif
#endsub
#for {I = 0; I < DimOf(ExternalLibs); I++} ExternalLibsFileEntry

