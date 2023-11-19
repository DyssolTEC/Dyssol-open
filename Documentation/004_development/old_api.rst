.. _sec.old_classes:

=======
Old API
=======

Matrices
========

Several types of matrix classes, including the following types, are introduced in this section. 

- Dense 2-dimensional matrix: ``CDense2DMatrix``.

- Dense multidimensional matrix: ``CDenseMDMatrix``.

|

Two-dimensional matrix
----------------------

Basic information and functions of class ``CDense2DMatrix`` are introduced below.

|

.. code-block:: cpp

	CDense2DMatrix(CDense2DMatrix &_matrix)

**Copy constructor**. Creates matrix with the same dimensions as in ``_matrix`` and copies all data.

|

Get data
""""""""

.. code-block:: cpp

	double GetValue(unsigned _nRow, unsigned _nColumn)

Returns data by the specified indexes. Returns ``0`` if such indexes do not exist.

|

Set data
""""""""

.. code-block:: cpp

	void SetValue(unsigned _nRow, unsigned _ nColumn, double _dValue)

Sets data ``_dValue`` by the specified indexes.

|

Overloaded operators
""""""""""""""""""""

.. code-block:: cpp

	CDense2DMatrix& operator=(const CDense2DMatrix &_matrix)

Sets dimenions and data from the ``_matrix`` to a left matrix.

|

Other functions
"""""""""""""""

.. code-block:: cpp

	void ClearData()

Sets all elements in matrix to 0.

|

Multidimensional matrix
-----------------------

Basic information and functions of class ``CDenseMDMatrix`` are introduced below.

|

.. code-block:: cpp

	CDenseMDMatrix(const CDenseMDMatrix &_source)

**Copy constructor**. Creates matrix with the same dimensions as in ``_source`` and copies all data from there.

|

Overloaded operators
""""""""""""""""""""

.. code-block:: cpp

	CDenseMDMatrix& operator=(const CDenseMDMatrix &_matrix)

Sets dimenions and data from the ``_matrix`` to a left matrix.

|