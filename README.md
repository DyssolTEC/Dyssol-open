# Dyssol
Dyssol is a tool for dynamic flowsheet simulation of complex production processes with advanced treatment of granular materials.

For more information, please check the [documentation](https://dyssoltec.github.io/Dyssol-open/). 

Video intruductions: [Basics](https://youtu.be/IHzr0NVYW6M) and [Simulation of granulation process](https://youtu.be/ni54JwvCVDc)

For new versions and updates, please visit our [github page](https://github.com/DyssolTEC/Dyssol-open/releases).

To cite Dyssol please use the following: 
Skorych, V., Dosta, M., & Heinrich, S. (2020). Dyssol — An open-source flowsheet simulation framework for particulate materials. SoftwareX, 12, 100572. [doi.org/10.1016/j.softx.2020.100572](https://doi.org/10.1016/j.softx.2020.100572).

# Installation requirements 
## Windows
Dyssol should install and work on all latest versions of Windows.
Requires Visual C++ Redistributable for Visual Studio 2019 to run.

# Installation
## Windows
Run the provided installer and follow the instructions.

# Compilation
## Windows
### Compilation requirements on Windows
- [Microsoft Visual Studio 2019](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=Community&rel=16), Desktop development with C++ workload.
- [Qt 5.15.2](https://www.qt.io/download-qt-installer) msvc2019 / msvc2019_64
- [Qt Visual Studio Tools](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2019) for Visual Studio 2019
- [CMake](https://cmake.org/download/) 3.14 or higher
- [Python](https://www.python.org/downloads/) 3.8 or higher
- [Git](https://git-scm.com/downloads)
- PowerShell 5.0 (usually shipped with Windows)

#### Optional requirements
- [Doxygen](https://www.doxygen.nl/download.html)
- [Graphviz](https://graphviz.org/download/), with "add to PATH" option enabled

### Build on Windows
- Make sure all programs and tools from the [list](#Compilation-requirements-on-windows) are installed.
- Setup Qt Visual Studio Tools extension to point to the installed Qt libraries. In Visual Studio 2019, go to Extensions → Qt VS Tools → Qt Versions → add new Qt version → ... → Navigate in the Qt installation directory to `Qt/5.15.0/msvc2019/bin/qmake.exe` → rename Version to `msvc2019` → OK. Repeat for `Qt/5.15.0/msvc2019_64/bin/qmake.exe` and rename Version to `msvc2019_64`.
- Compile and build external libraries: zlib, HDF5, SUNDIALS, graphviz. To do this, navigate to `Dyssol/ExternalLibraries/` and execute file `RunAll.bat`. It will start building all the required libraries by executing files `CompileZLib.ps1`, `CompileHDF5.ps1`, `CompileSundials.ps1`, `CompileGraphviz.ps1`. To use the scripts, the following requirements apply: Visual Studio 16 2019, CMake, PowerShell 5.0.
- Open `Dyssol/Dyssol.sln` with Visual Studio and build the solution.

#### Build documentation and installers on Windows
- Make sure all [required](#Compilation-requirements-on-windows) and [optional](#Optional-requirements) programs and tools are installed.
- Install required python libraries by running in a terminal
```powershell
py -m ensurepip --upgrade
pip install -U sphinx sphinx-rtd-theme breathe
```
- In Visual Studio Solution Explorer go to `Installers`, right click on the `Installer` project and choose Build. The built `.exe` installer locates in `Dyssol/DyssolInstallers/Installers`. 

Also, other versions of Microsoft Visual Studio can be used, but additional preparations are required:
- Install build tools for the corresponding Visual Studio.
- Configure all *.ps1 scripts to use the required version of Visual Studio build tools before running `Dyssol/ExternalLibraries/RunAll.bat`.
- If the Models Creator SDK tool is required, files associated with it may need to be updated. They to be found in `Dyssol/DyssolInstallers/Data/ModelsCreatorSDK/`.

## Linux
### Compilation requirements on Linux
- gcc-9, g++-9
- CMake 3.1.0 or higher

### Compilation procedure on Linux
- Install the required build tools 
```sh
cd /path_to_repo
sudo add-apt-repository ppa:gladky-anton/sundials
sudo apt install build-essential cmake libsundials-dev libhdf5-serial-dev libqt5opengl5-dev libgraphviz-dev doxygen
py -m ensurepip --upgrade
pip install -U sphinx sphinx-rtd-theme breathe
mkdir install
mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../install
cmake --build . --parallel
make doc
make install
```
- The compiled executable file and all the units' libraries will appear in `/path_to_repo/install`

# Code organization
- BaseSolvers - interfaces for equation solvers
- CahceHandlers - dynamic data caching
- Documentation - manuals
- DyssolConsole - main project for command-line version of Dyssol
- DyssolInstallers - scripts and data needed to build installers for Windows
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
- Solvers - all solvers and templates for them
- Units - all units and temp0lates for them
- Utilities - auxiliary program components
- Dyssol.sln - main file of the Visual Studio solution
- LICENSE - license agreement
- Materials.dmdb - exemplary database of materials 
- README.md - this file

# Installation directory organization 
- Example flowsheets – flowsheet examples 
- Example solvers – source code of solvers (C++)
- Example units – source code of units (C++)
- Help – documentation files (pdf)
- Licenses – information about licenses 
- platforms - Qt libraries to support GUI
- Solvers – libraries of developed solvers
- styles - Qt libraries to support GUI
- Units – libraries of developed units
- ModelsCreatorSDK – Models Creator tool: template project for Microsoft Visual Studio 
- Dyssol.exe – main executable of Dyssol
- DyssolC.exe – command-line utility
- ExampleConfigFile.txt – example configuration file for command-line utility
- LICENSE - license agreement
- Materials.dmdb – default materials database
- Qt5*.dll - Qt libraries to support GUI
- *.dll - Other external libraries
- unins000.exe – Dyssol uninstaller

# Third-party tools and libraries
- [Qt](https://www.qt.io/) – The Qt Company - [LGPL v3](https://doc.qt.io/qt-5/lgpl.html)
- [HDF5](https://www.hdfgroup.org/downloads/hdf5/) – HDF Group - [HDF license](https://support.hdfgroup.org/ftp/HDF5/releases/COPYING)
- [zlib](https://www.zlib.net/) – Jean-loup Gailly and Mark Adler - [zlib license](https://www.zlib.net/zlib_license.html)
- [SUNDIALS](https://computing.llnl.gov/projects/sundials/) – Lawrence Livermore National Security and Southern Methodist University - [BSD-3-Clause license](https://computing.llnl.gov/projects/sundials/license)
- [Inno Setup](https://jrsoftware.org/isinfo.php) – Jordan Russell - [Inno Setup license](http://www.jrsoftware.org/files/is/license.txt)
- [KISS FFT](https://github.com/mborgerding/kissfft) – Mark Borgerding - [BSD-3-Clause license](https://github.com/mborgerding/kissfft/blob/master/COPYING)
- [Graphviz](https://graphviz.org/) – Graphviz contributors - [CPL v1.0 license](https://graphviz.org/license/)
