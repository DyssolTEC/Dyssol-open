.. _sec.development.api.distr_functions:

Distributions functions
=======================

Several global functions are defined to work with particle size distributions. These functions can be called from any place of the code.

All functions receive grid (``Grid``) as the input parameter. The grid can be previously obtained with the help of the function ``GetNumericGrid``, for more information please refer to :ref:`getNumericGrid <label-getNumericGrid>` in section :ref:`label-baseUnit`. 

.. doxygenfile:: DistributionsFunctions.h
..    :members:
..    :protected-members:
..    :private-members:
..    :members-only:
..    :outline:
..    :no-link:
..    :allow-dot-graphs:

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