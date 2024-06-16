.. _sec.development.api.thermodynamics:

Thermodynamics
==============

In Dyssol, thermodynamic functions are applied to calculate a material stream / holdup temperature :math:`T` from a given enthalpy value :math:`H`. In most cases, the enthalpy of a compound correlates with temperature *non-linearly*. Thus, for backward calculation of the temperature from an enthalpy value, you need to solve a non-linear equation :math:`T = f^{-1}(H)`.

This solving step is replaced by a *lookup table* functionality, i.e. co-dependent temperature and enthalpy are pre-calculated for a certain range of values and then stored in a table. This table can be used to determine temperature-enthalpy value pairs by interpolation between two neighbor points.

To add more functionality, these lookup tables can be called for every property defined in the materials database.

.. math::

	Val_{prop,i}(T/p) = f_i(T/p)
	
	f_{tot}(T/p) = \sum\limits_i w_i \, f_i(T/p)
	
	T/p = f_{tot}^{-1}(Val_{prop,tot})


The temperature or pressure (e.g. pressure from saturation pressure or different properties) is then calculated in the following steps:

	1.	Create tables of value pairs for each compound that is present in the system, i.e. temperature / pressure and respective dependent property value over a certain range of the temperature / pressure.

	2.	Combine the compound-lookup tables by mass-weighted summation of the tables.

	3.	Read-out of temperature / pressure from the resulting lookup-table at a certain point of the property by interpolation between neighbor value pairs.

The functions in base unit are less time-consuming than the material stream / holdup-function and therefore are implemented for usage during solving process (i.e. function :ref:`CalculateResiduals <label-CalculateResiduals>`), if the composition of the material stream / holdup varies in each iteration.

In :ref:`label-unitsLib`, ``Heater`` and ``HeatExchanger`` apply thermodynamic calculations.

|

For material stream and holdup
------------------------------

.. code-block:: cpp

	double CalcTemperatureFromProperty(ECompoundTPProperties Property, double Time, double Value)

Returns temperature of the material stream / holdup for a specific value ``Value`` of the property ``Property`` at the time point ``Time``. Possible properties are those defined in :ref:`sec.mdb`.

|

.. code-block:: cpp

	double CalcPressureFromProperty(ECompoundTPProperties Property, double Time, double Value)

Returns pressure of the material stream / holdup for a specific value ``Value`` of the property ``Property`` at the time point ``Time``. Possible properties are those defined in :ref:`sec.mdb`.

|

For base unit
-------------

.. code-block:: cpp

	double CalcTemperatureFromProperty(ECompoundTPProperties Property,vector<double>& CompoundFractions, double Value)

Returns temperature of a generic system of composition ``CompoundFractions`` for a specific value ``Value`` of the property ``Property``. Possible properties are those defined in :ref:`sec.mdb`.

|

.. code-block:: cpp

	double CalcPressureFromProperty(ECompoundTPProperties Property,vector<double>& CompoundFractions, double Value)

Returns pressure of a generic system of composition ``CompoundFractions`` for a specific value ``Value`` of the property ``Property``. Possible properties are those defined in :ref:`sec.mdb`.

|

.. code-block:: cpp

	void HeatExchange(CMaterialStream* Stream1, CMaterialStream* Stream2, double Time, double Efficiency);

Performs a heat exchange between material streams ``Stream1`` and ``Stream2`` at specified time point ``Time`` with a specified efficiency (0 ≤ ``Efficiency`` ≤ 1).

.. math::

	|\dot Q| = \varepsilon \, |\dot Q_{ideal}| = \varepsilon \, \left | \int_{T_1}^{T_{mix}} \dot m_1\,c_{P,1}(\theta)\,d\theta \right | = \varepsilon \, \left | \int_{T_2}^{T_{mix}} \dot m_2\,c_{P,2}(\theta)\,d\theta \right |

:math:`\varepsilon` stands for efficiency and :math:`\theta` for temperature as an integration variable.

|
