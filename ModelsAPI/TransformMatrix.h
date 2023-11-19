/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <vector>
#include <cstdint>
#include "Matrix2D.h"

/**
 * \brief Class for description of the transformation matrix.
 * \details Applied to describe laws of changes for multidimensional distributions.
 * Each cell of matrix describes how much of material will be transferred from one class of multidimensional distribution to another.
 * See also \verbatim embed:rst:inline :ref:`label-TM` \endverbatim
 */
class CTransformMatrix
{
private:
	std::vector<unsigned> m_vDimensions;	///< Types of the distributions
	std::vector<unsigned> m_vClasses;		///< Number of classes of the distributions
	double *m_pData;						///< Data itself
	size_t m_nSize;							///< Current matrix size

public:
	/**
	 * \brief Basic constructor.
	 * \details Creates an empty matrix.
	 */
	CTransformMatrix( void );
	/**
	 * \brief Creates matrix to transform one-dimensional distribution with the given type and number of classes.
	 * \details Type is one of the #EDistrTypes. All values in the matrix will be set to 0.
	 * \param _nType Distribution type.
	 * \param _nClasses Number of classes.
	 */
	CTransformMatrix( unsigned _nType, unsigned _nClasses );
	/**
	 * \brief Creates matrix to transform two-dimensional distribution with the given types and number of classes.
	 * \details Types are types from #EDistrTypes. All values in the matrix will be set to 0.
	 * \param _nType1 First distribution type.
	 * \param _nClasses1 Number of classes of first distribution.
	 * \param _nType2 Second distribution type.
	 * \param _nClasses2 Number of classes of second distribution.
	 */
	CTransformMatrix(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2);
	/**
	 * \brief Creates transformation matrix for distribution with specified types and classes.
	 * \details Vectors with types and classes must have the same length. Types are types from #EDistrTypes. All values in the matrix will be set to 0.
	 * \param _vTypes Distribution types.
	 * \param _vClasses Numbers of classes of distributions.
	 */
	CTransformMatrix( const std::vector<unsigned> &_vTypes, const std::vector<unsigned> &_vClasses );
	/**
	 * \private
	 * \brief Destructor.
	 */
	~CTransformMatrix( void );

	// ============= Functions to work with DIMENSIONS

	/**
	 * \brief Sets new dimensions set to the matrix in order to transform one-dimensional distribution with the given type and number of classes.
	 * \details Type is one of the #EDistrTypes. Old data will be erased and the matrix will be initialized with zeroes.
	 * \param _nType Distribution type.
	 * \param _nClasses Number of classes.
	 * \return Error flag.
	 */
	bool SetDimensions( unsigned _nType, unsigned _nClasses );
	/**
	 * \brief Sets new dimensions set to the matrix in order to transform two-dimensional distribution.
	 * \details Types are types of the #EDistrTypes. Types must be unique.
	 * Old data will be erased and the matrix will be initialized with zeroes.
	 * \param _nType1 First distribution type.
	 * \param _nClasses1 Number of classes of first distribution.
	 * \param _nType2 Second distribution type.
	 * \param _nClasses2 Number of classes of second distribution.
	 * \return Error flag.
	 */
	bool SetDimensions( unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2 );
	/**
	 * \brief Sets new dimensions set to the matrix in order to transform three-dimensional distribution.
	 * \details Types are one of the #EDistrTypes. Types must be unique.
	 * Old data will be erased and the matrix will be initialized with zeroes.
	 * \param _nType1 First distribution type.
	 * \param _nClasses1 Number of classes of first distribution.
	 * \param _nType2 Second distribution type.
	 * \param _nClasses2 Number of classes of second distribution.
	 * \param _nType3 Third distribution type.
	 * \param _nClasses3 Number of classes of third distribution.
	 * \return Error flag.
	 */
	bool SetDimensions( unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2, unsigned _nType3, unsigned _nClasses3 );
	/**
	 * \brief Sets new dimensions set with the given types and numbers of classes.
	 * \details Types is the vector of #EDistrTypes. All old data will be erased and the matrix will be initialized with zeroes.
	 * Sizes of vectors with types and classes must be equal.
	 * \param _vTypes Distribution types.
	 * \param _vClasses Numbers of classes of distributions.
	 * \return Error flag.
	 */
	bool SetDimensions( const std::vector<unsigned>& _vTypes, const std::vector<unsigned>& _vClasses );
	/**
	 * \brief Returns vector with all current defined dimensions types.
	 * \details Types is the vector of #EDistrTypes.
	 * \return Vector of defined dimensions types.
	 */
	std::vector<unsigned> GetDimensions() const;
	/**
	 * \brief Returns vector with current numbers of classes.
	 * \return Vector with numbers of classes.
	 */
	std::vector<unsigned> GetClasses() const;
	/**
	 * \brief Returns current number of dimensions.
	 * \return Number of defined dimensions
	 */
	size_t GetDimensionsNumber() const;
	/**
	 * \brief Clears all data and information about dimensions.
	 */
	void Clear();

	// ============= Functions to work with DATA

	/**
	 * \brief Clears all data in the matrix.
	 * \details Dimensions set won't be erased.
	 */
	void ClearData();

	/**
	 * \brief Normalizes data in matrix.
	 * \details Sets sum of material which transfers from each single class to 1.
	 */
	void Normalize();

	// ============= Data GETTERS

	/**
	 * \brief Returns value by specified coordinates according to all defined dimensions in transformation matrix for one-dimensional distribution.
	 * \details Works with one-dimensional distribution only. Returns -1 on error.
	 * \param _nCoordSrc Coordinate of a source class.
	 * \param _nCoordDst Coordinate of a destination class.
	 * \return Mass fraction, which will be transferred from the source class to the destination class.
	 */
	double GetValue( unsigned _nCoordSrc, unsigned _nCoordDst ) const;
	/**
	 * \brief Returns value by specified coordinates according to all defined dimensions in transformation matrix for two-dimensional distribution.
	 * \details Works with two-dimensional distribution only. Returns -1 on error.
	 * \param _nCoordSrc1 First coordinate of a source class.
	 * \param _nCoordSrc2 Second coordinate of a source class.
	 * \param _nCoordDst1 First coordinate of a destination class.
	 * \param _nCoordDst2 Second coordinate of a destination class.
	 * \return Mass fraction, which will be transferred from the source class to the destination class.
	 */
	double GetValue(unsigned _nCoordSrc1, unsigned _nCoordSrc2, unsigned _nCoordDst1, unsigned _nCoordDst2) const;
	/**
	 * \brief Returns value by specified coordinates according the all defined dimensions.
	 * \details Sizes of vectors with coordinates must be equal and must correspond to the number of currently defined dimensions. Returns -1 on error.
	 * \param _vCoordsSrc Coordinates of a source class.
	 * \param _vCoordsDst Coordinates of a destination class.
	 * \return Mass fraction, which will be transferred from the source class to the destination class.
	 */
	double GetValue( const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst ) const;
	/**
	 * \brief Returns value by specified coordinates and dimensions sequence.
	 * \details Number of dimensions must be the same as defined in the transformation matrix. Returns -1 on error.
	 * \param _vDimsSrc Dimensions of a source class.
	 * \param _vCoordsSrc Coordinates of a source class.
	 * \param _vDimsDst Dimensions of a destination class.
	 * \param _vCoordsDst Coordinates of a destination class.
	 * \return Mass fraction, which will be transferred from the source class to the destination class.
	 */
	double GetValue( const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
		const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst ) const;

	/**
	 * \brief Returns vector of values for specified coordinates according to all defined dimensions.
	 * \param _vCoordsSrc Coordinates of a source class.
	 * \param _vCoordsDst Coordinates of a destination class.
	 * \param _vResult Vector of mass fractions, which will be transferred from the source to the destination.
	 * \return Error flag.
	 */
	bool GetVectorValue( const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst, std::vector<double>& _vResult ) const;
	/**
	 * \brief Returns vector of values for specified coordinates according to all defined dimensions.
	 * \param _vCoordsSrc Coordinates of a source class.
	 * \param _vCoordsDst Coordinates of a destination class.
	 * \return Vector of mass fractions, which will be transferred from the source to the destination.
	 */
	std::vector<double> GetVectorValue(const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst) const;
	/**
	 * \brief Returns vector of values according to specified coordinates and dimensions sequence.
	 * \details Number of dimensions must be the same as defined in the transformation matrix.
	 * \param _vDimsSrc Dimensions of a source class.
	 * \param _vCoordsSrc Coordinates of a source class.
	 * \param _vDimsDst Dimensions of a destination class.
	 * \param _vCoordsDst Coordinates of a destination class.
	 * \param _vResult Vector of mass fractions, which will be transferred from the source to the destination.
	 * \return Error flag.
	 */
	bool GetVectorValue( const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
		const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst, std::vector<double>& _vResult ) const;

	// ============= Data SETTERS

	/**
	 * \brief Sets value by specified coordinates for 1-dimensional transformation matrix.
	 * \param _nCoordSrc Coordinate of a source class.
	 * \param _nCoordDst Coordinate of a destination class.
	 * \param _dValue Mass fraction, which will be transferred from the source class to the destination class.
	 * \return Error flag.
	 */
	bool SetValue( unsigned _nCoordSrc, unsigned _nCoordDst, double _dValue );
	/**
	 * \brief Sets value by specified coordinates for 2-dimensional transformation matrix.
	 * \param _nCoordSrc1 First coordinate of a source class.
	 * \param _nCoordSrc2 Second coordinate of a source class.
	 * \param _nCoordDst1 First coordinate of a destination class.
	 * \param _nCoordDst2 Second coordinate of a destination class.
	 * \param _dValue Mass fraction, which will be transferred from the source class to the destination class.
	 * \return Error flag.
	 */
	bool SetValue( unsigned _nCoordSrc1, unsigned _nCoordSrc2, unsigned _nCoordDst1, unsigned _nCoordDst2, double _dValue );
	/**
	 * \brief Sets value according the all defined dimensions.
	 * \details Number of dimensions must be the same as defined in the transformation matrix.
	 * \param _vCoordsSrc Coordinates of a source class.
	 * \param _vCoordsDst Coordinates of a destination class.
	 * \param _dValue Mass fraction, which will be transferred from the source class to the destination class.
	 * \return Error flag.
	 */
	bool SetValue( const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst, double _dValue );
	/**
	 * \brief Sets value by specified coordinates and dimensions sequence.
	 * \details Number of dimensions must be the same as defined in the transformation matrix.
	 * \param _vDimsSrc Dimensions of a source class.
	 * \param _vCoordsSrc Coordinates of a source class.
	 * \param _vDimsDst Dimensions of a destination class.
	 * \param _vCoordsDst Coordinates of a destination class.
	 * \param _dValue Mass fraction, which will be transferred from the source class to the destination class.
	 * \return Error flag.
	 */
	bool SetValue( const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
		const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst, double _dValue );

	/**
	 * \brief Sets vector of values for specified coordinates according to all defined dimensions.
	 * \param _vCoordsSrc Coordinates of a source class.
	 * \param _vCoordsDst Coordinates of a destination class.
	 * \param _vValue Vector of mass fractions, which will be transferred from the source to the destination.
	 * \return Error flag.
	 */
	bool SetVectorValue( const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst, const std::vector<double>& _vValue );
	/**
	 * \brief Sets vector of values according to specified dimensions and coordinates.
	 * \details Number of dimensions must be the same as defined in the transformation matrix.
	 * \param _vDimsSrc Dimensions of a source class.
	 * \param _vCoordsSrc Coordinates of a source class.
	 * \param _vDimsDst Dimensions of a destination class.
	 * \param _vCoordsDst Coordinates of a destination class.
	 * \param _vValue Vector of mass fractions, which will be transferred from the source to the destination.
	 * \return Error flag.
	 */
	bool SetVectorValue( const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
		const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst, const std::vector<double>& _vValue );

	/**
	 * \brief Returns 2D matrix of values.
	 * \return 2D matrix of values.
	 */
	CMatrix2D GetMatrix() const;
	/**
	 * \brief Sets matrix of values if the sizes correspond.
	 * \details If the sizes of matrices do not correspond, nothing is done.
	 * \param _matrix New matrix.
	 * \return Error flag.
	 */
	bool SetMatrix(const CMatrix2D& _matrix);

	// ============= Functions to work with other transformations

	/**
	 * \private
	 * \brief Removes last dimension in transformation matrix and recalculates others. For MDMatrix use.
	 */
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
