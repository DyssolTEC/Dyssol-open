.. _sec.units.agglomerator:

Agglomerator
============

This unit represents a simplified model of agglomeration process, see figure below.

.. image:: ../images/003_models/agglomerator.png
   :width: 400px
   :alt:
   :align: center

The model does not take into account attrition of particles inside the apparatus and does not keep properly any secondary distributed property except size.

Mass related density distribution of output stream is calculated according to following equations:

.. math::

	\frac{\partial n(v,t)}{\partial t} = B_{agg}(n,v,t) - D_{agg}(n,v,t) + \dot{n}_{in}(t) - \dot{n}_{out}(t)

.. math::

	B_{agg}(n,v,t) = \frac{1}{2}\,\beta_0\,\textstyle \int\limits_{0}^{v} \beta(u,v - u)\,n(u,t)\,n(v-u,t)\,du

.. math::

	D_{agg}(n,v,t) = \beta_0\,n(v,t)\, \textstyle \int\limits_{0}^{\infty}\,\beta(v,u)\,n(u,t)\,du

.. math::

	\dot{m}_{out}(t) = \dot{m}_{in}(t)


.. note:: Notations:

	:math:`v,u` – volumes of agglomerating particles

	:math:`n(v,t)` – number density function

	:math:`\dot{n}_{in}(t)`, :math:`\dot{n}_{out}(t)` – number density functions of inlet and outlet streams, correspondingly

	:math:`B_{agg}(n,v,t)`, :math:`D_{agg}(n,v,t)` –  birth and death rates of particles with volume :math:`v` caused due to agglomeration

	:math:`\beta_0` – agglomeration rate constant, dependent on operating conditions but independent from particle sizes

	:math:`\beta(v,u)` – the agglomeration kernel, see section :ref:`label-agg-kernels`.

	:math:`t` – time

	:math:`\dot{m}_{in}` – mass flow in the input stream

	:math:`\dot{m}_{out}` – mass flow in the output stream


.. note:: solid phase and particle size distribution are required for the simulation.


The method of calculating :math:`B_{agg}(n,v,t)` and :math:`D_{agg}(n,v,t)` is determined by the selected solver via unit parameter :ref:`label-agg-solvers`.


.. note:: Input parameters needed for the simulation:

	+--------+-----------------+-----------------------------------------------------------------------+-------+-----------------------------+
	| Name   | Symbol          | Description                                                           | Units | Boundaries                  |
	+========+=================+=======================================================================+=======+=============================+
	| Beta0  | :math:`\beta_0` | Size independent agglomeration rate constant                          | [--]  | 0 < Beta0 ≤ :math:`10^{20}` |
	+--------+-----------------+-----------------------------------------------------------------------+-------+-----------------------------+
	| Step   | --              | Maximum time step of internal DAE solver. Default value is 0.         | [--]  | 0 ≤ Step ≤ :math:`10^{9}`   |
	+--------+-----------------+-----------------------------------------------------------------------+-------+-----------------------------+
	| Solver | --              | Solver used to calculate birth and death rates                        | [--]  | --                          |
	+--------+-----------------+-----------------------------------------------------------------------+-------+-----------------------------+
	| Kernel | --              | Agglomeration kernel type, must be an integer                         | [--]  | 0 ≤ Kernel ≤ 9              |
	+--------+-----------------+-----------------------------------------------------------------------+-------+-----------------------------+
	| Rank   | --              | Rank of the kernel (applied for FFT solver only), must be an integer  | [--]  | 1 ≤ Rank ≤ 10               |
	+--------+-----------------+-----------------------------------------------------------------------+-------+-----------------------------+


.. seealso::

	a demostration file at ``Example Flowsheets/Units/Agglomerator.dlfw``.


.. seealso::

	V.Skorych, M. Dosta, E.-U. Hartge, S. Heinrich, R. Ahrens, S. Le Borne, Investigation of an FFT-based solver applied to dynamic flowsheet simulation of agglomeration processes, Advanced Powder Technology 30 (3) (2019), 555-564.

|

.. _label-agg-kernels:

Kernels
^^^^^^^

The agglomeration kernels are applied to describe the agglomeration frequency between particles of volumes :math:`v` and :math:`u`, which produce a new particle with the size :math:`(v + u)`. In Dyssol environment, 10 types of kernels are numbered with integers from 0 to 9, as listed below.

	+--------+----------------+--------------------------------------------------------------------------------------------------------------------+
	| Number | Name           | Kernel equation                                                                                                    |
	+========+================+====================================================================================================================+
	| 0      | Constant       | :math:`\beta (u,v)=1`                                                                                              |
	+--------+----------------+--------------------------------------------------------------------------------------------------------------------+
	| 1      | Sum            | :math:`\beta (u,v)=u+v`                                                                                            |
	+--------+----------------+--------------------------------------------------------------------------------------------------------------------+
	| 2      | Product        | :math:`\beta (u,v)=uv`                                                                                             |
	+--------+----------------+--------------------------------------------------------------------------------------------------------------------+
	| 3      | Brownian       | :math:`\beta (u,v)=\left(u^{\frac{1}{3}}+v^{\frac{1}{3}} \right)\,\left(u^{-\frac{1}{3}}+v^{-\frac{1}{3}} \right)` |
	+--------+----------------+--------------------------------------------------------------------------------------------------------------------+
	| 4      | Shear          | :math:`\beta (u,v)=\left(u^{\frac{1}{3}}+v^{\frac{1}{3}} \right)^{\frac{7}{3}}`                                    |
	+--------+----------------+--------------------------------------------------------------------------------------------------------------------+
	| 5      | Peglow         | :math:`\beta (u,v)=\dfrac{ (u+v)^{0.71} }{(uv)^{0.062} }`                                                          |
	+--------+----------------+--------------------------------------------------------------------------------------------------------------------+
	| 6      | Coagulation    | :math:`\beta(u,v)=u^{\frac{2}{3}}+v^{\frac{2}{3}}`                                                                 |
	+--------+----------------+--------------------------------------------------------------------------------------------------------------------+
	| 7      | Gravitational  | :math:`\beta(u,v)=\left(u^{\frac{1}{3}}+v^{\frac{1}{3}} \right)^2 \left|u^{\frac{1}{6}}-v^{\frac{1}{6}} \right|`   |
	+--------+----------------+--------------------------------------------------------------------------------------------------------------------+
	| 8      | Kinetic energy | :math:`\beta(u,v)=\left(u^{\frac{1}{3}}+v^{\frac{1}{3}} \right)^2 \, \sqrt{\frac{1}{u}+\frac{1}{v}}`               |
	+--------+----------------+--------------------------------------------------------------------------------------------------------------------+
	| 9      | Thompson       | :math:`\beta(u,v)=\dfrac{(u-v)^2}{u+v}`                                                                            |
	+--------+----------------+--------------------------------------------------------------------------------------------------------------------+

|

Solvers
^^^^^^^

Currenly, several :ref:`label-agg-solvers` are available in Dyssol. Please refer to :ref:`label-solver` for more information about the solvers.

|
