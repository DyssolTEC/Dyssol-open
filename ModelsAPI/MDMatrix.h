/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "TDArray.h"
#include "DenseMDMatrix.h"
#include "Matrix2D.h"
#include "TransformMatrix.h"
#include "H5Handler.h"
#include "MDMatrCacher.h"

#define DATA_SAVE_BLOCK	100

struct sFraction
{
	CTDArray tdArray;
	sFraction *pNext;
};

/** This class is used to describe multidimensional distributed data. All data depends on time.
*	One matrix is defined for all time points.*/
class CMDMatrix
{
private:
	static const unsigned m_cnSaveVersion;

	std::vector<unsigned> m_vDimensions;	///< Types of the distributions
	std::vector<unsigned> m_vClasses;		///< Number of classes of the distributions
	std::vector<double> m_vTimePoints;		///< Vector of current time points
	mutable sFraction *m_data;				///< Current data itself
	double m_dMinFraction;					///< Minimal fraction. All smaller values are interpreted as 0

	// ===== Variables for temporary use in recursive functions
	mutable double m_dTempT1;
	double m_dTempT2;
	mutable std::vector<unsigned> m_vTempCoords;
	mutable std::vector<unsigned> m_vTempDims;
	mutable std::vector<double> m_vTempValues;
	double m_dTempValue;
	CMDMatrix *m_pSortMatr;

	std::wstring m_sCachePath;
	bool m_bCacheEnabled;
	mutable CMDMatrCacher *m_pCacheHandler;
	mutable unsigned m_nCounter;
	mutable double m_dCurrWinStart;
	mutable double m_dCurrWinEnd;
	unsigned m_nCacheWindow;
	mutable unsigned m_nNonCachedTPNum;
	mutable size_t m_nCurrOffset;
	mutable bool m_bCacheCoherent;

public:
	CMDMatrix( void );
	~CMDMatrix( void );

	/** Clears all data, time points and dimensions.*/
	void Clear();

	// ========== Functions to work with DIMENSIONS

	/** Adds new dimension to a matrix. If dimension already exists, than nothing will be done.*/
	void AddDimension( unsigned _nDim, unsigned _nClasses );
	/** Removes specified dimension and erases ALL data.*/
	void DeleteDimension( unsigned _nDim );
	/** Removes specified dimensions and erases ALL data.*/
	void DeleteDimensions( const std::vector<unsigned>& _vDims );
	/** Returns vector with all current dimensions.*/
	std::vector<unsigned> GetDimensions() const;
	/** Returns vector with classes of all current dimensions.*/
	std::vector<unsigned> GetClasses() const;
	/** Sets dimension to a matrix and erases ALL data.*/
	void SetDimension( unsigned _nDim, unsigned _nClasses );
	/** Sets dimensions to a matrix and erases ALL data.*/
	void SetDimensions( const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vClasses );
	/** Returns current number of dimensions.*/
	size_t GetDimensionsNumber() const;
	/** Returns dimension by index. Returns -1 if there is no such dimension.*/
	int GetDimensionTypeByIndex( unsigned _nIndex ) const;
	/** Returns dimension size by index. Returns -1 if there is no such dimension.*/
	int GetDimensionSizeByIndex( unsigned _nIndex ) const;
	/** Returns dimension size by type. Returns 0 if there is no such dimension.*/
	unsigned GetDimensionSizeByType( unsigned _nDim ) const;
	/** Updates matrix with new set of dimensions.*/
	void UpdateDimensions( const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vClasses );
	/** Adds class to a dimension.*/
	void AddClass( unsigned _nDim );
	/** Removes specified class from dimension.*/
	void RemoveClass( unsigned _nDim, unsigned _nClassIndex );
	/** Returns true if matrices have the same dimensions.*/
	bool CompareDims( const CMDMatrix& _matr ) const;

	// ========== Functions to work with TIME POINTS

	/** Adds new time point. Data to a new time point will be copied from _dSourceTimePoint.
	*	If _dSourceTimePoint == -1, than data will be copied from the previous time point. If such time point already exists, than nothing will be done.*/
	void AddTimePoint( double _dTime, double _dSrcTimePoint = -1 );
	/** Changes specified time point. For UI purposes.*/
	void ChangeTimePoint( unsigned _nTimePointIndex, double _dNewTime );
	/** Removes specified time point.*/
	void RemoveTimePoint( double _dTime );
	/** Removes time points from interval (incl).*/
	void RemoveTimePoints( double _dStart, double _dEnd );
	/** Removes all time points after the specified time.
	*	If _bIncludeTime == true, than _dTime will be includede in this time interval.*/
	void RemoveTimePointsAfter( double _dTime, bool _bIncludeTime = false );
	/** Removes all defined time points.*/
	void RemoveAllTimePoints();
	/** Returns vector of time points for interval.*/
	std::vector<double> GetTimePoints( double _dStart, double _dEnd ) const;
	/** Returns all defined time points.*/
	std::vector<double> GetAllTimePoints() const;
	/** Returns time point with specified index. If there is no such time point, than -1 will be returned.*/
	double GetTimeForIndex( unsigned _nIndex ) const;
	/** Returns number of defined time points.*/
	size_t GetTimePointsNumber() const;

	// ========== Functions to work with MINIMAL FRACTION

	/** Returns current minimal fraction.*/
	double GetMinimalFraction() const;
	/** Sets minimal fraction for all matrices.s*/
	void SetMinimalFraction( double _dValue );

	// ========== Functions to GET and SET VALUES

	/** Returns single value according to specified dimension and coordinate for time point with specified index.
	*	A dimensions set can be reduced. If dimension, coordinate or index doesn't exist, than 0 will be returned.
	*	Works with non-negative values only. If value is less then m_dMinFraction, than 0 will be returned.*/
	double GetValue(size_t _nTimeIndex, unsigned _nDim, unsigned _nCoord) const;
	/** Returns single value according to specified dimension and coordinate for existing time point _dTime
	*	or its approximation if _dTime wasn't defined. A dimensions set can be reduced.
	*	If dimension or coordinate doesn't exist, than 0 will be returned. Works with non-negative values only.
	*	If value is less then m_dMinFraction, than 0 will be returned.*/
	double GetValue(double _dTime, unsigned _nDim, unsigned _nCoord) const;
	/** Returns single value according to specified dimensions and coordinates for existing time point _dTime
	*	or its approximation if _dTime wasn't defined. A dimensions set can be reduced.
	*	If dimension or coordinate doesn't exist, than 0 will be returned. Works with non-negative values only.
	*	If value is less then m_dMinFraction, than 0 will be returned.*/
	double GetValue(double _dTime, unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2) const;
	/** Returns single value according to specified dimensions and coordinates for existing time point _dTime
	*	or its approximation if _dTime wasn't defined. A dimensions set can be reduced.
	*	If dimension or coordinate doesn't exist, than 0 will be returned. Works with non-negative values only.
	*	If value is less then m_dMinFraction, than 0 will be returned.*/
	double GetValue(double _dTime, unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nDim3, unsigned _nCoord3) const;
	/** Returns single value according to all defined dimensions and specified coordinates for existing time point _dTime
	*	or its approximation if _dTime wasn't defined.
	*	If coordinate doesn't exist, than 0 will be returned. Works with non-negative values only.
	*	If value is less then m_dMinFraction, than 0 will be returned.*/
	double GetValue(double _dTime, const std::vector<unsigned>& _vCoords) const;
	/** Returns single value according to specified dimensions and coordinates for existing time point _dTime
	*	or its approximation if _dTime wasn't defined. A dimensions set can be reduced.
	*	If dimension or coordinate doesn't exist, than 0 will be returned. Works with non-negative values only.
	*	If value is less then m_dMinFraction, than 0 will be returned.*/
	double GetValue(double _dTime, const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords) const;

	/** Sets value according to specified dimension and coordinate for the time point with specified index.
	*	Dimensions set can be reduced. All negative values will be transformed to a 0. Returns false on error.*/
	bool SetValue( unsigned _nTimeIndex, unsigned _nDim, unsigned _nCoord, double _dValue, bool _bExternal = true );
	/** Sets value according to specified dimension and coordinate for the existing time point. Dimensions set can be reduced.
	*	If time point wasn't defined, than nothing will be done. All negative values will be transformed to a 0. Returns false on error.*/
	bool SetValue( double _dTime, unsigned _nDim, unsigned _nCoord, double _dValue, bool _bExternal = true );
	/** Sets value according to specified dimensions and coordinates for the existing time point. Dimensions set can be reduced.
	*	If time point wasn't defined, than nothing will be done. All negative values will be transformed to a 0. Returns false on error.*/
	bool SetValue( double _dTime, unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, double _dValue, bool _bExternal = true );
	/** Sets value according to specified dimensions and coordinates for the existing time point. Dimensions set can be reduced.
	*	If time point wasn't defined, than nothing will be done. All negative values will be transformed to a 0. Returns false on error.*/
	bool SetValue( double _dTime, unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nDim3, unsigned _nCoord3, double _dValue, bool _bExternal = true );
	/** Sets value according to all defined dimensions and specified coordinates for the existing time point.
	*	If time point wasn't defined, than nothing will be done. All negative values will be transformed to a 0. Returns false on error.*/
	bool SetValue( double _dTime, const std::vector<unsigned>& _vCoords, double _dValue, bool _bExternal = true );
	/** Sets value according to specified dimensions and coordinates for the existing time point. Dimensions set can be reduced.
	*	If time point wasn't defined, than nothing will be done. All negative values will be transformed to a 0. Returns false on error.*/
	bool SetValue( double _dTime, const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, double _dValue, bool _bExternal = true );

	// ========== Functions to GET and SET VECTORS

	/** Returns vector value according to specified dimension for time point with specified index.
	*	A dimensions set can be reduced. Returns false on error and on vector of zeros in _vResult.*/
	std::vector<double> GetVectorValue(unsigned _nTimeIndex, unsigned _nDim) const;
	/** Returns vector value according to specified dimension for time point with specified index.
	*	A dimensions set can be reduced. Returns false on error and on vector of zeros in _vResult.*/
	bool GetVectorValue(unsigned _nTimeIndex, unsigned _nDim, std::vector<double>& _vResult) const;
	/** Returns vector value according to specified dimension for existing time point _dTime
	*	or its approximation if _dTime wasn't defined. A dimensions set can be reduced. */
	std::vector<double> GetVectorValue(double _dTime, unsigned _nDim) const;
	/** Returns vector value according to specified dimension for existing time point _dTime
	*	or its approximation if _dTime wasn't defined. A dimensions set can be reduced. Returns false on error and on vector of zeros in _vResult.*/
	bool GetVectorValue(double _dTime, unsigned _nDim, std::vector<double>& _vResult) const;
	/** Returns vector value according to specified dimensions and coordinate for existing time point _dTime
	*	or its approximation if _dTime wasn't defined. A dimensions set can be reduced. */
	std::vector<double> GetVectorValue(double _dTime, unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2) const;
	/** Returns vector value according to specified dimensions and coordinate for existing time point _dTime
	*	or its approximation if _dTime wasn't defined. A dimensions set can be reduced. Returns false on error and on vector of zeros in _vResult.*/
	bool GetVectorValue(double _dTime, unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, std::vector<double>& _vResult) const;
	/** Returns vector value according to specified dimensions and coordinate for existing time point _dTime
	*	or its approximation if _dTime wasn't defined. A dimensions set can be reduced. Number of coordinates must be one less then number of dimensions.*/
	std::vector<double> GetVectorValue(double _dTime, const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords) const;
	/** Returns vector value according to specified dimensions and coordinate for existing time point _dTime
	*	or its approximation if _dTime wasn't defined. A dimensions set can be reduced. Number of coordinates must be one less then number of dimensions.
	*	Returns false on error and on vector of zeros in _vResult.*/
	bool GetVectorValue(double _dTime, const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, std::vector<double>& _vResult) const;
	std::vector<double> GetValues(unsigned _nDim, unsigned _nCoord) const;

	/** Sets vector value according to specified dimension for the time point with specified index.
	*	Dimensions set can be reduced. Sets value only if the value of the previous level (dimension in structure) is already set.
	*	Returns false on error.*/
	bool SetVectorValue( unsigned _nTimeIndex, unsigned _nDim, const std::vector<double>& _vValue, bool _bExternal = false );
	/** Sets vector value according to specified dimension for the existing time point. Dimensions set can be reduced.
	*	If time point wasn't defined, than nothing will be done. Sets value only if the value of the previous level (dimension in structure)
	*	is already set. Returns false on error.*/
	bool SetVectorValue( double _dTime, unsigned _nDim, const std::vector<double>& _vValue, bool _bExternal = false );
	/** Sets vector value according to specified dimensions and coordinate for the existing time point. Dimensions set can be reduced.
	*	If time point wasn't defined, than nothing will be done. Sets value only if the value of the previous level (dimension in structure)
	*	is already set. Returns false on error.*/
	bool SetVectorValue( double _dTime, unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, const std::vector<double>& _vValue, bool _bExternal = false );
	/** Sets vector value according to specified dimensions and coordinates for the existing time point. Dimensions set can be reduced.
	*	If time point wasn't defined, than nothing will be done. Number of coordinates must be one less then number of dimensions.
	*	Sets value only if the value of the previous level (dimension in structure) is already set. Returns false on error.*/
	bool SetVectorValue( double _dTime, const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, const std::vector<double>& _vValue, bool _bExternal = false );

	// ========== Functions to GET and SET MATRICES

	/** Gets 2-dimensional matrix according to last 2 dimensions in _vDimType. Returns false on error.
	*	Must be ( _vDimType.size() == _vCoord.size() + 2 ) and full size of dimensions vector. For UI purposes.*/
	bool GetMatrixValue( double _dTime, const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, std::vector<std::vector<double>>& _vResult );
	/** Gets 2-dimensional matrix according to last 2 dimensions in _vDimType. Returns false on error.
	*	Must be ( _vDimType.size() == _vCoord.size() + 2 ) and full size of dimensions vector. For UI purposes.*/
	std::vector<std::vector<double>> GetMatrixValue(double _dTime, const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords);
	/** Sets 2-dimensional matrix according to last 2 dimensions in _vDimType. Returns false on error.
	*	Must be ( _vDimType.size() == _vCoord.size() + 2 ) and full size of dimensions vector. For UI purposes.*/
	bool SetMatrixValue( double _dTime, const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, const std::vector<std::vector<double>>& _vValue );

	// ========== Functions to GET and SET DISTRIBUTIONS

	/** Return distribution by all defined dimensions and specified time. Uses GetVectorValue() functions.*/
	CDenseMDMatrix GetDistribution(double _dTime) const;
	/** Return distribution by all defined dimensions and specified time. Uses GetVectorValue() functions.*/
	bool GetDistribution(double _dTime, CDenseMDMatrix& _Result) const;
	/** Return distribution by specified dimension and time. Uses GetVectorValue() functions.*/
	bool GetDistribution(double _dTime, unsigned _nDim, std::vector<double>& _vResult) const;
	/** Return distribution by specified dimension and time. Uses GetVectorValue() functions.*/
	std::vector<double> GetDistribution(double _dTime, unsigned _nDim) const;
	/** Return distribution by specified dimensions and time. Uses GetVectorValue() functions.*/
	bool GetDistribution(double _dTime, unsigned _nDim1, unsigned _nDim2, CMatrix2D& _Result) const;
	/** Return distribution by specified dimensions and time. Uses GetVectorValue() functions.*/
	CMatrix2D GetDistribution(double _dTime, unsigned _nDim1, unsigned _nDim2) const;
	/** Return distribution by specified dimensions and time. Uses GetVectorValue() functions.*/
	bool GetDistribution(double _dTime, unsigned _nDim1, unsigned _nDim2, unsigned _nDim3, CDenseMDMatrix& _Result) const;
	/** Return distribution by specified dimensions and time. Uses GetVectorValue() functions.*/
	CDenseMDMatrix GetDistribution(double _dTime, unsigned _nDim1, unsigned _nDim2, unsigned _nDim3) const;
	/** Return distribution by specified dimensions and time. Uses GetVectorValue() functions.*/
	bool GetDistribution(double _dTime, const std::vector<unsigned>& _vDims, CDenseMDMatrix& _Result) const;
	/** Return distribution by specified dimensions and time. Uses GetVectorValue() functions.*/
	CDenseMDMatrix GetDistribution(double _dTime, const std::vector<unsigned>& _vDims) const;

	/** Sets distribution by specified dimension and time. Uses SetVectorValue() functions.*/
	bool SetDistribution( double _dTime, unsigned _nDim, const std::vector<double>& _vDistr );
	/** Sets distribution by specified dimensions and time. Uses SetVectorValue() functions.*/
	bool SetDistribution( double _dTime, unsigned _nDim1, unsigned _nDim2, const CMatrix2D& _Distr );
	/** Sets distribution by specified dimensions and time. Uses SetVectorValue() functions.*/
	bool SetDistribution( double _dTime, const CDenseMDMatrix& _Distr );

	// ========== Functions to work with matrix TRANSFORMATIONS

	/** Transforms matrix according to matrix _Transformation. Returns false on error.*/
	bool Transform( double _dTime, const CTransformMatrix& _TMatrix );

	// ========== Functions for matrix NORMALIZATION

	/** Normalizes data in matrix for specified time point. If time point wasn't defined, than nothing will be done.*/
	void NormalizeMatrix( double _dTime );
	/** Normalizes data in matrix for time interval (incl).*/
	void NormalizeMatrix( double _dStart, double _dEnd );
	/** Normalizes data in matrix for all time points.*/
	void NormalizeMatrix();
	/** Returns true if matrix is normalized in specified time point.*/
	bool IsNormalized( double _dTime );

	// ========== Functions to work with ANOTHER MATRICES

	/** Copy data from another MDMatrix on specified time point. If time point doesn't exist, it will be created.*/
	bool CopyFrom( const CMDMatrix& _Source, double _dTime );
	/** Copy data from another MDMatrix on specified time interval.*/
	bool CopyFrom( const CMDMatrix& _Source, double _dStart, double _dEnd );
	/** Copy data from time point _dTimeSrc of another MDMatrix to time point _dTimeDest of this matrix.
	*	If time point doesn't exist, it will be created.*/
	bool CopyFromTimePoint( const CMDMatrix& _Source, double _dTimeSrc, double _dTimeDest );

	///** Adds _srcMatr to a matrix with specified factors for time point _dTime.*/
	//void AddMatrix( CMDMatrix& _srcMatr, double _dFactorDst, double _dFactorSrc, double _dTime );
	///** Adds _srcMatr to a matrix with specified factors for time intervals.*/
	//void AddMatrix( CMDMatrix& _srcMatr, const std::vector<double>& _vFactorsDst, const std::vector<double>& _vFactorsSrc, const std::vector<double>& _vTimePoints );

	// ========== Functions to SAVE / LOAD matrix

	/** Save data to file.*/
	void SaveToFile( CH5Handler& _h5File, const std::string& _sPath );
	void SaveMDBlockToFile(CH5Handler& _h5File, const std::string& _sPath, unsigned _iFirst, unsigned _iLast, std::vector<std::vector<double>>& _vvBuf);
	/** Load data from file*/
	void LoadFromFile( CH5Handler& _h5File, const std::string& _sPath );
	void LoadMDBlockFromFile(CH5Handler& _h5File, const std::string& _sPath, unsigned _iFirst, unsigned _iLast, std::vector<std::vector<double>>& vvBuf);

	void SetCachePath(const std::wstring& _sPath);
	void SetCacheParams( bool _bEnabled, unsigned _nWindow );

	/** Removes all data, which can be approximated.*/
	void CompressData( double _dStartTime, double _dEndTime, double _dATol, double _dRTol );

	void ExtrapolateToPoint( double _dT1, double _dT2, double _dTExtra );
	void ExtrapolateToPoint( double _dT0, double _dT1, double _dT2, double _dTExtra );

private:
	/** Checks the duplicates in vector. Return true if there are no duplicates.*/
	bool CheckDuplicates( const std::vector<unsigned>& _vVec ) const;
	/** Returns index of time point. Strict search returns -1 if there is no such time, not strict search returns index to paste.*/
	unsigned GetTimeIndex(double _dTime, bool _bIsStrict = true) const;
	/** Initializes current fraction with specified size.*/
	sFraction* IitialiseDimension( unsigned _nSize ) const;
	/*	Increments last coordinate for getting/setting vectors according to a dimensions set. Must be _vCoords.size()+1 == _vSizes.size().
	*	Returns false if the end is reached.*/
	bool IncrementCoords( std::vector<unsigned>& _vCoords, const std::vector<unsigned>& _vSizes ) const;
	/** Sorts matrix for time point _dSrcTime according to the order of dimensions in _dstMatrix and sets it to time-point _dDstTime.
	*	Returns false on error.*/
	bool SortMatrix( double _dSrcTime, double _dDstTime, CMDMatrix& _dstMatrix );
	/** Sorts matrix according to order in _dstMatrix for all time points.*/
	bool SortMatrix( CMDMatrix& _dstMatrix );
	/** Removes dimensions and puts new sorted matrix in _sortMatr. New dimensions and classes set will be in _vNewDims and _vNewClasses.*/
	void DeleteDimsWithSort( const std::vector<unsigned>& _vDims, std::vector<unsigned>& _vNewDims, std::vector<unsigned>& _vNewClasses, CMDMatrix& _sortMatr );

	// ========== Recursive functions

	void NormalizeWrapper( const std::vector<double>& _vTimes );

	/** Copies all data from _pSrc to a current matrix.*/
	sFraction* CopyFractionsRecursive( sFraction *_pSrc, unsigned _nNesting = 0 );
	/** Removes all fractions starting from _pFraction.*/
	sFraction* RemoveFractionsRecursive( sFraction *_pFraction, unsigned _nNesting = 0 );
	/** Adds specified time point m_dTempT1 to each fraction. Data to a new time point will be copied from m_dTempT2.*/
	void AddTimePointRecursive( sFraction *_pFraction, unsigned _nNesting = 0 );
	/** Changes time point m_dTempT1 to a m_dTempT2.*/
	void ChangeTimePointRecursive( sFraction *_pFraction, unsigned _nNesting = 0 );
	/** Removes time points from interval [m_dTempT1..m_dTempT2] or single time point m_dTempT1 (if m_dTempT2 == -1)*/
	sFraction* RemoveTimePointsRecursive( sFraction *_pFraction, unsigned _nNesting = 0 );
	/** Returns value for time m_dTempT1 according to specified dimensions m_vTempDims and coordinates m_vTempCoords. Dimensions set can be reduced.*/
	double GetValueRecursive(sFraction *_pFraction, unsigned _nLevel = 1, unsigned _nNesting = 0) const;
	/** Sets value m_dTempValue for time m_dTempT1 according to specified dimensions m_vTempDims and coordinates m_vTempCoords.
	*	Dimensions set can be reduced. If time point wasn't defined, nothing will be done. Returns false on error.*/
	bool SetValueRecursive( sFraction *_pFraction, bool _bExternal = true, unsigned _nLevel = 1, unsigned _nNesting = 0 );
	/** Returns vector value for time m_dTempT1 according to specified dimensions m_vTempDims and coordinates m_vTempCoords.
	*	Dimensions set can be reduced. Returns false on error.*/
	bool GetVectorValueRecursive(sFraction *_pFraction, std::vector<double>& _vRes, unsigned _nNesting = 0) const;
	/*	Sets vector value m_vTempValues for time m_dTempT1 according to specified dimensions m_vTempDims and coordinates m_vTempCoords.
	*	Dimensions set can be reduced. Returns false on error.*/
	bool SetVectorValueRecursive( sFraction *_pFraction, unsigned _nNesting = 0 );
	/** Transforms matrix m_pSortMatr according to a transformation matrix _TMatr.
	*	Doesn't check the correspondence of dimensions between m_pSortMatr, _TMatr and this matrix.*/
	void TransformRecurcive( const CTransformMatrix& _TMatr );
	/** Normalizes matrix for time point m_vTempValues.*/
	void NormalizeMatrixBySumRecursive( sFraction *_pFraction, unsigned _nNesting = 0 );
	bool NormalizeMatrixRecursive( sFraction *_pFraction, unsigned _nNesting = 0 );
	bool CheckNormalizationRecursive( sFraction *_pFraction, unsigned _nNesting = 0 );
	/** Copies data from another matrix for time interval [m_dTempT1, m_dTempT2].*/
	sFraction* CopyFromRecursive( sFraction *_pDest, sFraction *_pSource, unsigned _nNesting = 0 );
	/** Copies data from another matrix from time point m_dTempT1 to m_dTempT2.*/
	sFraction* CopyFromTimePointRecursive( sFraction *_pDest, sFraction *_pSource, unsigned _nNesting = 0 );
	/** Adds new class to a dimension with index _nDimIndex.*/
	sFraction* AddClassRecursive( sFraction *_pFraction, unsigned _nDimIndex, unsigned _nNesting = 0 );
	/** Removes class _nClassIndex from a dimension _nDimIndex.*/
	sFraction* RemoveClassRecursive( sFraction *_pFraction, unsigned _nDimIndex, unsigned _nClassIndex, unsigned _nNesting = 0 );
	/** Removes all data, which can be approximated in time interval [m_dTempT1, m_dTempT2].*/
	void CompressDataRecursive( sFraction *_pFraction, double _dATol, double _dRTol, unsigned _nNesting = 0 );
	//void NormalizeToOneRecursive( sFraction *_pFraction, unsigned _nNesting = 0 );
	//void NormalizeToZeroRecursive( sFraction *_pFraction, unsigned _nNesting = 0 );
	//sFraction* CreateWithOneRecursive( sFraction *_pFraction, unsigned _nNesting = 0 );
	sFraction* SetToZeroRecursive( sFraction *_pFraction, unsigned _nNesting = 0 );
	sFraction* SetToOneRecursive( sFraction *_pFraction, unsigned _nNesting = 0 );
	void Extrapolate2ToPointRecursive( sFraction *_pFraction, unsigned _nNesting = 0 );
	void Extrapolate3ToPointRecursive( sFraction *_pFraction, unsigned _nNesting = 0 );


	void UnCacheData(double _dTP) const;
	void UnCacheData(double _dT1, double _dT2) const;
	void FlushToCache() const;
	void CheckCacheNeed() const;
	void CacheData() const;
	void CorrectWinBoundary() const;
	void ClearCache() const;

	sFraction* UnCacheDataRecursive( sFraction *_pFraction, std::vector<std::vector<double>>& _vData, unsigned _nNesting = 0 ) const;
	void CacheDataRecursive( sFraction *_pFraction, std::vector<std::vector<double>>& _vData, unsigned _nNesting = 0 ) const;

	void GetDataForSaveRecursive( sFraction *_pFraction, std::vector<std::vector<double>>& _vData, unsigned _nNesting = 0 ) const;
	sFraction* SetDataForLoadRecursive( sFraction *_pFraction, std::vector<std::vector<double>>& _vData, unsigned _nNesting = 0 ) const;
};
