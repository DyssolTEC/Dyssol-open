# Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information.

################################################################################
### Initializing

[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
$CURRENT_PATH = (Get-Item -Path ".\" -Verbose).FullName
if (-not (Get-Command Expand-7Zip -ErrorAction Ignore)) {
	Install-Package -Scope CurrentUser -Force 7Zip4PowerShell > $null
}

################################################################################
### Paths

$SUNDIALS_VERSION = "3.1.0"
$SUNDIALS_DOWNLOAD_ADDRESS = "https://computation.llnl.gov/projects/sundials/download/sundials-$SUNDIALS_VERSION.tar.gz"
$SUNDIALS_NAME = "sundials-$SUNDIALS_VERSION"
$SUNDIALS_TAR_NAME = "$SUNDIALS_NAME.tar"
$SUNDIALS_ZIP_NAME = "$SUNDIALS_NAME.tar.gz"
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
cmake -G "Visual Studio 14 2015" $SUNDIALS_SRC_PATH -DCMAKE_INSTALL_PREFIX:PATH=$SUNDIALS_INSTALL_PATH -DBUILD_ARKODE=NO -DBUILD_CVODE=NO -DBUILD_CVODES=NO -DBUILD_IDA=YES -DBUILD_IDAS=NO -DBUILD_KINSOL=YES -DBUILD_SHARED_LIBS=NO -DBUILD_STATIC_LIBS=NO -DEXAMPLES_ENABLE_C=NO -DEXAMPLES_ENABLE_CXX=NO -DEXAMPLES_INSTALL=NO
cmake --build . --target INSTALL --config Debug
Get-ChildItem -Path "$SUNDIALS_INSTALL_PATH\lib" -Filter "*.lib" | Rename-Item -NewName {$_.BaseName + "_d" + $_.extension}
cmake --build . --target INSTALL --config Release
Rename-Item -Path "$SUNDIALS_INSTALL_PATH\lib" -NewName "$SUNDIALS_INSTALL_PATH\lib32"

# Build x64
New-Item $SUNDIALS_BUILD_PATH\x64 -ItemType directory
Set-Location $SUNDIALS_BUILD_PATH\x64
cmake -G "Visual Studio 14 2015 Win64" $SUNDIALS_SRC_PATH -DCMAKE_INSTALL_PREFIX:PATH=$SUNDIALS_INSTALL_PATH -DBUILD_ARKODE=NO -DBUILD_CVODE=NO -DBUILD_CVODES=NO -DBUILD_IDA=YES -DBUILD_IDAS=NO -DBUILD_KINSOL=YES -DBUILD_SHARED_LIBS=NO -DBUILD_STATIC_LIBS=NO -DEXAMPLES_ENABLE_C=NO -DEXAMPLES_ENABLE_CXX=NO -DEXAMPLES_INSTALL=NO
cmake --build . --target INSTALL --config Debug
Get-ChildItem -Path "$SUNDIALS_INSTALL_PATH\lib" -Filter "*.lib" | Rename-Item -NewName {$_.BaseName + "_d" + $_.extension}
cmake --build . --target INSTALL --config Release
Rename-Item -Path "$SUNDIALS_INSTALL_PATH\lib" -NewName "$SUNDIALS_INSTALL_PATH\lib64"

################################################################################
### Copy additional files

# Copy header files
Copy-Item "$SUNDIALS_SRC_PATH\src\ida\ida_direct_impl.h" "$SUNDIALS_INSTALL_PATH\include\ida\ida_direct_impl.h"
Copy-Item "$SUNDIALS_SRC_PATH\src\kinsol\kinsol_direct_impl.h" "$SUNDIALS_INSTALL_PATH\include\kinsol\kinsol_direct_impl.h"

# Copy *.pdb files
Copy-Item "$SUNDIALS_BUILD_PATH\x32\src\ida\sundials_ida_static.dir\Debug\sundials_ida_static.pdb" "$SUNDIALS_INSTALL_PATH\lib32\sundials_ida_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x64\src\ida\sundials_ida_static.dir\Debug\sundials_ida_static.pdb" "$SUNDIALS_INSTALL_PATH\lib64\sundials_ida_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x32\src\kinsol\sundials_kinsol_static.dir\Debug\sundials_kinsol_static.pdb" "$SUNDIALS_INSTALL_PATH\lib32\sundials_kinsol_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\x64\src\kinsol\sundials_kinsol_static.dir\Debug\sundials_kinsol_static.pdb" "$SUNDIALS_INSTALL_PATH\lib64\sundials_kinsol_static.pdb"

################################################################################
### Clean installation directory

$REM_ROOT_LIST = @("LICENSE")
$REM_LIB_LIST = @(
	"sundials_nvecserial", 
	"sundials_sunlinsolband", 
	"sundials_sunlinsoldense", 
	"sundials_sunlinsolpcg", 
	"sundials_sunlinsolspbcgs", 
	"sundials_sunlinsolspfgmr", 
	"sundials_sunlinsolspgmr", 
	"sundials_sunlinsolsptfqmr", 
	"sundials_sunmatrixband", 
	"sundials_sunmatrixdense", 
	"sundials_sunmatrixsparse"
)
$REM_INCLUDE_LIST = @(
	"ida\ida_bbdpre",
	"ida\ida_spils",
	"kinsol\kinsol_bbdpre",
	"kinsol\kinsol_spils",
	"sundials\sundials_band",
	"sundials\sundials_fconfig",
	"sundials\sundials_fnvector",
	"sundials\sundials_math",
	"sundials\sundials_pcg",
	"sundials\sundials_sparse",
	"sundials\sundials_spbcgs",
	"sundials\sundials_spfgmr",
	"sundials\sundials_spgmr",
	"sundials\sundials_sptfqmr",
	"sundials\sundials_version",
	"sunlinsol\sunlinsol_band",
	"sunlinsol\sunlinsol_pcg",
	"sunlinsol\sunlinsol_spbcgs",
	"sunlinsol\sunlinsol_spfgmr",
	"sunlinsol\sunlinsol_spgmr",
	"sunlinsol\sunlinsol_sptfqmr",
	"sunmatrix\sunmatrix_band",
	"sunmatrix\sunmatrix_sparse"
)

# gather
$REM_LIST = @("TEMP_TO_DEL")
foreach ($item in $REM_ROOT_LIST) {
	$REM_LIST += $SUNDIALS_INSTALL_PATH + '\' + $item
}
foreach ($item in $REM_LIB_LIST) {
	$REM_LIST += $SUNDIALS_INSTALL_PATH + '\lib32\' + $item + '.lib'
	$REM_LIST += $SUNDIALS_INSTALL_PATH + '\lib64\' + $item + '.lib'
	$REM_LIST += $SUNDIALS_INSTALL_PATH + '\lib32\' + $item + '_d.lib'
	$REM_LIST += $SUNDIALS_INSTALL_PATH + '\lib64\' + $item + '_d.lib'
}
foreach ($item in $REM_INCLUDE_LIST) {
	$REM_LIST += $SUNDIALS_INSTALL_PATH + '\include\' + $item + '.h'
}
# remove
foreach ($item in $REM_LIST) {
	Remove-Item "$item" -Force -ErrorAction Ignore
}

################################################################################
### Clean work directory

Set-Location $CURRENT_PATH

Remove-Item $SUNDIALS_BUILD_PATH -Force -Recurse
Remove-Item $SUNDIALS_SRC_PATH -Force -Recurse
Remove-Item $SUNDIALS_ZIP_NAME -Force -Recurse
Remove-Item $SUNDIALS_TAR_NAME -Force -Recurse