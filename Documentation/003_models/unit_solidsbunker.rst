.. _sec.units.solidsbunker:

Solids bunker
=============

Bunker unit performs accumulation of the solid part of the input material with ideal mixing, see figure below.

.. image:: ../images/003_models/solidsbunker.png
   :width: 200px
   :alt:
   :align: center

The model takes into account only the solid phase, the rest of the phases are bypassed.

.. math::
	\frac{dm}{dt} = \dot{m}_{in} - \dot{m}_{out}


Two models for the bunker outflow :math:`m_{out}` are available:

- **Adaptive**: User defines only the target mass :math:`m_{target}` of the bunker and :math:`\dot{m}_{out}` is being adjusted by the system to match the user-defined target mass :math:`m_{target}`, depending on inflow mass :math:`\dot{m}_{in}`, current bunker mass :math:`{m}` and :math:`m_{target}`: .

.. math::
	\dot{m}_{out} = \dot{m}_{in}\left(\frac{2m}{m + m_{target}}\right)^2

- **Constant**: User defines timepoints with the desired bunker outflow :math:`\dot{m}_{requested}`. The system tries to provide this outflow, if enough material :math:`{m}` is in bunker. Otherwise the :math:`\dot{m}_{out} = \dot{m}_{in}`. The smoothing function is implemented to let the numerical solver provide reliable results:

.. math::
	f_{smooth} = \frac{1}{2} + \frac{1}{2} \cdot \tanh{\left(50\cdot\left(m - \dot{m}_{requested}\cdot{dt}\right)\right)}

.. math::
	\dot{m}_{out} = f_{smooth} \cdot \dot{m}_{requested} + \left(1 - f_{smooth} \right) \cdot \min\left(\dot{m}_{in}, \dot{m}_{requested}\right)


To correctly take into account the dynamics of the process, norms of each overall parameter (mass flow, temperature, pressure) are maintained as:


.. math::
	\frac{d||X||}{dt} = (X(t) - X(t-1))^2 - ||X||

For compounds fractions:

.. math::
	\frac{d||C||}{dt} = \sqrt{\sum_{i}^{N_{c}}{(w_{i}(t) - w_{i}(t-1))^2}} - ||C||

For each distributed parameter:

.. math::
	\frac{d||D_{i}||}{dt} = \sqrt{\sum_{j}^{N_{D_{i}}}{(w_{i,j}(t) - w_{i,j}(t-1))^2}} - ||D||

.. note:: Notations:

	:math:`{m}` – current mass inside the bunker

	:math:`m_{target}` – target mass inside the bunker

	:math:`\dot{m}_{in}` – solids input mass flow

	:math:`\dot{m}_{out}` – solids output mass flow

	:math:`X(t)` – value of an overall parameter at time point :math:`t`

	:math:`w(t)` – mass fraction at time point :math:`t`

	:math:`N_{c}` – number of defined compounds

	:math:`N_{D_{i}}` – number of classes in distribution :math:`i`

.. note:: Solid phase is required for the simulation.


.. note:: Input parameters needed for the simulation:

	+--------------------+-----------------------------------+-------+---------------------------------+
	| Name               | Description                       | Units | Boundaries                      |
	+====================+===================================+=======+=================================+
	| Target mass        | Target mass within the bunker     | [kg]  | Target mass > 0                 |
	+--------------------+-----------------------------------+-------+---------------------------------+
	| Relative tolerance | Relative tolerance for DAE solver | [-]   | >0 (0 for flowsheet-wide value) |
	+--------------------+-----------------------------------+-------+---------------------------------+
	| Absolute tolerance | Absolute tolerance for DAE solver | [-]   | >0 (0 for flowsheet-wide value) |
	+--------------------+-----------------------------------+-------+---------------------------------+


.. seealso::

	a demostration file at ``Example Flowsheets/Units/Bunker.dlfw``.

|
