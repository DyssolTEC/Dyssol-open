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

$HDF5_MAJOR_VERSION    = "1"
$HDF5_MIDDLE_VERSION   = "14"
$HDF5_MINOR_VERSION    = "6"
$HDF5_VERSION          = "$HDF5_MAJOR_VERSION.$HDF5_MIDDLE_VERSION.$HDF5_MINOR_VERSION"
$HDF5_GIT_ADDRESS      = "https://github.com/HDFGroup/hdf5.git"
$HDF5_NAME             = "hdf5-$HDF5_VERSION"
$HDF5_INSTALL_PATH     = "$CURRENT_PATH\hdf5"
$HDF5_SRC_PATH         = "$CURRENT_PATH\$HDF5_NAME"
$HDF5_BUILD_PATH       = "$HDF5_SRC_PATH\build"

# Libs
$ZLIB_INSTALL_PATH = "$CURRENT_PATH\zlib"

################################################################################
### Clear old

Remove-Item $HDF5_INSTALL_PATH -Force -Recurse -ErrorAction Ignore
Remove-Item $HDF5_BUILD_PATH -Force -Recurse -ErrorAction Ignore
Remove-Item $HDF5_SRC_PATH -Force -Recurse -ErrorAction Ignore

################################################################################
### Download

# Clone selected version
git -c advice.detachedHead=false clone --branch $HDF5_NAME --depth 1 $HDF5_GIT_ADDRESS $HDF5_SRC_PATH

################################################################################
### Build and install

# Build
New-Item $HDF5_BUILD_PATH -ItemType directory
Set-Location $HDF5_BUILD_PATH
cmake -G "Visual Studio 17 2022" -A x64 $HDF5_SRC_PATH `
	-DCMAKE_C_FLAGS="/W0" `
	-DCMAKE_CXX_FLAGS="/W0" `
	-DCMAKE_INSTALL_PREFIX:PATH=$HDF5_INSTALL_PATH `
	-DBUILD_SHARED_LIBS=NO `
	-DBUILD_TESTING=NO `
	-DHDF5_BUILD_CPP_LIB=YES `
	-DHDF5_BUILD_HL_LIB=YES `
	-DHDF5_BUILD_EXAMPLES=NO `
	-DHDF5_BUILD_STATIC_TOOLS=NO `
	-DHDF5_BUILD_TOOLS=NO `
	-DHDF5_BUILD_UTILS=NO `
	-DHDF5_ENABLE_ALL_WARNINGS=NO `
	-DHDF5_ENABLE_Z_LIB_SUPPORT=YES `
	-DHDF5_USE_ZLIB_STATIC=YES `
	-DZLIB_INCLUDE_DIR="$ZLIB_INSTALL_PATH/include" `
	-DZLIB_LIBRARY_DEBUG="$ZLIB_INSTALL_PATH/lib/zlibstaticd.lib" `
	-DZLIB_LIBRARY_RELEASE="$ZLIB_INSTALL_PATH/lib/zlibstatic.lib"
cmake --build . --parallel --target INSTALL --config Debug
cmake --build . --parallel --target INSTALL --config Release

################################################################################
### Copy *.pdb files

Copy-Item "$HDF5_BUILD_PATH\bin\Debug\*.pdb"        "$HDF5_INSTALL_PATH\lib\"

################################################################################
### Clean installation directory

# Need to clean up to avoid putting unnecessary files into the installer
$REM_HDF5_ROOT_LIST = @(
	"bin",
	"cmake",
	"lib\libhdf5.settings",
	"lib\pkgconfig",
	"COPYING",
	"RELEASE.txt",
	"USING_HDF5_CMake.txt",
	"USING_HDF5_VS.txt"
)
$REM_HDF5_INCLUDE_LIST = @(
	"H5ACpkg",
	"H5Alltypes",
	"H5Apkg",
	"H5B2pkg",
	"H5B2public",
	"H5Bpkg",
	"H5Bpublic",
	"H5Cpkg",
	"H5CppDoc",
	"H5DOpublic",
	"H5Dpkg",
	"H5DSpublic",
	"H5EApkg",
	"H5Edefin",
	"H5Einit",
	"H5Epkg",
	"H5Eterm",
	"H5FApkg",
	"H5FDpkg",
	"H5FDs3comms"
	"H5Fpkg",
	"H5FSpkg",
	"H5FSpublic",
	"H5Gpkg",
	"H5HFpkg",
	"H5HFpublic",
	"H5HGpkg",
	"H5HGpublic",
	"H5HLpkg",
	"H5HLpublic",
	"H5IMpublic",
	"H5Ipkg",
	"H5LDpublic",
	"H5Lpkg",
	"H5LTparse",
	"H5LTpublic",
	"H5MPpkg",
	"H5Opkg",
	"H5Oshared",
	"H5overflow",
	"H5PBpkg",
	"H5PLextern",
	"H5PLpkg",
	"H5Ppkg",
	"H5Rpkg",
	"H5SMpkg",
	"H5Spkg",
	"H5TBpublic",
	"H5Tpkg",
	"H5Zpkg",
	"hdf5_hl"
)

# Gather
$REM_LIST = @("TEMP_TO_DEL")
foreach ($item in $REM_HDF5_ROOT_LIST) {
	$REM_LIST += $HDF5_INSTALL_PATH + '\' + $item
}
foreach ($item in $REM_HDF5_INCLUDE_LIST) {
	$REM_LIST += $HDF5_INSTALL_PATH + '\include\' + $item + '.h'
}
# Remove
foreach ($item in $REM_LIST) {
	Remove-Item "$item" -Force -Recurse -ErrorAction Ignore
}

################################################################################
### Clean work directory

Set-Location $CURRENT_PATH

Remove-Item $HDF5_BUILD_PATH -Force -Recurse
Remove-Item $HDF5_SRC_PATH -Force -Recurse

Write-Host "Completed! $HDF5_NAME library is installed at: $HDF5_INSTALL_PATH" -ForegroundColor Green
