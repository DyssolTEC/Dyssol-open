; Copyright (c) 2020, Dyssol Development Team. 
; Copyright (c) 2024, DyssolTEC GmbH. 
; All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#include "CommonConstants.iss"
#include "UnitsList.iss"

[Files]
#sub UnitsDllFileEntry
Source: "..\..\x64\Release\Unit_{#UnitsDll[I]}.dll";   DestDir: "{app}\{code:DirUnitsDll}";                              Flags: ignoreversion
#ifdef IsWithSDK
Source: "..\..\x64\Debug\Unit_{#UnitsDll[I]}.dll";     DestDir: "{app}\{code:DirModelsCreator}\{code:DirUnitsDebugDll}"; Flags: ignoreversion
#endif
#endsub
#for {I = 0; I < DimOf(UnitsDll); I++} UnitsDllFileEntry

[Dirs]
Name: "{app}\{code:DirUnitsDll}";                              Flags: uninsalwaysuninstall
#ifdef IsWithSDK
Name: "{app}\{code:DirModelsCreator}\{code:DirUnitsDebugDll}"; Flags: uninsalwaysuninstall
#endif
