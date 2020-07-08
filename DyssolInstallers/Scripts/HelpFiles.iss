; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#include "CommonConstants.iss"

[Files]
Source: "..\..\Documentation\*.pdf"; DestDir: "{app}\{code:DirHelp}"; Flags: ignoreversion
Source: "..\..\Documentation\Development\*.pdf"; DestDir: "{app}\{code:DirHelp}\Development"; Flags: ignoreversion
Source: "..\..\Documentation\Development\Program Interfaces\*.pdf"; DestDir: "{app}\{code:DirHelp}\Development\Program Interfaces"; Flags: ignoreversion
Source: "..\..\Documentation\Introduction\*.pdf"; DestDir: "{app}\{code:DirHelp}\Introduction"; Flags: ignoreversion
Source: "..\..\Documentation\Solvers\*.pdf"; DestDir: "{app}\{code:DirHelp}\Solvers"; Flags: ignoreversion
Source: "..\..\Documentation\Units\*.pdf"; DestDir: "{app}\{code:DirHelp}\Units"; Flags: ignoreversion

[Dirs]
Name: "{app}\{code:DirHelp}"; Flags: uninsalwaysuninstall
Name: "{app}\{code:DirHelp}\Development"; Flags: uninsalwaysuninstall
Name: "{app}\{code:DirHelp}\Development\Program Interfaces"; Flags: uninsalwaysuninstall
Name: "{app}\{code:DirHelp}\Introduction"; Flags: uninsalwaysuninstall
Name: "{app}\{code:DirHelp}\Solvers"; Flags: uninsalwaysuninstall
Name: "{app}\{code:DirHelp}\Units"; Flags: uninsalwaysuninstall
