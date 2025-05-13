# Copyright (c) 2020, Dyssol Development Team. 
# Copyright (c) 2024, DyssolTEC GmbH.
# All rights reserved. This file is part of Dyssol. See LICENSE file for license information.

################################################################################
### Initializing

[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
# Obtain path to this script
$CURRENT_PATH = (Get-Item -Path ".\" -Verbose).FullName
# Check git is available
try { $null = git --version }
catch [System.Management.Automation.CommandNotFoundException] { 
	throw "Git is not available. Install it and add to PATH." 
}
# Check CMake is available
try { $null = cmake --version }
catch [System.Management.Automation.CommandNotFoundException] { 
	throw "CMake is not available. Install it and add to PATH." 
}

################################################################################
### Paths

$SUNDIALS_MAJOR_VERSION    = "7"
$SUNDIALS_MIDDLE_VERSION   = "2"
$SUNDIALS_MINOR_VERSION    = "1"
$SUNDIALS_VERSION          = "v$SUNDIALS_MAJOR_VERSION.$SUNDIALS_MIDDLE_VERSION.$SUNDIALS_MINOR_VERSION"
$SUNDIALS_GIT_ADDRESS      = "https://github.com/LLNL/sundials.git"
$SUNDIALS_NAME             = "sundials-$SUNDIALS_VERSION"
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

# Clone selected version
git -c advice.detachedHead=false clone --branch $SUNDIALS_VERSION --depth 1 $SUNDIALS_GIT_ADDRESS $SUNDIALS_SRC_PATH

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
### Clean installation directory

# Need to clean up to avoid putting unnecessary files into the installer
$REM_ROOT_LIST = @(
	"include\sunadaptcontroller"
	"include\sunmemory"
	"include\sunnonlinsol"
	"include\sundials\LICENSE"
	"include\sundials\NOTICE"
	"lib\cmake"
)
$REM_LIB_LIST = @(
	"sundials_nvecmanyvector_static", 
	"sundials_nvecserial_static", 
	"sundials_sunlinsolband_static", 
	"sundials_sunlinsoldense_static", 
	"sundials_sunlinsolpcg_static", 
	"sundials_sunlinsolspbcgs_static", 
	"sundials_sunlinsolspfgmr_static", 
	"sundials_sunlinsolspgmr_static", 
	"sundials_sunlinsolsptfqmr_static", 
	"sundials_sunmatrixband_static", 
	"sundials_sunmatrixdense_static", 
	"sundials_sunmatrixsparse_static", 
	"sundials_sunnonlinsolfixedpoint_static",
	"sundials_sunnonlinsolnewton_static"
)
$REM_INCLUDE_LIST = @(
	"ida\ida_bbdpre",
	"kinsol\kinsol_bbdpre",
	"nvector\nvector_manyvector",
	"sundials\sundials_band",
	"sundials\sundials_futils",
	"sundials\sundials_mpi_types",
	"sundials\sundials_stepper",
	"sundials\sundials_types_deprecated",
	"sundials\priv\sundials_errors_impl",
	"sunlinsol\sunlinsol_band",
	"sunlinsol\sunlinsol_pcg",
	"sunlinsol\sunlinsol_spbcgs",
	"sunlinsol\sunlinsol_spfgmr",
	"sunlinsol\sunlinsol_spgmr",
	"sunlinsol\sunlinsol_sptfqmr",
	"sunmatrix\sunmatrix_band",
	"sunmatrix\sunmatrix_sparse"
)
$REM_INCLUDE_HPP_LIST = @(
	"sundials\sundials_base",
	"sundials\sundials_convertibleto",
	"sundials\sundials_core",
	"sundials\sundials_linearsolver",
	"sundials\sundials_matrix",
	"sundials\sundials_memory",
	"sundials\sundials_nonlinearsolver",
	"sundials\sundials_profiler"
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
foreach ($item in $REM_INCLUDE_HPP_LIST) {
	$REM_LIST += $SUNDIALS_INSTALL_PATH + '\include\' + $item + '.hpp'
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

Write-Host "Completed! $SUNDIALS_NAME library is installed at: $SUNDIALS_INSTALL_PATH" -ForegroundColor Green
