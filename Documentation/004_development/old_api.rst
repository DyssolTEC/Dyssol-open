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

Transformation matrix
---------------------

:ref:`label-TM` (class ``CTransformMatrix``) are applied to describe laws of changes for multidimensional distributions. Each cell of matrix describes how much of material will be transferred from one class of multidimensional distribution to another.

|

.. code-block:: cpp

	CTransformMatrix()
	
Basic **constructor**. Creates an empty matrix.

|

.. code-block:: cpp

	CTransformMatrix (unsigned _nType, unsigned _nClasses)

Creates matrix to transform one-dimensional distribution with type ``_nType`` and ``_nClasses`` classes. ``_nType`` is one of the :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. 

All values in matrix will be set to 0.

|

.. code-block:: cpp

	CTransformMatrix(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2)

Creates matrix to transform two-dimensional distribution with types ``_nType1`` and ``_nType2`` and classes ``_nClasses1`` and ``_nClasses2``. ``_nType1`` and ``_nType2`` are types from :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. 

All values in matrix will be set to 0.

|

.. code-block:: cpp

	CTransformMatrix(const std::vector<unsigned> &_vTypes, const std::vector<unsigned> &_vClasses)

Creates transformation matrix for distribution with specified types and classes. ``_vTypes`` and ``_vClasses`` must have the same length. ``_vTypes`` is the vector of types from :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. 

All values in matrix will be set to 0.

|

Dimensions
""""""""""

.. code-block:: cpp

	bool SetDimensions(unsigned _nType, unsigned _nClasses)

Sets new dimensions set to the matrix in order to transform one-dimensional distribution with type ``_nType`` and ``_nClasses`` classes. ``_nType`` is one of the :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. 

Old data will be erased and matrix will be initialized with zeroes. Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetDimensions(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2)

Sets new dimensions set to the matrix in order to transform two-dimensional distribution. 

``_nType1`` and ``_nType2`` are types of the :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. Types must be unique. ``_nClasses1`` and ``_nClasses2`` are number of classes in corresponding distributions. 

Old data will be erased and matrix will be initialized with zeroes. Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetDimensions(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2, unsigned _nType3, unsigned _nClasses3)

Sets new dimensions set to the matrix in order to transform three-dimensional distribution. 

``_nType1``, ``_nType2`` and ``_nType3`` are one of the :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. Types must be unique.  ``_nClasses1``, ``_nClasses2`` and ``_nClasses3`` are number of classes in corresponding distributions. 

Old data will be erased and matrix will be initialized with zeroes. Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetDimensions(const std::vector<unsigned> &_vTypes, const std::vector<unsigned> &_vClasses)

Sets new dimensions set with types ``_vTypes`` and numbers of classes ``_vClasses``. ``_vTypes`` is the vector of :ref:`pre-defined types of solid distributions <label-EDistrTypes>`. 

All old data will be erased and matrix will be initialized with zeroes. Sizes of vectors ``_vTypes`` and ``_vClasses`` must be equal. Returns ``false`` on error.

|

.. code-block:: cpp

	std::vector<unsigned> GetDimensions()

Returns vector with all current defined dimensions types.

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

	double GetValue(unsigned _nCoordSrc, unsigned _nCoordDst)

Returns value by specified coordinates according to all defined dimensions in transformation matrix for one-dimensional distribution. ``_nCoordSrc`` is coordinate of a source class, ``_nCoordDst`` is coordinate of a destination class. 

Returning value is a mass fraction, which will be transferred from the source class to the destination class. Works with one-dimensional distribution only. Returns ``-1`` on error.

|

.. code-block:: cpp

	double GetValue(unsigned _nCoordSrc1, unsigned _nCoordSrc2, unsigned _nCoordDst1, unsigned _nCoordDst2)

Returns value by specified coordinates according to all defined dimensions in transformation matrix for two-dimensional distribution. ``_nCoordSrc1`` and ``_nCoordSrc2`` are coordinates of a source class, ``_nCoordDst1`` and ``_nCoordDst2`` are coordinate of a destination class. 

Returning value is a mass fraction, which will be transferred from the source class to the destination class. Works with two-dimensional distribution only. Returns ``-1`` on error.

|

.. code-block:: cpp

	double GetValue(const std::vector<unsigned> &_vCoordsSrc, const std::vector<unsigned> &_vCoordsDst)

Returns value by specified coordinates according to all defined dimensions. ``_vCoordsSrc`` are coordinates of a source class, ``_vCoordsDst`` are coordinates of a destination class. Sizes of vectors ``_vCoordsSrc`` and ``_vCoordsDst`` must be equal and must correspond to the number of currently defined dimensions. 

Returning value is a mass fraction, which will be transferred from the source class to the destination class. Returns ``-1`` on error.

|

.. code-block:: cpp

	double GetValue(const std::vector<unsigned> &_vDimsSrc, const std::vector<unsigned> &_vCoordsSrc, const std::vector<unsigned> &_vDimsDst, const std::vector<unsigned> &_vCoordsDst)

Returns value according to specified coordinates and dimensions. Number of dimensions must be the same as defined in the transformation matrix, but their sequence can be different. Sizes of all vectors must be equal. 

Returning value is a mass fraction, which will be transferred from the source class to the destination class. Returns ``-1`` on error.

|

.. code-block:: cpp

	bool GetVectorValue(const std::vector<unsigned> &_vCoordsSrc, const std::vector<unsigned> &_vCoordsDst, std::vector<double> &_vResult)

Returns vector value by specified coordinates according to all defined dimensions. Size of one vector of coordinates must be equal to the number of dimensions in transformation matrix; size of the second one must be one less. 

Returning value ``_vResult`` is a vector of mass fractions, which will be transferred from the source to the destination. Returns ``false`` on error.

|

.. code-block:: cpp

	bool GetVectorValue(const std::vector<unsigned> &_vDimsSrc, const std::vector<unsigned> &_vCoordsSrc, const std::vector<unsigned> &_vDimsDst, const std::vector<unsigned> &_vCoordsDst, std::vector<double> &_vResult)

Returns vector of values according to specified coordinates and dimensions sequence. Number of dimensions must be the same as defined in the transformation matrix, but their sequence can be different. Size of one vector of coordinates must be equal to the number of dimensions in transformation matrix; size of the second one must be one less. 

Returning value ``_vResult`` is a vector of mass fractions, which will be transferred from the source to the destination. Returns ``false`` on error.

|

Set data
""""""""

.. code-block:: cpp

	bool SetValue(unsigned _nCoordSrc, unsigned _nCoordDst, double _dValue)

Sets value by specified coordinates for one-dimensional distribution. ``_nCoordSrc`` is a coordinate of the source class; ``_nCoordDst`` is a coordinate of the destination class. ``_dValue`` is a mass fraction, which will be transferred from the source class to the destination class. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetValue (unsigned _nCoordSrc1, unsigned _nCoordSrc2, unsigned _nCoordDst1, unsigned _nCoordDst2, double _dValue)

Sets value by specified coordinates for two-dimensional distribution. 

``_nCoordSrc1`` and ``_nCoordSrc2`` are coordinate of the source class; ``_nCoordDst1`` and ``_nCoordDst2`` are coordinate of the destination class. ``_dValue`` is a mass fraction, which will be transferred from the source class to the destination class. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetValue (const std::vector<unsigned> &_vCoordsSrc, const std::vector<unsigned> &_vCoordsDst, double _dValue)

Sets value by specified coordinates and full dimensions set. ``_vCoordsSrc`` are coordinates of the source class, ``_vCoordsDst`` are coordinates of the destination class. Sizes of vectors ``_vCoordsSrc`` and ``_vCoordsDst`` must be equal. ``_dValue`` is a mass fraction, which will be transferred from the source class to the destination class. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetValue (const std::vector<unsigned> &_vDimsSrc, const std::vector<unsigned> &_vCoordsSrc, const std::vector<unsigned> &_vDimsDst, const std::vector<unsigned> &_vCoordsDst, double _dValue)

Sets value according to specified coordinates and dimensions. Number of dimensions must be the same as defined in the transformation matrix, but their sequence can be different. Sizes of all vectors must be equal. ``_dValue`` is a mass fraction, which will be transferred from the source class to the destination class. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetVectorValue (const std::vector<unsigned> &_vCoordsSrc, const std::vector<unsigned> &_vCoordsDst, const std::vector<double> &_vValue)

Sets vector of values by specified coordinates according to all defined dimensions. Size of one vector of coordinates must be equal to the number of dimensions in transformation matrix; size of the second one must be one less. ``_vValue`` is a vector of mass fractions, which will be transferred from the source to the destination. 

Returns ``false`` on error.

|

.. code-block:: cpp

	bool SetVectorValue (const std::vector<unsigned> &_vDimsSrc, const std::vector<unsigned> &_vCoordsSrc, const std::vector<unsigned> &_vDimsDst, const std::vector<unsigned> &_vCoordsDst, const std::vector<double> &_vValue)

Sets vector of values according to specified coordinates and dimensions sequence. Number of dimensions must be the same as defined in the transformation matrix, but their sequence can be different. Size of one vector of coordinates must be equal to the number of dimensions in transformation matrix; size of the second one must be one less. ``_vValue`` is a vector of mass fractions, which will be transferred from the source to the destination. 

Returns ``false`` on error.

|

Other functions
"""""""""""""""

.. code-block:: cpp

	void Normalize()

Normalizes data in matrix: sets sum of material which transfers from each single class to 1.

|

.. code-block:: cpp

	void ClearData()

Sets all data in matrix to 0.

|

.. code-block:: cpp

	void Clear()

Removes all data and information about dimensions from the matrix.

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
