.. _sec.units.timedelay:

Time delay
==========

Constant delay of input signal

.. image:: ../images/003_models/timedelay.png
   :width: 300px
   :alt:
   :align: center

|

Simple shift
^^^^^^^^^^^^

Copies all time points :math:`t` from the input stream :math:`In` to the output stream :math:`Out` at the timepoint :math:`t + \Delta t`, delaying the signal by a constant value :math:`\Delta t`.

Norm-based
^^^^^^^^^^^^

.. math::
	\frac{dm}{dt} = \dot{m}_{in}(t-\Delta t) - m

To correctly take into account the dynamics of the process, norms of each overall parameter (mass flow, temperature, pressure) are maintained as:

.. math::
	\frac{d||X||}{dt} = (X(t) - X(t-1))^2 - ||X||

For phase fractions:

.. math::
	\frac{d||P||}{dt} = \sqrt{\sum_{i}^{N_{P}}{(w_{i}(t) - w_{i}(t-1))^2}} - ||P||

For compound fractions in each phase:

.. math::
	\frac{d||C_{i}||}{dt} = \sqrt{\sum_{j}^{N_{C_{i}}}{(w_{i,j}(t) - w_{i, j}(t-1))^2}} - ||C||

For each distributed parameter:

.. math::
	\frac{d||D_{i}||}{dt} = \sqrt{\sum_{j}^{N_{D_{i}}}{(w_{i,j}(t) - w_{i,j}(t-1))^2}} - ||D||

.. note:: Notations:

	:math:`{m}` – current mass

	:math:`\dot{m}_{in}` – input mass flow

	:math:`\Delta t` – time delay

	:math:`X(t)` – value of an overall parameter at time point :math:`t`

	:math:`w(t)` – mass fraction at time point :math:`t`

	:math:`N_{P}` – number of defined phases

	:math:`N_{C_{i}}` – number of defined compounds in phase :math:`i`

	:math:`N_{D_{i}}` – number of classes in distribution :math:`i`

.. note:: Model parameters:

	+--------------------+------------------+-----------------------------------+-------+---------------------------------+
	| Name               | Symbol           | Description                       | Units | Boundaries                      |
	+====================+==================+===================================+=======+=================================+
	| Time delay         |                  | Model to use                      |       | Norm based, Simple shift        |
	+--------------------+------------------+-----------------------------------+-------+---------------------------------+
	| Time delay         | :math:`\Delta t` | Time delay                        | [s]   | >=0                             |
	+--------------------+------------------+-----------------------------------+-------+---------------------------------+
	| Relative tolerance |                  | Relative tolerance for DAE solver | [-]   | >0 (0 for flowsheet-wide value) |
	+--------------------+------------------+-----------------------------------+-------+---------------------------------+
	| Absolute tolerance |                  | Absolute tolerance for DAE solver | [-]   | >0 (0 for flowsheet-wide value) |
	+--------------------+------------------+-----------------------------------+-------+---------------------------------+


.. seealso::

	a demostration file at ``Example Flowsheets/Units/Time Delay.dlfw``.

|
