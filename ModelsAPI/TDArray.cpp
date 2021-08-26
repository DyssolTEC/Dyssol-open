/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "TDArray.h"
#include "DyssolUtilities.h"

CTDArray::CTDArray(void):
	m_nLastTimePos(0)
	{}

CTDArray::CTDArray(CTDArray& _source):
	m_data(_source.m_data),
	m_nLastTimePos(_source.m_nLastTimePos)
	{}

CTDArray::~CTDArray(void)
{
	Clear();
}

void CTDArray::AddTimePoint(double _dTime, double _dSourceTimePoint /*= -1 */)
{
	if( m_data.empty() ) // nothing to do if its empty
		return;

	if( _dTime < 0 ) // wrong time
		return;

	size_t index = GetIndexByTime( _dTime, false ); // get new index to insert
	if( index < m_data.size() )
		if( m_data[index].time == _dTime ) // time point already exists
			return;

	if( _dSourceTimePoint == -1 )
	{
		if( index != 0 )
			SetData( index, _dTime, m_data[index-1].value );
		else
			SetData( index, _dTime, 0 );
	}
	else
		SetData( index, _dTime, GetValue( _dSourceTimePoint ) );
}

void CTDArray::RemoveTimePoint(double _dTime)
{
	size_t index = GetIndexByTime( _dTime );
	if( index == -1 ) // no such time point
		return;

	m_data.erase( m_data.begin() + index );
}

void CTDArray::RemoveTimePoints(double _dStartTime, double _dEndTime)
{
	if( _dStartTime > _dEndTime ) // wrong time interval
		return;

	if( m_data.empty() ) // nothing to remove
		return;

	size_t iLast = GetIndexByTime( _dEndTime, false );
	size_t iFirst = GetIndexByTime( _dStartTime, false );

	if( ( iLast >= m_data.size() ) || ( m_data[iLast].time != _dEndTime )) // iLast correction
		iLast--;

	m_data.erase( m_data.begin() + iFirst, m_data.begin() + (iLast + 1) );
}

void CTDArray::ChangeTimePoint(double _dOldTime, double _dNewTime)
{
	size_t index = GetIndexByTime( _dOldTime );
	if( index == -1 ) // no such time point
		return;

	if( m_data.size() > 1 )
	{
		if(( index != 0 ) && ( m_data[index-1].time >= _dNewTime ) ) // new time value is before the previous time point
			return;
		if(( index != m_data.size()-1 ) && ( m_data[index+1].time <= _dNewTime ) ) // new time value is bigger than the next time point
			return;
	}

	SetData( index, _dNewTime, -1 );
}

double CTDArray::GetValue(double _dTime)
{
	if( m_data.size() == 0 ) // no time points
		return 0;

	size_t index = GetIndexByTime( _dTime );
	if( index != -1 ) // time point is found
		return m_data[index].value;

	if( m_data.size() == 1 ) // not enough data for interpolation
		return m_data.front().value;

	size_t indexAfter = GetIndexByTime( _dTime, false );
	if(( indexAfter != m_data.size() ) && ( indexAfter != 0 )) // point inside - interpolation
		//return GetInterpolation( indexAfter-1, indexAfter, _dTime );
		return Interpolate( m_data[indexAfter].value, m_data[indexAfter-1].value, m_data[indexAfter].time, m_data[indexAfter-1].time, _dTime );
	else if( indexAfter == m_data.size() ) // point after the last - extrapolation
		//return GetInterpolation( indexAfter-2, indexAfter-1, _dTime );
		return m_data.back().value;
	else // point at the beginning - extrapolation
		//return GetInterpolation( indexAfter+1, indexAfter+0, _dTime );
		return m_data.front().value;
}

void CTDArray::GetVectorValue(const std::vector<double>& _dTimes, std::vector<double>& _vRes)
{
	_vRes.resize( _dTimes.size() );
	for(size_t i=0; i<_dTimes.size(); ++i )
		_vRes[i] = GetValue( _dTimes[i] );
}

void CTDArray::SetValue(double _dTime, double _dValue)
{
	if( _dTime < 0 ) // wrong time
		return;

	if( _dValue < 0 )
		_dValue = 0;

	size_t index = GetIndexByTime( _dTime );
	if( index == -1 ) // no such time point
	{
		index = GetIndexByTime( _dTime, false ); // get new index to insert
		SetData( index, _dTime, _dValue );
	}
	else // set new value for time point
	{
		SetData( index, -1, _dValue );
	}
}

void CTDArray::CopyFrom(CTDArray& _source, double _dTime)
{
	SetValue( _dTime, _source.GetValue( _dTime ) );
}

void CTDArray::CopyFrom(CTDArray& _source, double _dStartTime, double _dEndTime)
{
	if( _dStartTime == _dEndTime ) // for single time point
		SetValue( _dStartTime, _source.GetValue( _dStartTime ) );
	else // for time interval
	{
		size_t index = _source.GetIndexByTime( _dStartTime, false );
		if( ( index != -1 ) && ( index <_source.m_data.size() ) && ( _source.m_data[index].time != _dStartTime ) ) // left boundary of the interval
			SetValue( _dStartTime, _source.GetValue( _dStartTime ) );
		while( ( index < _source.m_data.size() ) && ( _source.m_data[index].time <= _dEndTime ) ) // interval
		{
			SetValue( _source.m_data[index].time, _source.m_data[index].value );
			index++;
		}
		if( ( index <_source.m_data.size() ) && ( _source.m_data[index].time != _dEndTime ) ) // right boundary of the interval
			SetValue( _dEndTime, _source.GetValue( _dEndTime ) );
	}
}

void CTDArray::CopyFromTimePoint(CTDArray& _source, double _dTimeSrc, double _dTimeDest)
{
	SetValue( _dTimeDest, _source.GetValue( _dTimeSrc ) );
}

void CTDArray::GetCacheArray( const std::vector<double>& _vTP, std::vector<double>& _vOut )
{
	_vOut.resize( _vTP.size(), -1 );
	size_t nInternalCnt = 0;
	for(size_t i=0; i<_vTP.size(); ++i )
	{
		if( ( nInternalCnt < m_data.size() ) && ( m_data[nInternalCnt].time == _vTP[i] ) )
		{
			_vOut[i] = m_data[nInternalCnt].value;
			nInternalCnt++;
		}
	}
	RemoveTimePoints( _vTP.front(), _vTP.back() );
	std::vector<STDValue>(m_data).swap(m_data);
}

bool CTDArray::SetCacheArray( const std::vector<double>& _vTP, const std::vector<double>& _vData )
{
	if( _vTP.size() > _vData.size() )
		return false;

	if( !m_data.empty() )
		//return false;
		m_data.clear();

	for(size_t i=0; i<_vTP.size(); ++i )
		if( ( _vData[i] != -1 ) && ( _vTP[i] != -1 ) )
			m_data.push_back( STDValue( _vTP[i], _vData[i] ) );

	return !m_data.empty();
}

void CTDArray::GetDataForSave(const std::vector<double>& _vTP, std::vector<double>& _vOut)
{
	_vOut.resize(_vTP.size());
	size_t iCnt = GetIndexByTime(_vTP.front());
	double dFirstValue = -2;
	bool bAllAreEqual = true;
	for (size_t i = 0; i < _vTP.size(); ++i)
	{
		if ((iCnt < m_data.size()) && (m_data[iCnt].time == _vTP[i]))
		{
			_vOut[i] = m_data[iCnt].value;
			if (dFirstValue == -2)
				dFirstValue = m_data[iCnt].value;
			else if (bAllAreEqual && (dFirstValue != m_data[iCnt].value))
				bAllAreEqual = false;
			iCnt++;
		}
	}

	if (bAllAreEqual && !_vOut.empty())
		_vOut.resize(1, _vOut.front());
}

bool CTDArray::SetDataForLoad(const std::vector<double>& _vTP, const std::vector<double>& _vData)
{
	std::vector<double> vData = _vData;
	if (_vTP.size() != vData.size())
	{
		if (vData.size() == 1)
		{
			vData.resize(_vTP.size(), vData.front());
		}
		else
		{
			return false;
		}
	}
	size_t nEmptyCnt = 0;
	for (size_t i = 0; i < _vTP.size(); ++i)
		if ((vData[i] != -1) && (_vTP[i] != -1))
			m_data.push_back(STDValue(_vTP[i], vData[i]));
		else
			nEmptyCnt++;

	return vData.size() != nEmptyCnt;
}

void CTDArray::Clear()
{
	m_data.clear();
}

bool CTDArray::IsEmpty() const
{
	return m_data.empty();
}

size_t CTDArray::GetDataLength() const
{
	return m_data.size();
}

CTDArray& CTDArray::operator=(CTDArray& _source)
{
	if(this == &_source)
		return *this;

	Clear();

	m_data = _source.m_data;
	m_nLastTimePos = _source.m_nLastTimePos;

	return *this;
}

void CTDArray::CompressData( double _dStartTime, double _dEndTime, double _dATol, double _dRTol )
{
	// get indexes of time boundaries
	size_t iStart = GetIndexByTime( _dStartTime, false );
	size_t iEnd = GetIndexByTime( _dEndTime, false );
	if( iEnd == m_data.size() )
		iEnd--;

	if( iEnd - iStart < 3 ) // nothing to compress
		return;

	size_t i = iStart+1;
	while( i<iEnd-1 )
	{
		//double dInterpVal = GetInterpolation( i-1, i+1, m_data[i].time );
		double dInterpVal = Interpolate( m_data[i-1].value, m_data[i+1].value, m_data[i-1].time, m_data[i+1].time, m_data[i].time );
		if(std::fabs( m_data[i].value - dInterpVal ) <= std::fabs( m_data[i].value ) * _dRTol + _dATol ) // value can be interpolated. remove
		{
			m_data.erase( m_data.begin() + i );
			iEnd--;
		}
		else
			i++;
	}
}

size_t CTDArray::GetIndexByTime(double _dTime, bool _bIsStrict /*= true */)
{
	// check if empty
	if( m_data.empty() )
	{
		if( _bIsStrict )
			return -1;
		else
			return 0;
	}

	// check last used point
	if( ( m_nLastTimePos < m_data.size() ) && ( m_data.at( m_nLastTimePos ).time == _dTime ) )
		return m_nLastTimePos;
	// check next of last used point
	if( ( ( m_nLastTimePos+1 ) < m_data.size() ) && ( m_data.at( m_nLastTimePos+1 ).time == _dTime ))
		return ++m_nLastTimePos;
	// check previous of last used point
	if( ( m_nLastTimePos > 0 ) && ( ( m_nLastTimePos-1 ) < m_data.size() ) && ( m_data.at( m_nLastTimePos-1 ).time == _dTime ))
		return --m_nLastTimePos;
	// check boundaries
	if( m_data.front().time > _dTime )
	{
		if( _bIsStrict )
			return -1;
		else
			return 0;
	}
	if( m_data.back().time < _dTime )
	{
		if( _bIsStrict )
			return -1;
		else
			return m_data.size();
	}

	// binary search
	size_t nFirst = 0;
	size_t nLast = m_data.size();
	size_t nMid = (size_t)(nFirst + nLast) >> 1;
	while( nFirst < nLast )
	{
		if ( _dTime <= m_data[nMid].time )
			nLast = nMid;
		else
			nFirst = nMid + 1;
		nMid = (size_t)(nFirst + nLast) >> 1;
	}
	if( ( m_data[nLast].time == _dTime ) || ( !_bIsStrict ) )
	{
		m_nLastTimePos = nLast;
		return m_nLastTimePos;
	}
	else
		return -1;
}

void CTDArray::SetData(size_t _nIndex, double _dTime, double _dValue)
{
	if( _dTime == -1 ) // value changing, no inserting
	{
		m_data[_nIndex].value = _dValue;
	}
	else if( _dValue == -1 ) // time changing, no inserting
	{
		m_data[_nIndex].time = _dTime;
	}
	else // adding of new point, inserting
	{
		m_data.insert( m_data.begin() + _nIndex, STDValue( _dTime, _dValue ) );
	}

	//if( _dTime == -1 ) // value changing, no inserting
	//{
	//	if( m_data.size() < 3 ) // just set new value
	//	{
	//		m_data[_nIndex].value = _dValue;
	//	}
	//	else if( _nIndex == 0 ) // changing at the beginning
	//	{
	//		m_data[_nIndex].value = _dValue;

	//		// check if the next value can be interpolated
	//		double dInterpVal = GetInterpolation( _nIndex, _nIndex+2, m_data[_nIndex+1].time );
	//		double dRealVal = m_data[_nIndex+1].value;
	//		if( std::fabs( dRealVal - dInterpVal ) <= std::fabs( dRealVal ) * m_dRTol + m_dATol ) // next value can be removed
	//			m_data.erase( m_data.begin()+_nIndex+1 );
	//	}
	//	else if( _nIndex == m_data.size()-1 ) // changing at the end
	//	{
	//		m_data[_nIndex].value = _dValue;

	//		// check if the previous value can be interpolated
	//		double dInterpVal = GetInterpolation( _nIndex-2, _nIndex, m_data[_nIndex-1].time );
	//		double dRealVal = m_data[_nIndex-1].value;
	//		if( std::fabs( dRealVal - dInterpVal ) <= std::fabs( dRealVal ) * m_dRTol + m_dATol ) // previous value can be removed
	//			m_data.erase( m_data.begin()+_nIndex-1 );
	//	}
	//	else // changing inside
	//	{
	//		// check if the value can be interpolated
	//		double dInterpVal = GetInterpolation( _nIndex-1, _nIndex+1, m_data[_nIndex].time );
	//		if( std::fabs( _dValue - dInterpVal ) > std::fabs( _dValue ) * m_dRTol + m_dATol ) // value can't be interpolated. set
	//			m_data[_nIndex].value = _dValue;
	//	}
	//}
	//else if( _dValue == -1 ) // time changing, no inserting
	//{
	//	if( m_data.size() < 3 ) // just set new time
	//	{
	//		m_data[_nIndex].time = _dTime;
	//	}
	//	else if( _nIndex == 0 ) // changing at the beginning
	//	{
	//		m_data[_nIndex].time = _dTime;

	//		// check if the next value can be interpolated
	//		double dInterpVal = GetInterpolation( _nIndex, _nIndex+2, m_data[_nIndex+1].time );
	//		double dRealVal = m_data[_nIndex+1].value;
	//		if( std::fabs( dRealVal - dInterpVal ) <= std::fabs( dRealVal ) * m_dRTol + m_dATol ) // next value can be removed
	//			m_data.erase( m_data.begin()+_nIndex+1 );
	//	}
	//	else if( _nIndex == m_data.size()-1 ) // changing at the end
	//	{
	//		m_data[_nIndex].time = _dTime;

	//		// check if the previous value can be interpolated
	//		double dInterpVal = GetInterpolation( _nIndex-2, _nIndex, m_data[_nIndex-1].time );
	//		double dRealVal = m_data[_nIndex-1].value;
	//		if( std::fabs( dRealVal - dInterpVal ) <= std::fabs( dRealVal ) * m_dRTol + m_dATol ) // previous value can be removed
	//			m_data.erase( m_data.begin()+_nIndex-1 );
	//	}
	//	else // changing inside
	//	{
	//		// check if the value can be interpolated
	//		double dInterpVal = GetInterpolation( _nIndex-1, _nIndex+1, _dTime );
	//		double dRealVal = m_data[_nIndex].value;
	//		if( std::fabs( dRealVal - dInterpVal ) > std::fabs( dRealVal ) * m_dRTol + m_dATol ) // value can't be interpolated. set
	//			m_data[_nIndex].time = _dTime;
	//	}
	//}
	//else // adding of new point, inserting
	//{
	//	if( m_data.size() < 2 ) // just set new value
	//	{
	//		m_data.insert( m_data.begin() + _nIndex, sTDValue( _dTime, _dValue ) );
	//	}
	//	else if( _nIndex == 0 ) // adding to the beginning
	//	{
	//		double dInterpVal = GetInterpolation( _dValue, m_data[_nIndex+1].value, _dTime, m_data[_nIndex+1].time, m_data[_nIndex].time );
	//		double dRealVal = m_data[_nIndex].value;
	//		if( std::fabs( dRealVal - dInterpVal ) <= std::fabs( dRealVal ) * m_dRTol + m_dATol ) // next value can be removed
	//		{
	//			m_data[_nIndex].time = _dTime;
	//			m_data[_nIndex].value = _dValue;
	//		}
	//		else // insert new
	//		{
	//			m_data.insert( m_data.begin() + _nIndex, sTDValue( _dTime, _dValue ) );
	//		}
	//	}
	//	else if( _nIndex == m_data.size() ) // adding to the end
	//	{
	//		double dInterpVal = GetInterpolation( m_data[_nIndex-2].value, _dValue, m_data[_nIndex-2].time, _dTime, m_data[_nIndex-1].time );
	//		double dRealVal = m_data[_nIndex-1].value;
	//		if( std::fabs( dRealVal - dInterpVal ) <= std::fabs( dRealVal ) * m_dRTol + m_dATol ) // next value can be removed
	//		{
	//			m_data[_nIndex-1].time = _dTime;
	//			m_data[_nIndex-1].value = _dValue;
	//		}
	//		else // insert new
	//		{
	//			m_data.insert( m_data.begin() + _nIndex, sTDValue( _dTime, _dValue ) );
	//		}
	//	}
	//	else // inserting
	//	{
	//		// check if the value can be interpolated
	//		double dInterpVal = GetInterpolation( _nIndex-1, _nIndex, _dTime );
	//		if( std::fabs( _dValue - dInterpVal ) > std::fabs( _dValue ) * m_dRTol + m_dATol ) // value can't be interpolated. insert
	//		{
	//			m_data.insert( m_data.begin() + _nIndex, sTDValue( _dTime, _dValue ) );
	//		}
	//	}
	//}
}

//double CTDArray::GetInterpolation(unsigned _nIndex1, unsigned _nIndex2, double _dTime) const
//{
//	return ( m_data[_nIndex2].value - m_data[_nIndex1].value ) / ( m_data[_nIndex2].time - m_data[_nIndex1].time ) * ( _dTime - m_data[_nIndex1].time ) + m_data[_nIndex1].value;
//}
//
//double CTDArray::GetInterpolation(double _dLVal, double _dRVal, double _dLTime, double _dRTime, double _dTime) const
//{
//	return ( _dRVal - _dLVal ) / ( _dRTime - _dLTime ) * ( _dTime - _dLTime ) + _dLVal;
//}

void CTDArray::ExtrapolateToPoint(double _dT1, double _dT2, double _dTExtra)
{
	double dV1 = GetValue( _dT1 );
	double dV2 = GetValue( _dT2 );

	//double dVExtra = ( dV2 - dV1 ) / ( _dT2 - _dT1 ) * ( _dTExtra - _dT1 ) + dV1;
	double dVExtra = Interpolate( dV1, dV2, _dT1, _dT2, _dTExtra );
	SetValue( _dTExtra, dVExtra );
}

void CTDArray::ExtrapolateToPoint(double _dT0, double _dT1, double _dT2, double _dTExtra)
{
	double dV0 = GetValue( _dT0 );
	double dV1 = GetValue( _dT1 );
	double dV2 = GetValue( _dT2 );

	double dVExtra = Extrapolate( dV0, dV1, dV2, _dT0, _dT1, _dT2, _dTExtra );
	SetValue( _dTExtra, dVExtra );
}
