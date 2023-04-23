.. _sec.units.screen:

Screen
======

Screen unit is designed for classification of input material into two fractions according to particle size distribution (PSD), as shown below.

.. image:: ../static/images/003_models/screen.png
   :width: 300px
   :alt: screen
   :align: center


In Dyssol, 4 models are available to describe the screen grade efficiency:

	- Plitt's model
	- Molerus & Hoffmann model
	- Probability model
	- Teipel / Hennig model

In the following figure, several grade efficiency curves for different parameters of separations sharpness are shown.

.. note:: This figure only applies to the Plitt's model and Molerus & Hoffmann model.

.. image:: ../static/images/003_models/splitter-alpha.png
   :width: 500px
   :alt: splitter
   :align: center

|

.. _label-screenPlitt:

Plitt's model
^^^^^^^^^^^^^

This model is described using the following equation:

.. math::

	G(x_i) = 1 - exp\left(-0.693\,\left(\frac{x_i}{x_{cut}}\right)^\alpha\right)


.. note:: Notations applied in the models:

	:math:`G(x_i)` – grade efficiency: mass fraction of material within the size class :math:`i` in the feed (:math:`\dot{m}_{i,input}`) that leaves the screen in the coarse stream (:math:`\dot{m}_{i,coarse}`)

	:math:`x_{cut}` – cut size of the classification model in meter

	:math:`\alpha` – sharpness of separation

	:math:`x_i` – size of a particle


.. note:: Input parameters needed for the simulation:

	+-------+----------------+--------------------------------------+-------+-----------------+
	| Name  |Symbol          | Description                          | Units | Boundaries      |
	+=======+================+======================================+=======+=================+
	| Xcut  |:math:`x_{cut}` | Cut size of the classification model | [m]   | Xcut > 0        |
	+-------+----------------+--------------------------------------+-------+-----------------+
	| Alpha |:math:`\alpha`  | Sharpness of separation              | [--]  | 0 ≤ Alpha ≤ 100 |
	+-------+----------------+--------------------------------------+-------+-----------------+

.. seealso::

	a demostration file at ``Example Flowsheets/Units/Screen Plitt.dlfw``.


.. seealso::

	Plitt, L.R.: The analysis of solid–solid separations in classifiers. CIM Bulletin 64 (708), p. 42–47, 1971.

|

Molerus & Hoffmann model
^^^^^^^^^^^^^^^^^^^^^^^^

This model is described using the following equation:

.. math::

	G(x_i) = \dfrac{1}{1 + \left( \dfrac{x_{cut}}{x_i} \right)^2 \cdot exp\left( \alpha \,\left( 1 - \left(\dfrac{x_i}{x_{cut}}\right)^2 \right)\right)}


.. note:: Notations applied in the models:

	:math:`G(x_i)` – grade efficiency: mass fraction of material within the size class :math:`i` in the feed that leaves the screen in the coarse stream

	:math:`x_{cut}` – cut size of the classification model

	:math:`\alpha` – sharpness of separation

	:math:`x_i` – size of a particle


.. note:: Input parameters needed for the simulation:

	+-------+-----------------+--------------------------------------+-------+-----------------+
	| Name  | Symbol          | Description                          | Units | Boundaries      |
	+=======+=================+======================================+=======+=================+
	| Xcut  | :math:`x_{cut}` | Cut size of the classification model | [m]   | Xcut > 0        |
	+-------+-----------------+--------------------------------------+-------+-----------------+
	| Alpha | :math:`\alpha`  | Sharpness of separation              | [--]  | 0 < Alpha ≤ 100 |
	+-------+-----------------+--------------------------------------+-------+-----------------+

.. seealso::

	a demostration file at ``Example Flowsheets/Units/Screen Molerus-Hoffmann.dlfw``.


.. seealso::

	Molerus, O.; Hoffmann, H.: Darstellung von Windsichtertrennkurven durch ein stochastisches Modell, Chemie Ingenieur Technik, 41 (5+6), 1969, pp. 340-344.

|

Probability model
^^^^^^^^^^^^^^^^^

This model is described using the following equation:

.. math::

	G(x_i) = \dfrac{ \sum\limits^{x_i}_{0} e^{-\dfrac{(x_i - \mu)^2}{2\sigma^2}} }{ \sum\limits^{N}_{0} e^{-\dfrac{(x_i - \mu)^2}{2\sigma^2}} }


.. note:: Notations applied in this model:

	:math:`G(x_i)` – grade efficiency: mass fraction of material within the size class :math:`i` in the feed that leaves the screen in the coarse stream

	:math:`x_i` – size of a particle

	:math:`\sigma` – standard deviation of the normal output distribution

	:math:`\mu` – mean of the normal output distribution

	:math:`N` – number of classes of particle size distribution


.. note:: Input parameters needed for the simulation:

	+------------------------+----------------+----------------------------------------------------------------+-------+-------------------------+
	| Name                   | Symbol         | Description                                                    | Units | Boundaries              |
	+========================+================+================================================================+=======+=========================+
	| Mean                   |:math:`\mu`     | Mean of the normal output distribution                         | [m]   | Mean > 0                |
	+------------------------+----------------+----------------------------------------------------------------+-------+-------------------------+
	| Standard deviation     |:math:`\sigma`  | Standard deviation of the normal output distribution           | [m]   | Standard deviation > 0  |
	+------------------------+----------------+----------------------------------------------------------------+-------+-------------------------+


.. seealso::

	a demostration file at ``Example Flowsheets/Units/Screen Probability.dlfw``.


.. seealso::
	Radichkov, R.; Müller, T.; Kienle, A.; Heinrich, S.; Peglow, M.; Mörl, L.: A numerical bifurcation analysis of continuous fluidized bed spray granulation with external product classification, Chemical Engineering and Processing 45, 2006, pp. 826–837.

|

Teipel / Hennig model
^^^^^^^^^^^^^^^^^^^^^

This model is described using the following equation:

.. math::

	G(x_i) = \left(  1-   \left( 1 + 3 \cdot \left( \dfrac{x_i}{x_{cut}} \right)^{\left(\dfrac{x_i}{x_{cut}} + \alpha \right)\cdot \beta} \right)^{-1/2}	\right) \cdot (1 - a) + a


.. note:: Notations applied in the models:

	:math:`G(x_i)` – grade efficiency: mass fraction of material within the size class :math:`i` in the feed that leaves the screen in the coarse stream

	:math:`x_{cut}` – cut size of the classification model

	:math:`\alpha` – sharpness of separation

	:math:`\beta` - sharpness of separation

	:math:`a` - separation offset

	:math:`x_i` – size of a particle


.. note:: Input parameters needed for the simulation:

	+-------+----------------+----------------------------------------+-------+-----------------+
	| Name  |Symbol          | Description                            | Units | Boundaries      |
	+=======+================+========================================+=======+=================+
	| Xcut  |:math:`x_{cut}` |   Cut size of the classification model | [m]   | Xcut > 0        |
	+-------+----------------+----------------------------------------+-------+-----------------+
	| Alpha |:math:`\alpha`  |   Sharpness of separation 1            | [--]  | 0 < Alpha ≤ 100 |
	+-------+----------------+----------------------------------------+-------+-----------------+
	| Beta  |:math:`\beta`   |  Sharpness of separation 2             | [--]  | 0 < Beta ≤ 100  |
	+-------+----------------+----------------------------------------+-------+-----------------+
	| Offset|:math:`a`       | Separation offset                      | [--]  | 0 ≤ Offset ≤ 1  |
	+-------+----------------+----------------------------------------+-------+-----------------+

.. seealso::

	a demostration file at ``Example Flowsheets/Units/Screen Teipel-Hennig.dlfw``.


.. seealso::

	Hennig, M. and Teipel, U. (2016), Stationäre Siebklassierung. Chemie Ingenieur Technik, 88: 911–918.

|
