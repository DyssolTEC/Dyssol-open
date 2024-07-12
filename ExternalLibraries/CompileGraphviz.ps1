# Copyright (c) 2021, Dyssol Development Team.
# Copyright (c) 2024, DyssolTEC GmbH.
# All rights reserved. This file is part of Dyssol. See LICENSE file for license information.

################################################################################
### Initializing

[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
$CURRENT_PATH = (Get-Item -Path ".\" -Verbose).FullName
# Check git is available
try { $null = git --version }
catch [System.Management.Automation.CommandNotFoundException] { Throw "Git is not available. Install it and add to PATH." }
# Check cmake is available
try { $null = cmake --version }
catch [System.Management.Automation.CommandNotFoundException] { Throw "CMake is not available. Install it and add to PATH." }
# Check python3 is available
try { $null = py --version }
catch [System.Management.Automation.CommandNotFoundException] { Throw "python3 is not available. Install it and add to PATH." }

################################################################################
### Paths

$GRAPHVIZ_MAJOR_VERSION  = "7"
$GRAPHVIZ_MIDDLE_VERSION = "0"
$GRAPHVIZ_MINOR_VERSION  = "4"
$GRAPHVIZ_VERSION        = "$GRAPHVIZ_MAJOR_VERSION.$GRAPHVIZ_MIDDLE_VERSION.$GRAPHVIZ_MINOR_VERSION"
$GRAPHVIZ_GIT_ADDRESS    = "https://gitlab.com/graphviz/graphviz.git"
$GRAPHVIZ_NAME           = "graphviz-$GRAPHVIZ_VERSION"
$GRAPHVIZ_INSTALL_PATH   = "$CURRENT_PATH\graphviz"
$GRAPHVIZ_SRC_PATH       = "$CURRENT_PATH\$GRAPHVIZ_NAME"
$GRAPHVIZ_BUILD_PATH     = "$GRAPHVIZ_SRC_PATH\build"

# Libs
$ZLIB_INSTALL_PATH = "$CURRENT_PATH\zlib"

################################################################################
### Clear old

Remove-Item $GRAPHVIZ_INSTALL_PATH -Force -Recurse -ErrorAction Ignore
Remove-Item $GRAPHVIZ_BUILD_PATH   -Force -Recurse -ErrorAction Ignore
Remove-Item $GRAPHVIZ_SRC_PATH     -Force -Recurse -ErrorAction Ignore

################################################################################
### Download

# Clone selected version
git clone --branch $GRAPHVIZ_VERSION --depth 1 $GRAPHVIZ_GIT_ADDRESS $GRAPHVIZ_SRC_PATH

################################################################################
### Build and install

# Download and initialize dependencies
Set-Location $GRAPHVIZ_SRC_PATH
git submodule update --init

# Paths to dependencies
$DEPEND_PATH  = "$GRAPHVIZ_SRC_PATH\windows\dependencies"
$DEPEND_UTILS = "$DEPEND_PATH\graphviz-build-utilities"
$DEPEND_LIBS  = "$DEPEND_PATH\libraries\x64"

# Build
New-Item $GRAPHVIZ_BUILD_PATH -ItemType directory
Set-Location $GRAPHVIZ_BUILD_PATH
cmake -G "Visual Studio 16 2019" -A x64 $GRAPHVIZ_SRC_PATH `
	-DCMAKE_INSTALL_PREFIX:PATH=$GRAPHVIZ_INSTALL_PATH `
	-DCMAKE_DISABLE_FIND_PACKAGE_ANN=YES `
	-DCMAKE_DISABLE_FIND_PACKAGE_CAIRO=YES `
	-DCMAKE_DISABLE_FIND_PACKAGE_GD=YES `
	-DCMAKE_DISABLE_FIND_PACKAGE_GTK2=YES `
	-DCMAKE_DISABLE_FIND_PACKAGE_Freetype=YES `
	-DCMAKE_DISABLE_FIND_PACKAGE_NSIS=YES `
	-DCMAKE_DISABLE_FIND_PACKAGE_PANGOCAIRO=YES `
	-DPKG_CONFIG_EXECUTABLE="" `
	-DBISON_EXECUTABLE="$DEPEND_UTILS\winflexbison\win_bison.exe" `
	-DBUILD_TESTING=NO `
	-DEXPAT_INCLUDE_DIR="$DEPEND_LIBS\include" `
	-DEXPAT_LIBRARY="$DEPEND_LIBS\lib\expat.lib" `
	-DEXPAT_RUNTIME_LIBRARIES="$DEPEND_LIBS\bin\expat.dll" `
	-DFLEX_EXECUTABLE="$DEPEND_UTILS\winflexbison\win_flex.exe" `
	-DFLEX_INCLUDE_DIR="$DEPEND_UTILS\winflexbison" `
	-DLTDL_INCLUDE_DIR="$DEPEND_LIBS\include" `
	-DZLIB_INCLUDE_DIR="$ZLIB_INSTALL_PATH\include" `
	-DZLIB_LIBRARY_DEBUG="$ZLIB_INSTALL_PATH\lib\zlibstaticd.lib" `
	-DZLIB_LIBRARY_RELEASE="$ZLIB_INSTALL_PATH\lib\zlibstatic.lib" `
	-Denable_ltdl=YES `
	-Duse_coverage=NO `
	-Duse_sanitizers=NO `
	-Duse_win_pre_inst_libs=YES `
	-Dwith_cxx_api=NO `
	-Dwith_cxx_tests=NO `
	-Dwith_digcola=NO `
	-Dwith_expat=YES `
	-Dwith_gvedit=NO `
	-Dwith_ipsepcola=NO `
	-Dwith_ortho=NO `
	-Dwith_sfdp=NO `
	-Dwith_smyrna=NO `
	-Dwith_zlib=YES
cmake --build . --parallel --target INSTALL --config Release
Copy-Item -Path "$CURRENT_PATH\graphviz_config" -Destination "$GRAPHVIZ_INSTALL_PATH\bin\config6"

################################################################################
### Clean installation directory

$REM_GRAPHVIZ_ROOT_LIST = @(
	"lib\pkgconfig",
	"share"
)
$REM_GRAPHVIZ_DLL_LIST = @(
	"cairo", 
	"concrt140", 
	"fontconfig", 
	"getopt", 
	"glib-2", 
	"gobject-2", 
	"gvplugin_gd", 
	"gvplugin_neato_layout", 
	"gvplugin_pango", 
	"libgd", 
	"libharfbuzz-0", 
	"msvcp140", 
	"msvcp140_1",
	"msvcp140_2",
	"msvcp140_atomic_wait",
	"msvcp140_codecvt_ids",
	"pango-1",
	"pangocairo-1",
	"pangoft2-1",
	"pangowin32-1",
	"pixman-1",
	"vcruntime140"
	"vcruntime140_1"
)
$REM_GRAPHVIZ_INCLUDE_LIST = @(
	"color",
	"graphviz_version",
	"gvcjob",
	"gvcommon",
	"gvconfig",
	"gvplugin_device",
	"gvplugin_layout",
	"gvplugin_loadimage",
	"gvplugin_render",
	"gvplugin_textlayout",
	"pack",
	"pathplan",
	"xdot"
)
$REM_GRAPHVIZ_LIB_LIST = @(
	"gvplugin_core", 
	"gvplugin_dot_layout", 
	"gvplugin_gd", 
	"gvplugin_gdiplus", 
	"gvplugin_neato_layout", 
	"gvplugin_pango", 
	"pathplan", 
	"xdot"
)

# Gather
$REM_LIST = @("TEMP_TO_DEL")
foreach ($item in $REM_GRAPHVIZ_ROOT_LIST) {
	$REM_LIST += $GRAPHVIZ_INSTALL_PATH + '\' + $item
}
foreach ($item in $REM_GRAPHVIZ_DLL_LIST) {
	$REM_LIST += $GRAPHVIZ_INSTALL_PATH + '\bin\' + $item + '.dll'
}
foreach ($item in $REM_GRAPHVIZ_INCLUDE_LIST) {
	$REM_LIST += $GRAPHVIZ_INSTALL_PATH + '\include\graphviz\' + $item + '.h'
}
foreach ($item in $REM_GRAPHVIZ_LIB_LIST) {
	$REM_LIST += $GRAPHVIZ_INSTALL_PATH + '\lib\' + $item + '.lib'
}
# Remove
foreach ($item in $REM_LIST) {
	Remove-Item "$item" -Force -Recurse -ErrorAction Ignore
}
# Remove all binaries
Remove-Item "$GRAPHVIZ_INSTALL_PATH\*" -Recurse -Include *.exe

################################################################################
### Clean work directory

Set-Location $CURRENT_PATH

Remove-Item $GRAPHVIZ_BUILD_PATH -Force -Recurse
Remove-Item $GRAPHVIZ_SRC_PATH   -Force -Recurse
