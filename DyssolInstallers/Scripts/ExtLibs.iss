; Copyright (c) 2023, DyssolTEC GmbH. 
; Copyright (c) 2024, DyssolTEC GmbH. 
; All rights reserved. This file is part of Dyssol. See LICENSE file for license information.

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
Source: "..\..\ExternalLibraries\{#ExternalLibs[I]}\lib\*";     DestDir: "{app}\{code:DirModelsCreator}\{code:DirExternalLibs}\{#ExternalLibs[I]}\lib";     Flags: ignoreversion createallsubdirs recursesubdirs
#endsub
#for {I = 0; I < DimOf(ExternalLibs); I++} ExternalLibsFileEntry

