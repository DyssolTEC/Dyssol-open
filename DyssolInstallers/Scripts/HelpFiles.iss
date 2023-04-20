; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#include "CommonConstants.iss"

[Files]
Source: "..\..\install\share\Dyssol\docs\sphinx\html\*"; DestDir: "{app}\{code:DirHelp}"; Flags: ignoreversion recursesubdirs

[Dirs]
Name: "{app}\{code:DirHelp}"; Flags: uninsalwaysuninstall