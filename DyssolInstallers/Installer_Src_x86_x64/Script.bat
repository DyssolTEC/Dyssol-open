:: Copyright (c) 2020, Dyssol Development Team. All Rights Reserved. This file is part of Dyssol. See LICENSE file for license information.

devenv %2 /build "Release|Win32"
devenv %2 /build "Debug|Win32"
devenv %2 /build "Release|x64"
devenv %2 /build "Debug|x64"
set QtInstallDir="%3/.."

powershell -executionpolicy bypass -File %~dp0\wsl_create_docs.ps1

..\Compiler\ISCC ..\Scripts\Installer_Src_x86_x64.iss "/dQtPath=%QtInstallDir%"