
.. _label-solver:

==============
Solver library
==============

Basic solver provides interfaces to the simulation system and to units. 

Currently in Dyssol, you can develop your own :ref:`agglomeration <label-agg>` solver and add it to the solver library. Please refer to :ref:`label-solverDev` for detailed information. 

The solver can also be added to a unit as a parameter.

.. image:: ../static/images/003_models/solver-structure.png
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

.. toctree::
	:maxdepth: 2

	solver_cellaverage
	solver_fft
	solver_fixedpivot

|
