.. _sec.gui:

Graphical user interfaced
=========================

After installation and launching Dyssol, you will see the GUI, which is introduced below by corresponding screenshots.


Menu
^^^^

You can find File, Flowsheet, Modules, Tools and Help in the menu bar.

File
""""

	.. image:: ./static/images/gui/fileMenu.png
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
	
	Please refer to :ref:`run Dyssol in command line mode <sec.cli>` for more information.

	.. image:: ./static/images/gui/command.png
	   :width: 960px
	   :alt: 
	   :align: center

|
	   
Flowsheet
"""""""""

	.. image:: ./static/images/gui/flowsheets.png
	   :width: 960px
	   :alt: 
	   :align: center

|

- Compounds editor:

	.. image:: ./static/images/gui/compounds.png
	   :width: 960px
	   :alt: 
	   :align: center

|
	
- Phases editor:

	.. image:: ./static/images/gui/phases.png
	   :width: 960px
	   :alt: 
	   :align: center

|

- Grids specification:

	.. image:: ./static/images/gui/grid1.png
	   :width: 960px
	   :alt: 
	   :align: center

|
	   
	.. image:: ./static/images/gui/grid2.png
	   :width: 960px
	   :alt: 
	   :align: center

|

- Holdups editor:
	
	.. image:: ./static/images/gui/holdup1.png
	   :width: 960px
	   :alt: 
	   :align: center

	|
	
	- Main control:
	
	.. image:: ./static/images/gui/holdup2.png
	   :width: 960px
	   :alt: 
	   :align: center
	
	|
	
	- Concentrated parameters:
	
	.. image:: ./static/images/gui/holdup3.png
	   :width: 960px
	   :alt: 
	   :align: center

	|
	
	- Distributed parameters:
	
	.. image:: ./static/images/gui/holdup4.png
	   :width: 960px
	   :alt: 
	   :align: center

|

- Options: 
	
	.. image:: ./static/images/gui/options1.png
	   :width: 960px
	   :alt: 
	   :align: center
	   
	|
	
	- Convergence: 
	
		For more information, please refer to :ref:`label-convergence` or the help document in directory ``Dyssol\Help\Convergence.pdf``.
	
	.. image:: ./static/images/gui/options2.png
	   :width: 960px
	   :alt: 
	   :align: center

	|
	
	- Service:
	
	.. image:: ./static/images/gui/options3.png
	   :width: 480px
	   :alt: 
	   :align: center

|

- Calculation sequence:
	
	.. image:: ./static/images/gui/sequence.png
	   :width: 960px
	   :alt: 
	   :align: center

|

- Recycle streams:	
	
	.. image:: ./static/images/gui/recycle.png
	   :width: 960px
	   :alt: 
	   :align: center

|

Modules
"""""""
	.. image:: ./static/images/gui/modules.png
	   :width: 960px
	   :alt: 
	   :align: center

|

Tools
"""""

	.. image:: ./static/images/gui/tools.png
	   :width: 960px
	   :alt: 
	   :align: center

|

- Models manager:

	.. image:: ./static/images/gui/modelsManag.png
	   :width: 960px
	   :alt: 
	   :align: center

|

- Materials Database:

	.. image:: ./static/images/gui/database.png
	   :width: 960px
	   :alt: 
	   :align: center

	|

	- Tab Compounds - Compounds List:
	
	.. image:: ./static/images/gui/dat1.png
	   :width: 960px
	   :alt: 
	   :align: center

	|
	
	- Tab Compounds - Properties:
	
	.. image:: ./static/images/gui/dat2.png
	   :width: 960px
	   :alt: 
	   :align: center

	|
	
	- Tab Compounds - Correlations:
	
	.. image:: ./static/images/gui/dat3.png
	   :width: 960px
	   :alt: 
	   :align: center

	|
	
	- Tab Compounds - Correlations plot:
	
	.. image:: ./static/images/gui/dat4.png
	   :width: 960px
	   :alt: 
	   :align: center

	|
	
	- Tab Interactions:

	.. image:: ./static/images/gui/dat5.png
	   :width: 960px
	   :alt: 
	   :align: center

	|


Help
""""
	
	.. image:: ./static/images/gui/help.png
	   :width: 960px
	   :alt: 
	   :align: center

|

Tabs
^^^^

Tabs are needed for the parameter setup. They are named by Flowsheet, Simulator, Streams and Units.
	
	.. image:: ./static/images/gui/tabs.png
	   :width: 960px
	   :alt: 
	   :align: center
	
|

Flowsheet
"""""""""

- Units:
	
	.. image:: ./static/images/gui/units.png
	   :width: 960px
	   :alt: 
	   :align: center

- Streams:

	.. image:: ./static/images/gui/streams.png
	   :width: 960px
	   :alt: 
	   :align: center

- Model selection:

	.. image:: ./static/images/gui/modelSelect.png
	   :width: 960px
	   :alt: 
	   :align: center

- Ports connection:

	.. image:: ./static/images/gui/ports.png
	   :width: 960px
	   :alt: 
	   :align: center

- Unit parameters:

	.. image:: ./static/images/gui/unitParam.png
	   :width: 960px
	   :alt: 
	   :align: center

|

Simulator
"""""""""

Here you can start your simulation based on your time input.

	.. image:: ./static/images/gui/simulator.png
	   :width: 960px
	   :alt: 
	   :align: center

|

Stream
""""""

Here you will find the simulation results, including the stream results summarized in table and plot.

- Table view:

	.. image:: ./static/images/gui/table.png
	   :width: 960px
	   :alt: 
	   :align: center

- Plot view:

	.. image:: ./static/images/gui/plot.png
	   :width: 960px
	   :alt: 
	   :align: center

|

Units
"""""

In Units tab, you can control the operation units.

	.. image:: ./static/images/gui/unitsTab.png
	   :width: 960px
	   :alt: 
	   :align: center

|

