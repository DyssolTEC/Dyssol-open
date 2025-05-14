# Copyright (c) 2021, Dyssol Development Team.
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
# Check python3 is available
try { $null = py --version }
catch [System.Management.Automation.CommandNotFoundException] { 
	throw "python3 is not available. Install it and add to PATH." 
}

################################################################################
### Paths

$GRAPHVIZ_MAJOR_VERSION  = "12"
$GRAPHVIZ_MIDDLE_VERSION = "2"
$GRAPHVIZ_MINOR_VERSION  = "1"
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
git -c advice.detachedHead=false clone --branch $GRAPHVIZ_VERSION --depth 1 $GRAPHVIZ_GIT_ADDRESS $GRAPHVIZ_SRC_PATH

# Download and initialize dependencies
Set-Location $GRAPHVIZ_SRC_PATH
git submodule update --init

################################################################################
### Build and install

# Paths to dependencies
$DEPEND_PATH  = "$GRAPHVIZ_SRC_PATH\windows\dependencies"
$DEPEND_UTILS = "$DEPEND_PATH\graphviz-build-utilities"
$DEPEND_LIBS  = "$DEPEND_PATH\libraries\x64"

# Build
New-Item $GRAPHVIZ_BUILD_PATH -ItemType directory
Set-Location $GRAPHVIZ_BUILD_PATH
cmake -G "Visual Studio 17 2022" -A x64 $GRAPHVIZ_SRC_PATH `
	-DCMAKE_C_FLAGS="/W0" `
	-DCMAKE_CXX_FLAGS="/W0" `
	-DCMAKE_INSTALL_PREFIX:PATH=$GRAPHVIZ_INSTALL_PATH `
	-DCMAKE_DISABLE_FIND_PACKAGE_ANN=ON `
	-DCMAKE_DISABLE_FIND_PACKAGE_CAIRO=ON `
	-DCMAKE_DISABLE_FIND_PACKAGE_GD=ON `
	-DCMAKE_DISABLE_FIND_PACKAGE_GTK2=ON `
	-DCMAKE_DISABLE_FIND_PACKAGE_Freetype=ON `
	-DCMAKE_DISABLE_FIND_PACKAGE_NSIS=ON `
	-DCMAKE_DISABLE_FIND_PACKAGE_PANGOCAIRO=ON `
	-DBUILD_TESTING=OFF `
	-DPKG_CONFIG_EXECUTABLE="" `
	-DBISON_EXECUTABLE="$DEPEND_UTILS\winflexbison\win_bison.exe" `
	-DFLEX_EXECUTABLE="$DEPEND_UTILS\winflexbison\win_flex.exe" `
	-DWITH_ZLIB=ON `
	-DZLIB_INCLUDE_DIR="$ZLIB_INSTALL_PATH\include" `
	-DZLIB_LIBRARY_DEBUG="$ZLIB_INSTALL_PATH\lib\zlibstaticd.lib" `
	-DZLIB_LIBRARY_RELEASE="$ZLIB_INSTALL_PATH\lib\zlibstatic.lib" `
	-DENABLE_D=OFF `
	-DENABLE_GO=OFF `
	-DENABLE_GUILE=OFF `
	-DENABLE_JAVA=OFF `
	-DENABLE_JAVASCRIPT=OFF `
	-DENABLE_LTDL=ON `
	-DENABLE_LUA=OFF `
	-DENABLE_PERL=OFF `
	-DENABLE_PHP=OFF `
	-DENABLE_R=OFF `
	-DENABLE_SHARP=OFF `
	-DENABLE_SWIG=OFF `
	-DENABLE_TCL=OFF `
	-Duse_coverage=OFF `
	-Duse_win_pre_inst_libs=ON `
	-Dwith_cxx_api=OFF `
	-Dwith_cxx_tests=OFF `
	-Dwith_digcola=OFF `
	-Dwith_ipsepcola=OFF `
	-Dwith_ortho=OFF `
	-Dwith_sfdp=OFF
cmake --build . --parallel --target INSTALL --config Release
Copy-Item -Path "$CURRENT_PATH\graphviz_config" -Destination "$GRAPHVIZ_INSTALL_PATH\bin\config6"

################################################################################
### Clean installation directory

# Need to clean up to avoid putting unnecessary files into the installer
$REM_GRAPHVIZ_ROOT_LIST = @(
	"bin\dot_sandbox",
	"lib\graphviz",
	"lib\pkgconfig",
	"share"
)
$REM_GRAPHVIZ_DLL_LIST = @(
	"concrt140", 
	"getopt", 
	"gvplugin_neato_layout", 
	"gvplugin_vt", 
	"msvcp140", 
	"msvcp140_1",
	"msvcp140_2",
	"msvcp140_atomic_wait",
	"msvcp140_codecvt_ids",
	"tcl86t",
	"tcldot",
	"tcldot_builtin",
	"tclplan",
	"vcruntime140",
	"vcruntime140_1",
	"zlib1"
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
	"gvplugin_gdiplus", 
	"gvplugin_neato_layout", 
	"gvplugin_vt", 
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

Write-Host "Completed! $GRAPHVIZ_NAME library is installed at: $GRAPHVIZ_INSTALL_PATH" -ForegroundColor Green
