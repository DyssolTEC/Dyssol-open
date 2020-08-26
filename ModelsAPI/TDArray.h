/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolTypes.h"
#include <vector>

/** This class represents a time dependent array of values. Not all time point are saving - only those, which can't be obtained by data interpolation.*/
class CTDArray
{
private:
	std::vector<STDValue> m_data;	///< Time dependent data
	size_t m_nLastTimePos;		///< Last used index in m_data

public:
	CTDArray( void );
	CTDArray( CTDArray& _source );
	~CTDArray( void );

	// ========== Functions to work with TIME POINTS

	/** Adds new time point. If time point already exists, than nothing will be done.
	*	Data to a new time point will be copied from _dSourceTimePoint. If _dSourceTimePoint == -1, than data will be copied from the previous time point.*/
	void AddTimePoint( double _dTime, double _dSourceTimePoint = -1 );
	/** Removes time point.*/
	void RemoveTimePoint( double _dTime );
	/** Removes all time points from interval (incl).*/
	void RemoveTimePoints( double _dStartTime, double _dEndTime );
	/** Change time of time point (for UI purposes). Time point should be always bigger than the previous one and smaller than the next one.*/
	void ChangeTimePoint( double _dOldTime, double _dNewTime );

	// ========== Functions to GET and SET data

	/** Returns value according to a specified time. If there is no such point, returns interpolated value. If data can not be obtained, 0 will be returned.*/
	double GetValue( double _dTime );
	/** Returns vector of values according to vector of times with interpolation.*/
	void GetVectorValue( const std::vector<double>& _dTimes, std::vector<double>& _vRes );
	/** Sets new value to a specified time point. If time point doesn't exist, than it will be created. All negative values will be set to 0.*/
	void SetValue( double _dTime, double _dValue );

	// ========== Functions to work with another arrays

	/** Copy data from another array for time point. If time point wasn't defined in this array, it will be created.*/
	void CopyFrom( CTDArray& _source, double _dTime );
	/** Copy data from another array on time interval.*/
	void CopyFrom( CTDArray& _source, double _dStartTime, double _dEndTime );
	/** Copy data from another array to another time point.*/
	void CopyFromTimePoint( CTDArray& _source, double _dTimeSrc, double _dTimeDest );

	// ========== Functions to SAVE / LOAD arrays

	void GetCacheArray( const std::vector<double>& _vTP, std::vector<double>& _vOut );
	bool SetCacheArray( const std::vector<double>& _vTP, const std::vector<double>& _vData );

	void GetDataForSave( const std::vector<double>& _vTP, std::vector<double>& _vOut );
	bool SetDataForLoad( const std::vector<double>& _vTP, const std::vector<double>& _vData );

	// ========== Other functions

	// Clears all data in array.*/
	void Clear();
	/** Returns true if m_data doesn't contain data.*/
	bool IsEmpty() const;
	/** Returns number of values in m_data.*/
	size_t GetDataLength() const;

	CTDArray& operator=( CTDArray& _source );

	/** Removes all data, which can be approximated.*/
	void CompressData( double _dStartTime, double _dEndTime, double _dATol, double _dRTol );

	void ExtrapolateToPoint(double _dT1, double _dT2, double _dTExtra);
	void ExtrapolateToPoint(double _dT0, double _dT1, double _dT2, double _dTExtra);


private:
	/** Returns index of the time point with value _dTime. Strict search returns -1 if there is no such time, not strict search returns index to paste.*/
	size_t GetIndexByTime( double _dTime, bool _bIsStrict = true );

	/** Sets new data using the data approximation. Not using parameters must be set to -1.*/
	void SetData(size_t _nIndex, double _dTime, double _dValue);

	///** Returns interpolated value of data for time _dTime between time points with indexes _nIndex1 and _nIndex2.*/
	//double GetInterpolation( unsigned _nIndex1, unsigned _nIndex2, double _dTime ) const;

	///** Returns interpolated value of data for time _dTime between time points _dLTime and _dRTime with values _dLVal and _dRVal.*/
	//double GetInterpolation( double _dLVal, double _dRVal, double _dLTime, double _dRTime, double _dTime ) const;
};
