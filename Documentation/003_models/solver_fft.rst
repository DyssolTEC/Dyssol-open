.. _sec.solvers.fft:

FFT
---

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
