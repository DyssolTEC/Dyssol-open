; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#include "CommonConstants.iss"

[Files]
Source: "..\..\install\share\Dyssol\docs\sphinx\html\*.html"; DestDir: "{app}\{code:DirHelp}"; Flags: ignoreversion
Source: "..\..\install\share\Dyssol\docs\sphinx\html\.doctrees\*"; DestDir: "{app}\{code:DirHelp}\.doctrees"; Flags: ignoreversion recursesubdirs
Source: "..\..\install\share\Dyssol\docs\sphinx\html\_static\*"; DestDir: "{app}\{code:DirHelp}\_static"; Flags: ignoreversion recursesubdirs

Source: "..\..\install\share\Dyssol\docs\sphinx\html\_images\*"; DestDir: "{app}\{code:DirHelp}\_images"; Flags: ignoreversion recursesubdirs
Source: "..\..\install\share\Dyssol\docs\sphinx\html\000_get_started\*"; DestDir: "{app}\{code:DirHelp}\000_get_started"; Flags: ignoreversion recursesubdirs
Source: "..\..\install\share\Dyssol\docs\sphinx\html\001_ui\*"; DestDir: "{app}\{code:DirHelp}\001_ui"; Flags: ignoreversion recursesubdirs
Source: "..\..\install\share\Dyssol\docs\sphinx\html\002_theory\*"; DestDir: "{app}\{code:DirHelp}\002_theory"; Flags: ignoreversion recursesubdirs
Source: "..\..\install\share\Dyssol\docs\sphinx\html\003_models\*"; DestDir: "{app}\{code:DirHelp}\003_models"; Flags: ignoreversion recursesubdirs
Source: "..\..\install\share\Dyssol\docs\sphinx\html\004_development\*"; DestDir: "{app}\{code:DirHelp}\004_development"; Flags: ignoreversion recursesubdirs

[Dirs]
Name: "{app}\{code:DirHelp}"; Flags: uninsalwaysuninstall