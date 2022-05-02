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

$ZLIB_VERSION = "1.2.12"
$ZLIB_DOWNLOAD_ADDRESS = "http://www.zlib.net/zlib-$ZLIB_VERSION.tar.gz"
$ZLIB_NAME = "zlib-$ZLIB_VERSION"
$ZLIB_TAR_NAME = "$ZLIB_NAME.tar"
$ZLIB_ZIP_NAME = "$ZLIB_TAR_NAME.gz"
$ZLIB_INSTALL_PATH = "$CURRENT_PATH\zlib"
$ZLIB_SRC_PATH = "$CURRENT_PATH\$ZLIB_NAME"
$ZLIB_BUILD_PATH = "$ZLIB_SRC_PATH\build"

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

# Build x32
New-Item $ZLIB_BUILD_PATH\x32 -ItemType directory
Set-Location $ZLIB_BUILD_PATH\x32
cmake -G "Visual Studio 16 2019" -A Win32 $ZLIB_SRC_PATH -DCMAKE_INSTALL_PREFIX:PATH=$ZLIB_INSTALL_PATH
cmake --build . --target INSTALL --config Debug
cmake --build . --target INSTALL --config Release
Rename-Item -Path "$ZLIB_INSTALL_PATH\lib" -NewName "$ZLIB_INSTALL_PATH\lib32"

# Build x64
New-Item $ZLIB_BUILD_PATH\x64 -ItemType directory
Set-Location $ZLIB_BUILD_PATH\x64
cmake -G "Visual Studio 16 2019" -A x64 $ZLIB_SRC_PATH -DCMAKE_INSTALL_PREFIX:PATH=$ZLIB_INSTALL_PATH
cmake --build . --target INSTALL --config Debug
cmake --build . --target INSTALL --config Release
Rename-Item -Path "$ZLIB_INSTALL_PATH\lib" -NewName "$ZLIB_INSTALL_PATH\lib64"

################################################################################
### Copy *.pdb files

Copy-Item "$ZLIB_BUILD_PATH\x32\Debug\zlibstaticd.pdb" "$ZLIB_INSTALL_PATH\lib32\zlibstaticd.pdb"
Copy-Item "$ZLIB_BUILD_PATH\x64\Debug\zlibstaticd.pdb" "$ZLIB_INSTALL_PATH\lib64\zlibstaticd.pdb"

################################################################################
### Clean installation directory

$REM_ZLIB_ROOT_LIST = @(
	"bin",
	"share",
	"lib32\zlib.lib",
	"lib64\zlib.lib",
	"lib32\zlibd.lib",
	"lib64\zlibd.lib"
)

# gather
$REM_LIST = @("TEMP_TO_DEL")
foreach ($item in $REM_ZLIB_ROOT_LIST) {
	$REM_LIST += $ZLIB_INSTALL_PATH + '\' + $item
}
# remove
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