# Copyright (c) 2020, Dyssol Development Team. All rights reserved.
# Copyright (c) 2023, DyssolTEC GmbH. 
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

$ZLIB_VERSION          = "1.3"
$ZLIB_NAME             = "zlib-$ZLIB_VERSION"
$ZLIB_TAR_NAME         = "$ZLIB_NAME.tar"
$ZLIB_ZIP_NAME         = "$ZLIB_TAR_NAME.gz"
$ZLIB_DOWNLOAD_ADDRESS = "https://github.com/madler/zlib/releases/download/v$ZLIB_VERSION/$ZLIB_ZIP_NAME"
$ZLIB_INSTALL_PATH     = "$CURRENT_PATH\zlib"
$ZLIB_SRC_PATH         = "$CURRENT_PATH\$ZLIB_NAME"
$ZLIB_BUILD_PATH       = "$ZLIB_SRC_PATH\build"

################################################################################
### Clear old

Remove-Item $ZLIB_INSTALL_PATH -Force -Recurse -ErrorAction Ignore
Remove-Item $ZLIB_BUILD_PATH -Force -Recurse -ErrorAction Ignore
Remove-Item $ZLIB_SRC_PATH -Force -Recurse -ErrorAction Ignore

################################################################################
### Download

Invoke-WebRequest $ZLIB_DOWNLOAD_ADDRESS -OutFile $ZLIB_ZIP_NAME
Expand-7Zip $ZLIB_ZIP_NAME . | Expand-7Zip $ZLIB_TAR_NAME .

################################################################################
### Build and install

# Build
New-Item $ZLIB_BUILD_PATH -ItemType directory
Set-Location $ZLIB_BUILD_PATH
cmake -G "Visual Studio 16 2019" -A x64 $ZLIB_SRC_PATH `
	-DCMAKE_INSTALL_PREFIX:PATH=$ZLIB_INSTALL_PATH
cmake --build . --parallel --target INSTALL --config Debug
cmake --build . --parallel --target INSTALL --config Release

################################################################################
### Copy *.pdb files

Copy-Item "$ZLIB_BUILD_PATH\Debug\zlibstaticd.pdb" "$ZLIB_INSTALL_PATH\lib\zlibstaticd.pdb"

################################################################################
### Clean installation directory

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
Remove-Item $ZLIB_ZIP_NAME -Force -Recurse
Remove-Item $ZLIB_TAR_NAME -Force -Recurse