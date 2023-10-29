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
	/**
	* \brief Basic **constructor**.
	* \details Creates an empty matrix.
	*/
	CTransformMatrix( void );
	/**
	* \brief Creates matrix to transform one-dimensional distribution with type \p _nType and \p _nClasses classes.
	* \details \p _nType is one of the #EDistrTypes. All values in matrix will be set to 0.
	* \param _nType Distribution type.
	* \param _nClasses Number of classes.
	*/
	CTransformMatrix( unsigned _nType, unsigned _nClasses );
	/**
	* \brief Creates matrix to transform two-dimensional distribution with types \p _nType1 and \p _nType2 and classes \p _nClasses1 and \p _nClasses2.
	* \details \p _nType1 and \p _nType2 are types from #EDistrTypes. All values in matrix will be set to 0.
	* \param _nType1 Fisrt distribution type.
	* \param _nClasses1 Number of classes of first distribution.
	* \param _nType2 Second distribution type.
	* \param _nClasses2 Number of classes of second distribution.
	*/
	CTransformMatrix(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2);
	/**
	* \brief Creates transformation matrix for distribution with specified types and classes.
	* \details \p _vTypes and \p _vClasses must have the same length. \p _vTypes is the vector of types from #EDistrTypes. All values in matrix will be set to 0.
	* \param _vTypes Distribution types.
	* \param _vClasses Numbers of classes of distributions.
	*/
	CTransformMatrix( const std::vector<unsigned> &_vTypes, const std::vector<unsigned> &_vClasses );
	~CTransformMatrix( void );

	// ============= Functions to work with DIMENSIONS

	/**
	* \brief Sets new dimensions set to the matrix in order to transform one-dimensional distribution with type \p _nType and \p _nClasses classes.
	* \details \p _nType is one of the #EDistrTypes. Old data will be erased and matrix will be initialized with zeroes. Returns \p false on error.
	* \param _nType Distribution type.
	* \param _nClasses Number of classes.
	* \return Whether an error has occurred.
	*/
	bool SetDimensions( unsigned _nType, unsigned _nClasses );
	/**
	* \brief Sets new dimensions set to the matrix in order to transform two-dimensional distribution.
	* \details \p _nType1 and \p _nType2 are types of the #EDistrTypes. Types must be unique. \p _nClasses1 and \p _nClasses2 are number of classes in corresponding distributions.
	* Old data will be erased and matrix will be initialized with zeroes. Returns \p false on error.
	* \param _nType1 Fisrt distribution type.
	* \param _nClasses1 Number of classes of first distribution.
	* \param _nType2 Second distribution type.
	* \param _nClasses2 Number of classes of second distribution.
	* \return Whether an error has occurred.
	*/
	bool SetDimensions( unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2 );
	/**
	* \brief Sets new dimensions set to the matrix in order to transform three-dimensional distribution.
	* \details \p _nType1, \p _nType2 and \p _nType3 are one of the #EDistrTypes. Types must be unique. \p _nClasses1, \p _nClasses2 and \p _nClasses3 are number of classes in corresponding distributions.
	* Old data will be erased and matrix will be initialized with zeroes. Returns \p false on error.
	* \param _nType1 Fisrt distribution type.
	* \param _nClasses1 Number of classes of first distribution.
	* \param _nType2 Second distribution type.
	* \param _nClasses2 Number of classes of second distribution.
	* \param _nType3 Third distribution type.
	* \param _nClasses3 Number of classes of third distribution.
	* \return Whether an error has occurred.
	*/
	bool SetDimensions( unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2, unsigned _nType3, unsigned _nClasses3 );
	/**
	* \brief Sets new dimensions set with types \p _vTypes and numbers of classes \p _vClasses.
	* \details \p _vTypes is the vector of #EDistrTypes. All old data will be erased and matrix will be initialized with zeroes. Sizes of vectors \p _vTypes and \p _vClasses must be equal. Returns \p false on error.
	* \param _vTypes Distribution types.
	* \param _vClasses Numbers of classes of distributions.
	* \return Whether an error has occurred.
	*/
	bool SetDimensions( const std::vector<unsigned>& _vTypes, const std::vector<unsigned>& _vClasses );
	/**
	* \brief Returns vector with all current defined dimensions types.
	* \details
	* \return Vector of defined dimensions types.
	*/
	std::vector<unsigned> GetDimensions() const;
	/**
	* \brief Returns vector with current numbers of classes.
	* \details
	* \return Vector of defined classes.
	*/
	std::vector<unsigned> GetClasses() const;
	/**
	* \brief Returns current number of dimensions.
	* \details
	* \return Number of defined dimensions
	*/
	size_t GetDimensionsNumber() const;
	/**
	* \brief Clears all data and information about dimensions.
	* \details
	*/
	void Clear();

	// ============= Functions to work with DATA

	/**
	* \brief Clears all data in the matrix.
	* \details Dimensions set won't be erased.
	*/
	void ClearData();

	/**
	* \brief Normalizes data in matrix: sets sum of material which transfers from each single class to 1.
	* \details
	*/
	void Normalize();

	// ============= Data GETTERS

	/**
	* \brief Returns value by specified coordinates according to all defined dimensions in transformation matrix for one-dimensional distribution.
	* \details Works with one-dimensional distribution only. Returns \p -1 on error.
	* \param _nCoordSrc Coordinate of a source class.
	* \param _nCoordDst Coordinate of a destination class.
	* \return Mass fraction, which will be transferred from the source class to the destination class.
	*/
	double GetValue( unsigned _nCoordSrc, unsigned _nCoordDst ) const;
	/**
	* \brief Returns value by specified coordinates according to all defined dimensions in transformation matrix for two-dimensional distribution.
	* \details Works with two-dimensional distribution only. Returns \p -1 on error.
	* \param _nCoordSrc1 First coordinate of a source class.
	* \param _nCoordSrc2 Second coordinate of a source class.
	* \param _nCoordDst1 First coordinate of a destination class.
	* \param _nCoordDst2 Second coordinate of a destination class.
	* \return Mass fraction, which will be transferred from the source class to the destination class.
	*/
	double GetValue(unsigned _nCoordSrc1, unsigned _nCoordSrc2, unsigned _nCoordDst1, unsigned _nCoordDst2) const;
	/**
	* \brief Returns value by specified coordinates according the all defined dimensions.
	* \details Sizes of vectors \p _vCoordsSrc and \p _vCoordsDst must be equal and must correspond to the number of currently defined dimensions. Returns \p -1 on error.
	* \param _vCoordsSrc Coordinates of a source class.
	* \param _vCoordsDst Coordinates of a destination class.
	* \return Mass fraction, which will be transferred from the source class to the destination class.
	*/
	double GetValue( const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst ) const;
	/**
	* \brief Returns value by specified coordinates and dimensions sequence.
	* \details Number of dimensions must be the same as defined in the transformation matrix. Returns \p -1 on error.
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
	* \details Returns \p false on error.
	* \param _vCoordsSrc Coordinates of a source class.
	* \param _vCoordsDst Coordinates of a destination class.
	* \param _vResult Vector of mass fractions, which will be transferred from the source to the destination.
	* \return Whether an error has occurred.
	*/
	bool GetVectorValue( const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst, std::vector<double>& _vResult ) const;
	/** Returns vector of values for specified coordinates according to all defined dimensions.*/
	std::vector<double> GetVectorValue(const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst) const;
	/**
	* \brief Returns vector of values according to specified coordinates and dimensions sequence.
	* \details Number of dimensions must be the same as defined in the transformation matrix. Returns \p false on error.
	* \param _vDimsSrc Dimensions of a source class.
	* \param _vCoordsSrc Coordinates of a source class.
	* \param _vDimsDst Dimensions of a destination class.
	* \param _vCoordsDst Coordinates of a destination class.
	* \param _vResult Vector of mass fractions, which will be transferred from the source to the destination.
	* \return Whether an error has occurred.
	*/
	bool GetVectorValue( const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
		const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst, std::vector<double>& _vResult ) const;

	// ============= Data SETTERS

	/**
	* \brief Sets value by specified coordinates for 1-dimensional transformation matrix.
	* \details Returns \p true if success.
	* \param _nCoordSrc Coordinate of a source class.
	* \param _nCoordDst Coordinate of a destination class.
	* \param _dValue Mass fraction, which will be transferred from the source class to the destination class.
	* \return Whether an error has occurred.
	*/
	bool SetValue( unsigned _nCoordSrc, unsigned _nCoordDst, double _dValue );
	/**
	* \brief Sets value by specified coordinates for 2-dimensional transformation matrix.
	* \details Returns \p true if success.
	* \param _nCoordSrc1 First coordinate of a source class.
	* \param _nCoordSrc2 Second coordinate of a source class.
	* \param _nCoordDst1 First coordinate of a destination class.
	* \param _nCoordDst2 Second coordinate of a destination class.
	* \param _dValue Mass fraction, which will be transferred from the source class to the destination class.
	* \return Whether an error has occurred.
	*/
	bool SetValue( unsigned _nCoordSrc1, unsigned _nCoordSrc2, unsigned _nCoordDst1, unsigned _nCoordDst2, double _dValue );
	/**
	* \brief Sets value according the all defined dimensions.
	* \details Number of dimensions must be the same as defined in the transformation matrix. Returns \p true if success.
	* \param _vCoordsSrc Coordinates of a source class.
	* \param _vCoordsDst Coordinates of a destination class.
	* \param _dValue Mass fraction, which will be transferred from the source class to the destination class.
	* \return Whether an error has occurred.
	*/
	bool SetValue( const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst, double _dValue );
	/**
	* \brief Sets value by specified coordinates and dimensions sequence.
	* \details Number of dimensions must be the same as defined in the transformation matrix. Returns \p true if success.
	* \param _vDimsSrc Dimensions of a source class.
	* \param _vCoordsSrc Coordinates of a source class.
	* \param _vDimsDst Dimensions of a destination class.
	* \param _vCoordsDst Coordinates of a destination class.
	* \param _dValue Mass fraction, which will be transferred from the source class to the destination class.
	* \return Whether an error has occurred.
	*/
	bool SetValue( const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
		const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst, double _dValue );

	/**
	* \brief Sets vector of values for specified coordinates according to all defined dimensions.
	* \details Returns \p true if success.
	* \param _vCoordsSrc Coordinates of a source class.
	* \param _vCoordsDst Coordinates of a destination class.
	* \param _vValue Vector of mass fractions, which will be transferred from the source to the destination.
	*/
	bool SetVectorValue( const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst, const std::vector<double>& _vValue );
	/**
	* \brief Sets vector of values according to specified dimensions and coordinates.
	* \details Number of dimensions must be the same as defined in the transformation matrix. Returns \p true if success.
	* \param _vDimsSrc Dimensions of a source class.
	* \param _vCoordsSrc Coordinates of a source class.
	* \param _vDimsDst Dimensions of a destination class.
	* \param _vCoordsDst Coordinates of a destination class.
	* \param _vValue Vector of mass fractions, which will be transferred from the source to the destination.
	* \return Whether an error has occurred.
	*/
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
