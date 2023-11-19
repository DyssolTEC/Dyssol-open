/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <vector>
#include <cstddef>

/**
 * Multidimensional matrix in dense format.
 */
class CDenseMDMatrix
{
private:
	std::vector<unsigned> m_vDimensions;	///< Types of the distributions
	std::vector<unsigned> m_vClasses;		///< Number of classes of the distributions
	std::vector<double> m_vData;			///< Data itself

public:
	/**
	 * \brief Basic constructor. Creates an empty matrix.
	 */
	CDenseMDMatrix();
	/**
	 * \brief Constructor creating a matrix with the given types and number of classes.
	 * \param _vTypes Distribution types.
	 * \param _vClasses Numbers of classes of distributions.
	 */
	CDenseMDMatrix(const std::vector<unsigned>& _vTypes, const std::vector<unsigned>& _vClasses);

	// ============= Functions to work with DIMENSIONS

	/**
	 * \brief Sets new dimensions set with erasing of old data.
	 * \details Type is one of the #EDistrTypes.
	 * \param _nType Distribution type.
	 * \param _nClasses Number of classes.
	 * \return Error flag.
	 */
	bool SetDimensions(unsigned _nType, unsigned _nClasses);
	/**
	 * \brief Sets new dimensions set with erasing of old data.
	 * \details Types are types of the #EDistrTypes.
	 * \param _nType1 Fisrt distribution type.
	 * \param _nClasses1 Number of classes of first distribution.
	 * \param _nType2 Second distribution type.
	 * \param _nClasses2 Number of classes of second distribution.
	 * \return Error flag.
	 */
	bool SetDimensions(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2);
	/**
	 * \brief Sets new dimensions set with erasing of old data.
	 * \details Types are one of the #EDistrTypes.
	 * \param _nType1 First distribution type.
	 * \param _nClasses1 Number of classes of first distribution.
	 * \param _nType2 Second distribution type.
	 * \param _nClasses2 Number of classes of second distribution.
	 * \param _nType3 Third distribution type.
	 * \param _nClasses3 Number of classes of third distribution.
	 * \return Error flag.
	 */
	bool SetDimensions(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2, unsigned _nType3, unsigned _nClasses3);
	/**
	 * \brief Sets new dimensions set with erasing of old data.
	 * \details Types is the vector of #EDistrTypes.
	 * \param _vTypes Distribution types.
	 * \param _vClasses Numbers of classes of distributions.
	 * \return Error flag.
	 */
	bool SetDimensions( const std::vector<unsigned>& _vTypes, const std::vector<unsigned>& _vClasses );
	/**
	 * \brief Returns vector with all current defined dimensions types.
	 * \return Vector of defined dimensions types.
	 */
	std::vector<unsigned> GetDimensions() const;
	/**
	 * \brief Returns vector with current numbers of classes.
	 * \return Vector of defined classes.
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
	 * \brief Clears all data in the matrix by setting all values to 0.
	 * \details Dimensions set won't be erased.
	 */
	void ClearData();

	/**
	 * \brief Returns const pointer to data.
	 * \return Const pointer to data.
	 */
	const double* GetDataPtr() const;
	/**
	 * \brief Returns pointer to data.
	 * \return Pointer to data.
	 */
	double* GetDataPtr();

	/**
	 * \brief Returns length of the plain data array.
	 * \return Total data length.
	 */
	size_t GetDataLength() const;

	// ============= Data GETTERS

	/**
	 * \brief Returns value by specified dimension and coordinate with possible reducing of dimensions set.
	 * \details Returns -1 on error.
	 * \param _nDim Target dimension.
	 * \param _nCoord Coordinate.
	 * \return Value by specified dimension and coordinate.
	 */
	double GetValue( unsigned _nDim, unsigned _nCoord ) const;
	/**
	 * \brief Returns value by specified dimensions and coordinates with possible reducing of dimensions set.
	 * \details Returns -1 on error.
	 * \param _nDim1 First dimension.
	 * \param _nCoord1 First coordinate.
	 * \param _nDim2 Second dimension.
	 * \param _nCoord2 Second coordinate.
	 * \return Value by specified dimensions and coordinates.
	 */
	double GetValue( unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2 ) const;
	/**
	 * \brief Returns value by specified dimensions and coordinates with possible reducing of dimensions set.
	 * \details Returns -1 on error.
	 * \param _nDim1 First dimension.
	 * \param _nCoord1 First coordinate.
	 * \param _nDim2 Second dimension.
	 * \param _nCoord2 Second coordinate.
	 * \param _nDim3 Third dimension.
	 * \param _nCoord3 Third coordinate.
	 * \return Value by specified dimensions and coordinates.
	 */
	double GetValue( unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nDim3, unsigned _nCoord3 ) const;
	/**
	 * \brief Returns value by specified coordinates according to the full defined set of dimensions.
	 * \details Returns -1 on error.
	 * \param _vCoords Coordinates.
	 * \return Value by specified coordinates.
	 */
	double GetValue( const std::vector<unsigned>& _vCoords ) const;
	/**
	 * \brief Returns value by specified coordinates and dimensions sequence.
	 * \details Dimensions set can be reduced. Returns -1 on error.
	 * \param _vDims Dimensions.
	 * \param _vCoords Coordinates.
	 * \return Value by specified dimensions and coordinates sequence.
	 */
	double GetValue( const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords ) const;

	/**
	 * Returns vector of values according to specified dimension with possible reducing of dimensions set.
	 * \param _nDim Target dimension.
	 * \return Vector of values according to specified dimension.
	 */
	std::vector<double> GetVectorValue(unsigned _nDim) const;
	/**
	 * \brief Returns vector of values according to specified dimension with possible reducing of dimensions set.
	 * \param _nDim Target dimension.
	 * \param _vResult Vector of values according to specified dimension.
	 * \return Error flag.
	 */
	bool GetVectorValue( unsigned _nDim, std::vector<double>& _vResult ) const;
	/**
	 * \brief Returns vector of values according to specified dimensions and coordinates with possible reducing of dimensions set.
	 * \param _nDim1 First dimension.
	 * \param _nCoord1 First coordinate.
	 * \param _nDim2 Second dimension.
	 * \param _vResult Vector of values according to specified dimensions and coordinates.
	 * \return Error flag.
	 */
	bool GetVectorValue( unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, std::vector<double>& _vResult ) const;
	/**
	 * \brief Returns vector of values according to specified dimensions and coordinates with possible reducing of dimensions set.
	 * \param _nDim1 First dimension.
	 * \param _nCoord1 First coordinate.
	 * \param _nDim2 Second dimension.
	 * \param _nCoord2 Second coordinate.
	 * \param _nDim3 Third dimension.
	 * \param _vResult Vector of values according to specified dimensions and coordinates.
	 * \return Error flag.
	 */
	bool GetVectorValue( unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nDim3, std::vector<double>& _vResult ) const;
	/**
	 * Returns vector of values according to specified dimensions and coordinates with possible reducing of dimensions set.
	 * \param _nDim1 First dimension.
	 * \param _nCoord1 First coordinate.
	 * \param _nDim2 Second dimension.
	 * \param _nCoord2 Second coordinate.
	 * \param _nDim3 Third dimension.
	 * \return Vector of values according to specified dimensions and coordinates.
	 */
	std::vector<double> GetVectorValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nDim3) const;
	/**
	 * \brief Returns vector of values by specified coordinates according to a full defined dimensions set.
	 * \param _vCoords Coordinates.
	 * \param _vResult Vector of values by specified coordinates.
	 * \return Error flag.
	 */
	bool GetVectorValue( const std::vector<unsigned>& _vCoords, std::vector<double>& _vResult ) const;
	/**
	 * \brief Returns vector of values according to specified dimensions and coordinates.
	 * \details Dimensions set can be reduced.
	 * \param _vDims Dimensions.
	 * \param _vCoords Coordinates.
	 * \param _vResult Vector of values according to specified dimensions and coordinates.
	 * \return Error flag.
	 */
	bool GetVectorValue( const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, std::vector<double>& _vResult ) const;
	/**
	 * Returns vector of values according to specified dimensions and coordinates. Dimensions set can be reduced.
	 * \param _vDims Dimensions.
	 * \param _vCoords Coordinates.
	 * \return Vector of values according to specified dimensions and coordinates.
	 */
	std::vector<double> GetVectorValue(const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords) const;

	// ============= Data SETTERS

	/**
	 * \brief Sets value in 1D-matrix by specified coordinate.
	 * \details Sets value only if the number of dimensions is the same as in the matrix.
	 * \param _nCoord Coordinate.
	 * \param _dValue New value.
	 * \return Error flag.
	 */
	bool SetValue( unsigned _nCoord, double _dValue );
	/**
	 * \brief Sets value in 2D-matrix by specified dimensions and coordinates.
	 * \details Sets value only if the number of dimensions is the same as in the matrix.
	 * \param _nDim1 First dimension.
	 * \param _nCoord1 First coordinate.
	 * \param _nCoord2 Second coordinate.
	 * \param _dValue New value.
	 * \return Error flag.
	 */
	bool SetValue( unsigned _nDim1, unsigned _nCoord1, unsigned _nCoord2, double _dValue );
	/**
	 * \brief Sets value in 3D-matrix by specified dimensions and coordinates.
	 * \details Sets value only if the number of dimensions is the same as in the matrix.
	 * \param _nDim1 First dimension.
	 * \param _nCoord1 First coordinate.
	 * \param _nDim2 Second dimension.
	 * \param _nCoord2 Second coordinate.
	 * \param _nCoord3 Third coordinate.
	 * \param _dValue New value.
	 * \return Error flag.
	 */
	bool SetValue( unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nCoord3, double _dValue );
	/**
	 * \brief Sets value by specified coordinates according to the full defined set of dimensions.
	 * \details Sets value only if the number of dimensions is the same as in the matrix.
	 * \param _vCoords Coordinates.
	 * \param _dValue New value.
	 * \return Error flag.
	 */
	bool SetValue( const std::vector<unsigned>& _vCoords, double _dValue );
	/**
	 * \brief Sets value by specified coordinates and dimensions sequence.
	 * \details Sets value only if the number of dimensions is the same as in the matrix.
	 * \param _vDims Dimensions.
	 * \param _vCoords Coordinates.
	 * \param _dValue New value.
	 * \return Error flag.
	 */
	bool SetValue( const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, double _dValue );

	/**
	 * \brief Sets vector of values in 1D-matrix.
	 * \param _vValue New values.
	 * \return Error flag.
	 */
	bool SetVectorValue( const std::vector<double>& _vValue );
	/**
	 * \brief Sets vector of values according to specified dimensions and coordinates in 2D-matrix.
	 * \param _nDim Target dimension.
	 * \param _nCoord Coordinate.
	 * \param _vValue New values.
	 * \return Error flag.
	 */
	bool SetVectorValue( unsigned _nDim, unsigned _nCoord, const std::vector<double>& _vValue );
	/**
	 * \brief Sets vector of values according to specified dimensions and coordinates in 3D-matrix.
	 * \param _nDim1 First dimension.
	 * \param _nCoord1 First coordinate.
	 * \param _nDim2 Second dimension.
	 * \param _nCoord2 Second coordinate.
	 * \param _vValue New values.
	 * \return Error flag.
	 */
	bool SetVectorValue( unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, const std::vector<double>& _vValue );
	/**
	 * \brief Sets vector of values by specified coordinates according to a full defined dimensions set.
	 * \details Sets values only if the number of coordinates is one less than number of dimensions in the matrix.
	 * \param _vCoords Coordinates.
	 * \param _vValue New values.
	 * \return Whether an error has occurred.
	 * \return Error flag.
	 */
	bool SetVectorValue( const std::vector<unsigned>& _vCoords, const std::vector<double>& _vValue );
	/**
	 * \brief Sets vector of values according to specified dimensions and coordinates.
	 * \details Sets values only if the number of dimensions is one less than in the matrix.
	 * \param _vDims Dimensions.
	 * \param _vCoords Coordinates.
	 * \param _vValue New values.
	 * \return Error flag.
	 */
	bool SetVectorValue( const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, const std::vector<double>& _vValue );


	// ========== Normalization

	/**
	 * \brief Checks if the matrix is normalized.
	 * \return Whether the matrix is normalized.
	 */
	bool IsNormalized();
	/**
	 * \brief Normalizes the matrix so that the sum of all elements equals to 1.
	 */
	void Normalize();

	// ========== Overloaded operators

	/**
	 * \brief Adds matrix with the same dimensions.
	 * \details If dimensions are not the same, than the empty matrix will be returned.
	 * \param _matrix Other matrix.
	 */
	CDenseMDMatrix operator+( const CDenseMDMatrix& _matrix );
	/**
	 * \brief Subtracts matrix with the same dimensions.
	 * \details If dimensions are not the same, than the empty matrix will be returned.
	 * \param _matrix Other matrix.
	 */
	CDenseMDMatrix operator-( const CDenseMDMatrix& _matrix );

	/**
	 * \brief Multiplication of the matrix by a coefficient.
	 * \details
	 * \param _dFactor Coefficient.
	 */
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
