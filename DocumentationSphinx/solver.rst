
.. _label-solver:

==============
Solver library
==============

Basic solver provides interfaces to the simulation system and to units. 

Currently in Dyssol, you can develop your own :ref:`agglomeration <label-agg>` solver and add it to the solver library. Please refer to :ref:`label-solverDev` for detailed information. 

The solver can also be added to a unit as a parameter.

.. image:: ./pics/solver-structure.png
   :width: 600px
   :alt: 
   :align: center 

|

.. _label-agg-solvers:

Agglomeration solvers
=====================

Solvers are applied to calculate the birth and death rate of agglomerates according to different mathematical models. In Dyssol, 3 types of solver are available:

	- Cell average
	- Fast Fourier transformation (FFT)
	- Fixed pivot


The applied equations in all solvers are listed as follows. 

.. math::

	\frac{\partial n(v,t)}{\partial t} = B_{agg}(n,v,t) - D_{agg}(n,v,t)

.. math::

	B_{agg}(n,v,t) = \frac{1}{2}\,\beta_0\,\textstyle \int\limits_{0}^{v} \beta(u,v - u)\,n(u,t)\,n(v-u,t)\,du

.. math::
	
	D_{agg}(n,v,t) = \beta_0\,n(v,t)\, \textstyle \int\limits_{0}^{\infty}\,\beta(v,u)\,n(u,t)\,du


.. note:: Notations:

	:math:`v,u` – volumes of agglomerating particles
	
	:math:`n(v,t)` – number density function
	
	:math:`B_{agg}(n,v,t)`, :math:`D_{agg}(n,v,t)` –  birth and death rates of particles with volume :math:`v` caused due to agglomeration
	
	:math:`\beta_0` – agglomeration rate constant, dependent on operating conditions but independent from particle sizes
	
	:math:`\beta(v,u)` – the agglomeration kernel, see section :ref:`label-agg-kernels`.
	
	:math:`t` – time
		
|

Cell average solver
-------------------

This solver calculates the birth rate :math:`B_{agg}(n,v,t)` and death rate :math:`D_{agg}(n,v,t)` using a cell average technique. 


.. note:: solid phase and particle size distribution are required for the simulation. Equidistant volume grid for particle size distribution must be used.


.. seealso:: J.Kumar, M. Peglow, G. Warnecke, S. Heinrich, An efficient numerical technique for solving population balance equation involving aggregation, breakage, growth and nucleation, Powder Technology 182 (1) (2008), 81-104.

|

FFT solver
----------

This solver calculates the birth rate :math:`B_{agg}(n,v,t)` and death rate :math:`D_{agg}(n,v,t)` based on a separable approximation of the agglomeration kernel and a subsequent fast Fourier transformation.

The agglomeration kernel :math:`\beta (v,u)` is represented in a separable form with the separation rank :math:`M` as:

.. math::
	
	\beta (v,u) = \sum\limits_{i=1}^{M} a_i(v)\,b_i(u)
	
Then birth and death rate terms are transformed to following equations with :math:`\psi_i = a_i\,n` and :math:`\varphi_i = b_i\,n`.

.. math::

	B_{agg}(n,v,t) = \frac{1}{2}\,\textstyle \int\limits_{0}^{v} \psi_i(v - u,t)\,\varphi_i(u,t)\,du

.. math::
	
	D_{agg}(n,v,t) = \psi_i(v,t)\, \textstyle \int\limits_{0}^{\infty}\varphi_i(u,t)\,du	

The separation turns the birth rate agglomeration integral into a convolution form :math:`\varphi_i * \psi_i`, which after piecewise constant discretization, is computed based on the convolution theorem

.. math::
	
	\varphi_i * \psi_i = IFFT(FFT(\varphi_i) \odot FFT(\psi_i))

applying the direct and inverse fast Fourier transformation (:math:`FFT/IFFT`) and the elementwise product :math:`\odot`.


.. note:: List of kernels separation equations
	
	+------------------------------------------------------------------------------+-------------------------------------------------------------------------------------------------------+
	| Kernel                                                                       | Separation equation                                                                                   |
	+==============================================================================+=======================================================================================================+
	| Constant                                                                     | :math:`\beta(u,v) = \sum\limits_{i=1}^1 a_i(v)\,b_i(u),`                                              |
	|                                                                              | :math:`a_1 (v) = 1, b_1 (u) = 1`                                                                      |
	+------------------------------------------------------------------------------+-------------------------------------------------------------------------------------------------------+
	| Sum                                                                          | :math:`\beta(u,v) = \sum\limits_{i=1}^2 a_i(v)\,b_i(u),`                                              |
	|                                                                              | :math:`a_1 (v) = v, a_2 (v)= 1, b_1 (u) = 1, b_2 (u)= u`                                              |
	+------------------------------------------------------------------------------+-------------------------------------------------------------------------------------------------------+
	| Brownian                                                                     | :math:`\beta(u,v) = \sum\limits_{i=1}^3 a_i(v)\,b_i(u),`                                              |
	|                                                                              | :math:`a_1 (v) = \sqrt{2}, a_2 (v) = v^{\frac{1}{3}}, a_3 (v)= v^{-\frac{1}{3}}`                      |
	|                                                                              | :math:`b_1 (u) = \sqrt{2}, b_2 (u) = u^{-\frac{1}{3}}, b_3 (u)= u^{\frac{1}{3}}`                      |
	+------------------------------------------------------------------------------+-------------------------------------------------------------------------------------------------------+
	| Product, Shear, Peglow, Coagulation, Gravitational, Kinetic energy, Thompson | Approximated by a rank-M separable function                                                           |
	|                                                                              | :math:`\beta (v,u) \approx \sum\limits_{i=1}^{M} a_i(v)\,b_i(u)`                                      |
	|                                                                              | using adaptive cross approximation                                                                    |
	+------------------------------------------------------------------------------+-------------------------------------------------------------------------------------------------------+



.. note:: solid phase and particle size distribution are required for the simulation. Equidistant volume grid for particle size distribution must be used. Grid for particle size distribution must start from 0.


.. seealso:: 

	1. V.Skorych, M. Dosta, E.-U. Hartge, S. Heinrich, R. Ahrens, S. Le Borne, Investigation of an FFT-based solver applied to dynamic flowsheet simulation of agglomeration processes, Advanced Powder Technology 30 (3) (2019), 555-564. 

	2. S.Le Borne, L. Shahmuradyan, K. Sundmacher, Fast evaluation of univariate aggregation integrals on equidistant grids. Computers and Chemical Engineering 74 (2015), 115-127.
	
|

Fixed pivot solver
------------------

This solver calculates the birth rate :math:`B_{agg}(n,v,t)` and death rate :math:`D_{agg}(n,v,t)` using a fixed pivot technique. 


.. note:: solid phase and particle size distribution are required for the simulation. 


.. seealso:: S.Kumar, D. Ramkrishna, On the solution of population balance equations by discretization – I. A fixed pivot technique. Chem. Eng. Sci. 51 (8) (1996), 1311-1332.






















