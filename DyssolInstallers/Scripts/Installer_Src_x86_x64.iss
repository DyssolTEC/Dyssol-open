; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

; #define IsIncludeX32 # comes as a parameter from running script
; #define IsIncludeX64 # comes as a parameter from running script
#define IsWithSrc

; #define SolutionDir # comes as a parameter from running script
; #define QtPath      # comes as a parameter from running script

#define MyAppVersion GetStringFileInfo(SolutionDir+'\x64\Release\Dyssol.exe', 'ProductVersion')

#include "BaseInstaller.iss"

[Setup]
ArchitecturesInstallIn64BitMode=x64
