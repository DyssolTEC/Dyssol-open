/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <vector>
#include <cstdint>
#include "Matrix2D.h"

/** Class for description of the transformation matrix.*/
class CTransformMatrix
{
private:
	std::vector<unsigned> m_vDimensions;	///< Types of the distributions
	std::vector<unsigned> m_vClasses;		///< Number of classes of the distributions
	double *m_pData;						///< Data itself
	size_t m_nSize;							///< Current matrix size

public:
	CTransformMatrix( void );
	CTransformMatrix( unsigned _nType, unsigned _nClasses );
	CTransformMatrix(unsigned _nType, size_t _nClasses);
	CTransformMatrix(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2);
	CTransformMatrix( const std::vector<unsigned> &_vTypes, const std::vector<unsigned> &_vClasses );
	~CTransformMatrix( void );

	// ============= Functions to work with DIMENSIONS

	/** Sets new dimension with erasing of old data. Returns true if success.*/
	bool SetDimensions( unsigned _nType, unsigned _nClasses );
	/** Sets new dimensions set with erasing of old data. Returns true if success.*/
	bool SetDimensions( unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2 );
	bool SetDimensions( unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2, unsigned _nType3, unsigned _nClasses3 );
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

	/** Clears all data in the matrix. Dimensions set won't be erased.*/
	void ClearData();

	/** Normalizes data in matrix: sets sum of material which transfers from each single class to 1.*/
	void Normalize();

	// ============= Data GETTERS

	/** Returns value by specified coordinates for 1-dimensional transformation matrix. Returns -1 on error.*/
	double GetValue( unsigned _nCoordSrc, unsigned _nCoordDst ) const;
	/** Returns value by specified coordinates for 2-dimensional transformation matrix. Returns -1 on error.*/
	double GetValue(unsigned _nCoordSrc1, unsigned _nCoordSrc2, unsigned _nCoordDst1, unsigned _nCoordDst2) const;
	/** Returns value by specified coordinates according the all defined dimensions. Returns -1 on error.*/
	double GetValue( const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst ) const;
	/** Returns value by specified coordinates and dimensions sequence.
	*	Number of dimensions must be the same as defined in the transformation matrix. Returns -1 on error.*/
	double GetValue( const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
		const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst ) const;

	/** Returns vector of values for specified coordinates according to all defined dimensions. Returns true if success.*/
	bool GetVectorValue( const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst, std::vector<double>& _vResult ) const;
	/** Returns vector of values for specified coordinates according to all defined dimensions.*/
	std::vector<double> GetVectorValue(const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst) const;
	/** Returns vector of values according to specified coordinates and dimensions sequence.
	*	Number of dimensions must be the same as defined in the transformation matrix. Returns true if success.*/
	bool GetVectorValue( const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
		const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst, std::vector<double>& _vResult ) const;

	// ============= Data SETTERS

	/** Sets value by specified coordinates for 1-dimensional transformation matrix. Returns true if success.*/
	bool SetValue( unsigned _nCoordSrc, unsigned _nCoordDst, double _dValue );
	/** Sets value by specified coordinates for 1-dimensional transformation matrix. Returns true if success.*/
	bool SetValue(size_t _nCoordSrc, size_t _nCoordDst, double _dValue);
	/** Sets value by specified coordinates for 2-dimensional transformation matrix. Returns true if success.*/
	bool SetValue( unsigned _nCoordSrc1, unsigned _nCoordSrc2, unsigned _nCoordDst1, unsigned _nCoordDst2, double _dValue );
	/** Sets value according the all defined dimensions. Number of dimensions must be the same as defined in the transformation matrix. Returns true if success.*/
	bool SetValue( const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst, double _dValue );
	/** Sets value by specified coordinates and dimensions sequence.
	*	Number of dimensions must be the same as defined in the transformation matrix. Returns true if success.*/
	bool SetValue( const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
		const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst, double _dValue );

	/** Sets vector of values for specified coordinates according to all defined dimensions. Returns true if success.*/
	bool SetVectorValue( const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst, const std::vector<double>& _vValue );
	/** Sets vector of values according to specified dimensions and coordinates.
	*	Number of dimensions must be the same as defined in the transformation matrix. Returns true if success.*/
	bool SetVectorValue( const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
		const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst, const std::vector<double>& _vValue );

	/** Sets matrix of values if the sizes correspond. */
	CMatrix2D GetMatrix() const;
	bool SetMatrix(const CMatrix2D& _matrix);

	// ============= Functions to work with other transformations

	/** Removes last dimension in transformation matrix and recalculates others. For MDMatrix use.*/
	void ReduceLastDim( CTransformMatrix& _newTMatr ) const;

private:
	/** Checks the duplicates in dimensions vector. Return true if check is passed.*/
	bool CheckDuplicates( const std::vector<unsigned>& _vDims ) const;

	/** Returns index in m_pData according to incoming dimensions and coordinates.
	*	Doesn't check sizes of dimensions and coordinates in parameters. Returns -1 on error.*/
	size_t GetIndex(const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
					const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst) const;
	/**  If returns true: in _nResIndex will be an index in m_pData according to incoming dimensions and coordinates,
		in _nResStep will be offset between values in m_pdData to work with vector, in _nResVecLength - length of the result vector.*/
	bool GetIndexAndStep(const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
						const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst,
						size_t& _nResIndex, size_t& _nResStep, size_t& _nResVecLength) const;

	/** For function GetIndex*/
	bool MakeSizeAndCoord( std::vector<unsigned>& _vResSizes, std::vector<unsigned>& _vResCoords,
		const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc) const;
	/** For function GetIndexAndStep*/
	bool MakeSizeAndCoord(std::vector<unsigned>& _vResSizes, std::vector<unsigned>& _vResCoords,
		uint32_t& _nResStep, uint32_t& _nResIVec, bool& _bResFlagOk,
		const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc) const;
};
