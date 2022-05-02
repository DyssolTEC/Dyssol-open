.. _sec.get_started:

Get started
===========

Here you will get an overview of the first steps required to use Dyssol, including installation and running first simulations.

|

.. _sec.get_started.installation:

Installation
------------

.. _sec.get_started.installation.windows:

Windows
^^^^^^^

The latest Windows installer is available on `GitHub <https://github.com/FlowsheetSimulation/Dyssol-open/releases>`_. Download it, run and follow the instructions.

After installation, you can find the following folders and files in the installation directory, by default ``C:\Program Files\Dyssol\``:

* **Dyssol.exe**: Main executable file of Dyssol with a graphical user interface. See also: :ref:`sec.gui`.
* **DyssolC.exe**: Command line version of Dyssol. See also: :ref:`sec.cli`.
* **Materials.dmdb**: Default materials database. See also: :ref:`sec.mdb`.
* **Example flowsheets**: Preconfigured flowsheet examples as ``*.dflw`` files that can be run in GUI mode. See also: :ref:`sec.get_started.files`.
* **Example scripts**: Preconfigured flowsheet examples as ``*.txt`` scripts for command line mode. Assumes Dyssol is installed in ``C:\Program Files\Dyssol\``. See also: :ref:`sec.get_started.files`, :ref:`sec.cli`. 
* **Example units**: Source code in C++ and project files for Visual Studio of selected units. See also: :ref:`sec.models_development`, :ref:`sec.classes`.
* **Example solvers**: Source code in C++ and project files for Visual Studio of selected solvers. See also: :ref:`sec.models_development`, :ref:`sec.classes`.
* **Units**: Dynamic libraries with units' models.
* **Solvers**: Dynamic libraries with solvers' models.
* **Help**: Additional documentation files as ``*.pdf`` files.
* **VCProject**: Template Microsoft Visual Studio project for development of new models. See also: :ref:`sec.models_development`.
* **Licenses**: Licenses of used libraries and tools. 
* **LICENSE**: Dyssol license agreement.
* **unins000.exe**, **unins000.dat**: Dyssol uninstaller.
* **platforms**, **styles**, **\*.dll**, **config6**: Libraries required for graphical user interface.

|

.. _sec.get_started.installation.linux:

Linux
^^^^^^^

To use Dyssol on Linux, you need to build it from source, as described in :ref:`sec.for_developers.compilation.linux`.

|

.. _sec.get_started.files:

Files
-----

The following files are used and created by Dyssol:

* **\*.dflw**: Dyssol flowsheets.
* **\*.dmdb**: Dyssol materials databases.
* **\*.dll/\*.so**: Shared libraries with Dyssol models.
* **\*.txt**: Script files for command line mode.

| 

.. _sec.get_started.first_simulation:

Run your first simulation
-------------------------

In Dyssol, you can simulate your process either by the normal graphical user interface or by a pre-difined configuration file. 

Here you can find a detailed guide for creating and running the :ref:`label-screen` process.

A flowsheet of this example is shown below with all stream names.

	.. image:: ./pics/screenExample/flowsheet.png
	   :width: 500px
	   :alt: 
	   :align: center

|

You need to follow these 12 steps to complete the simulation and analyze the result.

1. Add **units** to the flowsheet and name them:

	.. image:: ./pics/screenExample/1.png
	   :width: 960px
	   :alt: 
	   :align: center


2. Add **streams** to the flowsheet and name them:

	.. image:: ./pics/screenExample/2.png
	   :width: 960px
	   :alt: 
	   :align: center


3. Select a **model** for each unit on the flowsheet:
	
	.. image:: ./pics/screenExample/3-1.png
	   :width: 960px
	   :alt: 
	   :align: center
	   
	.. image:: ./pics/screenExample/3-2.png
	   :width: 960px
	   :alt: 
	   :align: center

	.. image:: ./pics/screenExample/3-3.png
	   :width: 960px
	   :alt: 
	   :align: center

4. Connect **ports** of each unit to the streams:
	
	.. image:: ./pics/screenExample/4.png
	   :width: 960px
	   :alt: 
	   :align: center
	

5. Setup **parameters** of units:

	.. image:: ./pics/screenExample/5.png
	   :width: 960px
	   :alt: 
	   :align: center
	   

6. Add **compounds** to the flowsheet:

	.. image:: ./pics/screenExample/6.png
	   :width: 960px
	   :alt: 
	   :align: center
	   

7. Add **phases** to the flowsheet:

	.. image:: ./pics/screenExample/7.png
	   :width: 960px
	   :alt: 
	   :align: center


8. Specify **grids** for distributed parameters of solids:

	.. image:: ./pics/screenExample/8.png
	   :width: 960px
	   :alt: 
	   :align: center


9. Setup **feeds** of inlets and **holdups** of units:

	.. image:: ./pics/screenExample/9-1.png
	   :width: 960px
	   :alt: 
	   :align: center
	   
	.. image:: ./pics/screenExample/9-2.png
	   :width: 960px
	   :alt: 
	   :align: center

	.. image:: ./pics/screenExample/9-3.png
	   :width: 960px
	   :alt: 
	   :align: center
	   
	.. image:: ./pics/screenExample/9-4.png
	   :width: 960px
	   :alt: 
	   :align: center   


10. Specify the simulation **time**:

	.. image:: ./pics/screenExample/10.png
	   :width: 960px
	   :alt: 
	   :align: center


11. **Run** the simulation:

	.. image:: ./pics/screenExample/11.png
	   :width: 960px
	   :alt: 
	   :align: center
	   

12. **Analyze** the results:

	.. image:: ./pics/screenExample/12-1.png
	   :width: 960px
	   :alt: 
	   :align: center
	   
	.. image:: ./pics/screenExample/12-2.png
	   :width: 960px
	   :alt: 
	   :align: center   

|
