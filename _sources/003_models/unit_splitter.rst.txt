.. _sec.units.splitter:

Splitter
========

A splitter divides input stream (:math:`In`) into two output streams (:math:`Out1` and :math:`Out2`), as shown in the figure below.

.. image:: ../images/003_models/splitter.png
   :width: 300px
   :alt: mixer
   :align: center

Both output streams are defined for the same set of time points for which the input stream has been defined. The splitting of input stream into more than two fractions can be done by sequential connection of several splitter units.

You can specify the splitting factor :math:`K_{splitt}`, which is defined in following equations. Here :math:`\dot{m}` is a mass flow.

.. math::

	\dot{m}_{out1} &= K_{splitt} \cdot \dot{m}_{in}

	\dot{m}_{out2} &= (1-K_{splitt} ) \cdot \dot{m}_{in}


.. note:: Notations:

	:math:`\dot{m}` - mass flow

	:math:`K_{splitt}` - Splitting factor


.. note:: Input parameters needed for the simulation:

	+---------+------------------+-------+-----------------+
	| Name    | Description      | Units | Boundaries      |
	+=========+==================+=======+=================+
	| Ksplitt | Splitting factor | [--]  | 0 ≤ Ksplitt ≤ 1 |
	+---------+------------------+-------+-----------------+


.. seealso::

	a demostration file at ``Example Flowsheets/Units/Splitter.dlfw``.

|
