.. _sec.development.compilation:

Compilation
===========

.. _sec.development.compilation.windows:

Windows
-------

.. _sec.development.compilation.windows.requirements:

Compilation requirements
^^^^^^^^^^^^^^^^^^^^^^^^

- `Microsoft Visual Studio 2022 <https://visualstudio.microsoft.com/downloads>`_, Desktop development with C++ workload.
- `Qt 5.15.2 <https://www.qt.io/download-qt-installer>`_ MSVC 2019 64-bit component
- `Qt Visual Studio Tools <https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2022>`_ for Visual Studio 2022
- `CMake <https://cmake.org/download>`_ 3.5.0 or higher
- `Python <https://www.python.org/downloads>`_ 3.8 or higher
- `Git <https://git-scm.com/download/win>`_
- PowerShell 5.0 (usually shipped with Windows)

.. _sec.development.compilation.windows.build:

Build
^^^^^

- Make sure all programs and tools from the :ref:`list<sec.development.compilation.windows.requirements>` are installed.
- Setup Qt Visual Studio Tools extension to point to the installed Qt libraries. In Visual Studio, go to Extensions → Qt VS Tools → Qt Versions → add new Qt version → ... → Navigate in the Qt installation directory to ``Qt/5.15.2/msvc2019_64/bin/qmake.exe`` → rename Version to ``msvc2019_64`` → OK.
- Compile and build external libraries: HDF5, SUNDIALS, zlib, etc. To do this, navigate to ``Dyssol/ExternalLibraries/`` and execute file ``CompileAll.bat``. It will start building all the required libraries by executing files ``CompileZLib.ps1``, ``CompileHDF5.ps1``, ``CompileSundials.ps1``, etc. To use the scripts, the following tools are necessary: Visual Studio, CMake, PowerShell, Python.
- Open ``Dyssol/Dyssol.sln`` with Visual Studio and build the solution.

.. _sec.development.compilation.windows.docs:

Build documentation and installers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Install `Doxygen <https://www.doxygen.nl/download.html>`_.
- Install `MikTeX <https://miktex.org/download>`_ with packages: ``amscls``, ``anyfontsize``, ``preview``, ``zhmetrics``. With default installation, MikTeX will install the packages on-the-fly during the first build of the documentation (user confirmation may be required).
- Install required python libraries by running in terminal:

..  code-block:: powershell

    py -m ensurepip --upgrade
    pip install -U sphinx sphinx-rtd-theme breathe

- In Visual Studio Solution Explorer go to ``Installers``, right click on the ``Installer`` project and choose Build. The built installer locates in ``Dyssol/DyssolInstallers/Installers``. 

Also, other versions of Microsoft Visual Studio can be used, but additional preparations may be required:

- Install build tools for the corresponding Visual Studio.
- Configure all ``*.ps1`` scripts to use the required version of Visual Studio build tools before running ``Dyssol/ExternalLibraries/CompileAll.bat``.
- If the Models Creator SDK tool is required, files associated with it may need to be updated. They to be found in ``Dyssol/DyssolInstallers/Data/ModelsCreatorSDK/``.

.. _sec.development.compilation.linux:

Linux
-----

.. _sec.development.compilation.linux.requirements:

Compilation requirements
^^^^^^^^^^^^^^^^^^^^^^^^

- g++-9 or clang++-10
- CMake 3.5.0 or higher

.. _sec.development.compilation.linux.build:

Build
^^^^^

Tested on Ubuntu 20/22/24, Debian 11/12/13

..  code-block:: shell

	sudo apt install build-essential cmake libsundials-dev libhdf5-serial-dev libqt5opengl5-dev libgraphviz-dev libopenmpi-dev doxygen python3-sphinx python3-sphinx-rtd-theme python3-breathe texlive-latex-extra
	cd /path_to_repo
	mkdir install
	mkdir build
	cd build
	cmake .. -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../install
	cmake --build . --parallel
	make doc
	make install

The compiled executable file and all the units' libraries will appear in ``/path_to_repo/install``
