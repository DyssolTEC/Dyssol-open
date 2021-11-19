/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <vector>
#include <cstddef>

/** Multidimensional matrix in dense format.*/
class CDenseMDMatrix
{
private:
	std::vector<unsigned> m_vDimensions;	///< Types of the distributions
	std::vector<unsigned> m_vClasses;		///< Number of classes of the distributions
	std::vector<double> m_vData;			///< Data itself

public:
	CDenseMDMatrix();
	CDenseMDMatrix(const std::vector<unsigned>& _vTypes, const std::vector<unsigned>& _vClasses);

	// ============= Functions to work with DIMENSIONS

	/** Sets new dimensions set with erasing of old data. Returns true if success.*/
	bool SetDimensions(unsigned _nType, unsigned _nClasses);
	/** Sets new dimensions set with erasing of old data. Returns true if success.*/
	bool SetDimensions(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2);
	/** Sets new dimensions set with erasing of old data. Returns true if success.*/
	bool SetDimensions(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2, unsigned _nType3, unsigned _nClasses3);
	/** Sets new dimensions set with erasing of old data. Returns true if success.*/
	bool SetDimensions( const std::vector<unsigned>& _vTypes, const std::vector<unsigned>& _vClasses );
	/** Returns vector of defined types.*/
	std::vector<unsigned> GetDimensions() const;
	/** Returns vector of defined classes.*/
	std::vector<unsigned> GetClasses() const;
	/** Returns current number of defined dimensions.*/
	size_t GetDimensionsNumber() const;
	/** Clears all data and information about dimensions.*/
	void Clear();

	// ============= Functions to work with DATA

	/** Clears all data in the matrix by setting all values to 0. Dimensions set won't be erased.*/
	void ClearData();

	/** Returns pointer to data.*/
	const double* GetDataPtr() const;
	double* GetDataPtr();

	/** Returns length of the plain array m_vData*/
	size_t GetDataLength() const;

	// ============= Data GETTERS

	/** Returns value by specified dimension and coordinate with possible reducing of dimensions set. Returns -1 on error.*/
	double GetValue( unsigned _nDim, unsigned _nCoord ) const;
	/** Returns value by specified dimensions and coordinates with possible reducing of dimensions set. Returns -1 on error.*/
	double GetValue( unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2 ) const;
	/** Returns value by specified dimensions and coordinates with possible reducing of dimensions set. Returns -1 on error.*/
	double GetValue( unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nDim3, unsigned _nCoord3 ) const;
	/** Returns value by specified coordinates according to the full defined set of dimensions. Returns -1 on error.*/
	double GetValue( const std::vector<unsigned>& _vCoords ) const;
	/** Returns value by specified coordinates and dimensions sequence. Dimensions set can be reduced. Returns -1 on error.*/
	double GetValue( const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords ) const;

	/** Returns vector of values according to specified dimension with possible reducing of dimensions set.*/
	std::vector<double> GetVectorValue(unsigned _nDim) const;
	/** Returns vector of values according to specified dimension with possible reducing of dimensions set. Returns false on error.*/
	bool GetVectorValue( unsigned _nDim, std::vector<double>& _vResult ) const;
	/** Returns vector of values according to specified dimensions and coordinates with possible reducing of dimensions set. Returns false on error.*/
	bool GetVectorValue( unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, std::vector<double>& _vResult ) const;
	/** Returns vector of values according to specified dimensions and coordinates with possible reducing of dimensions set. Returns false on error.*/
	bool GetVectorValue( unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nDim3, std::vector<double>& _vResult ) const;
	/** Returns vector of values according to specified dimensions and coordinates with possible reducing of dimensions set. Returns false on error.*/
	std::vector<double> GetVectorValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nDim3) const;
	/** Returns vector of values by specified coordinates according to a full defined dimensions set. Returns false on error.*/
	bool GetVectorValue( const std::vector<unsigned>& _vCoords, std::vector<double>& _vResult ) const;
	/** Returns vector of values according to specified dimensions and coordinates. Dimensions set can be reduced. Returns false on error.*/
	bool GetVectorValue( const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, std::vector<double>& _vResult ) const;
	/** Returns vector of values according to specified dimensions and coordinates. Dimensions set can be reduced. Returns false on error.*/
	std::vector<double> GetVectorValue(const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords) const;

	// ============= Data SETTERS

	/** Sets value in 1D-matrix by specified coordinate. Sets value only if the number of dimensions is the same as in the matrix. Returns false on error.*/
	bool SetValue( unsigned _nCoord, double _dValue );
	/** Sets value in 2D-matrix by specified dimensions and coordinates. Sets value only if the number of dimensions is the same as in the matrix. Returns false on error.*/
	bool SetValue( unsigned _nDim1, unsigned _nCoord1, unsigned _nCoord2, double _dValue );
	/** Sets value in 3D-matrix by specified dimensions and coordinates. Sets value only if the number of dimensions is the same as in the matrix. Returns false on error.*/
	bool SetValue( unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nCoord3, double _dValue );
	/** Sets value by specified coordinates according to the full defined set of dimensions.
	*	Sets value only if the number of dimensions is the same as in the matrix. Returns false on error.*/
	bool SetValue( const std::vector<unsigned>& _vCoords, double _dValue );
	/** Sets value by specified coordinates and dimensions sequence.
	*	Sets value only if the number of dimensions is the same as in the matrix. Returns false on error.*/
	bool SetValue( const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, double _dValue );

	/** Sets vector of values in 1D-matrix. Returns false on error.*/
	bool SetVectorValue( const std::vector<double>& _vValue );
	/** Sets vector of values according to specified dimensions and coordinates in 2D-matrix. Returns false on error.*/
	bool SetVectorValue( unsigned _nDim, unsigned _nCoord, const std::vector<double>& _vValue );
	/** Sets vector of values according to specified dimensions and coordinates in 3D-matrix. Returns false on error.*/
	bool SetVectorValue( unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, const std::vector<double>& _vValue );
	/** Sets vector of values by specified coordinates according to a full defined dimensions set.
	*	Sets values only if the number of coordinates is one less than number of dimensions in the matrix. Returns false on error.*/
	bool SetVectorValue( const std::vector<unsigned>& _vCoords, const std::vector<double>& _vValue );
	/** Sets vector of values according to specified dimensions and coordinates.
	*	Sets values only if the number of dimensions is one less than in the matrix. Returns false on error.*/
	bool SetVectorValue( const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, const std::vector<double>& _vValue );


	// ========== Normalization

	bool IsNormalized();
	void Normalize();

	// ========== Overloaded operators

	/** Adds matrix with the same dimensions. If dimensions are not the same, than the empty matrix will be returned.*/
	CDenseMDMatrix operator+( const CDenseMDMatrix& _matrix );
	/** Subtracts matrix with the same dimensions. If dimensions are not the same, than the empty matrix will be returned.*/
	CDenseMDMatrix operator-( const CDenseMDMatrix& _matrix );
	/** Multiplication of the matrix by a coefficient.*/
	CDenseMDMatrix operator*( double _dFactor );


private:
	/** Checks the duplicates in dimensions vector. Return true if check is passed.*/
	bool CheckDuplicates( const std::vector<unsigned>& _vDims ) const;

	/** Returns index in m_vData according to incoming dimensions and coordinates.
	*	Doesn't check sizes of dimensions and coordinates in parameters. Returns -1 on error.*/
	size_t GetIndex( const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords ) const;
	/**  If returns true: in _pResIndex will be an index in m_vData according to incoming dimensions and coordinates,
		in _pResStep will be offset between values in m_vData to work with vector, in _pResVecLength - length of the result vector.*/
	bool GetIndexAndStep(const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, size_t* _pResIndex, size_t* _pResStep, size_t* _pResVecLength) const;

	/** Returns value by specified dimension and coordinate. Doesn't check sizes of dimensions and coordinates in parameters.
	*	Returns 0 if there is no such combination of dimensions and coordinates.*/
	double GetValueRecursive( std::vector<unsigned> _vDims, std::vector<unsigned> _vCoords ) const;
	/** Returns vector value by specified dimension and coordinate. Doesn't check sizes of dimensions and coordinates in parameters.
	*	Returns false if there is no such combination of dimensions and coordinates.*/
	bool GetVectorValueRecursive( std::vector<unsigned> _vDims, std::vector<unsigned> _vCoords, std::vector<double>& _pResult ) const;
};
