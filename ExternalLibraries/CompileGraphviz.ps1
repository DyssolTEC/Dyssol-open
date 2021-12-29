# Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information.

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

$GRAPHVIZ_MAJOR_VERSION  = "2"
$GRAPHVIZ_MIDDLE_VERSION = "49"
$GRAPHVIZ_MINOR_VERSION  = "3"
$GRAPHVIZ_VERSION        = "$GRAPHVIZ_MAJOR_VERSION.$GRAPHVIZ_MIDDLE_VERSION.$GRAPHVIZ_MINOR_VERSION"
$GRAPHVIZ_GIT_ADDRESS    = "https://gitlab.com/graphviz/graphviz.git"
$GRAPHVIZ_NAME           = "graphviz-$GRAPHVIZ_VERSION"
$GRAPHVIZ_INSTALL_PATH   = "$CURRENT_PATH\graphviz"
$GRAPHVIZ_SRC_PATH       = "$CURRENT_PATH\$GRAPHVIZ_NAME"
$GRAPHVIZ_BUILD_PATH     = "$GRAPHVIZ_SRC_PATH\build"

# libs
$ZLIB_INSTALL_PATH = "$CURRENT_PATH\zlib"

################################################################################
### Clear old

Remove-Item $GRAPHVIZ_INSTALL_PATH -Force -Recurse -ErrorAction Ignore
Remove-Item $GRAPHVIZ_BUILD_PATH   -Force -Recurse -ErrorAction Ignore
Remove-Item $GRAPHVIZ_SRC_PATH     -Force -Recurse -ErrorAction Ignore

################################################################################
### Download

# clone selected version
git clone --branch $GRAPHVIZ_VERSION --depth 1 $GRAPHVIZ_GIT_ADDRESS $GRAPHVIZ_SRC_PATH

################################################################################
### Build and install

# download and initialize dependencies
Set-Location $GRAPHVIZ_SRC_PATH
git submodule update --init

# paths to dependencies
$DEPEND_PATH    = "$GRAPHVIZ_SRC_PATH\windows\dependencies"
$DEPEND_UTILS   = "$DEPEND_PATH\graphviz-build-utilities"
$DEPEND_LIBS32  = "$DEPEND_PATH\libraries\x86"
$DEPEND_LIBS64  = "$DEPEND_PATH\libraries\x64"
$DEPEND_VCPKG32 = "$DEPEND_PATH\libraries\vcpkg\installed\x86-windows"
$DEPEND_VCPKG64 = "$DEPEND_PATH\libraries\vcpkg\installed\x64-windows"

# Build x32
New-Item $GRAPHVIZ_BUILD_PATH\x32 -ItemType directory
Set-Location $GRAPHVIZ_BUILD_PATH\x32
cmake -G "Visual Studio 16 2019" -A Win32 $GRAPHVIZ_SRC_PATH `
	-DCMAKE_INSTALL_PREFIX:PATH=$GRAPHVIZ_INSTALL_PATH `
	-DAWK_EXECUTABLE="$DEPEND_UTILS\awk.exe" `
	-DBISON_EXECUTABLE="$DEPEND_UTILS\winflexbison\win_bison.exe" `
	-DBUILD_TESTING=NO `
	-DCario_INCLUDE_DIR="$DEPEND_LIBS32\include" `
	-DCario_LIBRARY="$DEPEND_LIBS32\lib\cairo.lib" `
	-DCario_RUNTIME_LIBRARY="$DEPEND_LIBS32\bin\cairo.dll" `
	-DEXPAT_INCLUDE_DIR="$DEPEND_LIBS32\include" `
	-DEXPAT_LIBRARY="$DEPEND_LIBS32\lib\expat.lib" `
	-DEXPAT_RUNTIME_LIBRARIES="$DEPEND_LIBS32\bin\expat.dll" `
	-DEXPAT_RUNTIME_LIBRARY="$DEPEND_LIBS32\bin\expat.dll" `
	-DFLEX_EXECUTABLE="$DEPEND_UTILS\winflexbison\win_flex.exe" `
	-DFLEX_INCLUDE_DIR="$DEPEND_UTILS\winflexbison" `
	-DFONTCONFIG_RUNTIME_LIBRARY="$DEPEND_LIBS32\bin\fontconfig.dll" `
	-DGLIBCONFIG_INCLUDE_DIR="$DEPEND_LIBS32\include" `
	-DGLIB_INCLUDE_DIR="$DEPEND_LIBS32\include" `
	-DGLIB_LIBRARY="$DEPEND_LIBS32\lib\glib-2.0.lib" `
	-DGLIB_RUNTIME_LIBRARY="$DEPEND_LIBS32\bin\glib-2.dll" `
	-DGOBJECT_LIBRARY="$DEPEND_LIBS32\lib\gobject-2.0.lib" `
	-DGOBJECT_RUNTIME_LIBRARY="$DEPEND_LIBS32\bin\gobject-2.dll" `
	-DGetopt_INCLUDE_DIR="$DEPEND_VCPKG32\include" `
	-DGetopt_LIBRARY="$DEPEND_VCPKG32\lib\getopt.lib" `
	-DGetopt_RUNTIME_LIBRARY="$DEPEND_VCPKG32\bin\getopt.dll" `
	-DHARFBUZZ_RUNTIME_LIBRARY="$DEPEND_LIBS32\bin\libharfbuzz-0.dll" `
	-DLTDL_INCLUDE_DIR="$DEPEND_LIBS32\include" `
	-DNSIS_MAKE="$DEPEND_UTILS\NSIS" `
	-DPANGOFT_RUNTIME_LIBRARY="$DEPEND_LIBS32\bin\pangoft2-1.dll" `
	-DPANGOWIN_RUNTIME_LIBRARY="$DEPEND_LIBS32\bin\pangowin32-1.dll" `
	-DPANGO_LIBRARY="$DEPEND_LIBS32\lib\pango-1.0.lib" `
	-DPANGO_RUNTIME_LIBRARY="$DEPEND_LIBS32\bin\pango-1.dll" `
	-DPIXMAN_RUNTIME_LIBRARY="$DEPEND_LIBS32\bin\pixman-1.dll" `
	-DPangoCairo_INCLUDE_DIR="$DEPEND_LIBS32\include" `
	-DPangoCairo_LIBRARY="$DEPEND_LIBS32\lib\pangocairo-1.0.lib" `
	-DPangoCairo_RUNTIME_LIBRARY="$DEPEND_LIBS32\bin\pangocairo-1.dll" `
	-DZLIB_INCLUDE_DIR="$ZLIB_INSTALL_PATH\include" `
	-DZLIB_LIBRARY_DEBUG="$ZLIB_INSTALL_PATH\lib32\zlibstaticd.lib" `
	-DZLIB_LIBRARY_RELEASE="$ZLIB_INSTALL_PATH\lib32\zlibstatic.lib" `
	-Denable_ltdl=YES `
	-Duse_coverage=NO `
	-Duse_sanitizers=NO `
	-Duse_win_pre_inst_libs=YES `
	-Dwith_cxx_api=NO `
	-Dwith_cxx_tests=NO `
	-Dwith_digcola=YES `
	-Dwith_expat=YES `
	-Dwith_ipsepcola=NO `
	-Dwith_ortho=YES `
	-Dwith_sfdp=YES `
	-Dwith_smyrna=NO `
	-Dwith_zlib=YES
cmake --build . --target INSTALL --config Release
Rename-Item -Path "$GRAPHVIZ_INSTALL_PATH\bin" -NewName "$GRAPHVIZ_INSTALL_PATH\bin32"
Rename-Item -Path "$GRAPHVIZ_INSTALL_PATH\lib" -NewName "$GRAPHVIZ_INSTALL_PATH\lib32"
Copy-Item -Path "$CURRENT_PATH\graphviz_config" -Destination "$GRAPHVIZ_INSTALL_PATH\bin32\config6"

# Build x64
New-Item $GRAPHVIZ_BUILD_PATH\x64 -ItemType directory
Set-Location $GRAPHVIZ_BUILD_PATH\x64
cmake -G "Visual Studio 16 2019" -A x64 $GRAPHVIZ_SRC_PATH `
	-DCMAKE_INSTALL_PREFIX:PATH=$GRAPHVIZ_INSTALL_PATH `
	-DAWK_EXECUTABLE="$DEPEND_UTILS\awk.exe" `
	-DBISON_EXECUTABLE="$DEPEND_UTILS\winflexbison\win_bison.exe" `
	-DBUILD_TESTING=NO `
	-DCario_INCLUDE_DIR="$DEPEND_LIBS64\include" `
	-DCario_LIBRARY="$DEPEND_LIBS64\lib\cairo.lib" `
	-DCario_RUNTIME_LIBRARY="$DEPEND_LIBS64\bin\cairo.dll" `
	-DEXPAT_INCLUDE_DIR="$DEPEND_LIBS64\include" `
	-DEXPAT_LIBRARY="$DEPEND_LIBS64\lib\expat.lib" `
	-DEXPAT_RUNTIME_LIBRARIES="$DEPEND_LIBS64\bin\expat.dll" `
	-DEXPAT_RUNTIME_LIBRARY="$DEPEND_LIBS64\bin\expat.dll" `
	-DFLEX_EXECUTABLE="$DEPEND_UTILS\winflexbison\win_flex.exe" `
	-DFLEX_INCLUDE_DIR="$DEPEND_UTILS\winflexbison" `
	-DFONTCONFIG_RUNTIME_LIBRARY="$DEPEND_LIBS64\bin\fontconfig.dll" `
	-DGLIBCONFIG_INCLUDE_DIR="$DEPEND_LIBS64\include" `
	-DGLIB_INCLUDE_DIR="$DEPEND_LIBS64\include" `
	-DGLIB_LIBRARY="$DEPEND_LIBS64\lib\glib-2.0.lib" `
	-DGLIB_RUNTIME_LIBRARY="$DEPEND_LIBS64\bin\glib-2.dll" `
	-DGOBJECT_LIBRARY="$DEPEND_LIBS64\lib\gobject-2.0.lib" `
	-DGOBJECT_RUNTIME_LIBRARY="$DEPEND_LIBS64\bin\gobject-2.dll" `
	-DGetopt_INCLUDE_DIR="$DEPEND_VCPKG64\include" `
	-DGetopt_LIBRARY="$DEPEND_VCPKG64\lib\getopt.lib" `
	-DGetopt_RUNTIME_LIBRARY="$DEPEND_VCPKG64\bin\getopt.dll" `
	-DHARFBUZZ_RUNTIME_LIBRARY="$DEPEND_LIBS64\bin\libharfbuzz-0.dll" `
	-DLTDL_INCLUDE_DIR="$DEPEND_LIBS64\include" `
	-DNSIS_MAKE="$DEPEND_UTILS\NSIS" `
	-DPANGOFT_RUNTIME_LIBRARY="$DEPEND_LIBS64\bin\pangoft2-1.dll" `
	-DPANGOWIN_RUNTIME_LIBRARY="$DEPEND_LIBS64\bin\pangowin32-1.dll" `
	-DPANGO_LIBRARY="$DEPEND_LIBS64\lib\pango-1.0.lib" `
	-DPANGO_RUNTIME_LIBRARY="$DEPEND_LIBS64\bin\pango-1.dll" `
	-DPIXMAN_RUNTIME_LIBRARY="$DEPEND_LIBS64\bin\pixman-1.dll" `
	-DPangoCairo_INCLUDE_DIR="$DEPEND_LIBS64\include" `
	-DPangoCairo_LIBRARY="$DEPEND_LIBS64\lib\pangocairo-1.0.lib" `
	-DPangoCairo_RUNTIME_LIBRARY="$DEPEND_LIBS64\bin\pangocairo-1.dll" `
	-DZLIB_INCLUDE_DIR="$ZLIB_INSTALL_PATH\include" `
	-DZLIB_LIBRARY_DEBUG="$ZLIB_INSTALL_PATH\lib64\zlibstaticd.lib" `
	-DZLIB_LIBRARY_RELEASE="$ZLIB_INSTALL_PATH\lib64\zlibstatic.lib" `
	-Denable_ltdl=YES `
	-Duse_coverage=NO `
	-Duse_sanitizers=NO `
	-Duse_win_pre_inst_libs=YES `
	-Dwith_cxx_api=NO `
	-Dwith_cxx_tests=NO `
	-Dwith_digcola=YES `
	-Dwith_expat=YES `
	-Dwith_ipsepcola=NO `
	-Dwith_ortho=YES `
	-Dwith_sfdp=YES `
	-Dwith_smyrna=NO `
	-Dwith_zlib=YES
cmake --build . --target INSTALL --config Release
Rename-Item -Path "$GRAPHVIZ_INSTALL_PATH\bin" -NewName "$GRAPHVIZ_INSTALL_PATH\bin64"
Rename-Item -Path "$GRAPHVIZ_INSTALL_PATH\lib" -NewName "$GRAPHVIZ_INSTALL_PATH\lib64"
Copy-Item -Path "$CURRENT_PATH\graphviz_config" -Destination "$GRAPHVIZ_INSTALL_PATH\bin64\config6"

################################################################################
### Clean installation directory

$REM_GRAPHVIZ_ROOT_LIST = @(
	"lib32\pkgconfig",
	"lib64\pkgconfig",
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

# gather
$REM_LIST = @("TEMP_TO_DEL")
foreach ($item in $REM_GRAPHVIZ_ROOT_LIST) {
	$REM_LIST += $GRAPHVIZ_INSTALL_PATH + '\' + $item
}
foreach ($item in $REM_GRAPHVIZ_DLL_LIST) {
	$REM_LIST += $GRAPHVIZ_INSTALL_PATH + '\bin32\' + $item + '.dll'
	$REM_LIST += $GRAPHVIZ_INSTALL_PATH + '\bin64\' + $item + '.dll'
}
foreach ($item in $REM_GRAPHVIZ_INCLUDE_LIST) {
	$REM_LIST += $GRAPHVIZ_INSTALL_PATH + '\include\graphviz\' + $item + '.h'
}
foreach ($item in $REM_GRAPHVIZ_LIB_LIST) {
	$REM_LIST += $GRAPHVIZ_INSTALL_PATH + '\lib32\' + $item + '.lib'
	$REM_LIST += $GRAPHVIZ_INSTALL_PATH + '\lib64\' + $item + '.lib'
}
# remove
foreach ($item in $REM_LIST) {
	Remove-Item "$item" -Force -Recurse -ErrorAction Ignore
}
# remove all binaries
Remove-Item "$GRAPHVIZ_INSTALL_PATH\*" -Recurse -Include *.exe

################################################################################
### Clean work directory

Set-Location $CURRENT_PATH

Remove-Item $GRAPHVIZ_BUILD_PATH -Force -Recurse
Remove-Item $GRAPHVIZ_SRC_PATH   -Force -Recurse
