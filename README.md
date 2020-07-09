# Dyssol
Dyssol is a tool for dynamic flowsheet simulation of complex production processes with advanced treatment of granular materials.

For more information, please check the [documentation](https://github.com/FlowsheetSimulation/Dyssol-open/tree/master/Documentation). 

For new versions and updates, please check [here](https://github.com/FlowsheetSimulation/Dyssol-open/releases/latest).

# Requirements 
Dyssol should install and work on all latest versions of Windows.
Requires Visual C++ Redistributable for Visual Studio 2015 to run.

# Compilation
A fully functional version can be compiled and biult with Microsoft Visual Studio 2015. A command-line version can also be compiled on Linux.

Requirements on Windows:
- Microsoft Visual Studio 2015
- Qt 5.8 msvc2015 / Qt 5.8 msvc2015_64
- Qt Visual Studio Tools for Visual Studio 2015
- Git

Also, other versions of Microsoft Visual Studio can be used, but additional preparations are required:
- All statically linked external libraries must be precompiled using the selected version. Scripts in ./ExternalLibraries may be used for that. To use scripts, the following requirements apply: Visual Studio 14 2015, cmake, PowerShell 5.0.
- If required, files assiciated with the Models Creator tool should be updated. They to be found in ./DyssolInstallers/Data/VCProject

Requirements on Linux:
- cmake 3.11.2
- gcc-7, g++-7

Biuld on Linux:
- Move to ./DyssolLinux/
- Install required build tools or run ./install_compiler.sh
- Run ./install_hdf5.sh and ./install_sundials.sh to build all required third-party libraries
- Run ./copy_files.sh to gather source files
- Run ./make_dyssol to build Dyssol

# Installation
Run the provided installer and follow the instructions.

# Code organization
- BaseSolvers - interfaces for equation solvers
- CahceHandlers - dynamic data caching
- Documentation - manuals
- DyssolConsole - main project for command-line version of Dyssol
- DyssolInstallers - scripts and data needed to build installers for Windows
- DyssolLinux - scripts for compilation on Linux
- DyssolMainWindow - main project for GUI version of Dyssol
- EquationSolvers - built-in equation solvers based on SUNDIALS library
- ExternalLibraries - all third-party libraries and scripts to build them
- GUIDialogs - main GUI components
- GUIWidgets - auxiliary GUI components and tools
- HDF5Handler - wrapper for HDF5 library
- MaterialsDatabase - database of materials
- ModelsAPI - part of the core components needed to develop units
- Modules - additional modules
- PropertySheets - projects settings for Visual Studio
- SimulatorCore - core components
- Solvers - all solvers and templetes for them
- Units - all units and temp0lates for them
- Utilities - auxiliary programm components
- Dyssol.sln - main file of the Visual Studio solution
- LICENSE - license agreement
- Materials.dmdb - examplary database of meterials 
- README - this file

# Installation directory organization 
- Example flowsheets – flowsheet examples 
- Example units – source code of units (C++)
- Example solvers – source code of solvers (C++)
- Help – documentation files (pdf)
- Licenses – information about licenses 
- platforms - Qt libraries to support GUI
- Solvers – libraries of developed solvers
- Units – libraries of developed units
- VCProject – Models Creator tool: template project for Microsoft Visual Studio 
- Dyssol.exe – main executable of Dyssol
- DyssolC.exe – command-line utility
- ExampleConfigFile.txt – example configuration file for command-line utility
- LICENSE - license agreement
- Materials.dmdb – default materials database
- Qt5*.dll - Qt libraries to support GUI
- unins000.exe – Dyssol uninstaller

# Third-party tools and libraries
- [Qt 5.11.3](https://www.qt.io/) – The Qt Company - [LGPL v3](https://doc.qt.io/qt-5/lgpl.html)
- [HDF5 v1.10.2](https://www.hdfgroup.org/downloads/hdf5/) – HDF Group - [HDF Licence](https://support.hdfgroup.org/ftp/HDF5/current/src/unpacked/COPYING)
- [zlib v1.2.11](https://www.zlib.net/) – Jean-loup Gailly and Mark Adler - [zlib License](https://www.zlib.net/zlib_license.html)
- [SUNDIALS v3.1.0](https://computing.llnl.gov/projects/sundials/) – Lawrence Livermore National Security - [BSD-3-Clause License]( https://computation.llnl.gov/projects/sundials/license)
- [Inno Setup v5.6.1](https://jrsoftware.org/isinfo.php) – Jordan Russell - [Inno Setup License](http://www.jrsoftware.org/files/is/license.txt)
- [KISS FFT v131](https://github.com/mborgerding/kissfft) – Mark Borgerding - [BSD-3-Clause License](https://github.com/mborgerding/kissfft/blob/master/COPYING)
