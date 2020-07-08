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

$HDF5_MAJOR_VERSION = "1"
$HDF5_MIDDLE_VERSION = "10"
$HDF5_MINOR_VERSION = "2"
$HDF5_VERSION = "$HDF5_MAJOR_VERSION.$HDF5_MIDDLE_VERSION.$HDF5_MINOR_VERSION"
$HDF5_DOWNLOAD_ADDRESS = "https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-$HDF5_MAJOR_VERSION.$HDF5_MIDDLE_VERSION/hdf5-$HDF5_VERSION/src/hdf5-$HDF5_VERSION.zip"
$HDF5_NAME = "hdf5-$HDF5_VERSION"
$HDF5_ZIP_NAME = "$HDF5_NAME.zip"
$HDF5_INSTALL_PATH = "$CURRENT_PATH\hdf5"
$HDF5_SRC_PATH = "$CURRENT_PATH\$HDF5_NAME"
$HDF5_BUILD_PATH = "$HDF5_SRC_PATH\build"

# libs
$ZLIB_INSTALL_PATH = "$CURRENT_PATH\zlib"
$SZIP_INSTALL_PATH = "$CURRENT_PATH\szip"

################################################################################
### Clear old

Remove-Item $HDF5_INSTALL_PATH -Force -Recurse -ErrorAction Ignore
Remove-Item $HDF5_BUILD_PATH -Force -Recurse -ErrorAction Ignore
Remove-Item $HDF5_SRC_PATH -Force -Recurse -ErrorAction Ignore

################################################################################
### Download

Invoke-WebRequest $HDF5_DOWNLOAD_ADDRESS -OutFile $HDF5_ZIP_NAME
Expand-Archive -Path $HDF5_ZIP_NAME -DestinationPath .

################################################################################
### Build and install

# Build x32
New-Item $HDF5_BUILD_PATH\x32 -ItemType directory
Set-Location $HDF5_BUILD_PATH\x32
cmake -G "Visual Studio 14 2015" $HDF5_SRC_PATH -DCMAKE_INSTALL_PREFIX:PATH=$HDF5_INSTALL_PATH -DBUILD_SHARED_LIBS=NO -DBUILD_TESTING=NO -DHDF5_BUILD_CPP_LIB=YES -DHDF5_BUILD_HL_LIB=YES -DHDF5_BUILD_EXAMPLES=NO -DHDF5_BUILD_TOOLS=NO -DHDF5_ENABLE_Z_LIB_SUPPORT=YES -DZLIB_DIR="" -DZLIB_INCLUDE_DIR="$ZLIB_INSTALL_PATH/include" -DZLIB_LIBRARY_DEBUG="$ZLIB_INSTALL_PATH/lib32/zlibstaticd.lib" -DZLIB_LIBRARY_RELEASE="$ZLIB_INSTALL_PATH/lib32/zlibstatic.lib" -DHDF5_ENABLE_SZIP_SUPPORT=YES -DSZIP_DIR="" -DSZIP_INCLUDE_DIR="$SZIP_INSTALL_PATH/include" -DSZIP_LIBRARY_DEBUG="$SZIP_INSTALL_PATH/lib32/libszip_D.lib" -DSZIP_LIBRARY_RELEASE="$SZIP_INSTALL_PATH/lib32/libszip.lib"
cmake --build . --target INSTALL --config Debug
cmake --build . --target INSTALL --config Release
Rename-Item -Path "$HDF5_INSTALL_PATH\lib" -NewName "$HDF5_INSTALL_PATH\lib32"

# Build x64
New-Item $HDF5_BUILD_PATH\x64 -ItemType directory
Set-Location $HDF5_BUILD_PATH\x64
cmake -G "Visual Studio 14 2015 Win64" $HDF5_SRC_PATH -DCMAKE_INSTALL_PREFIX:PATH=$HDF5_INSTALL_PATH -DBUILD_SHARED_LIBS=NO -DBUILD_TESTING=NO -DHDF5_BUILD_CPP_LIB=YES -DHDF5_BUILD_HL_LIB=YES -DHDF5_BUILD_EXAMPLES=NO -DHDF5_BUILD_TOOLS=NO -DHDF5_ENABLE_Z_LIB_SUPPORT=YES -DZLIB_DIR="" -DZLIB_INCLUDE_DIR="$ZLIB_INSTALL_PATH/include" -DZLIB_LIBRARY_DEBUG="$ZLIB_INSTALL_PATH/lib64/zlibstaticd.lib" -DZLIB_LIBRARY_RELEASE="$ZLIB_INSTALL_PATH/lib64/zlibstatic.lib" -DHDF5_ENABLE_SZIP_SUPPORT=YES -DSZIP_DIR="" -DSZIP_INCLUDE_DIR="$SZIP_INSTALL_PATH/include" -DSZIP_LIBRARY_DEBUG="$SZIP_INSTALL_PATH/lib64/libszip_D.lib" -DSZIP_LIBRARY_RELEASE="$SZIP_INSTALL_PATH/lib64/libszip.lib"
cmake --build . --target INSTALL --config Debug
cmake --build . --target INSTALL --config Release
Rename-Item -Path "$HDF5_INSTALL_PATH\lib" -NewName "$HDF5_INSTALL_PATH\lib64"

################################################################################
### Copy *.pdb files

Copy-Item "$HDF5_BUILD_PATH\x32\bin\Debug\libhdf5_D.pdb" "$HDF5_INSTALL_PATH\lib32\libhdf5_D.pdb"
Copy-Item "$HDF5_BUILD_PATH\x64\bin\Debug\libhdf5_D.pdb" "$HDF5_INSTALL_PATH\lib64\libhdf5_D.pdb"
Copy-Item "$HDF5_BUILD_PATH\x32\bin\Debug\libhdf5_hl_D.pdb" "$HDF5_INSTALL_PATH\lib32\libhdf5_hl_D.pdb"
Copy-Item "$HDF5_BUILD_PATH\x64\bin\Debug\libhdf5_hl_D.pdb" "$HDF5_INSTALL_PATH\lib64\libhdf5_hl_D.pdb"
Copy-Item "$HDF5_BUILD_PATH\x32\bin\Debug\libhdf5_cpp_D.pdb" "$HDF5_INSTALL_PATH\lib32\libhdf5_cpp_D.pdb"
Copy-Item "$HDF5_BUILD_PATH\x64\bin\Debug\libhdf5_cpp_D.pdb" "$HDF5_INSTALL_PATH\lib64\libhdf5_cpp_D.pdb"
Copy-Item "$HDF5_BUILD_PATH\x32\bin\Debug\libhdf5_hl_cpp_D.pdb" "$HDF5_INSTALL_PATH\lib32\libhdf5_hl_cpp_D.pdb"
Copy-Item "$HDF5_BUILD_PATH\x64\bin\Debug\libhdf5_hl_cpp_D.pdb" "$HDF5_INSTALL_PATH\lib64\libhdf5_hl_cpp_D.pdb"

################################################################################
### Clean installation directory

$REM_HDF5_ROOT_LIST = @(
	"bin",
	"cmake",
	"lib32\libhdf5.settings",
	"lib64\libhdf5.settings",
	"lib32\pkgconfig",
	"lib64\pkgconfig",
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

# gather
$REM_LIST = @("TEMP_TO_DEL")
foreach ($item in $REM_HDF5_ROOT_LIST) {
	$REM_LIST += $HDF5_INSTALL_PATH + '\' + $item
}
foreach ($item in $REM_HDF5_INCLUDE_LIST) {
	$REM_LIST += $HDF5_INSTALL_PATH + '\include\' + $item + '.h'
}
# remove
foreach ($item in $REM_LIST) {
	Remove-Item "$item" -Force -Recurse -ErrorAction Ignore
}

################################################################################
### Clean work directory

Set-Location $CURRENT_PATH

Remove-Item $HDF5_BUILD_PATH -Force -Recurse
Remove-Item $HDF5_SRC_PATH -Force -Recurse
Remove-Item $HDF5_ZIP_NAME -Force -Recurse