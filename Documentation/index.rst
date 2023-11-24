.. _sec.index:

Dyssol documentation
====================

Dyssol, the DYnamic Simulation of SOLids processes, is a dynamic flowsheet modelling system designed to simulate the time-dependent behaviour of complex production processes in solids processing technology. Its distinctive features:

* Dynamic simulation of complex process structures

* Sequential-modular calculation algorithm

* Consideration of solid, liquid, gas phases and their mixtures

* Handling of multidimensional interdependent distributed parameters of solids

* Standardized interfaces and templates for implementation of new units

* High modularity and extensibility

This documentation will help you to get familiar with the Dyssol flowsheet simulations system. Here you will find an introduction to the structure and algorithms of the program, the principles of its operation, a description of the user interface, as well as the documentation of available units. It also provides information on how to create your own units and solvers.

.. _sec.contents:

Contents
========

.. toctree::
   :maxdepth: 2
   :caption: Get started

   000_get_started/get_started

|

.. toctree::
   :maxdepth: 2
   :caption: User interface

   001_ui/gui
   001_ui/cli

|

.. toctree::
   :maxdepth: 2
   :caption: Structure and algorithms

   002_theory/brief
   002_theory/simulation
   002_theory/data
   002_theory/materials
   002_theory/multiDim
   002_theory/time
   002_theory/theory

|

.. toctree::
   :maxdepth: 2
   :caption: Libraries

   003_models/units
   003_models/solvers

|

.. toctree::
   :maxdepth: 2
   :caption: For developers

   004_development/compilation
   004_development/models_development
   004_development/api

|

.. _sec.acknowledgement:

Acknowledgement
===============

Dyssol project was funded by German Research Foundation (DFG) via SPP 1679 "Dyn-Sim-FP" and via "Research Software Sustainability" program.

.. seealso::
	#. Skorych V., Dosta M., Hartge E.-U., Heinrich S.: Novel system for dynamic flowsheet simulation of solids processes. Powder Technology 314 (2017), 665-679. `10.1016/j.powtec.2017.01.061`_
	#. Skorych V., Dosta M., Hartge E.-U., Heinrich S., Ahrens R., Le Borne S.: Investigation of an FFT-based solver applied to dynamic flowsheet simulation of agglomeration processes. Advanced Powder Technology 30(3) (2019), 555-564. `10.1016/j.apt.2018.12.007`_
	#. Skorych V., Das N., Dosta M., Kumar J., Heinrich S.: Application of transformation matrices to the solution of population balance equations. Processes 7(8) (2019) 535. `10.3390/pr7080535`_
	#. Skorych V., Dosta M., Heinrich S.: Dyssol — An open-source flowsheet simulation framework for particulate materials. SoftwareX 12 (2020), 100572. `10.1016/j.softx.2020.100572`_
	#. Skorych V., Buchholz M., Dosta M., Heinrich S.: A Framework for Dynamic Simulation of Interconnected Solids Processes. In: Heinrich S. (eds) Dynamic Flowsheet Simulation of Solids Processes, 2020. Springer, Cham. `10.1007/978-3-030-45168-4_17`_

|

.. _`10.1016/j.powtec.2017.01.061`: https://doi.org/10.1016/j.powtec.2017.01.061
.. _`10.3390/pr7080535`: https://doi.org/10.3390/pr7080535
.. _`10.1016/j.apt.2018.12.007`: https://doi.org/10.1016/j.apt.2018.12.007
.. _`10.1016/j.softx.2020.100572`: https://doi.org/10.1016/j.softx.2020.100572
.. _`10.1007/978-3-030-45168-4_17`: https://doi.org/10.1007/978-3-030-45168-4_17

.. _sec.contact:

Contact
==================

`Vasyl Skorych`_

.. _`Vasyl Skorych`: vasyl.skorych@tuhh.de
