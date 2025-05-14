# Copyright (c) 2020, Dyssol Development Team. All rights reserved.
# Copyright (c) 2023, DyssolTEC GmbH. 
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

$ZLIB_MAJOR_VERSION  = "1"
$ZLIB_MIDDLE_VERSION = "3"
$ZLIB_MINOR_VERSION  = "1"
$ZLIB_VERSION        = "v$ZLIB_MAJOR_VERSION.$ZLIB_MIDDLE_VERSION.$ZLIB_MINOR_VERSION"
$ZLIB_GIT_ADDRESS    = "https://github.com/madler/zlib.git"
$ZLIB_NAME           = "zlib-$ZLIB_VERSION"
$ZLIB_INSTALL_PATH   = "$CURRENT_PATH\zlib"
$ZLIB_SRC_PATH       = "$CURRENT_PATH\$ZLIB_NAME"
$ZLIB_BUILD_PATH     = "$ZLIB_SRC_PATH\build"

################################################################################
### Clear old

Remove-Item $ZLIB_INSTALL_PATH -Force -Recurse -ErrorAction Ignore
Remove-Item $ZLIB_BUILD_PATH -Force -Recurse -ErrorAction Ignore
Remove-Item $ZLIB_SRC_PATH -Force -Recurse -ErrorAction Ignore

################################################################################
### Download

# Clone selected version
git -c advice.detachedHead=false clone --branch $ZLIB_VERSION --depth 1 $ZLIB_GIT_ADDRESS $ZLIB_SRC_PATH

################################################################################
### Build and install

# Build
New-Item $ZLIB_BUILD_PATH -ItemType directory
Set-Location $ZLIB_BUILD_PATH
cmake -G "Visual Studio 17 2022" -A x64 $ZLIB_SRC_PATH `
	-DCMAKE_C_FLAGS="/W0" `
	-DCMAKE_CXX_FLAGS="/W0" `
	-DCMAKE_INSTALL_PREFIX:PATH=$ZLIB_INSTALL_PATH `
	-DZLIB_BUILD_EXAMPLES=NO
cmake --build . --parallel --target INSTALL --config Debug
cmake --build . --parallel --target INSTALL --config Release

################################################################################
### Copy *.pdb files

Copy-Item "$ZLIB_BUILD_PATH\Debug\*.pdb" "$ZLIB_INSTALL_PATH\lib\"

################################################################################
### Clean installation directory

# Need to clean up to avoid putting unnecessary files into the installer
$REM_ZLIB_ROOT_LIST = @(
	"bin",
	"share",
	"lib\zlib.lib",
	"lib\zlibd.lib"
)

# Gather
$REM_LIST = @("TEMP_TO_DEL")
foreach ($item in $REM_ZLIB_ROOT_LIST) {
	$REM_LIST += $ZLIB_INSTALL_PATH + '\' + $item
}
# Remove
foreach ($item in $REM_LIST) {
	Remove-Item "$item" -Force -Recurse -ErrorAction Ignore
}

################################################################################
### Clean work directory

Set-Location $CURRENT_PATH

Remove-Item $ZLIB_BUILD_PATH -Force -Recurse
Remove-Item $ZLIB_SRC_PATH -Force -Recurse

Write-Host "Completed! $ZLIB_NAME library is installed at: $ZLIB_INSTALL_PATH" -ForegroundColor Green
