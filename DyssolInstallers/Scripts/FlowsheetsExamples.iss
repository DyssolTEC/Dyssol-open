; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#include "CommonConstants.iss"

[Files]
Source: "..\Data\Example Flowsheets\Processes\*.dflw"; DestDir: "{app}\{code:DirExampleFlowsheets}\Processes"; Flags: ignoreversion
Source: "..\Data\Example Flowsheets\Units\*.dflw";     DestDir: "{app}\{code:DirExampleFlowsheets}\Units";     Flags: ignoreversion
Source: "..\Data\Example Flowsheets\Processes\*.txt";  DestDir: "{app}\{code:DirExampleScripts}\Processes";    Flags: ignoreversion
Source: "..\Data\Example Flowsheets\Units\*.txt";      DestDir: "{app}\{code:DirExampleScripts}\Units";        Flags: ignoreversion

[Dirs]
Name: "{app}\{code:DirExampleFlowsheets}";           Flags: uninsalwaysuninstall
Name: "{app}\{code:DirExampleFlowsheets}\Processes"; Flags: uninsalwaysuninstall
Name: "{app}\{code:DirExampleFlowsheets}\Units";     Flags: uninsalwaysuninstall
Name: "{app}\{code:DirExampleScripts}";              Flags: uninsalwaysuninstall
Name: "{app}\{code:DirExampleScripts}\Processes";    Flags: uninsalwaysuninstall
Name: "{app}\{code:DirExampleScripts}\Units";        Flags: uninsalwaysuninstall
