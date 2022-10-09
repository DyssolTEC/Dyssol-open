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

* **\*.dflw**: Dyssol flowsheets

	* Flowsheet structure 
	* Flowsheet settings
	* Simulation results

* **\*.dmdb**: Dyssol materials databases. See also: :ref:`sec.mdb`.

	* Compounds 
	* Properties of compounds

* **\*.dll/\*.so**: Shared libraries with Dyssol models
* **\*.txt**: Script files for command line mode. See also: :ref:`sec.cli`.

| 

.. _sec.get_started.first_simulation:

Run your first simulation
-------------------------

Here you can find a detailed guide for creating and running the :ref:`label-screen` process.

A flowsheet of this example is shown below with all stream names.

.. image:: ../static/images/000_get_started/flowsheet.png
   :width: 500px
   :alt: flowsheet.png
   :align: center

|

Follow these steps to complete the simulation and analyze the result.

1. Add **units** to the flowsheet and give them names:

	1.1. Add 4 units.

	1.2. Rename them by double-clicking or pressing F2. Set names to:

        - Feed
        - Screen
        - Out coarse
        - Out fine
  
	.. image:: ../static/images/000_get_started/step01_1.png
		:width: 960px
		:alt: step01_1.png
		:align: center

2. Add **streams** to the flowsheet and give them names:

	2.1. Add 3 **streams** to the flowsheet 

	2.2. Rename them by double-clicking or pressing F2. Set names to:

        - In
        - Coarse
        - Fine

	.. image:: ../static/images/000_get_started/step02_1.png
	   :width: 960px
	   :alt: step02_1.png
	   :align: center

3. Select a **model** for each unit on the flowsheet:

	3.1. Select a unit

	3.2. Select a model from the list:

        - Feed : InletFlow
        - Screen : Screen Plitt
        - Out coarse : OutletFlow
        - Out fine : OutletFlow

	.. image:: ../static/images/000_get_started/step03_1.png
	   :width: 960px
	   :alt: step03_1.png
	   :align: center

4. Connect **ports** of each unit to the streams:
	
	4.1. Select a unit

	4.2. Connect a stream to each port:

        - Feed : InletMaterial - In
        - Screen : Input - In, Coarse - Coarse, Fines - Fine
        - Out coarse : In - Coarse
        - Out fine : In - Fine

	.. image:: ../static/images/000_get_started/step04_1.png
	   :width: 960px
	   :alt: step04_1.png
	   :align: center

5. Setup **parameters** of units:

	5.1. Select the Screen unit

	5.2. Set Xcut parameter to 0.002 m

	5.3. Select Alpha parameter 

	5.4. Add 2 time points

	5.5. Set time values to 0, 60, 180 s

	5.6. Set Alpha values to 10, 9, 8

	.. image:: ../static/images/000_get_started/step05_1.png
	   :width: 960px
	   :alt: step05_1.png
	   :align: center
	   

6. Add **compounds** to the flowsheet:

	6.1. Open Compounds editor

	6.2. Select Sand

	6.3. Add Sand to the flowsheet
	
	6.4. Apply and close Compounds editor

	.. image:: ../static/images/000_get_started/step06_1.png
	   :width: 960px
	   :alt: step06_1.png
	   :align: center

7. Add **phases** to the flowsheet:

	7.1. Open Phases editor

	7.2. Add a new phase

	7.3. Select Solid phase

	7.4. Rename the phase to 'Solid phase'
	
	7.5. Apply and close Phases editor

	.. image:: ../static/images/000_get_started/step07_1.png
	   :width: 960px
	   :alt: step07_1.png
	   :align: center

8. Specify **grids** for distributed parameters of solids:

	8.1. Open Grid editor

	8.2. Add a new grid

	8.3. Select Size distribution

	8.4. Set 100 classes

	8.5. Select Equidistant grid type

	8.6. Set grid limits: min - 0 mm, max - 4 mm
	
	8.7. Apply and close Grid editor

	.. image:: ../static/images/000_get_started/step08_1.png
	   :width: 960px
	   :alt: step08_1.png
	   :align: center

9. Setup **feeds** of inlets and **holdups** of units:

	9.1. Open Holdups editor

	9.2. Select Feed units

	9.3. Select InputMaterial holdup

	9.4. Add a new time point

	9.5. Set Mass flow to 10 kg/s

	9.6. Select Phase fractions tab

	9.7. Set Solid phase fraction to 1

	9.8. Select Solid phase tab

	9.9. Set mass fraction of sand to 1

	9.10. Select compound Sand

	9.11. Select Size as a distribution in rows

	9.12. Open Distributions editor

	9.13. Setup Normal distribution with D50 = 0.002 m and Standard deviation = 0.0002 m and press Ok to apply  
	
	9.14. Apply and close Holdups editor

	.. image:: ../static/images/000_get_started/step09_1.png
	   :width: 960px
	   :alt: step09_1.png
	   :align: center
	   
	.. image:: ../static/images/000_get_started/step09_2.png
	   :width: 960px
	   :alt: step09_2.png
	   :align: center

	.. image:: ../static/images/000_get_started/step09_3.png
	   :width: 960px
	   :alt: step09_3.png
	   :align: center

10. Specify simulation **time**:

	10.1. Open Simulator tab 

	10.2. Set End simulation time to 240 s

	.. image:: ../static/images/000_get_started/step10_1.png
	   :width: 960px
	   :alt: step10_1.png
	   :align: center

11. **Run** the simulation:

	11.1. Run the simulation by pressing button Start simulation

	11.2. Wait until the simulation is finished

	.. image:: ../static/images/000_get_started/step11_1.png
	   :width: 960px
	   :alt: step11_1.png
	   :align: center

12. **Analyze** the results:

	12.1. Switch to Streams tab

	12.2. Select all 3 streams 

	12.3. Select the Mass parameter

	12.4. Check the results

	12.5. Select the Solid distribution parameter

	12.6. Switch to Plot view

	12.7. Move the time slider to the right position to show last state

	12.8. Check the results

	.. image:: ../static/images/000_get_started/step12_1.png
	   :width: 960px
	   :alt: step12_1.png
	   :align: center
	   
	.. image:: ../static/images/000_get_started/step12_2.png
	   :width: 960px
	   :alt: step12_2.png
	   :align: center 

|
