# Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information.

################################################################################
### Initializing

[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
$CURRENT_PATH = (Get-Item -Path ".\" -Verbose).FullName
if (-not (Get-Command Expand-7Zip -ErrorAction Ignore)) {
	Install-Package -Scope CurrentUser -Force -ProviderName PowerShellGet 7Zip4Powershell > $null
}

################################################################################
### Paths

$SUNDIALS_MAJOR_VERSION = "6"
$SUNDIALS_MIDDLE_VERSION = "4"
$SUNDIALS_MINOR_VERSION = "1"
$SUNDIALS_VERSION = "$SUNDIALS_MAJOR_VERSION.$SUNDIALS_MIDDLE_VERSION.$SUNDIALS_MINOR_VERSION"
$SUNDIALS_DOWNLOAD_ADDRESS = "https://github.com/LLNL/sundials/releases/download/v$SUNDIALS_VERSION/sundials-$SUNDIALS_VERSION.tar.gz"
$SUNDIALS_NAME = "sundials-$SUNDIALS_VERSION"
$SUNDIALS_TAR_NAME = "$SUNDIALS_NAME.tar"
$SUNDIALS_ZIP_NAME = "$SUNDIALS_TAR_NAME.gz"
$SUNDIALS_INSTALL_PATH = "$CURRENT_PATH\sundials"
$SUNDIALS_SRC_PATH = "$CURRENT_PATH\$SUNDIALS_NAME"
$SUNDIALS_BUILD_PATH = "$SUNDIALS_SRC_PATH\build"

################################################################################
### Clear old

Remove-Item $SUNDIALS_INSTALL_PATH -Force -Recurse -ErrorAction Ignore
Remove-Item $SUNDIALS_BUILD_PATH -Force -Recurse -ErrorAction Ignore
Remove-Item $SUNDIALS_SRC_PATH -Force -Recurse -ErrorAction Ignore

################################################################################
### Download

Invoke-WebRequest $SUNDIALS_DOWNLOAD_ADDRESS -OutFile $SUNDIALS_ZIP_NAME
Expand-7Zip $SUNDIALS_ZIP_NAME . | Expand-7Zip $SUNDIALS_TAR_NAME .

################################################################################
### Build and install

# Build x32
New-Item $SUNDIALS_BUILD_PATH\x32 -ItemType directory
Set-Location $SUNDIALS_BUILD_PATH\x32
cmake -G "Visual Studio 16 2019" -A Win32 $SUNDIALS_SRC_PATH -DCMAKE_INSTALL_PREFIX:PATH=$SUNDIALS_INSTALL_PATH -DBUILD_ARKODE=NO -DBUILD_CVODE=NO -DBUILD_CVODES=NO -DBUILD_IDA=YES -DBUILD_IDAS=NO -DBUILD_KINSOL=YES -DBUILD_SHARED_LIBS=NO -DBUILD_STATIC_LIBS=YES -DEXAMPLES_ENABLE_C=NO -DEXAMPLES_ENABLE_CXX=NO -DEXAMPLES_INSTALL=NO
cmake --build . --target INSTALL --config Debug
Get-ChildItem -Path "$SUNDIALS_INSTALL_PATH\lib" -Filter "*.lib" | Rename-Item -NewName {$_.BaseName + "_d" + $_.extension}
cmake --build . --target INSTALL --config Release
Rename-Item -Path "$SUNDIALS_INSTALL_PATH\lib" -NewName "$SUNDIALS_INSTALL_PATH\lib32"

# Build x64
New-Item $SUNDIALS_BUILD_PATH\x64 -ItemType directory
Set-Location $SUNDIALS_BUILD_PATH\x64
cmake -G "Visual Studio 16 2019" -A x64 $SUNDIALS_SRC_PATH -DCMAKE_INSTALL_PREFIX:PATH=$SUNDIALS_INSTALL_PATH -DBUILD_ARKODE=NO -DBUILD_CVODE=NO -DBUILD_CVODES=NO -DBUILD_IDA=YES -DBUILD_IDAS=NO -DBUILD_KINSOL=YES -DBUILD_SHARED_LIBS=NO -DBUILD_STATIC_LIBS=YES -DEXAMPLES_ENABLE_C=NO -DEXAMPLES_ENABLE_CXX=NO -DEXAMPLES_INSTALL=NO
cmake --build . --target INSTALL --config Debug
Get-ChildItem -Path "$SUNDIALS_INSTALL_PATH\lib" -Filter "*.lib" | Rename-Item -NewName {$_.BaseName + "_d" + $_.extension}
cmake --build . --target INSTALL --config Release
Rename-Item -Path "$SUNDIALS_INSTALL_PATH\lib" -NewName "$SUNDIALS_INSTALL_PATH\lib64"

################################################################################
### Copy additional files

# Copy header files
if ([int]$SUNDIALS_MAJOR_VERSION -eq 5) {
	Copy-Item "$SUNDIALS_SRC_PATH\src\ida\ida_impl.h" "$SUNDIALS_INSTALL_PATH\include\ida\ida_impl.h"
	Copy-Item "$SUNDIALS_SRC_PATH\src\ida\ida_ls_impl.h" "$SUNDIALS_INSTALL_PATH\include\ida\ida_ls_impl.h"
}
if ([int]$SUNDIALS_MAJOR_VERSION -eq 6) {
	Copy-Item "$SUNDIALS_SRC_PATH\src\ida\ida_impl.h" "$SUNDIALS_INSTALL_PATH\include\ida\ida_impl.h"
	Copy-Item "$SUNDIALS_SRC_PATH\src\ida\ida_ls_impl.h" "$SUNDIALS_INSTALL_PATH\include\ida\ida_ls_impl.h"
	Copy-Item "$SUNDIALS_SRC_PATH\src\sundials\sundials_context_impl.h" "$SUNDIALS_INSTALL_PATH\include\sundials_context_impl.h"
	Copy-Item "$SUNDIALS_SRC_PATH\src\sundials\sundials_hashmap.h" "$SUNDIALS_INSTALL_PATH\include\sundials_hashmap.h"
	Copy-Item "$SUNDIALS_SRC_PATH\src\sundials\sundials_logger_impl.h" "$SUNDIALS_INSTALL_PATH\include\sundials_logger_impl.h"
}

# Copy *.pdb files
Copy-Item "$SUNDIALS_BUILD_PATH\x32\src\ida\sundials_ida_obj_static.dir\Debug\sundials_ida_obj_static.pdb" "$SUNDIALS_INSTALL_PATH\lib32\sundials_ida_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x64\src\ida\sundials_ida_obj_static.dir\Debug\sundials_ida_obj_static.pdb" "$SUNDIALS_INSTALL_PATH\lib64\sundials_ida_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x32\src\kinsol\sundials_kinsol_obj_static.dir\Debug\sundials_kinsol_obj_static.pdb" "$SUNDIALS_INSTALL_PATH\lib32\sundials_kinsol_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x64\src\kinsol\sundials_kinsol_obj_static.dir\Debug\sundials_kinsol_obj_static.pdb" "$SUNDIALS_INSTALL_PATH\lib64\sundials_kinsol_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x32\src\sundials\sundials_generic_obj_static.dir\Debug\sundials_generic_obj_static.pdb" "$SUNDIALS_INSTALL_PATH\lib32\sundials_generic_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x64\src\sundials\sundials_generic_obj_static.dir\Debug\sundials_generic_obj_static.pdb" "$SUNDIALS_INSTALL_PATH\lib64\sundials_generic_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x32\src\nvector\serial\sundials_nvecserial_obj_static.dir\Debug\sundials_nvecserial_obj_static.pdb" "$SUNDIALS_INSTALL_PATH\lib32\sundials_nvecserial_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x64\src\nvector\serial\sundials_nvecserial_obj_static.dir\Debug\sundials_nvecserial_obj_static.pdb" "$SUNDIALS_INSTALL_PATH\lib64\sundials_nvecserial_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x32\src\sunmatrix\band\sundials_sunmatrixband_obj_static.dir\Debug\sundials_sunmatrixband_obj_static.pdb" "$SUNDIALS_INSTALL_PATH\lib32\sundials_sunmatrixband_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x64\src\sunmatrix\band\sundials_sunmatrixband_obj_static.dir\Debug\sundials_sunmatrixband_obj_static.pdb" "$SUNDIALS_INSTALL_PATH\lib64\sundials_sunmatrixband_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x32\src\sunmatrix\dense\sundials_sunmatrixdense_obj_static.dir\Debug\sundials_sunmatrixdense_obj_static.pdb" "$SUNDIALS_INSTALL_PATH\lib32\sundials_sunmatrixdense_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x64\src\sunmatrix\dense\sundials_sunmatrixdense_obj_static.dir\Debug\sundials_sunmatrixdense_obj_static.pdb" "$SUNDIALS_INSTALL_PATH\lib64\sundials_sunmatrixdense_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x32\src\sunlinsol\dense\sundials_sunlinsoldense_obj_static.dir\Debug\sundials_sunlinsoldense_obj_static.pdb" "$SUNDIALS_INSTALL_PATH\lib32\sundials_sunlinsoldense_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x64\src\sunlinsol\dense\sundials_sunlinsoldense_obj_static.dir\Debug\sundials_sunlinsoldense_obj_static.pdb" "$SUNDIALS_INSTALL_PATH\lib64\sundials_sunlinsoldense_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x32\src\sunnonlinsol\newton\sundials_sunnonlinsolnewton_obj_static.dir\Debug\sundials_sunnonlinsolnewton_obj_static.pdb" "$SUNDIALS_INSTALL_PATH\lib32\sundials_sunnonlinsolnewton_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x64\src\sunnonlinsol\newton\sundials_sunnonlinsolnewton_obj_static.dir\Debug\sundials_sunnonlinsolnewton_obj_static.pdb" "$SUNDIALS_INSTALL_PATH\lib64\sundials_sunnonlinsolnewton_obj_static.pdb"

################################################################################
### Clean work directory

Set-Location $CURRENT_PATH

Remove-Item $SUNDIALS_BUILD_PATH -Force -Recurse
Remove-Item $SUNDIALS_SRC_PATH -Force -Recurse
Remove-Item $SUNDIALS_ZIP_NAME -Force -Recurse
Remove-Item $SUNDIALS_TAR_NAME -Force -Recurse