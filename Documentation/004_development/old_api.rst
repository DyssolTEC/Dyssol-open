.. _sec.old_classes:

=======
Old API
=======

.. _label-old_baseUnit:

Basic unit
==========

.. _label-old_unitParameters:

Unit parameters
---------------

.. code-block:: cpp

	unsigned GetParametersNumber()

Returns number of unit parameters which have been defined in the unit. 

|

.. code-block:: cpp

	std::string GetParameterName(unsigned Index)

Returns name of the unit parameter with the specified index Index. Empty string is returned if such parameter has not been defined.

|

.. code-block:: cpp

	double GetParameterMinVal(std::string ParameterName)
	
Returns minimum allowable value of the time-dependent or constant unit parameter with the name ``ParameterName``. Returns ``0`` if such parameter has not been defined or this is not a constant or time-dependent parameter.

|

.. code-block:: cpp

	double GetParameterMaxVal(std::string ParameterName)

Returns maximum allowable value of the time-dependent or constant unit parameter with the name ``ParameterName``. Returns ``0`` if such parameter has not been defined or this is not a constant or time-dependent parameter.

|

.. code-block:: cpp

	std::vector<double> GetParameterTimePoints(std::string ParameterName, double TimeStart, double TimeEnd)
	
Returns all time points for which time-dependent parameter is defined within the specified time interval [``TimeStart``; ``TimeEnd``]. Returns empty vector if such parameter has not been defined or this is not a time-dependent parameter.

|

State variables
---------------


.. _label-old_AddStateVariable:

.. code-block:: cpp

	unsigned GetStateVariablesNumber()	

Returns number of state variables which have been defined in this unit. 

|

.. code-block:: cpp

	std::string GetStateVariableName(unsigned Index)
	
Returns the name of the state variable with specified index. Returns empty string if such variable has not been defined.

|

.. code-block:: cpp

	void ClearStateVariables()
	
Removes all state variables and history of their changes. 

|

.. _label-old_stream:

Stream
======

Functions in classes ``CMaterialStream`` and ``CHoldup`` are introduced below.

|

Basic stream properties
-----------------------

All functions in this section are for both ``CMaterialStream`` and ``CHoldup``.

.. code-block:: cpp

	std::string GetStreamName()
	
Returns the name of the material stream / holdup. 

|

.. code-block:: cpp

	void SetStreamName(std::string Name)
	
Sets the name of the material stream / holdup. 

|

Time points
-----------

All functions in this section are for both ``CMaterialStream`` and ``CHoldup``.

.. code-block:: cpp

	void AddTimePoint(double Time, double SourceTime = -1)
	
Adds new time point ``Time`` to the material stream / holdup. Data for this time point is copied from ``SourceTime``. By default (``SourceTime = -1``) data will be copied from the previous time point. If this is the first time point in the material stream / holdup, all data will be set to ``0``. If such time point already exists, nothing will be done.

|

.. code-block:: cpp

	void RemoveTimePoint(double Time)
	
Removes time point ``Time`` from the material stream / holdup, if such point exists. 

|

.. code-block:: cpp

	void RemoveTimePoints(double Start, double End)
	
Removes all time points from the specified interval, including boundaries.

|

.. code-block:: cpp

	void RemoveTimePointsAfter(double Start, bool IncludeStart = false)
	
Removes all data after the specified time point including (if ``IncludeStart`` is set to ``true``) or excluding (``IncludeStart`` is set to ``false``) point ``Start``.

|


.. code-block:: cpp

	std::vector<double> GetAllTimePoints()

Returns all time points which are defined in the material stream / holdup. 

|


.. code-block:: cpp

	std::vector<double> GetTimePointsForInterval(double Start, double End, bool ForceInclBoudaries = false)
	
Returns the list of time points for the specified time interval (incl. boundary points ``Start`` and ``End``). If ``ForceInclBoudaries`` is set to ``true``, resulting vector will contain boundary points even if they have not been defined in the material stream / holdup.

|


.. code-block:: cpp

	double GetLastTimePoint()

Returns last defined time point in the material stream / holdup. Returns ``-1`` if no time points have been defined.

|

.. code-block:: cpp

	double GetPreviousTimePoint(double Time)
	
Returns the nearest time point before ``Time``. Returns ``-1`` if there is no time points before the specified value.

|

Overall properties
------------------

.. _label-old_massFlow:

.. code-block:: cpp

	double GetMassFlow(double Time, unsigned Basis)
	
Specific function for ``CMaterialStream``. 

Returns mass or mole flow of the material stream at the specified time point ``Time``. If such time point has not been defined, interpolation of data will be done. 

``Basis`` is a basis of results (``BASIS_MASS`` in [kg/s] or ``BASIS_MOLL`` in [mol/s]):

``BASIS_MASS``: :math:`\dot m` in [kg/s], total mass flow of the material stream.

``BASIS_MOLL``: :math:`\sum\limits_i \dfrac{\dot m \cdot w_i}{M_i}` in [mol/s], with :math:`w_i` mass fraction of the phase :math:`i`, and :math:`M_i` molar mass of the phase :math:`i`.

|

.. _label-old_mass:

.. code-block:: cpp

	double GetMass(double Time, unsigned Basis) 
	
Specific function for ``CHoldup``. 

Returns mass or mole of the holdup at the specified time point ``Time``. If such time point has not been defined, interpolation of data will be done. 

``Basis`` is a basis of results (``BASIS_MASS`` in [kg] or ``BASIS_MOLL`` in [mol]):

``BASIS_MASS``: :math:`m` in [kg], total mass of the holdup.

``BASIS_MOLL``: :math:`\sum\limits_i \dfrac{m \cdot w_i}{M_i}` in [mol], with :math:`w_i` mass fraction of the phase :math:`i`, and :math:`M_i` molar mass of the phase :math:`i`.

|

.. _label-old_setMassFlow:

.. code-block:: cpp

	void SetMassFlow(double Time, double Value, unsigned Basis)
	
Specific function for ``CMaterialStream``. 
	
Sets mass flow of the material stream at the time point ``Time``. Negative values before setting will be converted to ``0``. If the time point Time has not been defined in the material stream, then the value will not be set. 

``Basis`` is a basis of results (``BASIS_MASS`` in [kg/s] or ``BASIS_MOLL`` in [mol/s]):

``BASIS_MASS``: in this case you directly have your input ``Value`` as mass flow: :math:`\dot{m} =` ``Value`` in [kg/s]

``BASIS_MOLL``: in this case you have ``Value`` as mole flow and this should be converted to mass flow. :math:`\dot{m} =` ``Value`` :math:`\cdot \sum\limits_i M_i \cdot w_i` in [mol/s], with :math:`w_i` mass fraction of the phase :math:`i`, and :math:`M_i` molar mass of the phase :math:`i`.

|

.. _label-old_setMass:

.. code-block:: cpp

	void SetMass(double Time, double Value, unsigned Basis)
	
Specific function for ``CHoldup``. 

Sets mass of the holdup at the time point ``Time``. Previously set negative values will be converted to ``0``. If the time point ``Time`` has not been defined in the holdup, then the value will not be set. 

``Basis`` is a basis of results (``BASIS_MASS`` in [kg] or ``BASIS_MOLL`` in [mol]):

``BASIS_MASS``: in this case you directly have your input ``Value`` as mass value: :math:`m =` ``Value`` in [kg].

``BASIS_MOLL``: in this case you have ``Value`` as mole amount and this should be converted to mass. :math:`m =` ``Value`` :math:`\cdot \sum\limits_i M_i \cdot w_i` in [mol], with :math:`w_i` mass fraction of the phase :math:`i`, and :math:`M_i` molar mass of the phase :math:`i`.

|

.. _label-old_temp:

.. code-block:: cpp

	double GetTemperature(double Time)
	
Function for both ``CMaterialStream`` and ``CHoldup``.

Returns temperature of the material stream / holdup at the specified time point ``Time`` in [K]. If such time point has not been defined, interpolation of data will be done.

|

.. _label-old_setTemp:

.. code-block:: cpp

	void SetTemperature(double Time, double Value)

Function for both ``CMaterialStream`` and ``CHoldup``.

Sets temperature of the material stream / holdup at the time point ``Time`` in [K]. Negative values before setting will be converted to ``0``. If time the point ``Time`` has not been defined in the material stream / holdup, the value will not be set.

|

.. _label-old_pressure:

.. code-block:: cpp

	double GetPressure(double Time)
	
Function for both ``CMaterialStream`` and ``CHoldup``.

Returns pressure of the material stream / holdup at the specified time point ``Time`` in [Pa]. If such time point has not been defined, interpolation of data will be done.

|

.. _label-old_setPressure:

.. code-block:: cpp

	void SetPressure(double Time, double Value)

Function for both ``CMaterialStream`` and ``CHoldup``.

Sets pressure of the material stream / holdup in the time point ``Time`` in [Pa]. Negative values before setting will be converted to ``0``. If the time point ``Time`` has not been defined in the material stream / holdup, the value will not be set.

|

.. code-block:: cpp

	double GetOverallProperty(double Time, unsigned Property, unsigned Basis)
		
Returns non-constant physical property value for the overall mixture at the specified time point ``Time``. If such time point has not been defined, interpolation of data will be done. 

``Basis`` is a basis of results (``BASIS_MASS`` or ``BASIS_MOLL``).

``Property`` is an identifier of a physical property. Available properties are:

	-	``FLOW`` and ``TOTAL_FLOW`` for material stream ``CMaterialStream``: refer to function :ref:`getMassFlow <label-massFlow>`.
	
	-	``MASS`` and ``TOTAL_MASS`` for holdup ``CHoldup``: refer to function :ref:`getMass <label-mass>`.
	
	-	``TEMPERATURE``: refer to function :ref:`getTemperature <label-temp>`.
	
	-	``PRESSURE``: refer to function :ref:`getPressure <label-pressure>`.
	
	-	``MOLAR_MASS``: :math:`\sum\limits_i M_i \cdot w_i`, with :math:`M` molar mass of the total flow, :math:`w_i` mass fraction of the phase :math:`i`, and :math:`M_i` molar mass of the phase :math:`i`.
	
	-	``ENTHALPY``:
	
		- Set ``Basis`` as ``BASIS_MASS``: :math:`\sum\limits_i H_i \cdot w_i`, with :math:`H_i` the enthalpy of the phase :math:`i`, and :math:`w_i` the mass fraction of the phase :math:`i`.
		
		- Set ``Basis`` as ``BASIS_MOLL``: :math:`\sum\limits_i H_i \cdot x_i`, with :math:`H_i` the enthalpy of the phase :math:`i`, and :math:`x_i` the mole fraction of the phase :math:`i`.


.. Note:: Definition of overall mixture properties:

	+--------------------------+--------------------+---------------------------------------------------------------------------+
	|   Define                 |   Name             |   Units                                                                   |
	+==========================+====================+===========================================================================+
	|``FLOW``, ``TOTAL_FLOW``  | Mass / mole flow   | [kg/s] or  [mol/s]                                                        |
	+--------------------------+--------------------+---------------------------------------------------------------------------+
	| ``MASS``, ``TOTAL_MASS`` | Mass / mole        | [kg] or [mol]                                                             |
	+--------------------------+--------------------+---------------------------------------------------------------------------+
	|   ``TEMPERATURE``        |   Temperature      |   [K]                                                                     |
	+--------------------------+--------------------+---------------------------------------------------------------------------+
	|   ``PRESSURE``           |   Pressure         |   [Pa]                                                                    |
	+--------------------------+--------------------+---------------------------------------------------------------------------+
	|   ``MOLAR_MASS``         |   Molar mass       |   [kg/mol]                                                                |
	+--------------------------+--------------------+---------------------------------------------------------------------------+
	|   ``ENTHALPY``           |   Enthalpy         |   [J/kg/s] or [J/mol/s] for material stream, [J/kg] or [J/mol] for holdup |
	+--------------------------+--------------------+---------------------------------------------------------------------------+

|

.. code-block:: cpp

	double SetOverallProperty(double Time, unsigned Property, double Value, unsigned Basis)
	
Sets non-constant physical property value for the overall mixture at the specified time point ``Time``.

Basis is a basis of the value (``BASIS_MASS`` or ``BASIS_MOLL``). 

``Property`` is an identifier of a physical property. Available properties are:

	-	``FLOW`` and ``TOTAL_FLOW`` for material stream ``CMaterialStream``: refer to function :ref:`setMassFlow <label-setMassFlow>`.
	
	-	``MASS`` and ``TOTAL_MASS`` for holdup ``CHoldup``: refer to function :ref:`setMass <label-setMass>`.
	
	-	``TEMPERATURE``: refer to function :ref:`setTemperature <label-setTemp>`.
	
	-	``PRESSURE``: refer to function :ref:`setPressure <label-setPressure>`.


.. Note:: Definition of overall mixture properties:

	+--------------------------+--------------------+-------------------------+
	|   Define                 |   Name             |   Units                 |
	+==========================+====================+=========================+
	|``FLOW``, ``TOTAL_FLOW``  |   Mass / mole flow | [kg/s] or  [mol/s]      |
	+--------------------------+--------------------+-------------------------+
	| ``MASS``, ``TOTAL_MASS`` | Mass / mole        | [kg] or [mol]           |
	+--------------------------+--------------------+-------------------------+
	|   ``TEMPERATURE``        |   Temperature      |   [K]                   |
	+--------------------------+--------------------+-------------------------+
	|   ``PRESSURE``           |   Pressure         |   [Pa]                  |
	+--------------------------+--------------------+-------------------------+
	|   ``MOLAR_MASS``         |   Molar mass       |   [kg/mol]              |
	+--------------------------+--------------------+-------------------------+
	|   ``ENTHALPY``           |   Enthalpy         |   [J/kg/s] or [J/mol/s] |
	+--------------------------+--------------------+-------------------------+
	
|

.. code-block:: cpp

	double CalcTemperatureFromProperty(ECompoundTPProperties Property, double Time, double Value)

Function for both ``CMaterialStream`` and ``CHoldup``.

Returns temperature of the material stream / holdup for a specific value ``Value`` of the property ``Property`` at the time point ``Time``. Available properties are those defined in :ref:`material database <label-materialDataDetailed>`. 

For further information, please refer to :ref:`label-thermo` on this page.

|

.. code-block:: cpp

	double CalcPressureFromProperty(ECompoundTPProperties Property, double Time, double Value)

Function for both ``CMaterialStream`` and ``CHoldup``.
	
Returns pressure of the material stream / holdup for a specific value ``Value`` of the property ``Property`` at the time point ``Time``. Available properties are those defined in :ref:`material database <label-materialDataDetailed>`. 

For further information, please refer to :ref:`label-thermo` on this page.

|

Compounds
---------

.. code-block:: cpp

	double GetCompoundFraction(double Time , std::string CompoundKey, unsigned Basis)

Function for both ``CMaterialStream`` and ``CHoldup``.
	
Returns total fraction of the compound with key ``CompoundKey`` at the time point ``Time``. If such time point has not been defined, interpolation of data will be done.

Basis can be ``BASIS_MASS`` or ``BASIS_MOLL``.

``BASIS_MASS``: :math:`f_i = \sum \limits_i w_i \cdot f_{i}`, with :math:`f_i` the mass fraction of compound :math:`i`, and :math:`w_i` the mass fraction of phase :math:`i`.

``BASIS_MOLL``: :math:`f_i^{mol} = \sum \limits_i w_i \dfrac{f_i}{M_i \cdot \sum\limits_j \frac{f_{i,j}}{M_j}}`, with :math:`f_i^{mol}` the mole fraction of compound :math:`i`, :math:`f_{i,j}` the mass fraction of compound :math:`j` in phase :math:`i`, and :math:`M_j` the molar mass of compound :math:`j`.

|

.. code-block:: cpp

	double GetCompoundPhaseFraction(double Time, std::string CompoundKey, unsigned Phase, unsigned Basis)

Function for both ``CMaterialStream`` and ``CHoldup``.

Returns fraction of the compound with the key ``CompoundKey`` in the phase ``Phase`` (``SOA_SOLID``, ``SOA_LIQUID``, ``SOA_VAPOR``) for the time point ``Time``. If such time point has not been defined, interpolation of data will be done.

Basis can be ``BASIS_MASS`` or ``BASIS_MOLL``.

``BASIS_MASS``: :math:`f_{i,j}`, mass fraction of compound :math:`j` in phase :math:`i`.

``BASIS_MOLL``: :math:`f_{i,j}^{mol} = \sum \limits_i w_i \dfrac{f_{i,j}}{M_i \cdot \sum\limits_j \frac{f_{i,j}}{M_j}}`, with :math:`f_{i,j}^{mol}` the mole fraction of compound :math:`j` in phase :math:`i`, and :math:`M_j` the molar mass of compound :math:`j`.

|

.. code-block:: cpp

	void SetCompoundPhaseFraction (double Time, std::string CompoundKey, unsigned Phase, double Fraction, unsigned Basis)

Function for both ``CMaterialStream`` and ``CHoldup``.

Sets fraction of the compound with key ``CompoundKey`` in phase ``Phase`` (``SOA_SOLID``, ``SOA_LIQUID``, ``SOA_VAPOR``) for the time point ``Time``. If such time point has not been defined, nothing will be done. Negative values before setting will be converted to ``0``.

Basis can be ``BASIS_MASS`` or ``BASIS_MOLL``.

``BASIS_MASS``: in this case you have your input ``Fraction`` directly as mass fraction of compound :math:`j` in phase :math:`i`: :math:`f_{i,j} =` ``Fraction``.

``BASIS_MOLL``: :math:`f_{i,j} =` ``Fraction`` :math:`\cdot \dfrac{M_i}{\sum\limits_j \frac{f_{i,j}}{M_j}}`, with :math:`f_{i,j}^{mol}` the mole fraction of compound :math:`j` in phase :math:`i`, and :math:`M_j` the molar mass of compound :math:`j`.

|

.. code-block:: cpp

	double GetCompoundMassFlow(double Time, std::string CompoundKey, unsigned Phase, unsigned Basis)

Specific function for ``CMaterialStream``. 

Returns mass flow of the compound with key CompoundKey in phase Phase (``SOA_SOLID``, ``SOA_LIQUID``, ``SOA_VAPOR``) for the time point ``Time``. If such time point has not been defined, interpolation of data will be done. 

Basis is a basis of value (``BASIS_MASS`` in [kg/s] or ``BASIS_MOLL`` in [mol/s]).

``BASIS_MASS``: :math:`\dot{m}_{i,j} = w_i \cdot f_{i,j} \cdot \dot{m}`, with :math:`\dot m_{i,j}` the mass flow of compound :math:`j` in phase :math:`i`, :math:`w_i` the mass fraction of phase :math:`i`, and :math:`f_{i,j}` the mass fraction of compound :math:`j` in phase :math:`i`.

``BASIS_MOLL``: :math:`\dot{m}_{i,j} = w_i \cdot f_{i,j} \cdot \sum\limits_k \dfrac{\dot{m} \cdot w_k}{M_k}`, with :math:`\dot m` the total mass flow of the material stream, and :math:`M_k` the molar mass of phase :math:`k`.

|

.. code-block:: cpp

	double GetCompoundMass(double Time, std::string CompoundKey, unsigned Phase, unsigned Basis)

Specific function for ``CHoldup``. 

Returns mass of the compound with key CompoundKey in phase Phase (``SOA_SOLID``, ``SOA_LIQUID``, ``SOA_VAPOR``) for the time point ``Time``. If such time point has not been defined, interpolation of data will be done. 

Basis is a basis of value (``BASIS_MASS`` in [kg] or ``BASIS_MOLL`` in [mol]).

``BASIS_MASS``: :math:`m_{i,j} = w_i \cdot f_{i,j} \cdot m`, with :math:`m_{i,j}` the mass of compound :math:`j` in phase :math:`i`, :math:`w_i` the mass fraction of phase :math:`i`, and :math:`f_{i,j}` the mass fraction of compound :math:`j` in phase :math:`i`.

``BASIS_MOLL``: :math:`m_{i,j} = w_i \cdot f_{i,j} \cdot \sum\limits_k \dfrac{m \cdot w_k}{M_k}`, with :math:`m` the total mass of the holdup, and :math:`M_k` the molar mass of phase :math:`k`.

|

.. code-block:: cpp

	double GetCompoundConstant(std::string CompoundKey, ECompoundConstProperties ConstProperty)

Function for both ``CMaterialStream`` and ``CHoldup``.
	
Returns value of the constant physical property ``ConstProperty`` for the specified compound. These properties are stored in :ref:`material database <label-materialDataDetailed>`. Available constants are:
	
	-	``SOA_AT_NORMAL_CONDITIONS``

	-	``NORMAL_BOILING_POINT``
	
	-	``NORMAL_FREEZING_POINT``
	
	-	``CRITICAL_TEMPERATURE``
	
	-	``CRITICAL_PRESSURE``
	
	-	``MOLAR_MASS``
	
	-	``STANDARD_FORMATION_ENTHALPY``
	
	-	``HEAT_OF_FUSION_AT_NORMAL_FREEZING_POINT``
	
	-	``HEAT_OF_VAPORIZATION_AT_NORMAL_BOILING_POINT``
		
	-	``REACTIVITY_TYPE``
	
	-	``CONST_PROP_USER_DEFINED_XX``


.. Note:: Definition of constant properties for pure compounds:

	+----------------------------------------------------+------------------------------------------------+------------+
	|   Define                                           |   Name                                         |   Unit     |
	+====================================================+================================================+============+
	|   ``SOA_AT_NORMAL_CONDITIONS``                     |   State of aggregation at normal conditions    |   [-]      |
	+----------------------------------------------------+------------------------------------------------+------------+
	|   ``NORMAL_BOILING_POINT``                         |   Normal boiling point                         |   [K]      |
	+----------------------------------------------------+------------------------------------------------+------------+
	|   ``NORMAL_FREEZING_POINT``                        |   Normal freezing point                        |   [K]      |
	+----------------------------------------------------+------------------------------------------------+------------+
	|   ``CRITICAL_TEMPERATURE``                         |   Critical temperature                         |  [K]       |
	+----------------------------------------------------+------------------------------------------------+------------+
	|   ``CRITICAL_PRESSURE``                            |   Critical pressure                            |   [Pa]     |
	+----------------------------------------------------+------------------------------------------------+------------+
	|   ``MOLAR_MASS``                                   |   Molar mass                                   |   [kg/mol] |
	+----------------------------------------------------+------------------------------------------------+------------+
	|   ``STANDARD_FORMATION_ENTHALPY``                  |   Standard formation enthalpy                  |   [J/mol]  |
	+----------------------------------------------------+------------------------------------------------+------------+
	|   ``HEAT_OF_FUSION_AT_NORMAL_FREEZING_POINT``      |   Heat of fusion at normal freezing point      |   [J/mol]  |
	+----------------------------------------------------+------------------------------------------------+------------+
	|   ``HEAT_OF_VAPORIZATION_AT_NORMAL_BOILING_POINT`` |   Heat of vaporization at normal boiling point |   [J/mol]  |
	+----------------------------------------------------+------------------------------------------------+------------+
	|   ``REACTIVITY_TYPE``                              |   Reactivity type                              |   [-]      |
	+----------------------------------------------------+------------------------------------------------+------------+
	|   ``CONST_PROP_USER_DEFINED_XX``                   |   User defined property                        |   [-]      |
	+----------------------------------------------------+------------------------------------------------+------------+

|

.. code-block:: cpp

	double GetCompoundTPDProp(std::string CompoundKey, unsigned Property, double Temperature, double Pressure)

Function for both ``CMaterialStream`` and ``CHoldup``.
	
Returns value of the temperature / pressure-dependent physical Property (which are stored in the database of materials) for the compound with the specified ``Temperature`` in [K] and ``Pressure`` in [Pa]. Available properties are:

	-	``DENSITY``
	
	-	``HEAT_CAPACITY_CP``
	
	-	``VAPOR_PRESSURE``
	
	-	``VISCOSITY``
	
	-	``THERMAL_CONDUCTIVITY``
	
	-	``PERMITTIVITY``
	
	-	``ENTHALPY``
	
	-	``TP_PROP_USER_DEFINED_XX``


.. Note:: Definition of temperature-dependent compound properties:

	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   Define                      |   Name                      |   Unit                                                                    |
	+===============================+=============================+===========================================================================+
	|   ``DENSITY``                 |   Density                   |   [kg/m :math:`^3`]                                                       |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``HEAT_CAPACITY_CP``        |   Heat capacity :math:`C_p` |   [J/(kg·K)]                                                              |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``VAPOR_PRESSURE``          |   Vapor pressure            |   [Pa]                                                                    |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``VISCOSITY``               |   Viscosity                 |   [Pa·s]                                                                  |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``THERMAL_CONDUCTIVITY``    |   Thermal conductivity      |   [W/(m·K)]                                                               |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``PERMITTIVITY``            |   Permittivity              |   [F/m]                                                                   |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``ENTHALPY``                |   Enthalpy                  |   [J/kg/s] or [J/mol/s] for material stream, [J/kg] or [J/mol] for holdup |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``TP_PROP_USER_DEFINED_XX`` |   User defined property     |  [-]                                                                      |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+

|

.. code-block:: cpp

	double GetCompoundTPDProp(double Time, std::string CompoundKey, unsigned Property)
	
Function for both ``CMaterialStream`` and ``CHoldup``.

Returns value of the temperature / pressure-dependent physical ``Property`` (which are stored in the database of materials) for the compound with the current temperature and pressure. Available properties are:

	-	``DENSITY``
	
	-	``HEAT_CAPACITY_CP``
	
	-	``VAPOR_PRESSURE``
	
	-	``VISCOSITY``
	
	-	``THERMAL_CONDUCTIVITY``
	
	-	``PERMITTIVITY``
	
	-	``ENTHALPY``
	
	-	``TP_PROP_USER_DEFINED_XX``


.. Note:: Definition of temperature-dependent compound properties:

	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   Define                      |   Name                      |   Unit                                                                    |
	+===============================+=============================+===========================================================================+
	|   ``DENSITY``                 |   Density                   |   [kg/m :math:`^3`]                                                       |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``HEAT_CAPACITY_CP``        |   Heat capacity :math:`C_p` |   [J/(kg·K)]                                                              |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``VAPOR_PRESSURE``          |   Vapor pressure            |   [Pa]                                                                    |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``VISCOSITY``               |   Viscosity                 |   [Pa·s]                                                                  |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``THERMAL_CONDUCTIVITY``    |   Thermal conductivity      |   [W/(m·K)]                                                               |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``PERMITTIVITY``            |   Permittivity              |   [F/m]                                                                   |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``ENTHALPY``                |   Enthalpy                  |   [J/kg/s] or [J/mol/s] for material stream, [J/kg] or [J/mol] for holdup |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``TP_PROP_USER_DEFINED_XX`` |   User defined property     |  [-]                                                                      |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+

|

.. code-block:: cpp

	double GetCompoundInteractionProp(std::string CompoundKey1, std::string CompoundKey2, unsigned Property, double Temperature, double Pressure)
	
Function for both ``CMaterialStream`` and ``CHoldup``.

Returns the value of the interaction property ``Property`` for the selected compounds under the specified ``Temperature`` in [K] and ``Pressure`` in [Pa]. These properties are stored in the :ref:`material database <label-materialDataDetailed>`. Available properties are:

	-	``INTERFACE_TENSION``
	
	-	``INT_PROP_USER_DEFINED_XX``
	
.. Note:: Definition of interaction properties between two pure compounds:
	
	+--------------------------------+-------------------------+---------+
	|   Define                       |   Name                  |   Unit  |
	+================================+=========================+=========+
	|   ``INTERFACE_TENSION``        |   Interface tension     |   [N/m] |
	+--------------------------------+-------------------------+---------+
	|   ``INT_PROP_USER_DEFINED_XX`` |   User defined property |   [-]   |
	+--------------------------------+-------------------------+---------+	

|

.. code-block:: cpp

	double GetCompoundInteractionProp(double Time, std::string CompoundKey1, std::string CompoundKey2, unsigned Property)
	
Function for both ``CMaterialStream`` and ``CHoldup``.

Returns the value of the interaction property ``Property`` for the selected compounds under the current temperature and pressure. These properties are stored in the :ref:`material database <label-materialDataDetailed>`. Available properties are:

	-	``INTERFACE_TENSION``
	
	-	``INT_PROP_USER_DEFINED_XX``
	
.. Note:: Definition of interaction properties between two pure compounds:
	
	+--------------------------------+-------------------------+---------+
	|   Define                       |   Name                  |   Unit  |
	+================================+=========================+=========+
	|   ``INTERFACE_TENSION``        |   Interface tension     |   [N/m] |
	+--------------------------------+-------------------------+---------+
	|   ``INT_PROP_USER_DEFINED_XX`` |   User defined property |   [-]   |
	+--------------------------------+-------------------------+---------+	

|

Phases
------

.. code-block:: cpp

	double GetPhaseMassFlow(double Time, unsigned Phase, unsigned Basis = BASIS_MASS)

Specific function for ``CMaterialStream``. 

Returns mass flow of the specified phase ``Phase`` (``SOA_SOLID``, ``SOA_LIQUID``, ``SOA_VAPOR``) in the material stream for the time point ``Time``. If such time point has not been defined, the value will be interpolated. 

Basis is a basis of value (``BASIS_MASS`` in [kg/s] or ``BASIS_MOLL`` in [mol/s]).

``BASIS_MASS``: :math:`\dot{m}_i = \dot{m} \cdot w_i`, with :math:`\dot{m}_i` the mass flow of phase :math:`i`, :math:`w_i` the mass fraction of phase :math:`i`, and :math:`\dot{m}` the total mass flow of the material stream.

``BASIS_MOLL``: :math:`\dot{n}_i = \dfrac{\dot{m} \cdot w_i}{M_i}`, with :math:`\dot{n}_i` the mole flow of phase :math:`i`, :math:`w_i` the mass fraction of phase :math:`i`, :math:`\dot{m}` the total mass flow of the material stream, and :math:`M_i` the molar mass of phase :math:`i`.

|

.. code-block:: cpp

	double GetPhaseMass(double Time, unsigned Phase, unsigned Basis = BASIS_MASS)

Specific function for ``CHoldup``. 

Returns mass of the specified phase ``Phase`` (``SOA_SOLID``, ``SOA_LIQUID``, ``SOA_VAPOR``) in the holdup for the time point ``Time``. If such time point has not been defined, the value will be interpolated.

Basis is a basis of value (``BASIS_MASS`` in [kg] or ``BASIS_MOLL`` in [mol]).

``BASIS_MASS``: :math:`m_i = m \cdot w_i`, with :math:`m_i` the mass of phase :math:`i`, :math:`w_i` the mass fraction of phase :math:`i`, and :math:`m` the total mass flow of the material stream.

``BASIS_MOLL``: :math:`n_i = \dfrac{m \cdot w_i}{M_i}`, with :math:`n_i` the mole of phase :math:`i`, :math:`w_i` the mass fraction of phase :math:`i`, :math:`m` the total mass flow of the material stream, and :math:`M_i` the molar mass of phase :math:`i`.

|

.. _label-old_setPhaseMassFlow:

.. code-block:: cpp
	
	void SetPhaseMassFlow(double Time, unsigned Phase, double Value, unsigned Basis)

Specific function for ``CMaterialStream``. 
	
Sets mass flow of the specified phase ``Phase`` (``SOA_SOLID``, ``SOA_LIQUID``, ``SOA_VAPOR``) in the material stream for the time point ``Time``. 

Is performed by calculation and setting of a new total mass flow of the material stream and new phase fractions (according to the new mass flow of the specified phase). Negative values before setting will be converted to ``0``. If there is no specified time point or phase in the material stream, the value will not be set. 

Basis is a basis of value (``BASIS_MASS`` in [kg/s] or ``BASIS_MOLL`` in [mol/s]).

``BASIS_MASS``: in this case you have your input ``Value`` as mass flow of one defined phase: :math:`\dot m_i =` ``Value`` and :math:`w_i = \dot m_i / \dot m`. Meanwhile, the total mass flow :math:`\dot m` changes due to assignment for :math:`\dot m_i`: :math:`\dot m = \dot m_{old} + (` ``Value`` :math:`- \dot{m}_{i,old})`. Here :math:`\dot{m}_i` stands for the mass flow of phase :math:`i`, :math:`w_i` for the mass fraction of phase :math:`i`, and :math:`\dot{m}` for the total mass flow of the material stream.

``BASIS_MOLL``: in this case you have your input ``Value`` as mole flow of one defined phase: :math:`\dot m_i =` ``Value`` :math:`\cdot M_i ` and :math:`w_i = \dot m_i / \dot m`. Meanwhile, the total mass flow :math:`\dot m` changes due to assignment for :math:`\dot m_i`: :math:`\dot m = \dot m_{old} + (` ``Value`` :math:`\cdot M_i - \dot{m}_{i,old})`. Here :math:`w_i` stands for the mass fraction of phase :math:`i`, :math:`\dot{m}` for the total mass flow of the material stream, and :math:`M_i` for the molar mass of phase :math:`i`.

|

.. _label-old_setPhaseMass:

.. code-block:: cpp
	
	void SetPhaseMass(double Time, unsigned Phase, double Value, unsigned Basis)

Specific function for ``CHoldup``. 
	
Sets mass of the specified phase ``Phase`` (``SOA_SOLID``, ``SOA_LIQUID``, ``SOA_VAPOR``) in the holdup for the time point ``Time``. 

Is performed by calculation and setting of a new total mass flow of the holdup and new phase fractions (according to the new mass of the specified phase). Negative values before setting will be converted to ``0``. If there is no specified time point or phase in the holdup, the value will not be set. 

Basis is a basis of value (``BASIS_MASS`` in [kg] or ``BASIS_MOLL`` in [mol]).

``BASIS_MASS``: in this case you have your input ``Value`` as the mass of one defined phase: :math:`m_i =` ``Value`` and :math:`w_i = m_i / m`. Meanwhile, the total mass :math:`m` changes due to assignment for :math:`m_i`: :math:`\dot m = \dot m_{old} + (` ``Value`` :math:`- \dot{m}_{i,old})`. Here :math:`m_i` stands for the mass of phase :math:`i`, :math:`w_i` for the mass fraction of phase :math:`i`, and :math:`m` for the total mass of the holdup. 

``BASIS_MOLL``: in this case you have your input ``Value`` as mole flow of one defined phase: :math:`m_i = ` ``Value`` :math:`\cdot M_i` , :math:`w_i = m_i / m`. Meanwhile, the total mass :math:`m` changes due to assignment for :math:`m_i`: :math:`m = m_{old} + (` ``Value`` :math:`\cdot M_i - m_{i,old})`. Here :math:`m_i` stands for the mass of phase :math:`i`, :math:`w_i` for the mass fraction of phase :math:`i`, :math:`m` for the total mass of the holdup, and :math:`M_i` for the molar mass of phase :math:`i`.

|

.. code-block:: cpp
	
	double GetSinglePhaseProp(double Time, unsigned Property, unsigned Phase, unsigned Basis)

Function for both ``CMaterialStream`` and ``CHoldup``.

Returns non-constant physical property value for the phase mixture ``Phase`` (``SOA_SOLID``, ``SOA_LIQUID``, ``SOA_VAPOR``) for the specified time point. If such time point has not been defined, interpolation of data will be done. 

Basis is a basis of results (``BASIS_MASS`` or ``BASIS_MOLL``).

``Property`` is an identifier of a physical property. Available properties are:

	-	``FLOW``: only for class ``CMaterialStream``. Refer to function :ref:`getMassFlow <label-massFlow>`.
	
	-	``MASS``: only for class ``CHoldup``. Refer to function :ref:`getMass <label-mass>`.
	
	-	``TEMPERATURE``: refer to function :ref:`getTemperature <label-temp>`.
	
	-	``PRESSURE``: refer to function :ref:`getPressure <label-pressure>`.
	
	-	``PHASE_FRACTION``, ``FRACTION``: 
		
		- ``BASIS_MASS``: function returns :math:`w_i`, mass fraction of phase :math:`i`.
		
		- ``BASIS_MOLL``: function returns result of :math:`\left ( \dfrac{w_i}{M_i \cdot \sum\limits_j \frac{w_j}{M_j}} \right )`, with :math:`w_i` the mass fraction of phase :math:`i`, and :math:`M_i` the molar mass of phase :math:`i`.
	
	-	``MOLAR_MASS``: calculate the molar mass of the phase :math:`M` by :math:`\left ( \frac{1}{M} = \sum\limits_i \frac{w_i}{M_i} \right )`, with :math:`M_i` the molar mass of phase :math:`i`, and :math:`w_i` the mass fraction of phase :math:`i`.
	
	-	``DENSITY``: refer to function :ref:`getPhaseTPDProp <label-getPhaseTPD>`.
	
	-	``HEAT_CAPACITY_CP``: refer to function :ref:`getPhaseTPDProp <label-getPhaseTPD>`.
	
	-	``THERMAL_CONDUCTIVITY``: refer to function :ref:`getPhaseTPDProp <label-getPhaseTPD>`.
	
	-	``VISCOSITY``: refer to function :ref:`getPhaseTPDProp <label-getPhaseTPD>`.
	
	-	``VAPOR_PRESSURE``: refer to function :ref:`getPhaseTPDProp <label-getPhaseTPD>`.
	
	-	``PERMITTIVITY``: refer to function :ref:`getPhaseTPDProp <label-getPhaseTPD>`.
	
	-	``TP_PROP_USER_DEFINED_XX``: refer to function :ref:`getPhaseTPDProp <label-getPhaseTPD>`.
	
	-	``ENTHALPY``:
		
		For solid and liquid phase: :math:`h = h_0 + C_p \cdot \Delta T + \frac{M}{\rho} (P - P_0)`
		
		- ``BASIS_MASS``: :math:`H = \sum\limits_i \frac{h_i \cdot f_i}{M_i}`
		
		- ``BASIS_MOLL``: :math:`H = \sum\limits_i h_i \cdot f_i`
		
		For vapor phase: :math:`h = h_0 + C_p \cdot \Delta T`
		
		- ``BASIS_MASS``: :math:`H = \sum\limits_i \frac{h_i \cdot f_i}{M_i}`
		
		- ``BASIS_MOLL``: :math:`H = \sum\limits_i h_i \cdot f_i`
		
		.. Note:: Notations for enthalpy:
		
			:math:`H` – enthalpy of the phase. [J/kg/s] or [J/mol/s] for material stream; [J/kg] or [J/mol] for holdup
			
			:math:`h_i` – enthalpy of the compound :math:`i` [J/mol]
			
			:math:`f_i` – mass fraction of the compound :math:`i` in phase
			
			:math:`M_i` – molar mass of the compound :math:`i`
			
			:math:`h_0` – formation enthalpy [J/mol]
			
			:math:`C_p` – heat capacity for constant pressure of the compound
			
			:math:`\Delta T` – difference between the temperature at normal conditions (298.15 K) and current temperature
			
			:math:`P` – current pressure
			
			:math:`P_0` – pressure at normal conditions (101325 Pa)

|

.. Note:: Definition of single-phase mixture properties:

	+------------------------------------+-----------------------------+--------------------------------------------------------------------------+
	|   Define                           |   Name                      |   Unit                                                                   |
	+====================================+=============================+==========================================================================+
	|   ``FLOW``                         |   Mass flow                 |   [kg/s] or [mol/s]                                                      |
	+------------------------------------+-----------------------------+--------------------------------------------------------------------------+
	| ``MASS``                           | Mass                        | [kg] or [mol]                                                            |
	+------------------------------------+-----------------------------+--------------------------------------------------------------------------+
	|   ``TEMPERATURE``                  |   Temperature               |   [K]                                                                    |
	+------------------------------------+-----------------------------+--------------------------------------------------------------------------+
	|   ``PRESSURE``                     |   Pressure                  |   [Pa]                                                                   |
	+------------------------------------+-----------------------------+--------------------------------------------------------------------------+
	|   ``PHASE_FRACTION``, ``FRACTION`` |   Phase fraction            |   [-]                                                                    |
	+------------------------------------+-----------------------------+--------------------------------------------------------------------------+
	|   ``MOLAR_MASS``                   |   Molar mass                |   [kg/mol]                                                               |
	+------------------------------------+-----------------------------+--------------------------------------------------------------------------+
	|   ``DENSITY``                      |   Density                   |   [kg/m :math:`^3`]                                                      |
	+------------------------------------+-----------------------------+--------------------------------------------------------------------------+
	|   ``HEAT_CAPACITY_CP``             |   Heat capacity :math:`C_p` |   [J/(kg·K)]                                                             |
	+------------------------------------+-----------------------------+--------------------------------------------------------------------------+
	|   ``THERMAL_CONDUCTIVITY``         |   Thermal conductivity      |   [W/(m·K)]                                                              |
	+------------------------------------+-----------------------------+--------------------------------------------------------------------------+
	|  ``VISCOSITY``                     |   Viscosity                 |   [Pa·s]                                                                 |
	+------------------------------------+-----------------------------+--------------------------------------------------------------------------+
	|  ``VAPOR_PRESSURE``                |   Vapor pressure            |   [Pa]                                                                   |
	+------------------------------------+-----------------------------+--------------------------------------------------------------------------+
	|   ``ENTHALPY``                     |   Enthalpy                  |   [J/kg/s]or [J/mol/s] for material stream, [J/kg] or [J/mol] for holdup |
	+------------------------------------+-----------------------------+--------------------------------------------------------------------------+
	|   ``PERMITTIVITY``                 |   Permittivity              |   [F/m]                                                                  |
	+------------------------------------+-----------------------------+--------------------------------------------------------------------------+
	|   ``TP_PROP_USER_DEFINED_XX``      |   User defined property     |   [-]                                                                    |
	+------------------------------------+-----------------------------+--------------------------------------------------------------------------+

|

.. code-block:: cpp

	void SetSinglePhaseProp(double Time, unsigned Property, unsigned Phase, double Value, unsigned Basis)

Function for both ``CMaterialStream`` and ``CHoldup``.
	
Sets non-constant physical property value for phase mixture ``Phase`` (``SOA_SOLID``, ``SOA_LIQUID``, ``SOA_VAPOR``) for the specified time point ``Time``. If there is no specified time point or phase in the material stream or holdup, the value will not be set. 

``Property`` is an identifier of a physical property. Available properties are:

	-	``FLOW``: only for class ``CMaterialStream``. Refer to function :ref:`setPhaseMassFlow <label-setPhaseMassFlow>`.
	
	-	``MASS``: only for class ``CHoldup``. Refer to function :ref:`setPhaseMass <label-setPhaseMass>`.
	
	-	``FRACTION``: mass fraction of the ``Phase`` is set to ``Value``.


Basis is a basis of value (``BASIS_MASS`` or ``BASIS_MOLL``).

|

.. _label-old_getPhaseTPD:

.. code-block:: cpp

		double GetPhaseTPDProp(double Time, unsigned Property, unsigned Phase)

Function for both ``CMaterialStream`` and ``CHoldup``.
		
Returns value of temperature / pressure-dependent physical property for specified phase (``SOA_SOLID``, ``SOA_LIQUID``, ``SOA_VAPOR``) for the time point ``Time``. If such time point has not been defined, interpolation of data will be done.

Available properties are:

	-	``DENSITY``:
		
		- For solid phase: is calculated by :math:`\rho = \sum\limits_{i,j} \rho_i \, (1 - \varepsilon_j)\,f_{i,j}`, with :math:`\varepsilon_j` the porosity in interval :math:`j`, and :math:`f_{i,j}` the mass fraction of compound :math:`i` with porosity :math:`j`.
		
		- For liquid and vapor phase: is calculated by :math:`\frac{1}{\rho} = \sum\limits_i \frac{w_i}{\rho_i}`, with :math:`w_i` the mass fraction of compound :math:`i` in ``Phase``.
	
	-	``HEAT_CAPACITY_CP``: is calculated by :math:`C_p = \sum\limits_i w_i \cdot C_{p,i}`, with :math:`C_{p,i}` the heat capacity of compound :math:`i`, and :math:`w_i` the mass fraction of compound :math:`i` in ``Phase``.
	
	-	``VAPOR_PRESSURE``: is calculated by :math:`P_v = \min\limits_{i} (P_v)_i`, with :math:`(P_v)_i` vapor pressure of compound :math:`i`.
	
	-	``VISCOSITY``: 
		
		- For solid phase: is calculated by :math:`\eta = \sum\limits_i w_i\, \eta_i`, with :math:`\eta_i` the viscosity of compound :math:`i`, and :math:`w_i` the mass fraction of compound :math:`i`.
		
		- For liquid phase: is calculated by :math:`\ln \eta = \dfrac{\sum\limits_i w_i\,\ln \eta_i}{\sum\limits_i x_i\,\sqrt{M_i}}`, with :math:`\eta_i` the viscosity of compound :math:`i`, :math:`w_i` the mass fraction of compound :math:`i` in `Phase` and :math:`x_i` the mole fraction of compound :math:`i` in ``Phase``.
		
		
		- For vapor phase: :math:`\eta = \dfrac{\sum\limits_i x_i\,\sqrt{M_i}\,\eta_i}{\sum\limits_i x_i\,\sqrt{M_i}}`, with :math:`\eta_i` the viscosity of compound :math:`i`, :math:`w_i` the mass fraction of compound :math:`i` in `Phase`, and :math:`x_i` the mole fraction of compound :math:`i` in ``Phase``.
		
	
	-	``THERMAL_CONDUCTIVITY``:
	
		- For solid phase: is calculated by :math:`\lambda = \sum\limits_i w_i \, \lambda_i`, with :math:`\lambda_i` the thermal conductivity of compound :math:`i`.
		
		- For liquid phase: is calculated by :math:`\lambda = \dfrac{1}{\sqrt{\sum\limits_i x_i \, \lambda_i^{-2}}}`, with :math:`\lambda_i` the thermal conductivity of compound :math:`i`.
		
		- For vapor phase: is calculated by :math:`\lambda = \sum\limits_i \dfrac{x_i\,\lambda_i}{\sum\limits_j x_j\, F_{i,j}}`, :math:`F_{i,j} = \frac{(1 + \sqrt{\lambda_i^4 / \lambda_j} \sqrt{M_j / M_i})^2}{\sqrt{8(1 + M_i / M_j)}}`. With :math:`M_i` the molar mass of compound :math:`i`.

	-	``PERMITTIVITY``: is calculated by :math:`\varepsilon = \sum\limits_i w_i\,\varepsilon_i`, with :math:`\varepsilon_i` the permittivity of compound :math:`i`, and :math:`w_i` the mass fraction of compound :math:`i` in ``Phase``.
	
	-	``ENTHALPY``: is calculated by :math:`H = \sum\limits_i w_i\,H_i`, with :math:`H_i` the enthalpy of compound :math:`i`, and :math:`w_i` the mass fraction of compound :math:`i` in ``Phase``.
	
	-	``TP_PROP_USER_DEFINED_XX``: is calculated by :math:`Y = \sum\limits_i w_i\,Y_i`, with :math:`Y_i` the property value of compound :math:`i`, and :math:`w_i` the mass fraction of compound :math:`i` in ``Phase``.

.. Note:: Definition of temperature-dependent compound properties:

	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   Define                      |   Name                      |   Unit                                                                    |
	+===============================+=============================+===========================================================================+
	|   ``DENSITY``                 |   Density                   |   [kg/m :math:`^3`]                                                       |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``HEAT_CAPACITY_CP``        |   Heat capacity :math:`C_p` |   [J/(kg·K)]                                                              |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``VAPOR_PRESSURE``          |   Vapor pressure            |   [Pa]                                                                    |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``VISCOSITY``               |   Viscosity                 |   [Pa·s]                                                                  |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``THERMAL_CONDUCTIVITY``    |   Thermal conductivity      |   [W/(m·K)]                                                               |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``PERMITTIVITY``            |   Permittivity              |   [F/m]                                                                   |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``ENTHALPY``                |   Enthalpy                  |   [J/kg/s] or [J/mol/s] for material stream, [J/kg] or [J/mol] for holdup |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``TP_PROP_USER_DEFINED_XX`` |   User defined property     |  [-]                                                                      |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+

|

Solid distributed properties
----------------------------

All functions in this section are for both ``CMaterialStream`` and ``CHoldup``.

.. code-block:: cpp

	double GetFraction(double Time, std::vector<unsigned> Coords)

Returns solid mass fraction by specified coordinates according to all defined distributions. If such time point has not been defined, interpolation of data will be done.

|

.. code-block:: cpp

	void SetFraction(double Time, std::vector<unsigned> Coords, double Value)
	
Sets solid mass fraction by specified coordinates according to all defined distributions. If such time point has not been defined in the material stream / holdup, nothing will be done. 

Direct setting of fractions to the material stream / holdup leads to a change of all dependent distributions. Approach with transformation matrix should be used to avoid this.

|

.. code-block:: cpp

	bool GetDistribution(double Time, EDistrTypes Dim, std::vector<double>& Result)
	
Returns vector of distributed property for specified time point ``Time`` and dimension ``Dim``. If such time point has not been defined in the material stream / holdup, then linear interpolation will be used to obtain data. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool GetDistribution(double Time, EDistrTypes Dim1, EDistrTypes Dim2, CDense2DMatrix& Result)
	
Returns matrix of two distributed dependent properties ``Dim1`` and ``Dim2`` for the specified time point ``Time``. 

If such time point has not been defined in the material stream / holdup, then linear interpolation will be used to obtain data. Rows of resulting matrix will correspond to ``Dim1``, columns – to ``Dim2``. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool GetDistribution(double Time, std::vector<EDistrTypes> Dims, CDenseMDMatrix& Result)

Returns multidimensional matrix of distributed dependent properties for specified time point ``Time`` and dimensions ``Dims``. If such time point has not been defined in the material stream / holdup, then linear interpolation will be used to obtain data. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool GetDistribution(double Time, EDistrTypes Dim, std::string Compound, std::vector<double>& Result)
	
Returns vector of distributed property for specified time point ``Time``, dimension ``Dim`` and compound ``Compound``. 

Input dimensions should not include distribution by compounds (``DISTR_COMPOUNDS``). If specified compound has not been defined in the material stream / holdup, nothing will be done. If specified time point has not been defined, then linear interpolation will be used to obtain data. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool GetDistribution(double Time, EDistrTypes Dim1, EDistrTypes Dim2, std::string Compound, CDense2DMatrix& 2DResult)
	
Returns matrix of two distributed dependent properties ``Dim1`` and ``Dim2`` for specified compound ``Compound`` and time point ``Time``. 

Input dimensions should not include distribution by compounds (``DISTR_COMPOUNDS``). If specified compound has not been defined in the material stream / holdup, nothing will be done. If specified time point has not been defined, then linear interpolation will be used to obtain data. Rows of resulting matrix will correspond to ``Dim1``, columns to ``Dim2``. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool GetDistribution(double Time, std::vector<EDistrTypes> Dims, std::string Compound, CDenseMDMatrix& MDResult)
	
Returns multidimensional matrix of distributed dependent properties for specified time point ``Time``, dimensions ``Dims`` and compound ``Compound``.

Input dimensions should not include distribution by compounds (``DISTR_COMPOUNDS``). If specified compound has not been defined in the material stream / holdup, nothing will be done. If specified time point has not been defined, then linear interpolation will be used to obtain data. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetDistribution(double Time, EDistrTypes Dim, std::vector<double> Distr)
	
Sets distributed property ``Distr`` of type ``Dim`` for specified time point ``Time``. 

If such time point or dimension doesn’t exist, nothing will be done. Returns ``false`` on error. 

Direct setting of distribution to the material stream / holdup leads to a change of all dependent distributions. Approach with transformation matrix should be used to avoid this.

|

.. code-block:: cpp

	bool SetDistribution(double Time, EDistrTypes Dim1, EDistrTypes Dim2, CDense2DMatrixDistr)
	
Sets matrix ``Distr`` of two dependent distributed properties of types ``Dim1`` and ``Dim2`` for specified time point ``Time``. If such time point or dimensions don’t exist nothing will be done. 

Returns ``false`` on error. 

Direct setting of distribution to the material stream / holdup leads to a change of all dependent distributions. Approach with transformation matrix should be used to avoid this.

|

.. code-block:: cpp

	bool SetDistribution(double Time,  CDenseMDMatrix Distr)
	
Sets multidimensional matrix ``Distr`` of dependent distributed properties for specified time point ``Time``. If such time point or dimensions, which are specified in ``Distr``, don’t exist, nothing will be done. 

Returns ``false`` on error. 

Direct setting of distribution to the material stream / holdup leads to a change of all dependent distributions. Approach with transformation matrix should be used to avoid this.

|

.. code-block:: cpp

	bool SetDistribution(double Time, EDistrTypes Dim, std::string Compound, std::vector<double> Distr)
	
Sets distributed property ``Distr`` of type ``Dim`` for specified compound ``Compound`` and time point ``Time``. If such time point, compound or dimension doesn’t exist, nothing will be done. Input dimensions should not include distribution by compounds (``DISTR_COMPOUNDS``). 

Returns ``false`` on error. 

Direct setting of distribution to the holdup leads to a change of all dependent distributions. Approach with transformation matrix should be used to avoid this.

|

.. code-block:: cpp

	bool SetDistribution(double Time, EDistrTypes Dim1, EDistrTypes Dim2, std::string Compound, CDense2DMatrix 2DDistr)
	
Sets matrix ``2DDistr`` of two dependent distributed properties of types ``Dim1`` and ``Dim2`` for specified compound ``Compound`` and time point ``Time``. If such time point, compound or dimensions don’t exist, nothing will be done. Input dimensions should not include distribution by compounds (``DISTR_COMPOUNDS``). 

Returns ``false`` on error. 

Direct setting of distribution to the holdup leads to a change of all dependent distributions. Approach with transformation matrix should be used to avoid this.

|

.. code-block:: cpp

	bool SetDistribution(double Time, std::string Compound, CDenseMDMatrix MDDistr)

Sets multidimensional matrix ``MDDistr`` of dependent distributed properties for specified compound ``Compound`` and time point ``Time``. If such time point, compound or dimensions, which are specified in ``MDDistr``, don’t exist, nothing will be done. Input dimensions should not include distribution by compounds (``DISTR_COMPOUNDS``). 

Returns ``false`` on error. 

Direct setting of distribution to the holdup leads to a change of all dependent distributions. Approach with transformation matrix should be used to avoid this.

|

.. code-block:: cpp

	bool ApplyTM(double Time, CTransformMatrix Transformation)

Transforms matrix of distributed parameters of solids for time point ``Time`` by applying a movement matrix ``Transformation``. Returns ``true`` if the transformation was successful.

|

.. code-block:: cpp

	bool ApplyTM (double Time, std::string Compound, CTransformMatrix Transformation)

Transforms matrix of distributed parameters of solids for specified compound ``Compound`` and time point ``Time`` by applying a movement matrix ``Transformation``. Dimensions of transformation matrix should not include distribution by compounds (``DISTR_COMPOUNDS``). Returns ``true`` if the transformation was successful.

|

.. code-block:: cpp

	void NormalizeDistribution(double Time)
	
Normalizes data in solid distribution matrix at the specified time point ``Time``. If ``Time`` has not been defined, nothing will be done.

|

.. code-block:: cpp

	void NormalizeDistribution(double Start, double End)
	
Normalizes data in solid distribution matrix in each time point from interval [``Start``; ``End``]. 

|

.. code-block:: cpp

	void NormalizeDistribution()
	
Normalizes data in solid distribution matrix in all defined time points.

|

Praticle size distribution
--------------------------

All functions in this section are for both ``CMaterialStream`` and ``CHoldup``.

.. _label-old_getPSD:

.. code-block:: cpp

	std::vector<double> GetPSD(double Time, EPSDType PSDType, EPSDGridType PSDGridType)
	
Returns particle size distribution of the total mixture of the solid phase at the time point ``Time``. 

``PSDGridType`` defines grid units if needed: ``EPSDGridType::DIAMETER`` for diameter in [m]; or ``EPSDGridType::VOLUME`` for volume in [m :math:`^3`]. 

``PSDType`` is a type of distribution. Available types are: ``PSD_q0``, ``PSD_Q0``, ``PSD_q2``, ``PSD_Q2``, ``PSD_q3``, ``PSD_Q3``, ``PSD_MassFrac``, ``PSD_Number``. 

PSD data is originally stored in a form of mass fractions and all transformations are performed by the following equations:

- ``PSD_q0``: number-related distribution of particles: :math:`q_{0,i} = \dfrac{N_i}{N_{tot} \cdot \Delta d_i}`;

- ``PSD_Q0``: :math:`Q_{0,i} = Q_{0,i-1} + q_{0,i} \cdot \Delta d_i`

- ``PSD_q2``: surface-area-related distribution of particles: :math:`q_{2,i} = \dfrac{Q_{2,i} - Q_{2,i-1}}{\Delta d_i}`

- ``PSD_Q2``: :math:`Q_{3,i} = \dfrac{\sum\limits_{j=0}^i N_j\,\pi\,d_j^2}{\sum\limits_j N_j \,\pi d_j^2}`

- ``PSD_q3``: :math:`q_{3,i} = w_i / \Delta d_i`

- ``PSD_Q3``: :math:`Q_{3,0} = w_0`, :math:`Q_{3,i} = Q_{3,i-1} + w_i`

- ``PSD_MassFrac``: returns the size distribution in the form of mass fractions with the total sum of 1.

- ``PSD_Number``: obtains number-related distribution of particles depends on several conditions. Three cases of calculation can be distinguished:

	1. If only one compound is specified: :math:`N_i = \dfrac{m_i}{\rho \, \frac{\pi}{6}\, d_i^3}`.
	
	2. For several compounds: :math:`N_i = \sum\limits_j \dfrac{M_{tot} \cdot w_{i,j}}{\frac{\pi \cdot d_i^3}{6} \cdot \rho_j}`.
	
	3. If distribution by particle porosity has been defined: :math:`N_i = \sum\limits_j N_{i,j}`, with 
	:math:`N_{i,j} = \sum\limits_k \dfrac{M_{tot} \cdot w_{i,j,k}}{\frac{\pi \cdot d_i^3}{6} \cdot \rho_j \cdot (1 - \varepsilon_k)}`.

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

.. code-block:: cpp

	std::vector<double> GetPSD(double Time, EPSDType PSDType, std::string Compound, EPSDGridType PSDGridType)
	
Returns particle size distribution of compound ``Compound`` of the solid phase of the material stream / holdup at the time point ``Time``. 

``PSDType`` is a type of distribution, please refer to function :ref:`getPSD(Time, PSDType, PSDGridType) <label-getPSD>` for detailed information.

``PSDGridType`` defines grid units if needed: ``EPSDGridType::DIAMETER`` for diameter in [m]; or ``EPSDGridType::VOLUME`` for volume in [m :math:`^3`]. 

|

.. code-block:: cpp

	void SetPSD(double Time, EPSDType PSDType, std::vector<double> PSD, EPSDGridType PSDGridType)
	
Sets particle size distribution with type ··PSDType·· to the solid phase of the material stream / holdup for time point ``Time``. 

Direct setting of :abbr:`PSD (Particle size distribution)` to the material stream / holdup leads to a change of all dependent distributions. Approach with transformation matrix should be used to avoid this. 

Available ``PSDType`` are: ``PSD_q0``, ``PSD_Q0``, ``PSD_q2``, ``PSD_Q2``, ``PSD_q3``, ``PSD_Q3``, ``PSD_MassFrac``, ``PSD_Number``. 

Please note that when using ``PSD_Number``, if the total mass of the particles given in the number distribution differs from the material stream / holdup’s total particle mass, the number will be normalized to match the material stream / holdup’s particle mass. 

As mass fractions are used to store data, :abbr:`PSD (Particle size distribution)` will be converted using functions ``Convertq0ToMassFractions()``, ``ConvertQ0ToMassFractions()``, ``Convertq3ToMassFractions()``, ``ConvertQ3ToMassFractions()``, ``ConvertNumbersToMassFractions()``, please refer to section :ref:`label-PSD` for detailed explanation about those functions.

``PSDGridType`` defines grid units if needed: ``EPSDGridType::DIAMETER`` for diameter in [m]; or ``EPSDGridType::VOLUME`` for volume in [m :math:`^3`]. 

|

.. code-block:: cpp

	void SetPSD(double Time, EPSDType PSDType, std::string Compound, std::vector<double> PSD, EPSDGridType PSDGridType)
	
Sets :abbr:`PSD (Particle size distribution)` with type ``PSDType`` for the specific compound ``Compound`` to the solid phase of the material stream / holdup for time point Time. 

Direct setting of :abbr:`PSD (Particle size distribution)` to the material stream / holdup leads to a change of all dependent distributions. Approach with transformation matrix should be used to avoid this. 

Available ``PSDType`` are: ``PSD_q0``, ``PSD_Q0``, ``PSD_q2``, ``PSD_Q2``, ``PSD_q3``, ``PSD_Q3``, ``PSD_MassFrac``, ``PSD_Number``. 

Please note that when using PSD_Number, If the total mass of the particles given in the number distribution differs from the material stream / holdup’s total particle mass, the number will be normalized to match the material stream/holdup’s particle mass. 

As mass fractions are used to store data, :abbr:`PSD (Particle size distribution)` will be converted using functions ``Convertq0ToMassFractions()``, ``ConvertQ0ToMassFractions()``, ``Convertq3ToMassFractions()``, ``ConvertQ3ToMassFractions()``, ``ConvertNumbersToMassFractions()``, please refer to section :ref:`label-PSD` for detailed explanation about those functions.

``PSDGridType`` defines grid units if needed: ``EPSDGridType::DIAMETER`` for diameter in [m]; or ``EPSDGridType::VOLUME`` for volume in [m :math:`^3`]. 

|

Lookup tables
-------------

All functions in this section are for both ``CMaterialStream`` and ``CHoldup``.

.. code-block:: cpp

	CLookupTable* GetLookupTable(ECompoundTPProperties Property, EDependencyTypes DependencyType, double Time)
	
Creates (if not yet exists), fills with compounds fractions and returns a corresponding lookup table for the specified ``Property`` (see table at the end of this section), ``DependencyType`` (``DEPENDENCE_TEMP`` or ``DEPENDENCE_PRES``) and ``Time``. 

For more information, please refer to section :ref:`label-thermo`.

|

.. code-block:: cpp

	double CalcTemperatureFromProperty(ECompoundTPProperties Property, double Time, double Value)
	
Reads the temperature from the corresponding lookup table for a specific ``Value`` of the selected ``Property`` (see table at the end of this section)  at the corresponding time point ``Time``. 

For more information, please refer to section :ref:`label-thermo`.

|

.. code-block:: cpp

	double CalcPressureFromProperty(ECompoundTPProperties Property, double Time, double Value)
	
Reads the pressure from the corresponding lookup table for a specific ``Value`` of the selected ``Property`` (see table at the end of this section) at the corresponding time point ``Time``. 

For more information, please refer to section :ref:`label-thermo`.

|

.. code-block:: cpp

	double CalcPropertyFromTemperature(ECompoundTPProperties Property, double Time, double T)
	
Reads the value of the specified ``Property`` (see table at the end of this section) at ``Time`` from the corresponding lookup table for the given temperature ``T``. 

For more information, please refer to section :ref:`label-thermo`.

|

.. code-block:: cpp

	double CalcPropertyFromPressure(ECompoundTPProperties Property, double Time, double P)
	
Reads the value of the specified ``Property`` (see table at the end of this section) at ``Time`` from the corresponding lookup table for the given pressure ``P``.

For more information, please refer to section :ref:`label-thermo`.

|

.. Note:: Definition of temperature-dependent compound properties:

	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   Define                      |   Name                      |   Unit                                                                    |
	+===============================+=============================+===========================================================================+
	|   ``DENSITY``                 |   Density                   |   [kg/m :math:`^3`]                                                       |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``HEAT_CAPACITY_CP``        |   Heat capacity :math:`C_p` |   [J/(kg·K)]                                                              |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``VAPOR_PRESSURE``          |   Vapor pressure            |   [Pa]                                                                    |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``VISCOSITY``               |   Viscosity                 |   [Pa·s]                                                                  |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``THERMAL_CONDUCTIVITY``    |   Thermal conductivity      |   [W/(m·K)]                                                               |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``PERMITTIVITY``            |   Permittivity              |   [F/m]                                                                   |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``ENTHALPY``                |   Enthalpy                  |   [J/kg/s] or [J/mol/s] for material stream, [J/kg] or [J/mol] for holdup |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+
	|   ``TP_PROP_USER_DEFINED_XX`` |   User defined property     |  [-]                                                                      |
	+-------------------------------+-----------------------------+---------------------------------------------------------------------------+

|

Other streams
-------------

Following functions are for class ``CMaterialStream``.

.. code-block:: cpp

	void CopyFromStream(CMaterialStream *SrcStream, double Time, bool DeleteDataAfter = true)
	
Copies all stream data from ``SrcStream`` for specified time point ``Time`` to the current material stream. 

If flag ``DeleteDataAfter`` is set to ``true``, all data after the time point ``Time`` in the destination stream will be removed before being copied.

|

.. code-block:: cpp

	void CopyFromStream(CMaterialStream *SrcStream, double Start, double End, bool DeleteDataAfter = true)

Copies all stream data from ``SrcStream`` on the certain time interval to the current material stream. Boundary points ``Start`` and ``End`` are included into this interval. 

If flag ``DeleteDataAfter`` is set to ``true``, all data after the time point ``Time`` in the destination stream will be removed before being copied.

|

.. code-block:: cpp

	void CopyFromStream(double TimeDst, CMaterialStream *SrcStream, double TimeSrc, bool DeleteDataAfter = true)

Copies all stream data from time point ``TimeSrc`` of material stream ``SrcStream`` to the time point ``TimeDst`` of this material stream. 

If flag ``DeleteDataAfter`` is set to ``true``, all data after the time point ``Time`` in the destination stream will be removed before being copied.

|

.. code-block:: cpp

	void CopyFromHoldup(CHoldup *SrcHoldup, double Time, double MassFlow, bool DeleteDataAfter = true)

Copies all data from ``SrcHoldup`` for the specified time point to the current material stream and sets mass flow ``MassFlow``. 

If flag ``DeleteDataAfter`` is set to ``true``, all data after the time point ``Time`` in the destination stream will be removed before being copied.

|

.. code-block:: cpp

	void CopyFromHoldup(double TimeDst, CHoldup *SrcHoldup, double TimeSrc, double MassFlow, bool DeleteDataAfter = true)

Copies all stream data from time point ``TimeSrc`` of holdup ``SrcHoldup`` to the time point ``TimeDst`` of this material stream with setting of new mass flow ``MassFlow``. 

If flag ``DeleteDataAfter`` is set to ``true``, all data after the time point ``Time`` in the destination stream will be removed before being copied.

|

.. code-block:: cpp

	void AddStream (CMaterialStream *Stream, double Time)

Performs a mixing of this material stream with material stream ``Stream`` for the specified time point ``Time``.

|

.. code-block:: cpp

	void AddStream (CMaterialStream *Stream, double Start, double End, unsigned TPType = BOTH_TP)
	
Performs a mixing of this material stream with material stream ``Stream`` for the specified time interval. Boundary points ``Start`` and ``End`` are included into this interval. 

Parameter ``TPType`` specifies which time points will be present in the resulting stream: combining points from two streams (``BOTH_TP``), only from the first stream (``DST_TP``), or only from the second stream (``SRC_TP``). 

Data for non-existent points are obtained by linear interpolation. 

|

Following functions are for class ``CHoldup``.

.. code-block:: cpp

	void CopyFromHoldup(CHoldup *SrcHoldup, double Time, bool DeleteDataAfter = true)
	
Copies all holdup data from ``SrcHoldup`` for the specified time point ``Time`` to the current holdup. 

If flag ``DeleteDataAfter`` is set to ``true``, all data after the time point ``Time`` in the destination holdup will be removed before being copied.

|

.. code-block:: cpp

	void CopyFromHoldup(CHoldup *SrcHoldup, double Start, double End, bool DeleteDataAfter = true)
	
Copies all holdup data from ``SrcHoldup`` on the certain time interval to the current holdup. Boundary points ``Start`` and ``End`` are included into this interval. 

If flag ``DeleteDataAfter`` is set to ``true``, all data after the time point ``Time`` in the destination holdup will be removed before being copied.


|

.. code-block:: cpp

	void CopyFromHoldup(double TimeDst, CHoldup *SrcHoldup, double TimeSrc, bool DeleteDataAfter = true)
	
Copies all holdup data from time point ``TimeSrc`` of holdup ``SrcHoldup`` to the time point ``TimeDst`` of this holdup. 

If flag ``DeleteDataAfter`` is set to ``true``, all data after the time point ``Time`` in the destination holdup will be removed before being copied.

|

.. _label-old_AddHoldup:

.. code-block:: cpp

	void AddHoldup(CHoldup *Holdup, double Time)
	
Performs a mixing of this holdup with holdup ``Holdup`` for the specified time point ``Time``.

|

.. code-block:: cpp

	void AddHoldup(CHoldup *Holdup, double Start, double End, unsigned TPType)
	
Performs a mixing of this holdup with ``Holdup`` for the specified time interval. Boundary points ``Start`` and ``End`` are included into this interval. 

Parameter ``TPType`` specifies which time points will be present in the resulting holdup: combining points from two holdups (``BOTH_TP``), only from the first holdup (``DST_TP``), or only from the second holdup (``SRC_TP``). 

Data for non-existent points are obtained by linear approximation. 

|

.. code-block:: cpp

	void AddStream(CMaterialStream *Stream, double Start, double End)
	
Performs a mixing of this holdup with material stream ``Stream`` for the specified time interval. Boundary points ``Start`` and ``End`` are included into this interval. 

Data for non-existent points are obtained by linear interpolation. 

|


.. _label-old_PSD:

Particle size distribution
==========================

Several global functions are defined to work with particle size distributions. These functions can be called from any place of the code.

All functions receive grid (``Grid``) as the input parameter. The grid can be previously obtained with the help of the function ``GetNumericGrid``, for more information please refer to :ref:`getNumericGrid <label-getNumericGrid>` in section :ref:`label-baseUnit`. 

.. Note:: Notations:

	:math:`d_i` – diameter of particle in class :math:`i`
	
	:math:`\Delta d_i` – size of the class :math:`i`
	
	:math:`M_k` – :math:`k`-th moment 
	
	:math:`q` – density distribution
	
	:math:`q_0` – number related density distribution
	
	:math:`Q_0` – number related cumulative distribution
	
	:math:`q_2` – surface-area-related density distribution
	
	:math:`Q_2` – surface-area-related cumulative distribution
	
	:math:`q_3` – mass-related density distribution
	
	:math:`Q_3` – mass-related cumulative distribution
	
	:math:`w_i` – mass fraction of particles of class :math:`i`
	
	:math:`N_i` – number of particles of class :math:`i`
	
	:math:`N_{tot}` – total number of particles
	
|

.. code-block:: cpp

	double GetMMoment(Moment, Grid, InDistr)

Calculates moment of the density distribution by :math:`M_k = \sum\limits_i d_i^k \, q_i \, \Delta d_i`.

|

.. _label-old_convb0s0: 

.. b for big (capital letters), s for small (lower letters)

.. code-block:: cpp

	vector<double> ConvertQ0Toq0(Grid, InDistr)

Performs conversion from :math:`Q_0` to :math:`q_0` distributions using information about the size grid: :math:`q_{0,0} = \dfrac{Q_{0,0}}{\Delta d_i}` and :math:`q_{0,i} = \dfrac{Q_{0,i} - Q_{0,i-1}}{\Delta d_i}`.

|

.. _label-old_convs0b0:

.. code-block:: cpp

	vector<double> Convertq0ToQ0(Grid, InDistr)

Performs conversion from :math:`q_0` to :math:`Q_0` distributions using information about the size grid: :math:`Q_{0,i} = \sum\limits_i q_{0,i} \, \Delta d_i = Q_{0,i-1} + q_{0,i} \, \Delta d_i`.

|

.. _label-old_convb2s2:

.. code-block:: cpp

	vector<double> ConvertQ2Toq2(Grid, InDistr)

Performs conversion from :math:`Q_2` to :math:`q_2` distributions using information about the size grid: :math:`q_{2,0} = \dfrac{Q_{2,0}}{\Delta d_i}` and :math:`q_{2,i} = \dfrac{Q_{2,i} - Q_{2,i-1}}{\Delta d_i}`.

|

.. _label-old_convs2b2:

.. code-block:: cpp

	vector<double> Convertq2ToQ2(Grid, InDistr)
	
Performs conversion from :math:`q_2` to :math:`Q_2` distributions using information about the size grid: :math:`Q_{2,i} = \sum\limits_i q_{2,i} \, \Delta d_i = Q_{2,i-1} + q_{2,i} \, \Delta d_i`.
	
|

.. _label-old_convb3s3:

.. code-block:: cpp

	vector<double> ConvertQ3Toq3(Grid, InDistr)

Performs conversion from :math:`Q_3` to :math:`q_3` distributions using information about the size grid: :math:`q_{3,0} = \dfrac{Q_{3,0}}{\Delta d_i}` and :math:`q_{3,i} = \dfrac{Q_{3,i} - Q_{3,i-1}}{\Delta d_i}`.

|

.. _label-old_convs3b3:

.. code-block:: cpp

	vector<double> Convertq3ToQ3(Grid, InDistr)

Performs conversion from :math:`q_3` to :math:`Q_3` distributions using information about the size grid: :math:`Q_{3,i} = \sum\limits_i q_{3,i} \, \Delta d_i = Q_{3,i-1} + q_{3,i} \, \Delta d_i`.

|

.. _label-old_convs0s2:

.. code-block:: cpp

	vector<double> Convertq0Toq2(Grid, InDistr)

Performs conversion from :math:`q_0` to :math:`q_2` distributions using information about the size grid by :math:`q_{2,i} = \dfrac{d_i^2 \, q_{0,i}}{M_2(q_0)}`.

|

.. _label-old_convs0s3:

.. code-block:: cpp

	vector<double> Convertq0Toq3(Grid, InDistr)

Performs conversion from :math:`q_0` to :math:`q_3` distributions using information about the size grid by :math:`q_{3,i} = \dfrac{d_i^3 \, q_{0,i}}{M_3(q_0)}`.

|

.. _label-old_convs2s0:

.. code-block:: cpp
	
	vector<double> Convertq2Toq0(Grid, InDistr)

Performs conversion from :math:`q_2` to :math:`q_0` distributions using information about the size grid by :math:`q_{0,i} = \dfrac{d_i^{-2} \, q_{2,i}}{M_{-2}(q_2)}`.

|

.. _label-old_convs2s3:

.. code-block:: cpp
	
	vector<double> Convertq2Toq3(Grid, InDistr)

Performs conversion from :math:`q_2` to :math:`q_3` distributions using information about the size grid by :math:`q_{3,i} = \dfrac{d_i \, q_{2,i}}{M_1(q_2)}`.

|

.. _label-old_convs3s0:

.. code-block:: cpp

	vector<double> Convertq3Toq0(Grid, InDistr)

Performs conversion from :math:`q_3` to :math:`q_0` distributions using information about the size grid by :math:`q_{0,i} = \dfrac{d_i^{-3} \, q_{3,i}}{M_{-3}(q_3)}`.

|

.. _label-old_convs3s2:

.. code-block:: cpp

	vector<double> Convertq3Toq2(Grid, InDistr)

Performs conversion from :math:`q_3` to :math:`q_2` distributions using information about the size grid by :math:`q_{2,i} = \dfrac{d_i^{-1} \, q_{3,i}}{M_{-1}(q_3)}`.

|

.. _label-old_convms3:

.. code-block:: cpp

	vector<double> ConvertMassFractionsToq3(Grid, InDistr)

Calculates :math:`q_3` distribution using the size grid and the distribution of mass fractions by :math:`q_3 = w_i / \Delta d_i`.

|

.. _label-old_convmb3:

.. code-block:: cpp

	vector<double> ConvertMassFractionsToQ3(InDistr)

Calculates :math:`Q_3` distribution using the distribution of mass fractions: :math:`Q_{3,0} = w_i` and :math:`Q_{3,i} = Q_{3,i-1} + w_i`.

|

.. _label-old_convms0:

.. code-block:: cpp

	vector<double> ConvertMassFractionsToq0(Grid, InDistr)

Calculates :math:`q_0` distribution using the functions :ref:`ConvertMassFractionsToq3 <label-convms3>` and :ref:`Convertq3Toq0 <label-convs3s0>`.

|

.. _label-old_convmb0:

.. code-block:: cpp

	vector<double> ConvertMassFractionsToQ0(Grid, InDistr)
	
Calculates :math:`Q_0` distribution using the functions :ref:`ConvertMassFractionsToq0 <label-convms0>` and :ref:`Convertq0ToQ0 <label-convs0b0>`.

|

.. _label-old_convms2:

.. code-block:: cpp
	
	vector<double> ConvertMassFractionsToq2(Grid, InDistr)

Calculates :math:`q_0` distribution using the functions :ref:`ConvertMassFractionsToq3 <label-convms3>` and :ref:`Convertq3Toq2 <label-convs3s2>`.

|

.. _label-old_convmb2:

.. code-block:: cpp

	vector<double> ConvertMassFractionsToQ2(Grid, InDistr)

Calculates :math:`Q_0` distribution using the functions :ref:`ConvertMassFractionsToq2 <label-convms2>` and :ref:`Convertq2ToQ2 <label-convs2b2>`.

|

.. _label-old_convs3m:

.. code-block:: cpp

	vector<double> Convertq3ToMassFractions(Grid, InDistr)
		
Calculates mass fractions from :math:`q_3` distribution using the size grid by :math:`w_i = q_{3,i}\cdot \Delta d_i`.

|

.. _label-old_convb3m:

.. code-block:: cpp

	vector<double> ConvertQ3ToMassFractions(InDistr)

Calculates mass fractions from :math:`Q_3` distribution using the size grid: :math:`w_0 = Q_{3,0}` and :math:`w_i = Q_{3,i} - Q_{3,i-1}`.

|

.. _label-old_convs0m:

.. code-block:: cpp

	vector<double> Convertq0ToMassFractions(Grid, InDistr)

Calculates mass fractions from :math:`q_0` distribution using the functions :ref:`Convertq0Toq3 <label-convs0s3>` and :ref:`Convertq3ToMassFractions <label-convs3m>`.

|

.. _label-old_convb0m:

.. code-block:: cpp

	vector<double> ConvertQ0ToMassFractions(Grid, InDistr)

Calculates mass fractions from :math:`Q_0` distribution using the functions :ref:`ConvertQ0Toq0 <label-convb0s0>` and :ref:`Convertq0ToMassFractions <label-convs0m>`.

|

.. _label-old_convs2m:

.. code-block:: cpp

	vector<double> Convertq2ToMassFractions(Grid, InDistr)
	
Calculates mass fractions from :math:`q_2` distribution using the functions :ref:`Convertq2Toq3 <label-convs2s3>` and :ref:`Convertq3ToMassFractions <label-convs3m>`.

|

.. _label-old_convb2m:

.. code-block:: cpp

	vector<double> ConvertQ2ToMassFractions(Grid, InDistr)

Calculates mass fractions from :math:`Q_2` distribution using the functions :ref:`ConvertQ2Toq2 <label-convb2s2>` and :ref:`Convertq2ToMassFractions <label-convs2m>`.

|

.. _label-old_convns0:

.. code-block:: cpp

	vector<double> ConvertNumbersToq0(Grid, InDistr)
	
Calculates :math:`q_0` distribution using the number distribution and the size grid by :math:`q_{0,i} = \dfrac{N_i}{\Delta d_i \, N_{tot}}`.

|

.. _label-old_convnb2:

.. code-block:: cpp

	vector<double> ConvertNumbersToQ2(Grid, InDistr)
	
Calculates :math:`Q_2` distribution using the number distribution and the size grid by :math:`Q_{2,i} = \dfrac{\sum\limits_{j=0}^i N_j \, \pi\,d_j^2}{\sum\limits_j N_j\, \pi\,d_j^2}`.

|

.. code-block:: cpp

	vector<double> ConvertNumbersToQ0(Grid, InDistr)

Calculates :math:`Q_0` distribution using the number distribution and the functions :ref:`ConvertNumbersToq0 <label-convns0>` and :ref:`Convertq0ToQ0 <label-convs0b0>`.

|

.. code-block:: cpp

	vector<double> ConvertNumbersToq2(Grid, InDistr)

Calculates :math:`q_2` distribution using the number distribution and the functions :ref:`ConvertNumbersToQ2 <label-convnb2>` and :ref:`ConvertQ2Toq2 <label-convb2s2>`.

|

.. _label-old_convns3:

.. code-block:: cpp

	vector<double> ConvertNumbersToq3(Grid, InDistr)
	
Calculates :math:`q_3` distribution using the number distribution and the functions :ref:`ConvertNumbersToq0 <label-convns0>` and :ref:`Convertq0Toq3 <label-convs0s3>`.

|

.. code-block:: cpp

	vector<double> ConvertNumbersToQ3(Grid, InDistr)
	
Calculates :math:`Q_3` distribution using the number distribution and the functions :ref:`ConvertNumbersToq3 <label-convns3>` and :ref:`Convertq3ToQ3 <label-convs3b3>`.

|

.. code-block:: cpp

	vector<double> ConvertNumbersToMassFractions(Grid, InDistr)
	
Calculates mass fractions from the number distribution using the functions :ref:`ConvertNumberToq0 <label-convns0>` and :ref:`Convertq0ToMassFractions <label-convms0>`.

|

.. code-block:: cpp

	vector<double> Convertq0Toq0(OldGrid, OldDistr, NewGrid)
	
Converts :math:`q_0` distribution to the same distribution on the modified size grid.

|

.. code-block:: cpp

	vector<double> Convertq2Toq2(OldGrid, OldDistr, NewGrid)
	
Converts :math:`q_2` distributions to the same distribution on the modified size grid.

|

.. code-block:: cpp

	vector<double> Convertq3Toq3(OldGrid, OldDistr, NewGrid)

Converts :math:`q_3` distributions to the same distribution on the modified size grid.

|

.. code-block:: cpp

	NormalizeDensityDistribution(Grid, qiDistr)
	
Normalizes density distribution :math:`q_0` or :math:`q_3` by :math:`q_i = \dfrac{q_i}{\sum\limits_j q_j\,\Delta d_j}`.

|

.. code-block:: cpp

	double GetDistributionMedian(Grid, QxDistr)
	
Returns median in [m] of :math:`Q_0` or :math:`Q_3` distribution. Median is a diameter, which corresponds to a value of distribution equal to 0.5.

|

.. code-block:: cpp

	double GetDistributionValue (Grid, QxDistr, Val)

Returns diameter in [m], which corresponds to a specified value of cumulative distribution :math:`Q_0` or :math:`Q_3`. Input value ``Val`` should range between 0 and 1.

|

.. code-block:: cpp

	double GetDistributionMode (Grid, qxDistr)

Returns diameter in [m], which corresponds to a maximum value of density distribution.

|

.. code-block:: cpp

	double GetAverageDiameter (Grid, qxDistr)

Returns average diameter in [m] of the distribution :math:`q_0` or :math:`q_3`.

|

.. code-block:: cpp

	double GetSauterDiameter (Grid, q3Distr)

Calculates Sauter diameter (:math:`d_{32}`) of :math:`q_3` distribution in [m].

|

.. code-block:: cpp

	double GetSpecificSurface (Grid, q3Distr)

Calculates specific surface of :math:`q_3` distribution in [m :math:`^2`].

|

Matrices
========

Several types of matrix classes, including the following types, are introduced in this section. 

- Transform matrix: ``CTransformMatrix``.

- Dense 2-dimensional matrix: ``CDense2DMatrix``.

- Dense multidimensional matrix: ``CDenseMDMatrix``.

|

Transformation matrix
---------------------

:ref:`label-TM` (class ``CTransformMatrix``) are applied to describe laws of changes for multidimensional distributions. Each cell of matrix describes how much of material will be transferred from one class of multidimensional distribution to another.

|

.. code-block:: cpp

	CTransformMatrix()
	
Basic **constructor**. Creates an empty matrix.

|

.. code-block:: cpp

	CTransformMatrix (unsigned _nType, unsigned _nClasses)

Creates matrix to transform one-dimensional distribution with type ``_nType`` and ``_nClasses`` classes. ``_nType`` is one of the :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. 

All values in matrix will be set to 0.

|

.. code-block:: cpp

	CTransformMatrix(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2)

Creates matrix to transform two-dimensional distribution with types ``_nType1`` and ``_nType2`` and classes ``_nClasses1`` and ``_nClasses2``. ``_nType1`` and ``_nType2`` are types from :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. 

All values in matrix will be set to 0.

|

.. code-block:: cpp

	CTransformMatrix(const std::vector<unsigned> &_vTypes, const std::vector<unsigned> &_vClasses)

Creates transformation matrix for distribution with specified types and classes. ``_vTypes`` and ``_vClasses`` must have the same length. ``_vTypes`` is the vector of types from :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. 

All values in matrix will be set to 0.

|

Dimensions
""""""""""

.. code-block:: cpp

	bool SetDimensions(unsigned _nType, unsigned _nClasses)

Sets new dimensions set to the matrix in order to transform one-dimensional distribution with type ``_nType`` and ``_nClasses`` classes. ``_nType`` is one of the :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. 

Old data will be erased and matrix will be initialized with zeroes. Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetDimensions(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2)

Sets new dimensions set to the matrix in order to transform two-dimensional distribution. 

``_nType1`` and ``_nType2`` are types of the :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. Types must be unique. ``_nClasses1`` and ``_nClasses2`` are number of classes in corresponding distributions. 

Old data will be erased and matrix will be initialized with zeroes. Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetDimensions(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2, unsigned _nType3, unsigned _nClasses3)

Sets new dimensions set to the matrix in order to transform three-dimensional distribution. 

``_nType1``, ``_nType2`` and ``_nType3`` are one of the :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. Types must be unique.  ``_nClasses1``, ``_nClasses2`` and ``_nClasses3`` are number of classes in corresponding distributions. 

Old data will be erased and matrix will be initialized with zeroes. Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetDimensions(const std::vector<unsigned> &_vTypes, const std::vector<unsigned> &_vClasses)

Sets new dimensions set with types ``_vTypes`` and numbers of classes ``_vClasses``. ``_vTypes`` is the vector of :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. 

All old data will be erased and matrix will be initialized with zeroes. Sizes of vectors ``_vTypes`` and ``_vClasses`` must be equal. Returns ``false`` on error.

|

.. code-block:: cpp

	std::vector<unsigned> GetDimensions()

Returns vector with all current defined dimensions types.

|

.. code-block:: cpp

	std::vector<unsigned> GetClasses()

Returns vector with current numbers of classes.

|

.. code-block:: cpp

	unsigned GetDimensionsNumber() 

Returns current number of dimensions.

|

Get data
""""""""

.. code-block:: cpp

	double GetValue(unsigned _nCoordSrc, unsigned _nCoordDst)

Returns value by specified coordinates according to all defined dimensions in transformation matrix for one-dimensional distribution. ``_nCoordSrc`` is coordinate of a source class, ``_nCoordDst`` is coordinate of a destination class. 

Returning value is a mass fraction, which will be transferred from the source class to the destination class. Works with one-dimensional distribution only. Returns ``-1`` on error.

|

.. code-block:: cpp

	double GetValue(unsigned _nCoordSrc1, unsigned _nCoordSrc2, unsigned _nCoordDst1, unsigned _nCoordDst2)

Returns value by specified coordinates according to all defined dimensions in transformation matrix for two-dimensional distribution. ``_nCoordSrc1`` and ``_nCoordSrc2`` are coordinates of a source class, ``_nCoordDst1`` and ``_nCoordDst2`` are coordinate of a destination class. 

Returning value is a mass fraction, which will be transferred from the source class to the destination class. Works with two-dimensional distribution only. Returns ``-1`` on error.

|

.. code-block:: cpp

	double GetValue(const std::vector<unsigned> &_vCoordsSrc, const std::vector<unsigned> &_vCoordsDst)

Returns value by specified coordinates according to all defined dimensions. ``_vCoordsSrc`` are coordinates of a source class, ``_vCoordsDst`` are coordinates of a destination class. Sizes of vectors ``_vCoordsSrc`` and ``_vCoordsDst`` must be equal and must correspond to the number of currently defined dimensions. 

Returning value is a mass fraction, which will be transferred from the source class to the destination class. Returns ``-1`` on error.

|

.. code-block:: cpp

	double GetValue(const std::vector<unsigned> &_vDimsSrc, const std::vector<unsigned> &_vCoordsSrc, const std::vector<unsigned> &_vDimsDst, const std::vector<unsigned> &_vCoordsDst)

Returns value according to specified coordinates and dimensions. Number of dimensions must be the same as defined in the transformation matrix, but their sequence can be different. Sizes of all vectors must be equal. 

Returning value is a mass fraction, which will be transferred from the source class to the destination class. Returns ``-1`` on error.

|

.. code-block:: cpp

	bool GetVectorValue(const std::vector<unsigned> &_vCoordsSrc, const std::vector<unsigned> &_vCoordsDst, std::vector<double> &_vResult)

Returns vector value by specified coordinates according to all defined dimensions. Size of one vector of coordinates must be equal to the number of dimensions in transformation matrix; size of the second one must be one less. 

Returning value ``_vResult`` is a vector of mass fractions, which will be transferred from the source to the destination. Returns ``false`` on error.

|

.. code-block:: cpp

	bool GetVectorValue(const std::vector<unsigned> &_vDimsSrc, const std::vector<unsigned> &_vCoordsSrc, const std::vector<unsigned> &_vDimsDst, const std::vector<unsigned> &_vCoordsDst, std::vector<double> &_vResult)

Returns vector of values according to specified coordinates and dimensions sequence. Number of dimensions must be the same as defined in the transformation matrix, but their sequence can be different. Size of one vector of coordinates must be equal to the number of dimensions in transformation matrix; size of the second one must be one less. 

Returning value ``_vResult`` is a vector of mass fractions, which will be transferred from the source to the destination. Returns ``false`` on error.

|

Set data
""""""""

.. code-block:: cpp

	bool SetValue(unsigned _nCoordSrc, unsigned _nCoordDst, double _dValue)

Sets value by specified coordinates for one-dimensional distribution. ``_nCoordSrc`` is a coordinate of the source class; ``_nCoordDst`` is a coordinate of the destination class. ``_dValue`` is a mass fraction, which will be transferred from the source class to the destination class. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetValue (unsigned _nCoordSrc1, unsigned _nCoordSrc2, unsigned _nCoordDst1, unsigned _nCoordDst2, double _dValue)

Sets value by specified coordinates for two-dimensional distribution. 

``_nCoordSrc1`` and ``_nCoordSrc2`` are coordinate of the source class; ``_nCoordDst1`` and ``_nCoordDst2`` are coordinate of the destination class. ``_dValue`` is a mass fraction, which will be transferred from the source class to the destination class. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetValue (const std::vector<unsigned> &_vCoordsSrc, const std::vector<unsigned> &_vCoordsDst, double _dValue)

Sets value by specified coordinates and full dimensions set. ``_vCoordsSrc`` are coordinates of the source class, ``_vCoordsDst`` are coordinates of the destination class. Sizes of vectors ``_vCoordsSrc`` and ``_vCoordsDst`` must be equal. ``_dValue`` is a mass fraction, which will be transferred from the source class to the destination class. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetValue (const std::vector<unsigned> &_vDimsSrc, const std::vector<unsigned> &_vCoordsSrc, const std::vector<unsigned> &_vDimsDst, const std::vector<unsigned> &_vCoordsDst, double _dValue)

Sets value according to specified coordinates and dimensions. Number of dimensions must be the same as defined in the transformation matrix, but their sequence can be different. Sizes of all vectors must be equal. ``_dValue`` is a mass fraction, which will be transferred from the source class to the destination class. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetVectorValue (const std::vector<unsigned> &_vCoordsSrc, const std::vector<unsigned> &_vCoordsDst, const std::vector<double> &_vValue)

Sets vector of values by specified coordinates according to all defined dimensions. Size of one vector of coordinates must be equal to the number of dimensions in transformation matrix; size of the second one must be one less. ``_vValue`` is a vector of mass fractions, which will be transferred from the source to the destination. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetVectorValue (const std::vector<unsigned> &_vDimsSrc, const std::vector<unsigned> &_vCoordsSrc, const std::vector<unsigned> &_vDimsDst, const std::vector<unsigned> &_vCoordsDst, const std::vector<double> &_vValue)

Sets vector of values according to specified coordinates and dimensions sequence. Number of dimensions must be the same as defined in the transformation matrix, but their sequence can be different. Size of one vector of coordinates must be equal to the number of dimensions in transformation matrix; size of the second one must be one less. ``_vValue`` is a vector of mass fractions, which will be transferred from the source to the destination. 

Returns ``false`` on error.

|

Other functions
"""""""""""""""

.. code-block:: cpp

	void Normalize()

Normalizes data in matrix: sets sum of material which transfers from each single class to 1.

|

.. code-block:: cpp

	void ClearData()

Sets all data in matrix to 0.

|

.. code-block:: cpp

	void Clear()

Removes all data and information about dimensions from the matrix.

|

Two-dimensional matrix
----------------------

Basic information and functions of class ``CDense2DMatrix`` are introduced below.

|

.. code-block:: cpp

	CDense2DMatrix()

**Basic constructor**. Creates empty matrix with zero in all rows and columns.

|

.. code-block:: cpp

	CDense2DMatrix(CDense2DMatrix &_matrix)

**Copy constructor**. Creates matrix with the same dimensions as in ``_matrix`` and copies all data.

|

.. code-block:: cpp

	CDense2DMatrix(unsigned _nRows, unsigned _nColumns)

Creates new matrix with specified number of rows and columns. All data will be set to 0.

|

Dimensions
""""""""""

.. code-block:: cpp

	void SetDimensions(unsigned _nRows, unsigned _nColumns)

Sets new dimensions to the matrix. Old data is removed and all entries are set to zero.

|

.. code-block:: cpp

	unsigned GetRowsNumber()

Returns number of rows in the matrix.

|

.. code-block:: cpp

	unsigned GetColumnsNumber()

Returns number of columns in the matrix.

|

Get data
""""""""

.. code-block:: cpp

	double GetValue(unsigned _nRow, unsigned _nColumn)

Returns data by the specified indexes. Returns ``0`` if such indexes do not exist.

|

.. code-block:: cpp

	std::vector<double> GetRow(unsigned _nRow)

Returns vector of data for specified row. Returns empty vector if such row does not exist.

|

.. code-block:: cpp

	std::vector<double> GetColumn(unsigned _nColumn)

Returns vector of data for specified column. Returns empty vector if such column does not exist.

|

.. code-block:: cpp

	std::vector<std::vector<double>> GetMatrix()

Returns all data in form of vector-of-vectors.

|

Set data
""""""""

.. code-block:: cpp

	void SetValue(unsigned _nRow, unsigned _ nColumn, double _dValue)

Sets data ``_dValue`` by the specified indexes.

|

.. code-block:: cpp

	void SetRow(unsigned _nRow, const std::vector<double>& _Values)

Sets data ``_Values`` to a specified row.

|

.. code-block:: cpp

	void SetColumn(unsigned _nColumn, const std::vector<double>& _Values)

Sets data ``_Values`` to a specified column.

|

.. code-block:: cpp

	void SetMatrix(const std::vector<std::vector<double>>& _matr)

Sets all values in form vector-of-vectors ``_matr`` to matrix. ``_matr`` must have the same dimensions as the matrix itself.

|

Overloaded operators
""""""""""""""""""""

.. code-block:: cpp

	CDense2DMatrix operator+(const CDense2DMatrix& _Matrix1, const CDense2DMatrix &_Matrix2)

Performs **addition** of two matrices with the same dimensions. Returns an empty matrix in case of different dimensions.

|

.. code-block:: cpp

	CDense2DMatrix operator-(const CDense2DMatrix &_Matrix1, const CDense2DMatrix &_Matrix2)

Performs **subtraction** of two matrices with the same dimensions. Returns an empty matrix in case of different dimensions.

|

.. code-block:: cpp

	CDense2DMatrix operator*(double _dMultiplier)

Performs **multiplication** of the matrix with a coefficient ``_dMultiplier``.

|

.. code-block:: cpp

	CDense2DMatrix& operator=(const CDense2DMatrix &_matrix)

Sets dimenions and data from the ``_matrix`` to a left matrix.

|

Other functions
"""""""""""""""

.. code-block:: cpp

	void Normalize()

Normalizes the matrix so that the sum of all elements equals to 1.

|

.. code-block:: cpp

	void ClearData()

Sets all elements in matrix to 0.

|

.. code-block:: cpp

	void Clear()
	
Removes all data and sets number of rows and columns equal to 0.

|

Multidimensional matrix
-----------------------

Basic information and functions of class ``CDenseMDMatrix`` are introduced below.

|

.. code-block:: cpp

	CDenseMDMatrix()

**Basic constructor**. Creates an empty matrix.

|

.. code-block:: cpp

	CDenseMDMatrix(const CDenseMDMatrix &_source)

**Copy constructor**. Creates matrix with the same dimensions as in ``_source`` and copies all data from there.

|

Dimensions
""""""""""

.. code-block:: cpp

	bool SetDimensions(unsigned _nType, unsigned _nClasses)

Sets **one-dimensional** distribution of type _nType and numbers of classes ``_nClasses``. 

``_nType`` is one of :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. All old data will be erased. Matrix will be initialized with zero values. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetDimensions(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2)

Sets **two-dimensional** distribution of types ``_nType1`` and ``_nType2`` and numbers of classes ``_nClasses1`` and ``_nClasses2``. 

``_nType1`` and ``_nType2`` are from :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. All types must be unique. All old data will be erased. Matrix will be initialized with zero values. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetDimensions(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2, unsigned _nType3, unsigned _nClasses3)

Sets **three-dimensional** distribution of types ``_nType1``, ``nType2`` and ``_nType3`` and numbers of classes ``_nClasses1``, ``_nClasses2`` and ``_nClasses3``. 

``_nType1``, ``nType2`` and ``_nType3`` are from :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. All types must be unique. All old data will be erased. Matrix will be initialized with zero values. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetDimensions(const std::vector<unsigned> &_vTypes, const std::vector<unsigned> &_vClasses)

Sets types ``_vTypes`` of dimensions and numbers of classes ``_vClasses``. 

``_vTypes`` is the vector of :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. All types must be unique. All old data will be erased. Matrix will be initialized with zero values. 

Returns ``false`` on error.

|

.. code-block:: cpp

	std::vector<unsigned> GetDimensions()

Returns vector with all currently defined dimensions types.

|

.. code-block:: cpp

	std::vector<unsigned> GetClasses()

Returns vector with current numbers of classes.

|

.. code-block:: cpp

	unsigned GetDimensionsNumber() 

Returns current number of dimensions.

|

Get data
""""""""

.. code-block:: cpp

	double GetValue(unsigned _nDim, unsigned _nCoord)

Returns value with specified coordinate ``_nCoord`` of specified dimension ``_nDim``. It is possible to use this function if matrix has more than one dimension: if number of dimensions does not conform to the matrix, the sum of values by remaining dimensions will be returned. 

Returns ``-1`` on error.

|

.. code-block:: cpp

	double GetValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2)

Returns value according to specified coordinates and dimensions. It is possible to use this function if matrix has more than two dimensions: if number of dimensions does not conform to the matrix, the sum of values by remaining dimensions will be returned. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns ``-1`` on error.

|

.. code-block:: cpp

	double GetValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nDim3, unsigned _nCoord3)

Returns value according to specified coordinates and dimensions. It is possible to use this function if matrix has more than three dimensions: if number of dimensions does not conform to the matrix, the sum of values by remaining dimensions will be returned. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns ``-1`` on error.

|

.. code-block:: cpp

	double GetValue(const std::vector<unsigned> &_vDims, const std::vector<unsigned> &_vCoords)

Returns value according to specified coordinates and dimensions. It is possible to use this function if matrix has more dimensions than was defined in ``_vDims``: if number of dimensions does not conform to the matrix, the sum of values by remaining dimensions will be returned. Sequence of dimensions may not match the sequence, which was defined in the matrix. Number of dimensions ``_vDims`` and coordinates ``_vCoords`` must be the same. 

Returns ``-1`` on error.

|

.. code-block:: cpp

	double GetValue(const std::vector<unsigned> &_vCoords)

Returns value by specified coordinates according to the full defined set of dimensions. Returns ``-1`` on error.

|

.. code-block:: cpp

	bool GetVectorValue(unsigned _nDim, std::vector<double> &_vResult)

Returns vector ``_vResult`` according to specified dimension. If number of dimensions in the matrix is more than one, then the sum of values by remaining dimensions will be returned. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool GetVectorValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, std::vector<double> &_vResult)

Returns vector of values ``_vResult`` according to specified dimensions and coordinate. If number of dimensions in the matrix is more than two, then the sum of values by remaining dimensions will be returned. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool GetVectorValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nDim3, std::vector<double> &_vResult)

Returns vector of values ``_vResult`` according to specified dimensions and coordinates. If number of dimensions in the matrix is more than three, then the sum of values by remaining dimensions will be returned. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool GetVectorValue(const std::vector<unsigned> &_vDims, const std::vector<unsigned> &_vCoords, std::vector<double> &_vResult) 

Returns vector of values ``_vResult`` according to specified dimensions and coordinates. If number of dimensions in the matrix is more than it was specified in ``_vDims``, then the sum of values by remaining dimensions will be returned. Sequence of dimensions may not match the sequence, which was defined in the matrix. Number of coordinates ``_vCoords`` must be one less than the number of dimensions ``_vDims``. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool GetVectorValue(const std::vector<unsigned> &_vCoords, std::vector<double> &_vResult)

Returns vector of values ``_vResult`` by specified coordinates according to the full defined set of dimensions. Returns ``false`` on error.

|

Set data
""""""""

.. code-block:: cpp

	bool SetValue(unsigned _nCoord, double _dValue)

Sets value ``_dValue`` with coordinate ``_nCoord`` into one-dimensional matrix. Sets the value only if the matrix has one dimension. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nCoord2, double _dValue)

Sets value ``_dValue`` according to specified coordinates and dimensions into two-dimensional matrix. Sets the value only if the matrix has two dimensions. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns false on ``error``.

|

.. code-block:: cpp

	bool SetValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nCoord3, double _dValue)

Sets value ``_dValue`` according to specified coordinates and dimensions into three-dimensional matrix. Sets the value only if the matrix has three dimensions. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetValue(const std::vector<unsigned> &_vDims, const std::vector<unsigned> &_vCoords, double _dValue)

Sets value ``_dValue`` according to specified coordinates and dimensions. Sets the value only if the number of dimensions is the same as in the matrix. Number of dimensions ``_vDims`` and coordinates ``_vCoords`` must be the same. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetValue(const std::vector<unsigned> &_vCoords, double _dValue)

Sets value ``_dValue`` according to specified coordinates for full set of dimensions. Sets the value only if the number of coordinates is the same as the number of dimensions in the matrix. Number of coordinates ``_vCoords`` must be equal to a number of dimensions in matrix. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetVectorValue(const std::vector<double> &_vValue)

Sets vector of values ``_vValue`` in one-dimensional matrix. Sets the values only if the matrix has one dimension. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetVectorValue(unsigned _nDim, unsigned _nCoord, const std::vector<double> &_vValue)

Sets vector of values ``_vValue`` according to specified dimension and coordinate in two-dimensional matrix. Sets the values only if the matrix has two dimensions. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetVectorValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, const std::vector<double> &_vValue)

Sets vector of values ``_vValue`` according to specified dimensions and coordinates in three-dimensional matrix. Sets the values only if the matrix has three dimensions. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetVectorValue(const std::vector<unsigned> &_vDims, const std::vector<unsigned> &_vCoords, const std::vector<double> &_vValue) 

Sets vector of values ``_vValue`` according to specified dimensions and coordinates. Sets values only if the number of dimensions in ``_vDims`` is one less than in the matrix. Number of dimensions ``_vDims`` and coordinates ``_vCoords`` must be equal. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetVectorValue (const std::vector<unsigned> &_vCoords, const std::vector<double> &_vValue) 

Sets vector of values ``_vValue`` according to specified coordinates for full set of dimensions, which were defined in the matrix. Sets values only if the number of coordinates is one less than number of dimensions in the matrix. 

Returns ``false`` on error.

|

Overloaded operators
""""""""""""""""""""

.. code-block:: cpp
	
	CDenseMDMatrix operator+(const CDenseMDMatrix &_matrix)

Performs **addition** of the matrix with the same dimensions. Returns an empty matrix with 0 defined dimensions in case of different dimenions.

|

.. code-block:: cpp

	CDenseMDMatrix operator-(const CDenseMDMatrix &_matrix)

Performs **subtraction** of matrices with the same dimensions. Returns an empty matrix with 0 defined dimensions in case of different dimenions.

|

.. code-block:: cpp

	CDenseMDMatrix operator*(double _dFactor)

Performs **multiplication** of the matrix by a coefficient ``_dFactor``.

|

.. code-block:: cpp

	CDenseMDMatrix& operator=(const CDenseMDMatrix &_matrix)

Sets dimenions and data from the ``_matrix`` to a left matrix.

|

Other functions
"""""""""""""""

.. code-block:: cpp

	void Normalize()

Normalizes the matrix so that the sum of all elements equals to 1.

|

.. code-block:: cpp

	bool IsNormalized()

Returns ``true`` if the matrix is normalized.

|

.. code-block:: cpp

	void ClearData()

Sets all elements in matrix equal to 0.

|

.. code-block:: cpp

	void Clear()

Removes all data and information about dimensions from the matrix.

|
