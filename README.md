# Dyssol

Dyssol is a tool for dynamic flowsheet simulation of complex production processes with advanced treatment of granular materials.

To contact developers, visit [dyssoltec.com](https://www.dyssoltec.com/).

For more information and documentation, check [dyssoltec.github.io](https://dyssoltec.github.io/Dyssol-open/).

Video introductions: [Basics](https://youtu.be/IHzr0NVYW6M) and [Simulation of granulation process](https://youtu.be/ni54JwvCVDc).

New versions and updates on our [github page](https://github.com/DyssolTEC/Dyssol-open/releases).

To cite Dyssol please use the following:

- Skorych, V., Dosta, M., Hartge, E.-U., Heinrich, S. (2017). Novel system for dynamic flowsheet simulation of solids processes. Powder Technology, 314, 665-679. [doi.org/10.1016/j.powtec.2017.01.061](https://doi.org/10.1016/j.powtec.2017.01.061).
- Skorych, V., Dosta, M., Heinrich, S. (2020). Dyssol — An open-source flowsheet simulation framework for particulate materials. SoftwareX, 12, 100572. [doi.org/10.1016/j.softx.2020.100572](https://doi.org/10.1016/j.softx.2020.100572).

Dyssol® is a registered trade mark of DyssolTEC GmbH.

## Installation on Windows

Dyssol should install and work on all latest versions of Windows.
Requires Visual C++ Redistributable for Visual Studio 2022 to run.
To install, run the [installer](https://github.com/DyssolTEC/Dyssol-open/releases) and follow the instructions.

## Compilation on Windows

See the [documentation](https://dyssoltec.github.io/Dyssol-open/004_development/compilation.html#windows) for instructions.

## Compilation on Linux

See the [documentation](https://dyssoltec.github.io/Dyssol-open/004_development/compilation.html#linux) for instructions.

## Code organization

- BaseSolvers - interfaces for equation solvers
- CahceHandlers - dynamic data caching
- Documentation - manuals
- DyssolCLI - main project for command-line version of Dyssol
- DyssolGUI - main project for GUI version of Dyssol
- DyssolInstallers - scripts and data needed to build installers for Windows
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
- Units - all units and templates for them
- Utilities - auxiliary program components
- Dyssol.sln - main file of the Visual Studio solution
- LICENSE - license agreement
- Materials.dmdb - exemplary database of materials
- README.md - this file

## Installation directory organization

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
- Qt6*.dll - Qt libraries to support GUI
- *.dll - Other external libraries
- unins000.exe – Dyssol uninstaller

## Third-party tools and libraries

- [Breathe](https://www.breathe-doc.org) – Michael Jones – [Modified BSD license](https://github.com/breathe-doc/breathe/blob/master/LICENSE)
- [Doxygen](https://www.doxygen.nl) – Dimitri van Heesch – [GPL](https://github.com/doxygen/doxygen/blob/master/LICENSE)
- [Graphviz](https://graphviz.org) – Graphviz contributors – [CPL v1.0 license](https://graphviz.org/license)
- [HDF5](https://www.hdfgroup.org/solutions/hdf5) – The HDF Group – [BSD-like license](https://github.com/HDFGroup/hdf5/blob/master/LICENSE)
- [Inno Setup](https://jrsoftware.org/isinfo.php) – Jordan Russell – [Inno Setup license](http://www.jrsoftware.org/files/is/license.txt)
- [KISS FFT](https://github.com/mborgerding/kissfft) – Mark Borgerding – [BSD-3-Clause license](https://github.com/mborgerding/kissfft/blob/master/COPYING)
- [Qt](https://www.qt.io) – The Qt Company – [LGPL v3](https://doc.qt.io/qt-5/lgpl.html)
- [Sphinx](https://www.sphinx-doc.org) – Sphinx team – [BSD-2-Clause license](https://github.com/sphinx-doc/sphinx/blob/master/LICENSE.rst)
- [sphinx_rtd_theme](https://sphinx-rtd-theme.readthedocs.io) – Dave Snider, Read the Docs, Inc. & contributors – [MIT](https://github.com/readthedocs/sphinx_rtd_theme/blob/master/LICENSE)
- [SUNDIALS](https://computing.llnl.gov/projects/sundials) – Lawrence Livermore National Laboratory – [BSD-3-Clause license](https://computing.llnl.gov/projects/sundials/license)
- [zlib](https://www.zlib.net) – Jean-loup Gailly and Mark Adler – [zlib license](https://www.zlib.net/zlib_license.html)
