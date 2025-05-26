# Dyssol

[![License: BSD-3-Clause](https://img.shields.io/badge/License-BSD--3--Clause-blue.svg)](LICENSE)
[![Website](https://img.shields.io/badge/Website-dyssoltec.com-brightgreen)](https://www.dyssoltec.com/)
[![Documentation](https://img.shields.io/badge/Docs-dyssoltec.github.io-blue)](https://dyssoltec.github.io/Dyssol-open/)
[![Releases](https://img.shields.io/github/v/release/DyssolTEC/Dyssol-open?include_prereleases)](https://github.com/DyssolTEC/Dyssol-open/releases)

**Dyssol** is an open-source simulation framework for dynamic flowsheet modeling of complex particulate processes. It provides advanced capabilities for simulating granular materials with multidimensional distributed properties and interconnected unit operations.

**Note**: *Dyssol®* is a registered trademark of DyssolTEC GmbH.

---

## 🔗 Project Links

- 🌐 Official website: [dyssoltec.com](https://www.dyssoltec.com/)
- 📘 Documentation: [dyssoltec.github.io](https://dyssoltec.github.io/Dyssol-open/)
- 🎥 Video tutorials: [Basics](https://youtu.be/IHzr0NVYW6M) | [Granulation simulation](https://youtu.be/ni54JwvCVDc)
- 🚀 Latest release: [GitHub Releases](https://github.com/DyssolTEC/Dyssol-open/releases)

---

## 📥 Installation

### Windows

- Download and run the installer from the [Releases page](https://github.com/DyssolTEC/Dyssol-open/releases).
- Requires: Visual C++ Redistributable for Visual Studio 2022.

### Ubuntu / Debian

```bash
sudo apt install dyssol
```

---

## 📦 Compilation

- **Windows**: See [build instructions](https://dyssoltec.github.io/Dyssol-open/004_development/compilation.html#windows)
- **Linux**: See [build instructions](https://dyssoltec.github.io/Dyssol-open/004_development/compilation.html#linux)

---

## 📚 Code Organization

| Folder / File     | Description                                           |
|:------------------|:------------------------------------------------------|
| BaseSolvers       | Interfaces for equation solvers                       |
| build             | Build directory (internal use only)                   |
| CahceHandlers     | Dynamic data caching mechanisms                       |
| cmake             | CMake configuration scripts                           |
| Documentation     | Manuals and docs                                      |
| DyssolCLI         | Command-line interface main project                   |
| DyssolGUI         | GUI-based simulator main project                      |
| DyssolInstallers  | Scripts and data for creating Windows installers      |
| EquationSolvers   | Built-in solvers using the SUNDIALS library           |
| ExternalLibraries | Third-party dependencies and build scripts            |
| GUIDialogs        | Main GUI dialog components                            |
| GUIWidgets        | Auxiliary widgets and GUI tools                       |
| HDF5Handler       | Wrapper for handling HDF5 file formats                |
| MaterialsDatabase | APIs to the database of materials                     |
| ModelsAPI         | APIs to create custom unit models                     |
| Modules           | Additional processing modules                         |
| PropertySheets    | Visual Studio project settings                        |
| ScriptInterface   | Library support for scripting the CLI version         |
| scripts           | Collection of helpful utility scripts                 |
| SimulatorCore     | Core simulation engine                                |
| Solvers           | All solvers and templates for them                    |
| test              | System and integration test cases                     |
| Units             | All units and templates for them                      |
| Utilities         | Supporting program utilities and helpers              |
| Materials.dmdb    | Example material database file                        |
| CHANGELOG         | List of changes since the last version                |
| CITATION          | Citation information for Dyssol                       |
| LICENSE           | Software license                                      |
| README.md         | This file                                             |
| Dyssol.sln        | Main Visual Studio solution file                      |

---

## 📂 Installation Directory

| Folder / File      | Description                                          |
|:-------------------|:-----------------------------------------------------|
| Example Flowsheets | Example GUI-based flowsheets                         |
| Example Scripts    | Example simulation scripts for CLI                   |
| Example solvers    | C++ source code examples for custom solvers          |
| Example units      | C++ source code examples for custom unit models      |
| Help               | User guides and documentation                        |
| Licenses           | License details of included third-party libraries    |
| ModelsCreatorSDK   | Visual Studio template project for model development |
| Solvers            | Precompiled libraries of available solvers           |
| Units              | Precompiled libraries of available unit models       |
| Dyssol.exe         | GUI version of the simulator                         |
| DyssolC.exe        | Command-line version of the simulator                |
| LICENSE            | License agreement file                               |
| Materials.dmdb     | Default materials database file                      |
| unins000.exe       | Uninstaller for Dyssol                               |

---

## 📖 Citations

If you use Dyssol in your work, please cite:

1. **Skorych, V., Dosta, M., Hartge, E.-U., Heinrich, S.** (2017). Novel system for dynamic flowsheet simulation of solids processes. *Powder Technology*, 314, 665–679. [doi:10.1016/j.powtec.2017.01.061](https://doi.org/10.1016/j.powtec.2017.01.061)
2. **Skorych, V., Dosta, M., Heinrich, S.** (2020). Dyssol — An open-source flowsheet simulation framework for particulate materials. *SoftwareX*, 12, 100572. [doi:10.1016/j.softx.2020.100572](https://doi.org/10.1016/j.softx.2020.100572)

---

## 🧩 Third-Party Tools and Libraries

Dyssol uses and integrates several open-source components, including:

- [Breathe](https://www.breathe-doc.org) – Doxygen-to-Sphinx bridge.
- [Doxygen](https://www.doxygen.nl) – Code documentation generator.
- [Graphviz](https://graphviz.org) – Graph rendering.
- [HDF5](https://www.hdfgroup.org/solutions/hdf5) – Data storage.
- [Inno Setup](https://jrsoftware.org/isinfo.php) – Windows installer creator.
- [KISS FFT](https://github.com/mborgerding/kissfft) – FFT computations.
- [Qt](https://www.qt.io) – GUI framework.
- [Sphinx](https://www.sphinx-doc.org) – Documentation generator.
- [sphinx_rtd_theme](https://sphinx-rtd-theme.readthedocs.io) – Sphinx theme.
- [SUNDIALS](https://computing.llnl.gov/projects/sundials) – Equation solvers.
- [zlib](https://www.zlib.net) – Data compression.

---

## 🤝 Contributing

We welcome contributions!

---

## 📫 Contact

- 🌐 Website: [www.dyssoltec.com](https://www.dyssoltec.com)
- 🐞 Issues: [GitHub Issue Tracker](https://github.com/DyssolTEC/Dyssol-open/issues)

---

© DyssolTEC GmbH
