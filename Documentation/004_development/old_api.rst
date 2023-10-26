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

.. _label-old_setMassFlow:

.. code-block:: cpp

	void SetMassFlow(double Time, double Value, unsigned Basis)
	
Specific function for ``CMaterialStream``. 
	
Sets mass flow of the material stream at the time point ``Time``. Negative values before setting will be converted to ``0``. If the time point Time has not been defined in the material stream, then the value will not be set. 

``Basis`` is a basis of results (``BASIS_MASS`` in [kg/s] or ``BASIS_MOLL`` in [mol/s]):

``BASIS_MASS``: in this case you directly have your input ``Value`` as mass flow: :math:`\dot{m} =` ``Value`` in [kg/s]

``BASIS_MOLL``: in this case you have ``Value`` as mole flow and this should be converted to mass flow. :math:`\dot{m} =` ``Value`` :math:`\cdot \sum\limits_i M_i \cdot w_i` in [mol/s], with :math:`w_i` mass fraction of the phase :math:`i`, and :math:`M_i` molar mass of the phase :math:`i`.


Compounds
---------

.. code-block:: cpp

	double GetCompoundMassFlow(double Time, std::string CompoundKey, unsigned Phase, unsigned Basis)

Specific function for ``CMaterialStream``. 

Returns mass flow of the compound with key CompoundKey in phase Phase (``SOA_SOLID``, ``SOA_LIQUID``, ``SOA_VAPOR``) for the time point ``Time``. If such time point has not been defined, interpolation of data will be done. 

Basis is a basis of value (``BASIS_MASS`` in [kg/s] or ``BASIS_MOLL`` in [mol/s]).

``BASIS_MASS``: :math:`\dot{m}_{i,j} = w_i \cdot f_{i,j} \cdot \dot{m}`, with :math:`\dot m_{i,j}` the mass flow of compound :math:`j` in phase :math:`i`, :math:`w_i` the mass fraction of phase :math:`i`, and :math:`f_{i,j}` the mass fraction of compound :math:`j` in phase :math:`i`.

``BASIS_MOLL``: :math:`\dot{m}_{i,j} = w_i \cdot f_{i,j} \cdot \sum\limits_k \dfrac{\dot{m} \cdot w_k}{M_k}`, with :math:`\dot m` the total mass flow of the material stream, and :math:`M_k` the molar mass of phase :math:`k`.

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

Other streams
-------------

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
