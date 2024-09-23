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
$SUNDIALS_MIDDLE_VERSION   = "1"
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
	-DEXAMPLES_ENABLE_C=NO `
	-DEXAMPLES_ENABLE_CXX=NO `
	-DEXAMPLES_INSTALL=NO `
	-DSUNDIALS_ENABLE_ERROR_CHECKS=NO
cmake --build . --parallel --target INSTALL --config Debug
Get-ChildItem -Path "$SUNDIALS_INSTALL_PATH\lib" -Filter "*.lib" | Rename-Item -NewName {$_.BaseName + "_d" + $_.extension}
cmake --build . --parallel --target INSTALL --config Release

################################################################################
### Copy additional files

# Copy header files
if ([int]$SUNDIALS_MAJOR_VERSION -eq 5) {
	Copy-Item "$SUNDIALS_SRC_PATH\src\ida\ida_impl.h"    "$SUNDIALS_INSTALL_PATH\include\ida\ida_impl.h"
	Copy-Item "$SUNDIALS_SRC_PATH\src\ida\ida_ls_impl.h" "$SUNDIALS_INSTALL_PATH\include\ida\ida_ls_impl.h"
}
if ([int]$SUNDIALS_MAJOR_VERSION -eq 6) {
	Copy-Item "$SUNDIALS_SRC_PATH\src\ida\ida_impl.h"                   "$SUNDIALS_INSTALL_PATH\include\ida\ida_impl.h"
	Copy-Item "$SUNDIALS_SRC_PATH\src\ida\ida_ls_impl.h"                "$SUNDIALS_INSTALL_PATH\include\ida\ida_ls_impl.h"
	Copy-Item "$SUNDIALS_SRC_PATH\src\sundials\sundials_context_impl.h" "$SUNDIALS_INSTALL_PATH\include\sundials_context_impl.h"
	Copy-Item "$SUNDIALS_SRC_PATH\src\sundials\sundials_hashmap.h"      "$SUNDIALS_INSTALL_PATH\include\sundials_hashmap.h"
	Copy-Item "$SUNDIALS_SRC_PATH\src\sundials\sundials_logger_impl.h"  "$SUNDIALS_INSTALL_PATH\include\sundials_logger_impl.h"
}
if ([int]$SUNDIALS_MAJOR_VERSION -eq 7) {
	Copy-Item "$SUNDIALS_SRC_PATH\src\ida\ida_impl.h"                   "$SUNDIALS_INSTALL_PATH\include\ida\ida_impl.h"
	Copy-Item "$SUNDIALS_SRC_PATH\src\sundials\sundials_hashmap_impl.h"      "$SUNDIALS_INSTALL_PATH\include\sundials_hashmap_impl.h"
	Copy-Item "$SUNDIALS_SRC_PATH\src\sundials\sundials_logger_impl.h"  "$SUNDIALS_INSTALL_PATH\include\sundials_logger_impl.h"
	Copy-Item "$SUNDIALS_SRC_PATH\src\sundials\sundials_macros.h"  "$SUNDIALS_INSTALL_PATH\include\sundials_macros.h"
	Copy-Item "$SUNDIALS_SRC_PATH\src\sundials\sundials_utils.h"  "$SUNDIALS_INSTALL_PATH\include\sundials_utils.h"
}

# Copy *.pdb files
Copy-Item "$SUNDIALS_BUILD_PATH\src\ida\sundials_ida_obj_static.dir\Debug\sundials_ida_obj_static.pdb"                                               "$SUNDIALS_INSTALL_PATH\lib\sundials_ida_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\src\kinsol\sundials_kinsol_obj_static.dir\Debug\sundials_kinsol_obj_static.pdb"                                      "$SUNDIALS_INSTALL_PATH\lib\sundials_kinsol_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\src\sundials\sundials_generic_obj_static.dir\Debug\sundials_generic_obj_static.pdb"                                  "$SUNDIALS_INSTALL_PATH\lib\sundials_generic_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\src\nvector\serial\sundials_nvecserial_obj_static.dir\Debug\sundials_nvecserial_obj_static.pdb"                      "$SUNDIALS_INSTALL_PATH\lib\sundials_nvecserial_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\src\sunmatrix\band\sundials_sunmatrixband_obj_static.dir\Debug\sundials_sunmatrixband_obj_static.pdb"                "$SUNDIALS_INSTALL_PATH\lib\sundials_sunmatrixband_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\src\sunmatrix\dense\sundials_sunmatrixdense_obj_static.dir\Debug\sundials_sunmatrixdense_obj_static.pdb"             "$SUNDIALS_INSTALL_PATH\lib\sundials_sunmatrixdense_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\src\sunlinsol\dense\sundials_sunlinsoldense_obj_static.dir\Debug\sundials_sunlinsoldense_obj_static.pdb"             "$SUNDIALS_INSTALL_PATH\lib\sundials_sunlinsoldense_obj_static.pdb"
Copy-Item "$SUNDIALS_BUILD_PATH\src\sunnonlinsol\newton\sundials_sunnonlinsolnewton_obj_static.dir\Debug\sundials_sunnonlinsolnewton_obj_static.pdb" "$SUNDIALS_INSTALL_PATH\lib\sundials_sunnonlinsolnewton_obj_static.pdb"

################################################################################
### Clean installation directory

$REM_ROOT_LIST = @(
	"include\sunnonlinsol"
	"include\sundials\LICENSE"
	"include\sundials\NOTICE"
	"lib\cmake"
)
$REM_LIB_LIST = @(
	"sundials_generic", 
	"sundials_nvecmanyvector", 
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
	"sundials_sunmatrixsparse",
	"sundials_sunnonlinsolfixedpoint",
	"sundials_sunnonlinsolnewton"
)
$REM_INCLUDE_LIST = @(
	"ida\ida_bbdpre",
	"ida\ida_direct",
	"ida\ida_spils",
	"kinsol\kinsol_bbdpre",
	"kinsol\kinsol_direct",
	"kinsol\kinsol_spils",
	"nvector\nvector_manyvector",
	"sundials\sundials_band",
	"sundials\sundials_fconfig",
	"sundials\sundials_fnvector",
	"sundials\sundials_futils",
	"sundials\sundials_math",
	"sundials\sundials_memory",
	"sundials\sundials_mpi_types",
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

# Gather
$REM_LIST = @("NOTHING")
foreach ($item in $REM_ROOT_LIST) {
	$REM_LIST += $SUNDIALS_INSTALL_PATH + '\' + $item
}
foreach ($item in $REM_LIB_LIST) {
	$REM_LIST += $SUNDIALS_INSTALL_PATH + '\lib\' + $item + '.lib'
	$REM_LIST += $SUNDIALS_INSTALL_PATH + '\lib\' + $item + '_d.lib'
}
foreach ($item in $REM_INCLUDE_LIST) {
	$REM_LIST += $SUNDIALS_INSTALL_PATH + '\include\' + $item + '.h'
}
# Remove
foreach ($item in $REM_LIST) {
	Remove-Item "$item" -Force -Recurse -ErrorAction Ignore
}

################################################################################
### Clean work directory

Set-Location $CURRENT_PATH

Remove-Item $SUNDIALS_BUILD_PATH -Force -Recurse
Remove-Item $SUNDIALS_SRC_PATH -Force -Recurse
Remove-Item $SUNDIALS_ZIP_NAME -Force -Recurse
Remove-Item $SUNDIALS_TAR_NAME -Force -Recurse