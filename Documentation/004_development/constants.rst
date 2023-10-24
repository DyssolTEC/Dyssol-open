.. _sec.development.api.constants:

List of constants
=================

Universal constants
-------------------

In Dyssol, certain universal constants are pre-defined and available for direct utilization in the code.

+------------------------------------------+----------------------------------------+--------------------+
|  Define                                  |   Typical value                        |   Unit             |
+==========================================+========================================+====================+
|   ``AVOGADRO_CONSTANT``                  |   :math:`6.02214199(47) \cdot 10^{23}` |   [1/mol]          |
+------------------------------------------+----------------------------------------+--------------------+
|   ``BOLTZMANN_CONSTANT``                 |   :math:`1.3806503(24) \cdot 10^{-23}` |   [J/K]            |
+------------------------------------------+----------------------------------------+--------------------+
|   ``IDEAL_GAS_STATE_REFERENCE_PRESSURE`` |   :math:`101\,325`                     |   [Pa]             |
+------------------------------------------+----------------------------------------+--------------------+
|   ``MOLAR_GAS_CONSTANT``                 |   :math:`8.314472(15)`                 |   [J/mol/K]        |
+------------------------------------------+----------------------------------------+--------------------+
|   ``SPEED_OF_LIGHT_IN_VACUUM``           |   :math:`2.99792458(1) \cdot 10^8`     |   [m/s]            |
+------------------------------------------+----------------------------------------+--------------------+
|   ``STANDARD_ACCELERATION_OF_GRAVITY``   |   :math:`9.80665`                      |   [m/s :math:`^2`] |
+------------------------------------------+----------------------------------------+--------------------+
|   ``MATH_PI``                            |   :math:`3.14159265358979323846`       |   [-]              |
+------------------------------------------+----------------------------------------+--------------------+

.. doxygendefine:: STANDARD_CONDITION_T
   :project: dyssol_models_api
   
.. doxygendefine:: STANDARD_CONDITION_P
   :project: dyssol_models_api

.. doxygenenum:: EUnitPort
    :project: dyssol_models_api

.. doxygenenum:: EPhase
    :project: dyssol_models_api
	
.. doxygenenum:: EDistrTypes
    :project: dyssol_models_api
	
.. doxygenenum:: EGridEntry
    :project: dyssol_models_api
	
.. doxygenenum:: ECompoundConstProperties
    :project: dyssol_models_api
	
.. doxygenenum:: ECompoundTPProperties
    :project: dyssol_models_api
	
.. doxygenenum:: EInteractionProperties
    :project: dyssol_models_api

.. doxygenenum:: EOverall
    :project: dyssol_models_api