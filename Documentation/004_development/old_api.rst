.. _sec.old_classes:

=======
Old API
=======

Matrices
========

Several types of matrix classes, including the following types, are introduced in this section. 

- Transform matrix: ``CTransformMatrix``.

- Dense 2-dimensional matrix: ``CDense2DMatrix``.

- Dense multidimensional matrix: ``CDenseMDMatrix``.

|

Two-dimensional matrix
----------------------

Basic information and functions of class ``CDense2DMatrix`` are introduced below.

|

.. code-block:: cpp

	CDense2DMatrix()

**Basic constructor**. Creates empty matrix with zero in all rows and columns.

|

.. code-block:: cpp

	CDense2DMatrix(CDense2DMatrix &_matrix)

**Copy constructor**. Creates matrix with the same dimensions as in ``_matrix`` and copies all data.

|

.. code-block:: cpp

	CDense2DMatrix(unsigned _nRows, unsigned _nColumns)

Creates new matrix with specified number of rows and columns. All data will be set to 0.

|

Dimensions
""""""""""

.. code-block:: cpp

	void SetDimensions(unsigned _nRows, unsigned _nColumns)

Sets new dimensions to the matrix. Old data is removed and all entries are set to zero.

|

.. code-block:: cpp

	unsigned GetRowsNumber()

Returns number of rows in the matrix.

|

.. code-block:: cpp

	unsigned GetColumnsNumber()

Returns number of columns in the matrix.

|

Get data
""""""""

.. code-block:: cpp

	double GetValue(unsigned _nRow, unsigned _nColumn)

Returns data by the specified indexes. Returns ``0`` if such indexes do not exist.

|

.. code-block:: cpp

	std::vector<double> GetRow(unsigned _nRow)

Returns vector of data for specified row. Returns empty vector if such row does not exist.

|

.. code-block:: cpp

	std::vector<double> GetColumn(unsigned _nColumn)

Returns vector of data for specified column. Returns empty vector if such column does not exist.

|

.. code-block:: cpp

	std::vector<std::vector<double>> GetMatrix()

Returns all data in form of vector-of-vectors.

|

Set data
""""""""

.. code-block:: cpp

	void SetValue(unsigned _nRow, unsigned _ nColumn, double _dValue)

Sets data ``_dValue`` by the specified indexes.

|

.. code-block:: cpp

	void SetRow(unsigned _nRow, const std::vector<double>& _Values)

Sets data ``_Values`` to a specified row.

|

.. code-block:: cpp

	void SetColumn(unsigned _nColumn, const std::vector<double>& _Values)

Sets data ``_Values`` to a specified column.

|

.. code-block:: cpp

	void SetMatrix(const std::vector<std::vector<double>>& _matr)

Sets all values in form vector-of-vectors ``_matr`` to matrix. ``_matr`` must have the same dimensions as the matrix itself.

|

Overloaded operators
""""""""""""""""""""

.. code-block:: cpp

	CDense2DMatrix operator+(const CDense2DMatrix& _Matrix1, const CDense2DMatrix &_Matrix2)

Performs **addition** of two matrices with the same dimensions. Returns an empty matrix in case of different dimensions.

|

.. code-block:: cpp

	CDense2DMatrix operator-(const CDense2DMatrix &_Matrix1, const CDense2DMatrix &_Matrix2)

Performs **subtraction** of two matrices with the same dimensions. Returns an empty matrix in case of different dimensions.

|

.. code-block:: cpp

	CDense2DMatrix operator*(double _dMultiplier)

Performs **multiplication** of the matrix with a coefficient ``_dMultiplier``.

|

.. code-block:: cpp

	CDense2DMatrix& operator=(const CDense2DMatrix &_matrix)

Sets dimenions and data from the ``_matrix`` to a left matrix.

|

Other functions
"""""""""""""""

.. code-block:: cpp

	void Normalize()

Normalizes the matrix so that the sum of all elements equals to 1.

|

.. code-block:: cpp

	void ClearData()

Sets all elements in matrix to 0.

|

.. code-block:: cpp

	void Clear()
	
Removes all data and sets number of rows and columns equal to 0.

|

Multidimensional matrix
-----------------------

Basic information and functions of class ``CDenseMDMatrix`` are introduced below.

|

.. code-block:: cpp

	CDenseMDMatrix()

**Basic constructor**. Creates an empty matrix.

|

.. code-block:: cpp

	CDenseMDMatrix(const CDenseMDMatrix &_source)

**Copy constructor**. Creates matrix with the same dimensions as in ``_source`` and copies all data from there.

|

Dimensions
""""""""""

.. code-block:: cpp

	bool SetDimensions(unsigned _nType, unsigned _nClasses)

Sets **one-dimensional** distribution of type _nType and numbers of classes ``_nClasses``. 

``_nType`` is one of :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. All old data will be erased. Matrix will be initialized with zero values. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetDimensions(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2)

Sets **two-dimensional** distribution of types ``_nType1`` and ``_nType2`` and numbers of classes ``_nClasses1`` and ``_nClasses2``. 

``_nType1`` and ``_nType2`` are from :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. All types must be unique. All old data will be erased. Matrix will be initialized with zero values. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetDimensions(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2, unsigned _nType3, unsigned _nClasses3)

Sets **three-dimensional** distribution of types ``_nType1``, ``nType2`` and ``_nType3`` and numbers of classes ``_nClasses1``, ``_nClasses2`` and ``_nClasses3``. 

``_nType1``, ``nType2`` and ``_nType3`` are from :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. All types must be unique. All old data will be erased. Matrix will be initialized with zero values. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetDimensions(const std::vector<unsigned> &_vTypes, const std::vector<unsigned> &_vClasses)

Sets types ``_vTypes`` of dimensions and numbers of classes ``_vClasses``. 

``_vTypes`` is the vector of :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. All types must be unique. All old data will be erased. Matrix will be initialized with zero values. 

Returns ``false`` on error.

|

.. code-block:: cpp

	std::vector<unsigned> GetDimensions()

Returns vector with all currently defined dimensions types.

|

.. code-block:: cpp

	std::vector<unsigned> GetClasses()

Returns vector with current numbers of classes.

|

.. code-block:: cpp

	unsigned GetDimensionsNumber() 

Returns current number of dimensions.

|

Get data
""""""""

.. code-block:: cpp

	double GetValue(unsigned _nDim, unsigned _nCoord)

Returns value with specified coordinate ``_nCoord`` of specified dimension ``_nDim``. It is possible to use this function if matrix has more than one dimension: if number of dimensions does not conform to the matrix, the sum of values by remaining dimensions will be returned. 

Returns ``-1`` on error.

|

.. code-block:: cpp

	double GetValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2)

Returns value according to specified coordinates and dimensions. It is possible to use this function if matrix has more than two dimensions: if number of dimensions does not conform to the matrix, the sum of values by remaining dimensions will be returned. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns ``-1`` on error.

|

.. code-block:: cpp

	double GetValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nDim3, unsigned _nCoord3)

Returns value according to specified coordinates and dimensions. It is possible to use this function if matrix has more than three dimensions: if number of dimensions does not conform to the matrix, the sum of values by remaining dimensions will be returned. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns ``-1`` on error.

|

.. code-block:: cpp

	double GetValue(const std::vector<unsigned> &_vDims, const std::vector<unsigned> &_vCoords)

Returns value according to specified coordinates and dimensions. It is possible to use this function if matrix has more dimensions than was defined in ``_vDims``: if number of dimensions does not conform to the matrix, the sum of values by remaining dimensions will be returned. Sequence of dimensions may not match the sequence, which was defined in the matrix. Number of dimensions ``_vDims`` and coordinates ``_vCoords`` must be the same. 

Returns ``-1`` on error.

|

.. code-block:: cpp

	double GetValue(const std::vector<unsigned> &_vCoords)

Returns value by specified coordinates according to the full defined set of dimensions. Returns ``-1`` on error.

|

.. code-block:: cpp

	bool GetVectorValue(unsigned _nDim, std::vector<double> &_vResult)

Returns vector ``_vResult`` according to specified dimension. If number of dimensions in the matrix is more than one, then the sum of values by remaining dimensions will be returned. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool GetVectorValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, std::vector<double> &_vResult)

Returns vector of values ``_vResult`` according to specified dimensions and coordinate. If number of dimensions in the matrix is more than two, then the sum of values by remaining dimensions will be returned. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool GetVectorValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nDim3, std::vector<double> &_vResult)

Returns vector of values ``_vResult`` according to specified dimensions and coordinates. If number of dimensions in the matrix is more than three, then the sum of values by remaining dimensions will be returned. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool GetVectorValue(const std::vector<unsigned> &_vDims, const std::vector<unsigned> &_vCoords, std::vector<double> &_vResult) 

Returns vector of values ``_vResult`` according to specified dimensions and coordinates. If number of dimensions in the matrix is more than it was specified in ``_vDims``, then the sum of values by remaining dimensions will be returned. Sequence of dimensions may not match the sequence, which was defined in the matrix. Number of coordinates ``_vCoords`` must be one less than the number of dimensions ``_vDims``. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool GetVectorValue(const std::vector<unsigned> &_vCoords, std::vector<double> &_vResult)

Returns vector of values ``_vResult`` by specified coordinates according to the full defined set of dimensions. Returns ``false`` on error.

|

Set data
""""""""

.. code-block:: cpp

	bool SetValue(unsigned _nCoord, double _dValue)

Sets value ``_dValue`` with coordinate ``_nCoord`` into one-dimensional matrix. Sets the value only if the matrix has one dimension. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nCoord2, double _dValue)

Sets value ``_dValue`` according to specified coordinates and dimensions into two-dimensional matrix. Sets the value only if the matrix has two dimensions. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns false on ``error``.

|

.. code-block:: cpp

	bool SetValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nCoord3, double _dValue)

Sets value ``_dValue`` according to specified coordinates and dimensions into three-dimensional matrix. Sets the value only if the matrix has three dimensions. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetValue(const std::vector<unsigned> &_vDims, const std::vector<unsigned> &_vCoords, double _dValue)

Sets value ``_dValue`` according to specified coordinates and dimensions. Sets the value only if the number of dimensions is the same as in the matrix. Number of dimensions ``_vDims`` and coordinates ``_vCoords`` must be the same. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetValue(const std::vector<unsigned> &_vCoords, double _dValue)

Sets value ``_dValue`` according to specified coordinates for full set of dimensions. Sets the value only if the number of coordinates is the same as the number of dimensions in the matrix. Number of coordinates ``_vCoords`` must be equal to a number of dimensions in matrix. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetVectorValue(const std::vector<double> &_vValue)

Sets vector of values ``_vValue`` in one-dimensional matrix. Sets the values only if the matrix has one dimension. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetVectorValue(unsigned _nDim, unsigned _nCoord, const std::vector<double> &_vValue)

Sets vector of values ``_vValue`` according to specified dimension and coordinate in two-dimensional matrix. Sets the values only if the matrix has two dimensions. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetVectorValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, const std::vector<double> &_vValue)

Sets vector of values ``_vValue`` according to specified dimensions and coordinates in three-dimensional matrix. Sets the values only if the matrix has three dimensions. Sequence of dimensions may not match the sequence, which was defined in the matrix. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetVectorValue(const std::vector<unsigned> &_vDims, const std::vector<unsigned> &_vCoords, const std::vector<double> &_vValue) 

Sets vector of values ``_vValue`` according to specified dimensions and coordinates. Sets values only if the number of dimensions in ``_vDims`` is one less than in the matrix. Number of dimensions ``_vDims`` and coordinates ``_vCoords`` must be equal. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetVectorValue (const std::vector<unsigned> &_vCoords, const std::vector<double> &_vValue) 

Sets vector of values ``_vValue`` according to specified coordinates for full set of dimensions, which were defined in the matrix. Sets values only if the number of coordinates is one less than number of dimensions in the matrix. 

Returns ``false`` on error.

|

Overloaded operators
""""""""""""""""""""

.. code-block:: cpp
	
	CDenseMDMatrix operator+(const CDenseMDMatrix &_matrix)

Performs **addition** of the matrix with the same dimensions. Returns an empty matrix with 0 defined dimensions in case of different dimenions.

|

.. code-block:: cpp

	CDenseMDMatrix operator-(const CDenseMDMatrix &_matrix)

Performs **subtraction** of matrices with the same dimensions. Returns an empty matrix with 0 defined dimensions in case of different dimenions.

|

.. code-block:: cpp

	CDenseMDMatrix operator*(double _dFactor)

Performs **multiplication** of the matrix by a coefficient ``_dFactor``.

|

.. code-block:: cpp

	CDenseMDMatrix& operator=(const CDenseMDMatrix &_matrix)

Sets dimenions and data from the ``_matrix`` to a left matrix.

|

Other functions
"""""""""""""""

.. code-block:: cpp

	void Normalize()

Normalizes the matrix so that the sum of all elements equals to 1.

|

.. code-block:: cpp

	bool IsNormalized()

Returns ``true`` if the matrix is normalized.

|

.. code-block:: cpp

	void ClearData()

Sets all elements in matrix equal to 0.

|

.. code-block:: cpp

	void Clear()

Removes all data and information about dimensions from the matrix.

|
