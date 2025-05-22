.. _sec.units.crusher:

Crusher
=======

A crusher comminutes the input material stream and reduces the average particle size. The schema is illustrated below.

.. image:: ../images/003_models/crusher.png
   :width: 200px
   :alt: splitter
   :align: center

This unit can be described using 3 models in Dyssol:

	- Bond's model
	- Cone model
	- Const model


Bond's model
^^^^^^^^^^^^

This model is used to perform milling of the input stream. The crushing is performed according to the model proposed by Bond. The simplification is made, and the particle size distribution of the output stream is described by the normal function.


.. math::

	x_{80,out} = \dfrac{1}{ \left( \dfrac{P}{10\,w_i\,\dot{m}} + \dfrac{1}{\sqrt{x_{80,in}}} \right)^2}

.. math::

	\mu = x_{80,out} - 0.83\sigma

.. math::

	q_3(x) = \frac{1}{\sigma\sqrt{2\pi}}\,e^{-\dfrac{(x-\mu)^2}{2\sigma^2}}



.. note:: Notations applied in this model:

	:math:`x_{80,out}` – characteristic particle size of the output stream

	:math:`x_{80,in}` – characteristic particle size of the input stream

	:math:`w_i` – Bond Work Index, dependent on the material

	:math:`P` – power input

	:math:`\dot{m}` – mass flow of solids in the input stream

	:math:`q_3(x)` – output mass related density distribution

	:math:`\sigma` – standard deviation of the output normal distribution

	:math:`\mu` – mean value of the output normal distribution


.. note:: Solid phase and particle size distribution are required for the simulation.


.. note:: Input parameters needed for the simulation:

	+--------------------+----------------+-----------------------------------------------+---------+------------------------+
	| Name               | Symbol         | Description                                   | Units   | Boundaries             |
	+====================+================+===============================================+=========+========================+
	| P                  |:math:`P`       | Power input                                   | [kW]    | P > 0                  |
	+--------------------+----------------+-----------------------------------------------+---------+------------------------+
	| Wi                 |:math:`w_i`     | Bond work index                               | [kWh/t] | 1 ≤ Wi ≤ 100           |
	+--------------------+----------------+-----------------------------------------------+---------+------------------------+
	| Standard deviation |:math:`\sigma`  | Standard deviation of the output distribution | [m]     | Standard deviation > 0 |
	+--------------------+----------------+-----------------------------------------------+---------+------------------------+


.. seealso::

	a demostration file at ``Example Flowsheets/Units/Crusher Bond.dlfw``.


.. seealso::

	1. F.C. Bond, Crushing and grinding calculation – Part I, British Chemical Engineering 6 (6) (1961) 378-385.

	2. F.C. Bond, Crushing and grinding calculation – Part II, British Chemical Engineering 6 (8), (1961) 543-548.

	3. Denver Sala Basic: Selection Guide for Process Equipment, 1993.

|

Average Bond Work Indices for various materials
"""""""""""""""""""""""""""""""""""""""""""""""

+-----------------------------+-------------------------+--------------------+-------------------------+
| Material                    | Work Bond Index [kWh/t] | Material           | Work Bond Index [kWh/t] |
+=============================+=========================+====================+=========================+
| Andesite                    | 20.08                   | Iron ore, oolitic  | 12.46                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Barite                      | 5.2                     | Iron ore, taconite | 16.07                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Basalt                      | 18.18                   | Lead ore           | 13.09                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Bauxite                     | 9.66                    | Lead-zinc ore      | 12.02                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Cement clinker              | 14.8                    | Limestone          | 14                      |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Clay                        | 6.93                    | Manganese ore      | 13.42                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Coal                        | 14.3                    | Magnesite          | 12.24                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Coke                        | 16.84                   | Molybdenum         | 14.08                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Copper ore                  | 13.99                   | Nickel ore         | 15.02                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Diorite                     | 22.99                   | Oil shale          | 17.43                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Dolomite                    | 12.4                    | Phosphate rock     | 10.91                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Emery                       | 62.45                   | Potash ore         | 8.86                    |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Feldspar                    | 11.88                   | Pyrite ore         | 9.83                    |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Ferro-chrome                | 8.4                     | Pyrrhotite ore     | 10.53                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Ferro-manganese             | 9.13                    | Quartzite          | 10.54                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Ferro-silicon               | 11                      | Quartz             | 14.93                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Flint                       | 28.78                   | Rutile ore         | 13.95                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Fluorspar                   | 9.8                     | Shale              | 17.46                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Gabbro                      | 20.3                    | Silica sand        | 15.51                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Glass                       | 13.54                   | Silicon carbide    | 27.46                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Gneiss                      | 22.14                   | Slag               | 11.26                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Gold ore                    | 16.42                   | Slate              | 15.73                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Granite                     | 16.64                   | Sodium silicate    | 14.74                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Graphite                    | 47.92                   | Spodumene ore      | 11.41                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Gravel                      | 17.67                   | Syenite            | 14.44                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Gypsum rock                 | 7.4                     | Tin ore            | 11.99                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Iron ore ,hematite          | 14.12                   | Titanium ore       | 13.56                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Iron ore, hematite-specular | 15.22                   | Trap rock          | 21.25                   |
+-----------------------------+-------------------------+--------------------+-------------------------+
| Iron ore, magnetite         | 10.97                   | Zinc ore           | 12.72                   |
+-----------------------------+-------------------------+--------------------+-------------------------+

|

Cone model
^^^^^^^^^^

The model is described below as

.. math::

	w_{out,i} = \sum\limits^{i}_{k=0} w_{in,k} \cdot S_k \cdot B_{ki} + (1-S_i)\,w_{in,i}

.. note:: Notations:

	:math:`w_{out,i}` – mass fraction of particles with size :math:`i` in output distribution

	:math:`w_{in,i}` – mass fraction of particles with size :math:`i` in inlet distribution

	:math:`S_k` – mass fraction of particles with size :math:`k`, which will be crushed

	:math:`B_{ki}` – mass fraction of particles with size :math:`i`, which get size after breakage less or equal to :math:`k`



:math:`S_k` is described by the King selection function.

.. math::

	S_k =
	\begin{cases}
    0 											  & x_k \leqslant x_{min} \\
    1 - \dfrac{x_{max} - x_i}{x_{max} - x_{min}}  & x_{min} < x_k < x_{max} \\
    1											  & x_k \geqslant x_{max}
	\end{cases}


.. math::

	x_{min} = CSS \cdot \alpha_1

	x_{max} = CSS \cdot \alpha_2


.. note:: Notations:

	:math:`x_k` – mean particle diameter in size-class :math:`k`

	:math:`CSS` – close size setting of a cone crusher

	:math:`\alpha_1, \alpha_2, n` – parameters of the King selection function


:math:`B_{ki}` is calculated by the Vogel breakage function.

.. math::

	B_{ki} =
	\begin{cases}
	0.5\, \left( \dfrac{x_i}{x_k} \right)^q \cdot \left( 1 + \tanh \left( \dfrac{x_k - x'}{x'} \right) \right) & i \geqslant k \\
	0 & i < k
	\end{cases}


.. note:: Notations:

	:math:`x'` – minimum fragment size which can be achieved by crushing

	:math:`q` – parameter of the Vogel breakage function


.. note:: Solid phase and particle size distribution are required for the simulation.


.. note:: Input parameters needed for the simulation:

	+--------+------------------+---------------------------------------------------------------------------------------+-------+---------------------+
	| Name   | Symbol           | Description                                                                           | Units | Boundaries          |
	+========+==================+=======================================================================================+=======+=====================+
	| CSS    | :math:`CSS`      | Close size setting of a cone crusher. Parameter of the King selection function        | [m]   | CSS > 0             |
	+--------+------------------+---------------------------------------------------------------------------------------+-------+---------------------+
	| alpha1 | :math:`\alpha_1` | Parameter of the King selection function                                              | [--]  | 0.5 ≤ alpha1 ≤ 0.95 |
	+--------+------------------+---------------------------------------------------------------------------------------+-------+---------------------+
	| alpha2 | :math:`\alpha_2` | Parameter of the King selection function                                              | [--]  | 1.7 ≤ alpha2 ≤ 3.5  |
	+--------+------------------+---------------------------------------------------------------------------------------+-------+---------------------+
	| n      | :math:`n`        | Parameter of the King selection function                                              | [--]  | 1 ≤ n ≤ 3           |
	+--------+------------------+---------------------------------------------------------------------------------------+-------+---------------------+
	| d’     | :math:`x'`       | Minimum fragment size achieved by crushing. Parameter of the Vogel breakage function  | [m]   | d’ > 0              |
	+--------+------------------+---------------------------------------------------------------------------------------+-------+---------------------+
	| q      | :math:`q`        | Parameter of the Vogel breakage function                                              | [--]  |                     |
	+--------+------------------+---------------------------------------------------------------------------------------+-------+---------------------+


.. seealso::

	a demostration file at ``Example Flowsheets/Units/Crusher Cone.dlfw``.


.. seealso::

	1. King, R. P., Modeling and simulation of mineral processing systems, Butterworth & Heinemann, Oxford, 2001.

	2. Vogel, L., Peukert, W., Modelling of Grinding in an Air Classifier Mill Based on A Fundamental Material Function, KONA, 21, 2003, 109-120.


|

Const output model
^^^^^^^^^^^^^^^^^^

This model sets a normal distribution with the specified constant parameters to the output stream. Outlet distribution does not depend on the inlet distribution.

.. math::

	q_3(x) = \frac{1}{\sigma\sqrt{2\pi}}\,e^{-\dfrac{(x-\mu)^2}{2\sigma^2}}


.. note:: Notations:

	:math:`q_3(x)` – output mass related density distribution

	:math:`\sigma` – standard deviation of the output normal distribution

	:math:`\mu` – mean value of the output normal distribution


.. note:: Solid phase and particle size distribution are required for the simulation.


.. note:: Input parameters needed for the simulation:

	+--------------------+----------------+------------------------------------------------------+-------+------------------------+
	| Name               | Symbol         | Description                                          | Units | Boundaries             |
	+====================+================+======================================================+=======+========================+
	| Mean               | :math:`\mu`    | Mean of the normal output distribution               | [m]   | Mean > 0               |
	+--------------------+----------------+------------------------------------------------------+-------+------------------------+
	| Standard deviation | :math:`\sigma` | Standard deviation of the normal output distribution | [m]   | Standard deviation > 0 |
	+--------------------+----------------+------------------------------------------------------+-------+------------------------+


.. seealso::

	a demostration file at ``Example Flowsheets/Units/Crusher Const.dlfw``.

|
