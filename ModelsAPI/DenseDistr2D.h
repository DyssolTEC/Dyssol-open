/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "H5Handler.h"
#include "DenseDistrCacher.h"

/** This class is used to describe time dependent distribution of one-dimensional parameter.
 *	Finally data is stored in two dimensional array. Time points are stored separately in vector.*/
class CDenseDistr2D
{
private:
	static const unsigned m_cnSaveVersion;

	unsigned m_nDimensions;
	std::vector<double> m_vTimePoints; // the time points
	mutable std::vector<std::vector<double>> m_Data; // data itself
	//unsigned m_nLastTimePos; // last used index
	std::vector<std::string> m_vLabels;

	std::wstring m_sCachePath;
	bool m_bCacheEnabled;
	mutable CDenseDistrCacher *m_pCacheHandler;
	mutable double m_dCurrWinStart;
	mutable double m_dCurrWinEnd;
	mutable size_t m_nCurrOffset;
	unsigned m_nCacheWindow;
	mutable bool m_bCacheCoherent;

public:
	CDenseDistr2D(unsigned _nDimensions = 0);
	~CDenseDistr2D();

	void Clear(); // clear all data and all time points

	void AddTimePoint(double _dTimePoint, double _dSourceTimePoint = -1 ); // adds new time point
	void RemoveTimePoint( double _dTimePoint ); // deletes specified time point
	void RemoveTimePoints( double _dStart, double _dEnd ); // removes data from interval
	void RemoveTimePoints(const std::vector<unsigned>& _vIndexes); // removes values of specified indexes - _vTimePoints. _vTimePoints array in ascending order
	// Removes all data after the specified time point.
	void RemoveAllDataAfter(double _dStartTime, bool _bIncludeStartTime = false);
	void ChangeTimePoint( unsigned _nTimePointIndex, double _dNewValue ); // changes specified time point
	void RemoveAllTimePoints();

	size_t GetTimePointsNumber() const;
	std::vector<double> GetAllTimePoints() const;
	// returns indexes of all time points which are situated in this time interval
	std::vector<unsigned> GetIndexesForInterval( double _dStartTime, double _dEndTime );
	std::vector<double> GetValueForIndex( unsigned _nIndex ) const;
	std::vector<double> GetValues(unsigned _nDimension) const;
	double GetTimeForIndex(unsigned _nIndex) const;
	// returns value for specific time point, and for specific property of this dimensions
	double GetValue( double _dTime, size_t _nDimension ) const;
	std::vector<double> GetValues(std::vector<double> _vTimes, unsigned _nDimension) const;
	// Returns vector of values defined for the specified time pointy (or interpolated). If the distribution is empty, returns vector of zeros with the proper size.
	std::vector<double> GetValue(double _dTime) const;

	//***  SETS NEW VALUE of distributed property.
	void SetValue(double _dT, const std::vector<double>& _newValue);
	void SetValue( unsigned _nTimeIndex, unsigned _nPropIndex, double _dNewValue );
	void SetValue( double _dTime, unsigned _nPropIndex, double _dNewValue );

	//void SaveToFile(std::string _sFileName);
	//void LoadFromFile(std::string _sFileName);

	/** Save data to file.*/
	void SaveToFile( CH5Handler& _h5File, const std::string& _sPath );
	/** Load data from file*/
	void LoadFromFile( CH5Handler& _h5File, const std::string& _sPath );

	unsigned GetDimensionsNumber() const;
	// resize matrix to the new dimension. if reducing should occur than all unnecessary values removed. By addition of new - zero entries are added
	void SetDimensionsNumber(unsigned _nNewNumber);

	//** work with labels **//
	void SetDimensionsLabels( std::vector<std::string>& _vLabels );
	void SetDimensionLabel( unsigned _nDimension, std::string _label );
	std::vector<std::string> GetLabels();
	std::string GetLabel( unsigned _nDimension ) const;

	void AddDimension();
	void RemoveDimension( unsigned _nIndex ); // removes specified dimension

	// copy one dense distribution into other for specified time interval - return true if copying was successful
	bool CopyFrom( CDenseDistr2D* _pSource, double _dStartTime, double _dEndTime );
	// copy one dense distribution into other for specified time point. if there is no such point it will be approximated
	bool CopyFrom( CDenseDistr2D* _pSource, double _dTime );
	// copy data to another time point
	bool CopyFromTimePoint( CDenseDistr2D* _pSource, double _dTimeSrc, double _dTimeDest );

	void SetCachePath(const std::wstring& _sPath);
	void SetCacheParams( bool _bEnabled, unsigned _nWindow );

	void ExtrapolateToPoint( double _dT1, double _dT2, double _dTExtra );
	void ExtrapolateToPoint( double _dT0, double _dT1, double _dT2, double _dTExtra );

private:
	void UnCacheData(double _dTP) const;
	void UnCacheData(double _dT1, double _dT2) const;
	void FlushToCache() const;
	void CheckCacheNeed();
	void CacheData();
	void CorrectWinBoundary();
	void ClearCache() const;

	/** Returns index of the time point with value _dTime. Returns index to paste if there is no such time point.*/
	size_t GetIndexByTime(double _dTime);
};
