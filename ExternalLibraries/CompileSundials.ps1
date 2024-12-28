# Copyright (c) 2020, Dyssol Development Team. 
# Copyright (c) 2024, DyssolTEC GmbH.
# All rights reserved. This file is part of Dyssol. See LICENSE file for license information.

################################################################################
### Initializing

[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
$CURRENT_PATH = (Get-Item -Path ".\" -Verbose).FullName
if (-not (Get-Command Expand-7Zip -ErrorAction Ignore)) {
	Install-Package -Scope CurrentUser -Force -ProviderName PowerShellGet 7Zip4Powershell > $null
}

################################################################################
### Paths

$SUNDIALS_MAJOR_VERSION    = "7"
$SUNDIALS_MIDDLE_VERSION   = "2"
$SUNDIALS_MINOR_VERSION    = "1"
$SUNDIALS_VERSION          = "$SUNDIALS_MAJOR_VERSION.$SUNDIALS_MIDDLE_VERSION.$SUNDIALS_MINOR_VERSION"
$SUNDIALS_DOWNLOAD_ADDRESS = "https://github.com/LLNL/sundials/releases/download/v$SUNDIALS_VERSION/sundials-$SUNDIALS_VERSION.tar.gz"
$SUNDIALS_NAME             = "sundials-$SUNDIALS_VERSION"
$SUNDIALS_TAR_NAME         = "$SUNDIALS_NAME.tar"
$SUNDIALS_ZIP_NAME         = "$SUNDIALS_TAR_NAME.gz"
$SUNDIALS_INSTALL_PATH     = "$CURRENT_PATH\sundials"
$SUNDIALS_SRC_PATH         = "$CURRENT_PATH\$SUNDIALS_NAME"
$SUNDIALS_BUILD_PATH       = "$SUNDIALS_SRC_PATH\build"

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

# Build
New-Item $SUNDIALS_BUILD_PATH -ItemType directory
Set-Location $SUNDIALS_BUILD_PATH
cmake -G "Visual Studio 16 2019" -A x64 $SUNDIALS_SRC_PATH `
	-DCMAKE_INSTALL_PREFIX:PATH=$SUNDIALS_INSTALL_PATH `
	-DBUILD_ARKODE=NO `
	-DBUILD_BENCHMARKS=NO `
	-DBUILD_CVODE=NO `
	-DBUILD_CVODES=NO `
	-DBUILD_FORTRAN_MODULE_INTERFACE=NO `
	-DBUILD_IDA=YES `
	-DBUILD_IDAS=NO `
	-DBUILD_KINSOL=YES `
	-DBUILD_SHARED_LIBS=NO `
	-DBUILD_STATIC_LIBS=YES `
	-DBUILD_TESTING=NO `
	-DEXAMPLES_ENABLE_C=NO `
	-DEXAMPLES_ENABLE_CXX=NO `
	-DEXAMPLES_INSTALL=NO `
	-DSUNDIALS_ENABLE_ERROR_CHECKS=NO
cmake --build . --parallel --target INSTALL --config Debug
Get-ChildItem -Path "$SUNDIALS_INSTALL_PATH\lib" -Filter "*.lib" | Rename-Item -NewName {$_.BaseName + "_d" + $_.extension}
cmake --build . --parallel --target INSTALL --config Release

################################################################################
### Copy additional files

# Copy *.pdb files
if ([int]$SUNDIALS_MAJOR_VERSION -le 4) {
	Copy-Item "$SUNDIALS_BUILD_PATH\src\ida\Debug\*.pdb"    "$SUNDIALS_INSTALL_PATH\lib\"
	Copy-Item "$SUNDIALS_BUILD_PATH\src\kinsol\Debug\*.pdb" "$SUNDIALS_INSTALL_PATH\lib\"
}
elseif ([int]$SUNDIALS_MAJOR_VERSION -le 6) {
	Copy-Item "$SUNDIALS_BUILD_PATH\src\ida\sundials_ida_obj_static.dir\Debug\*.pdb"                                "$SUNDIALS_INSTALL_PATH\lib\"
	Copy-Item "$SUNDIALS_BUILD_PATH\src\kinsol\sundials_kinsol_obj_static.dir\Debug\*.pdb"                          "$SUNDIALS_INSTALL_PATH\lib\"
	Copy-Item "$SUNDIALS_BUILD_PATH\src\nvector\serial\sundials_nvecserial_obj_static.dir\Debug\*.pdb"              "$SUNDIALS_INSTALL_PATH\lib\"
	Copy-Item "$SUNDIALS_BUILD_PATH\src\sundials\sundials_generic_obj_static.dir\Debug\*.pdb"                       "$SUNDIALS_INSTALL_PATH\lib\"
	Copy-Item "$SUNDIALS_BUILD_PATH\src\sunlinsol\dense\sundials_sunlinsoldense_obj_static.dir\Debug\*.pdb"         "$SUNDIALS_INSTALL_PATH\lib\"
	Copy-Item "$SUNDIALS_BUILD_PATH\src\sunmatrix\band\sundials_sunmatrixband_obj_static.dir\Debug\*.pdb"           "$SUNDIALS_INSTALL_PATH\lib\"
	Copy-Item "$SUNDIALS_BUILD_PATH\src\sunmatrix\dense\sundials_sunmatrixdense_obj_static.dir\Debug\*.pdb"         "$SUNDIALS_INSTALL_PATH\lib\"
	Copy-Item "$SUNDIALS_BUILD_PATH\src\sunnonlinsol\newton\sundials_sunnonlinsolnewton_obj_static.dir\Debug\*.pdb" "$SUNDIALS_INSTALL_PATH\lib\"
}
elseif ([int]$SUNDIALS_MAJOR_VERSION -ge 7) {
	Copy-Item "$SUNDIALS_BUILD_PATH\src\ida\sundials_ida_obj_static.dir\Debug\*.pdb"                                "$SUNDIALS_INSTALL_PATH\lib\"
	Copy-Item "$SUNDIALS_BUILD_PATH\src\kinsol\sundials_kinsol_obj_static.dir\Debug\*.pdb"                          "$SUNDIALS_INSTALL_PATH\lib\"
	Copy-Item "$SUNDIALS_BUILD_PATH\src\nvector\serial\sundials_nvecserial_obj_static.dir\Debug\*.pdb"              "$SUNDIALS_INSTALL_PATH\lib\"
	Copy-Item "$SUNDIALS_BUILD_PATH\src\sundials\sundials_core_obj_static.dir\Debug\*.pdb"                          "$SUNDIALS_INSTALL_PATH\lib\"
	Copy-Item "$SUNDIALS_BUILD_PATH\src\sunlinsol\dense\sundials_sunlinsoldense_obj_static.dir\Debug\*.pdb"         "$SUNDIALS_INSTALL_PATH\lib\"
	Copy-Item "$SUNDIALS_BUILD_PATH\src\sunmatrix\band\sundials_sunmatrixband_obj_static.dir\Debug\*.pdb"           "$SUNDIALS_INSTALL_PATH\lib\"
	Copy-Item "$SUNDIALS_BUILD_PATH\src\sunmatrix\dense\sundials_sunmatrixdense_obj_static.dir\Debug\*.pdb"         "$SUNDIALS_INSTALL_PATH\lib\"
	Copy-Item "$SUNDIALS_BUILD_PATH\src\sunnonlinsol\newton\sundials_sunnonlinsolnewton_obj_static.dir\Debug\*.pdb" "$SUNDIALS_INSTALL_PATH\lib\"
}

################################################################################
### Clean work directory

Set-Location $CURRENT_PATH

Remove-Item $SUNDIALS_BUILD_PATH -Force -Recurse
Remove-Item $SUNDIALS_SRC_PATH -Force -Recurse
Remove-Item $SUNDIALS_ZIP_NAME -Force -Recurse
Remove-Item $SUNDIALS_TAR_NAME -Force -Recurse