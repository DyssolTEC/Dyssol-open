; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#include "CommonConstants.iss"

[Files]
Source: "..\Data\Example Flowsheets\Processes\*.dflw"; DestDir: "{app}\{code:DirExampleFlowsheets}\Processes"; Flags: ignoreversion
Source: "..\Data\Example Flowsheets\Units\*.dflw"; DestDir: "{app}\{code:DirExampleFlowsheets}\Units"; Flags: ignoreversion

[Dirs]
Name: "{app}\{code:DirExampleFlowsheets}"; Flags: uninsalwaysuninstall
Name: "{app}\{code:DirExampleFlowsheets}\Processes"; Flags: uninsalwaysuninstall
Name: "{app}\{code:DirExampleFlowsheets}\Units"; Flags: uninsalwaysuninstall
