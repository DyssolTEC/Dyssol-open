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

$SZIP_VERSION = "2.1.1"
$SZIP_DOWNLOAD_ADDRESS = "https://support.hdfgroup.org/ftp/lib-external/szip/2.1.1/src/szip-$SZIP_VERSION.tar.gz"
$SZIP_NAME = "szip-$SZIP_VERSION"
$SZIP_TAR_NAME = "$SZIP_NAME.tar"
$SZIP_ZIP_NAME = "$SZIP_NAME.tar.gz"
$SZIP_INSTALL_PATH = "$CURRENT_PATH\szip"
$SZIP_SRC_PATH = "$CURRENT_PATH\$SZIP_NAME"
$SZIP_BUILD_PATH = "$SZIP_SRC_PATH\build"

################################################################################
### Clear old

Remove-Item $SZIP_INSTALL_PATH -Force -Recurse -ErrorAction Ignore
Remove-Item $SZIP_BUILD_PATH -Force -Recurse -ErrorAction Ignore
Remove-Item $SZIP_SRC_PATH -Force -Recurse -ErrorAction Ignore

################################################################################
### Download

Invoke-WebRequest $SZIP_DOWNLOAD_ADDRESS -OutFile $SZIP_ZIP_NAME
Expand-7Zip $SZIP_ZIP_NAME . | Expand-7Zip $SZIP_TAR_NAME .

################################################################################
### Build and install

# Build x32
New-Item $SZIP_BUILD_PATH\x32 -ItemType directory
Set-Location $SZIP_BUILD_PATH\x32
cmake -G "Visual Studio 14 2015" $SZIP_SRC_PATH -DCMAKE_INSTALL_PREFIX:PATH=$SZIP_INSTALL_PATH -DBUILD_SHARED_LIBS=NO
cmake --build . --target INSTALL --config Debug
cmake --build . --target INSTALL --config Release
Rename-Item -Path "$SZIP_INSTALL_PATH\lib" -NewName "$SZIP_INSTALL_PATH\lib32"

# Build x64
New-Item $SZIP_BUILD_PATH\x64 -ItemType directory
Set-Location $SZIP_BUILD_PATH\x64
cmake -G "Visual Studio 14 2015 Win64" $SZIP_SRC_PATH -DCMAKE_INSTALL_PREFIX:PATH=$SZIP_INSTALL_PATH -DBUILD_SHARED_LIBS=NO
cmake --build . --target INSTALL --config Debug
cmake --build . --target INSTALL --config Release
Rename-Item -Path "$SZIP_INSTALL_PATH\lib" -NewName "$SZIP_INSTALL_PATH\lib64"

################################################################################
### Copy *.pdb files

Copy-Item "$SZIP_BUILD_PATH\x32\bin\Debug\libszip_D.pdb" "$SZIP_INSTALL_PATH\lib32\libszip_D.pdb"
Copy-Item "$SZIP_BUILD_PATH\x64\bin\Debug\libszip_D.pdb" "$SZIP_INSTALL_PATH\lib64\libszip_D.pdb"


################################################################################
### Clean installation directory

$REM_SZIP_ROOT_LIST = @(
	"bin",
	"cmake",
#	"include",
	"COPYING",
	"INSTALL",
	"README",
	"RELEASE.txt"
)

# gather
$REM_LIST = @("TEMP_TO_DEL")
foreach ($item in $REM_SZIP_ROOT_LIST) {
	$REM_LIST += $SZIP_INSTALL_PATH + '\' + $item
}
# remove
foreach ($item in $REM_LIST) {
	Remove-Item "$item" -Force -Recurse -ErrorAction Ignore
}

################################################################################
### Clean work directory

Set-Location $CURRENT_PATH

Remove-Item $SZIP_BUILD_PATH -Force -Recurse
Remove-Item $SZIP_SRC_PATH -Force -Recurse
Remove-Item $SZIP_ZIP_NAME -Force -Recurse
Remove-Item $SZIP_TAR_NAME -Force -Recurse
