Get started
===========

Here you will get an overview of first steps you need to use Dyssol, including installation, introduction to the graphical user interface (GUI).

|

Installation
------------

After installation, you can find the following folders and files in the installation directory ``\Program Files\Dyssol``.

* Folders:

	**Example flowsheets**: flowsheet examples
	
	**Example units**: source code of units written in C++
	
	**Example solvers**: source code of solvers written in C++
	
	**Help**: documentation files in form of PDF
	
	**Licenses**: information about licenses 
	
	**Solvers**: libraries of developed solvers
	
	**Units**: libraries of developed units
	
	**VCProject**: template project for Microsoft Visual Studio 


* Files:

	``Dyssol.exe``: executable file of Dyssol program
	
	``DyssolC.exe``: command line utility of Dyssol program
	
	``ExampleConfigFile.txt``: example configuration file for command line utility
	
	``LICENSE``: license agreement
	
	``Materials.dmdb``: default materials database
	
	``unins000.dat``: shortcut for Dyssol uninstaller

|


Introduction to GUI
-------------------

After installation and launching Dyssol, you will see the GUI, which is introduced below by corresponding screenshots.


Menu
^^^^

You can find File, Flowsheet, Modules, Tools and Help in the menu bar.

File
""""

	.. image:: ./pics/gui/fileMenu.png
	   :width: 960px
	   :alt: 
	   :align: center

|

- Command line interface:

	Using this option, you can save your current simulation to a configuration file in ``*.txt`` form. In the configuration file, you may have the following activities. However, it is impossible to change the flowsheet structure in the configuration file.

	- Run simulations in command line mode
	
	- Batch runs of multiple simulations
	
	- Change simulation parameters: simulation time, unit parameters, grid parameters, holdups, distributed parameters of solids, simulation options
	
	- Possibility to save flowsheet as the configuration file for the command line mode
	
	Please refer to :ref:`run Dyssol in command line mode <label-config-file>` for more information.

	.. image:: ./pics/gui/command.png
	   :width: 960px
	   :alt: 
	   :align: center

|
	   
Flowsheet
"""""""""

	.. image:: ./pics/gui/flowsheets.png
	   :width: 960px
	   :alt: 
	   :align: center

|

- Compounds editor:

	.. image:: ./pics/gui/compounds.png
	   :width: 960px
	   :alt: 
	   :align: center

|
	
- Phases editor:

	.. image:: ./pics/gui/phases.png
	   :width: 960px
	   :alt: 
	   :align: center

|

- Grids specification:

	.. image:: ./pics/gui/grid1.png
	   :width: 960px
	   :alt: 
	   :align: center

|
	   
	.. image:: ./pics/gui/grid2.png
	   :width: 960px
	   :alt: 
	   :align: center

|

- Holdups editor:
	
	.. image:: ./pics/gui/holdup1.png
	   :width: 960px
	   :alt: 
	   :align: center

	|
	
	- Main control:
	
	.. image:: ./pics/gui/holdup2.png
	   :width: 960px
	   :alt: 
	   :align: center
	
	|
	
	- Concentrated parameters:
	
	.. image:: ./pics/gui/holdup3.png
	   :width: 960px
	   :alt: 
	   :align: center

	|
	
	- Distributed parameters:
	
	.. image:: ./pics/gui/holdup4.png
	   :width: 960px
	   :alt: 
	   :align: center

|

- Options: 
	
	.. image:: ./pics/gui/options1.png
	   :width: 960px
	   :alt: 
	   :align: center
	   
	|
	
	- Convergence: 
	
		For more information, please refer to :ref:`label-convergence` or the help document in directory ``Dyssol\Help\Convergence.pdf``.
	
	.. image:: ./pics/gui/options2.png
	   :width: 960px
	   :alt: 
	   :align: center

	|
	
	- Service:
	
	.. image:: ./pics/gui/options3.png
	   :width: 480px
	   :alt: 
	   :align: center

|

- Calculation sequence:
	
	.. image:: ./pics/gui/sequence.png
	   :width: 960px
	   :alt: 
	   :align: center

|

- Recycle streams:	
	
	.. image:: ./pics/gui/recycle.png
	   :width: 960px
	   :alt: 
	   :align: center

|

Modules
"""""""
	.. image:: ./pics/gui/modules.png
	   :width: 960px
	   :alt: 
	   :align: center

|

Tools
"""""

	.. image:: ./pics/gui/tools.png
	   :width: 960px
	   :alt: 
	   :align: center

|

- Models manager:

	.. image:: ./pics/gui/modelsManag.png
	   :width: 960px
	   :alt: 
	   :align: center

|

- Materials Database:

	.. image:: ./pics/gui/database.png
	   :width: 960px
	   :alt: 
	   :align: center

	|

	- Tab Compounds - Compounds List:
	
	.. image:: ./pics/gui/dat1.png
	   :width: 960px
	   :alt: 
	   :align: center

	|
	
	- Tab Compounds - Properties:
	
	.. image:: ./pics/gui/dat2.png
	   :width: 960px
	   :alt: 
	   :align: center

	|
	
	- Tab Compounds - Correlations:
	
	.. image:: ./pics/gui/dat3.png
	   :width: 960px
	   :alt: 
	   :align: center

	|
	
	- Tab Compounds - Correlations plot:
	
	.. image:: ./pics/gui/dat4.png
	   :width: 960px
	   :alt: 
	   :align: center

	|
	
	- Tab Interactions:

	.. image:: ./pics/gui/dat5.png
	   :width: 960px
	   :alt: 
	   :align: center

	|


Help
""""
	
	.. image:: ./pics/gui/help.png
	   :width: 960px
	   :alt: 
	   :align: center

|

Tabs
^^^^

Tabs are needed for the parameter setup. They are named by Flowsheet, Simulator, Streams and Units.
	
	.. image:: ./pics/gui/tabs.png
	   :width: 960px
	   :alt: 
	   :align: center
	
|

Flowsheet
"""""""""

- Units:
	
	.. image:: ./pics/gui/units.png
	   :width: 960px
	   :alt: 
	   :align: center

- Streams:

	.. image:: ./pics/gui/streams.png
	   :width: 960px
	   :alt: 
	   :align: center

- Model selection:

	.. image:: ./pics/gui/modelSelect.png
	   :width: 960px
	   :alt: 
	   :align: center

- Ports connection:

	.. image:: ./pics/gui/ports.png
	   :width: 960px
	   :alt: 
	   :align: center

- Unit parameters:

	.. image:: ./pics/gui/unitParam.png
	   :width: 960px
	   :alt: 
	   :align: center

|

Simulator
"""""""""

Here you can start your simulation based on your time input.

	.. image:: ./pics/gui/simulator.png
	   :width: 960px
	   :alt: 
	   :align: center

|

Stream
""""""

Here you will find the simulation results, including the stream results summarized in table and plot.

- Table view:

	.. image:: ./pics/gui/table.png
	   :width: 960px
	   :alt: 
	   :align: center

- Plot view:

	.. image:: ./pics/gui/plot.png
	   :width: 960px
	   :alt: 
	   :align: center

|

Units
"""""

In Units tab, you can control the operation units.

	.. image:: ./pics/gui/unitsTab.png
	   :width: 960px
	   :alt: 
	   :align: center

|

Run your first simulation
-------------------------

In Dyssol, you can simulate your process either by the normal graphical user interface or by a pre-difined configuration file. 


Graphical user interface
^^^^^^^^^^^^^^^^^^^^^^^^

Here you can find a detailed guide for simulating the :ref:`label-screen` process.

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

.. _label-config-file:

Configuration file
^^^^^^^^^^^^^^^^^^

You may start your simulation by command lines with the help of ``DyssolC.exe``, which can be found in the installation directory. The command ``DyssolC`` can be started from the command prompt. 

As the single input argument, a configuration file (in the following case named ``script.txt``) must be loaded.

.. code-block:: RST

	> DyssolC.exe script.txt

The configuration file is a text file describing all necessary parameters for your simulation file. Details about the parameters are described in the tables below.

You can find an exemplary configuration file ``ExampleConfigFile.txt`` in the installation directory.

Only 3 parameters from the list are mandatory parameters: ``SOURCE_FILE``, ``MODELS_PATH``, ``MATERIALS_DATABASE``. The rest are optional and will override parameters set in initial file, specified as ``SOURCE_FILE``. If ``RESULT_FILE`` parameter is not specified, results of the simulation will be written to a ``SOURCE_FILE``.

Parameters ``MODELS_PATH``, ``DISTRIBUTION_GRID``, ``UNIT_PARAMETER``, ``UNIT_HOLDUP_*`` can be mentioned several times in the script file, the rest should be places only ones.

You can find the lists of all available parameters in the configuration file.


General
"""""""

	+-------------------------------------------------------------------------------+---------------------------------------+
	| Identifier                                                                    | Parameter                             |
	+===============================================================================+=======================================+
	| ``SOURCE_FILE``:                                                              | ``C:\Path\to\source\file.dflw``       |
	| Full path to the \*.dflw file with initial flowsheet                          |                                       |
	+-------------------------------------------------------------------------------+---------------------------------------+
	| ``RESULT_FILE``:                                                              | ``C:\Path\to\result\file.dflw``       |
	| Full path to the file where simulation results will be written                |                                       |
	+-------------------------------------------------------------------------------+---------------------------------------+
	| ``MODELS_PATH``:                                                              + ``C:\Path\to\dir\with\models\``       |
	| Path to the directory where \*.dll libraries of units and solvers can be found|                                       |
	+-------------------------------------------------------------------------------+---------------------------------------+
	| ``MATERIALS_DATABASE``:                                                       + ``C:\Path\to\database.dmdb``          |
	| Full path to the file with materials database                                 |                                       |
	+-------------------------------------------------------------------------------+---------------------------------------+

|
	
Simulation
""""""""""

	+----------------------------+---------------+
	| Identifier                 | Parameter     |
	+============================+===============+
	| ``SIMULATION_TIME``:       | ``<value>``   |
	| End simulation time in [s] |               |
	+----------------------------+---------------+

|

Options
"""""""

	+-----------------------------------------+----------------------------------------+
	| Identifier                              | Parameter                              |
	+=========================================+========================================+
	| ``RELATIVE_TOLERANCE``                  | ``<value>``                            |
	+-----------------------------------------+----------------------------------------+
	| ``ABSOLUTE_TOLERANCE``                  | ``<value>``                            |
	+-----------------------------------------+----------------------------------------+
	| ``MINIMAL_FRACTION``                    | ``<value>``                            |
	+-----------------------------------------+----------------------------------------+
	| ``INIT_TIME_WINDOW``                    | ``<value>``                            |
	+-----------------------------------------+----------------------------------------+
	| ``MIN_TIME_WINDOW``                     | ``<value>``                            |
	+-----------------------------------------+----------------------------------------+
	| ``MAX_TIME_WINDOW``                     | ``<value>``                            |
	+-----------------------------------------+----------------------------------------+
	| ``MAX_ITERATIONS_NUM``                  | ``<value>``                            |
	+-----------------------------------------+----------------------------------------+
	| ``WINDOW_CHANGE_RATE``                  | ``<value>``                            |
	+-----------------------------------------+----------------------------------------+
	| ``ITER_UPPER_LIMIT``                    | ``<value>``                            |
	+-----------------------------------------+----------------------------------------+
	| ``ITER_LOWER_LIMIT``                    | ``<value>``                            |
	+-----------------------------------------+----------------------------------------+
	| ``ITER_UPPER_LIMIT_1``                  | ``<value>``                            |
	+-----------------------------------------+----------------------------------------+
	| ``CONVERGENCE_METHOD``                  | ``<method>`` represented by numbers:   |
	+                                         |                                        |
	|                                         | ``<method>`` = 0 – Direct substitution |
	+                                         |                                        |
	|                                         | ``<method>`` = 1 – Wegstein            |
	+                                         |                                        |
	|                                         | ``<method>`` = 2 – Steffensen          |
	+-----------------------------------------+----------------------------------------+
	+ ``ACCEL_PARAMETER``:                    + ``<value>``                            +
	| Parameter of Wegstein's method          |                                        |
	+-----------------------------------------+----------------------------------------+
	| ``RELAX_PARAMETER``:                    | ``<value>``                            |
	| Parameter of Direct Substitution method |                                        |
	+-----------------------------------------+----------------------------------------+
	| ``EXTRAPOL_METHOD``:                    | ``<method>`` represented by numbers:   |
	+ Extrapolation method                    |                                        |
	|                                         | ``<method>`` = 0 – Linear              |
	+                                         |                                        |
	|                                         | ``<method>`` = 1 – Cubic spline        |
	+                                         |                                        |
	|                                         | ``<method>`` = 2 – Nearest neighbor    |
	+-----------------------------------------+----------------------------------------+

|

Grids
"""""

	+--------------------------------------------------------------+------------------------------------------------------------------------------------------------------+
	| Identifier                                                   | Parameter                                                                                            |
	+==============================================================+======================================================================================================+
	| ``DISTRIBUTION_GRID``:                                       | General form:                                                                                        |
	| Specification of meshes for distributed parameters of solids |                                                                                                      |
	|                                                              | ``<distribution> <type> <classes> [<grid>] (<min> <max> | <boundaries> | <names>)``                  |
	+                                                              +------------------------------------------------------------------------------------------------------+
	|                                                              | For Continuous not Manual:                                                                           |
	|                                                              |                                                                                                      |
	|                                                              | ``<distribution> <type> <classes> <grid> <min> <max>``                                               |
	+                                                              +------------------------------------------------------------------------------------------------------+
	|                                                              | For Discrete or Continuous Manual:                                                                   |
	|                                                              |                                                                                                      |
	|                                                              | ``<distribution> <type> <classes> <boundaries>``                                                     |
	+                                                              +------------------------------------------------------------------------------------------------------+
	|                                                              | For Symbolic:                                                                                        |
	|                                                              |                                                                                                      |
	|                                                              | ``<distribution> <type> <classes> <names>``                                                          |
	+                                                              +------------------------------------------------------------------------------------------------------+
	|                                                              | ``<distribution>`` – index of the distribution as it stated in Grid Specification window             |
	+                                                              +------------------------------------------------------------------------------------------------------+
	|                                                              | ``<type>`` – distribution type represented by numbers:                                               |
	|                                                              |                                                                                                      |
	|                                                              | ``<type>`` = 0 – Continuous                                                                          |
	|                                                              |                                                                                                      |
	|                                                              | ``<type>`` = 1 – Discrete                                                                            |
	|                                                              |                                                                                                      |
	|                                                              | ``<type>`` = 2 – Symbolic                                                                            |
	+                                                              +------------------------------------------------------------------------------------------------------+
	|                                                              | ``<classes>`` – number of classes                                                                    |
	+                                                              +------------------------------------------------------------------------------------------------------+
	|                                                              | ``<grid>`` – type of the grid (if <type> = 0) represented by numbers:                                |
	|                                                              |                                                                                                      |
	|                                                              | ``<grid>`` = 0 – Manual                                                                              |
	|                                                              |                                                                                                      |
	|                                                              | ``<grid>`` = 1 – Equidistant                                                                         |
	|                                                              |                                                                                                      |
	|                                                              | ``<grid>`` = 2 – Geometric increasing                                                                |
	|                                                              |                                                                                                      |
	|                                                              | ``<grid>`` = 3 – Logarithmic increasing                                                              |
	|                                                              |                                                                                                      |
	|                                                              | ``<grid>`` = 4 – Geometric decreasing                                                                |
	|                                                              |                                                                                                      |
	|                                                              | ``<grid>`` = 5 – Logarithmic decreasing                                                              |
	+                                                              +------------------------------------------------------------------------------------------------------+
	|                                                              | ``<min>`` – min value, if ``<type>`` = 0 and ``<grid>`` :math:`\neq` 0                               |
	+                                                              +------------------------------------------------------------------------------------------------------+
	|                                                              | ``<max>`` – max value, if ``<type>`` = 0 and ``<grid>`` :math:`\neq` 0                               |
	+                                                              +------------------------------------------------------------------------------------------------------+
	|                                                              | ``<boundaries>`` – class boundary values, if (``<type>`` = 0 and ``<grid>`` = 0) or (``<type>`` = 1) |
	+                                                              +------------------------------------------------------------------------------------------------------+
	|                                                              | ``<names>`` – names of classes boundary values (if ``<type>`` = 2)                                   |
	+--------------------------------------------------------------+------------------------------------------------------------------------------------------------------+

|

Unit parameters
"""""""""""""""

	+------------------------------------------------------+----------------------------------------------------------------------------------------------+
	| Identifier                                           | Parameter                                                                                    |
	+======================================================+==============================================================================================+
	| ``UNIT_PARAMETER``: Specification of unit parameters | General form:                                                                                |
	|                                                      |                                                                                              |
	|                                                      | ``<unit> <parameter> (<value> | <time> <value> [<time> <value> […]] | <string> | <solver>)`` |
	+                                                      +----------------------------------------------------------------------------------------------+
	|                                                      | For constant parameter:                                                                      |
	|                                                      |                                                                                              |
	|                                                      | ``<unit> <parameter> <value>``                                                               |
	+                                                      +----------------------------------------------------------------------------------------------+
	|                                                      | For time-dependent parameter:                                                                |
	|                                                      |                                                                                              |
	|                                                      | ``<unit> <parameter> <time> <value> [<time> <value> […]]``                                   |
	+                                                      +----------------------------------------------------------------------------------------------+
	|                                                      | For string parameter:                                                                        |
	|                                                      |                                                                                              |
	|                                                      | ``<unit> <parameter> <string>``                                                              |
	+                                                      +----------------------------------------------------------------------------------------------+
	|                                                      | For solver parameter:                                                                        |
	|                                                      |                                                                                              |
	|                                                      | ``<unit> <parameter> <solver>``                                                              |
	+                                                      +----------------------------------------------------------------------------------------------+
	|                                                      | ``<unit>`` – index of the unit                                                               |
	+                                                      +----------------------------------------------------------------------------------------------+
	|                                                      | ``<parameter>`` – index of the parameter                                                     |
	+                                                      +----------------------------------------------------------------------------------------------+
	|                                                      | ``<value>`` – numerical value of the parameter                                               |
	+                                                      +----------------------------------------------------------------------------------------------+
	|                                                      | ``<time>`` – value of the time point (for time-dependent parameters)                         |
	+                                                      +----------------------------------------------------------------------------------------------+
	|                                                      | ``<string>`` – numerical value of the parameter (for string parameters)                      |
	+                                                      +----------------------------------------------------------------------------------------------+
	|                                                      | ``<solver>`` – name of a ``*.dll`` file with solver (for solver parameter)                   |
	+------------------------------------------------------+----------------------------------------------------------------------------------------------+

|

Holdups
"""""""
	
	+-------------------------------------------------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------+
	| Identifier                                                                    | Parameter                                                                                                                         |
	+===============================================================================+===================================================================================================================================+
	| ``UNIT_HOLDUP_MTP``: Specification of holdups: mass, temperature and pressure | ``<unit> <holdup> <timepoint> <mass> <temperature> <pressure>`` with:                                                             |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<unit>`` – index of the unit                                                                                                    |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<holdup>`` – index of the holdup in the unit                                                                                    |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<timepoint>`` – index of the time point                                                                                         |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<mass>`` – mass value                                                                                                           |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<temperature>`` – temperature value                                                                                             |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<pressure>`` – pressure value                                                                                                   |
	+-------------------------------------------------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------+
	| ``UNIT_HOLDUP_PHASES``: Specification of holdups: phase fractions             | ``<unit> <holdup> <timepoint> <fraction> [fraction […]]`` with:                                                                   |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<unit>`` – index of the unit                                                                                                    |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<holdup>`` – index of the holdup in the unit                                                                                    |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<timepoint>`` – index of the time point                                                                                         |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<fraction>`` – mass fraction of the phase, number of parameters must correspond to the number of phases                         |
	+-------------------------------------------------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------+
	| ``UNIT_HOLDUP_COMP``: Specification of holdups: compound fractions in phase   | ``<unit> <holdup> <phase> <timepoint> <fraction> [fraction […]]`` with:                                                           |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<unit>`` – index of the unit                                                                                                    |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<holdup>`` – index of the holdup in the unit                                                                                    |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<phase>`` – index of the phase                                                                                                  |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<timepoint>`` – index of the time point                                                                                         |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<fraction>`` – mass fraction of the compound, number of parameters must correspond to the number of compounds                   |
	+-------------------------------------------------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------+
	| ``UNIT_HOLDUP_SOLID``:                                                        | ``<unit> <holdup> <distribution> <compound> <timepoint> <psdtype> <function> <psdgridtype> (<param1> <param2> | <values>)`` with: |
	| Specification of holdups: distributed parameters of solids                    |                                                                                                                                   |
	|                                                                               | ``<unit>`` – index of the unit                                                                                                    |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<holdup>`` – index of the holdup in the unit                                                                                    |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<distribution>`` – index of the distribution as specified in Distributions Sequence                                             |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<compound>`` – index of the compound, ``<compound>`` = 0 for total mixture                                                      |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<timepoint>`` – index of the time point                                                                                         |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<psdtype>`` – PSD type, if <distribution> corresponds to a PSD. Represented with numbers:                                       |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<psdtype>`` = -1 – Not a PSD                                                                                                    |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<psdtype>`` = 0 – q3                                                                                                            |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<psdtype>`` = 1 – Q3                                                                                                            |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<psdtype>`` = 2 – q0                                                                                                            |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<psdtype>`` = 3 – Q0                                                                                                            |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<psdtype>`` = 4 – Mass fraction                                                                                                 |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<psdtype>`` = 5 – Number                                                                                                        |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<psdtype>`` = 6 – q2                                                                                                            |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<psdtype>`` = 7 – Q2                                                                                                            |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<function>`` – index of the distribution function represented by numbers:                                                       |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<function>`` = 0 – Manual                                                                                                       |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<function>`` = 1 – Normal distribution                                                                                          |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<function>`` = 2 – RRSB                                                                                                         |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<function>`` = 3 – GGS                                                                                                          |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<function>`` = 4 – Logarithmic Normal                                                                                           |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<psdgridtype>`` – type of the grid if ``<distribution>`` corresponds to a PSD:                                                  |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<psdgridtype>`` = -1 – Not a PSD                                                                                                |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<psdgridtype>`` = 0 – Diameters                                                                                                 |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<psdgridtype>`` = 1 – Volumes                                                                                                   |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<param1>``, ``<param2>`` – parameters of the distribution function (only if ``<function>`` :math:`\neq` 0)                      |
	|                                                                               |                                                                                                                                   |
	|                                                                               | ``<values>`` – list of distribution values (only if ``<function>`` = 0)                                                           |
	+-------------------------------------------------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------+
