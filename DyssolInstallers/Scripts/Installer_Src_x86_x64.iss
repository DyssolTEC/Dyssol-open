; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#define IsIncludeX32
#define IsIncludeX64
#define IsWithSrc

#define MyAppVersion GetFileVersion('..\..\x64\Release\Dyssol.exe')

#include "BaseInstaller.iss"

[Setup]
OutputBaseFilename={#MyAppName} {#MyAppVersion}
ArchitecturesInstallIn64BitMode=x64
