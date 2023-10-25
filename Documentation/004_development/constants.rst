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

.. doxygenenum:: EPSDTypes
    :project: dyssol_models_api

.. Note:: Notations:

	:math:`i` – index of size classes

	:math:`j` – index of compounds

	:math:`k` – index of porosities

	:math:`d_i` – particle diameter of class :math:`i`

	:math:`\Delta d_i` – size of the class :math:`i`

	:math:`m_i` – mass of particles of class :math:`i`

	:math:`M_{tot}` – total mass of particles

	:math:`N_i` – number of particles of class :math:`i`

	:math:`N_{i,j}` – number of particles of compound :math:`j` with size class :math:`i`

	:math:`N_{tot}` – total number of particles

	:math:`w_i` – mass fraction of particles of class :math:`i`

	:math:`w_{i,j}` – mass fraction of particles of compound :math:`j` with size class :math:`i`

	:math:`w_{i,j,k}` – mass fraction of particles of compound :math:`j` with size class :math:`i` and porosity :math:`k`

	:math:`\rho_j` – density of compound :math:`j`

	:math:`\varepsilon_k` – porosity of class :math:`k`

	:math:`q_0` – number-related density distribution

	:math:`Q_0` – number-related cumulative distribution

	:math:`q_2` – surface-area-related density distribution

	:math:`Q_2` – surface-area-related cumulative distribution

	:math:`q_3` – mass-related density distribution

	:math:`Q_3` – mass-related cumulative distribution

|

.. doxygenenum:: EPSDGridType
    :project: dyssol_models_api