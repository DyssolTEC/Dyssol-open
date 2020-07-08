/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DenseDistr2D.h"
#include "DyssolDefines.h"
#include "DyssolUtilities.h"
#include "DyssolStringConstants.h"

const unsigned CDenseDistr2D::m_cnSaveVersion	= 2;

CDenseDistr2D::CDenseDistr2D(unsigned _nDimensions ):
	/// Cache
	m_sCachePath(L""),
	m_pCacheHandler(NULL),
	m_bCacheEnabled(false),
	m_dCurrWinStart(0),
	m_dCurrWinEnd(0),
	m_nCacheWindow(DEFAULT_CACHE_WINDOW),
	m_nCurrOffset(0),
	m_bCacheCoherent(false)
{
	SetDimensionsNumber( _nDimensions );
	//m_nLastTimePos = 0;
}

CDenseDistr2D::~CDenseDistr2D()
{
	Clear();
	if( m_pCacheHandler )
		delete m_pCacheHandler;
}

void CDenseDistr2D::Clear()
{
	//for (unsigned i=0; i<m_Data.size(); i++ )
	//	delete m_Data[ i ];
	m_Data.clear();
	m_vTimePoints.clear();
	m_nDimensions = 0;
	//m_vLabels.clear();
	ClearCache();
}

void CDenseDistr2D::AddTimePoint( double _dTimePoint, double _dSourceTimePoint )
{
	//TODO: set time point to zero if it must be set at the first place
	if( _dTimePoint < 0 )
		return;

	std::vector<double> vNewVector( m_nDimensions, 0 );

	if ( m_vTimePoints.empty() )  // if this is the first data at all
	{
		UnCacheData( _dTimePoint );
		m_vTimePoints.push_back( _dTimePoint );
		m_Data.push_back( vNewVector );
	}
	else
	{
		size_t nIndex = GetIndexByTime( _dTimePoint );
		if( nIndex < m_vTimePoints.size() )
		{
			if( m_vTimePoints[nIndex] == _dTimePoint ) // already exists
				return;
			if( _dSourceTimePoint == -1 ) // copy previous data into the new time point
			{
				if( nIndex > 0 )
				{
					UnCacheData( m_vTimePoints[nIndex-1] );
					vNewVector = m_Data[ nIndex-1-m_nCurrOffset ];
				}
				else
				{
					UnCacheData( m_vTimePoints[nIndex] );
					vNewVector = m_Data[ nIndex-m_nCurrOffset ];
				}
			}
			else // copy data from _dSourceTimePoint
			{
				vNewVector = GetValue( _dSourceTimePoint );
			}
			UnCacheData( _dTimePoint );
			m_vTimePoints.insert( m_vTimePoints.begin() + nIndex, _dTimePoint );
			m_Data.insert( m_Data.begin() + (nIndex - m_nCurrOffset), vNewVector );
		}
		else
		{
			if( _dSourceTimePoint == -1 ) // copy previous data into the new time point
			{
				UnCacheData( m_vTimePoints[m_vTimePoints.size()-1] );
				vNewVector = m_Data.back();
			}
			else // copy data from _dSourceTimePoint
			{
				vNewVector = GetValue( _dSourceTimePoint );
			}
			UnCacheData( _dTimePoint );
			m_vTimePoints.push_back( _dTimePoint );
			m_Data.push_back( vNewVector );
		}
	}

	m_bCacheCoherent = false;
	CorrectWinBoundary();
	CheckCacheNeed();
}

void CDenseDistr2D::RemoveTimePoint( double _dTimePoint )
{
	if( m_vTimePoints.empty() ) return;
	size_t nIndex = GetIndexByTime( _dTimePoint );
	if( nIndex < m_vTimePoints.size() )
	{
		if( m_vTimePoints[ nIndex ] == _dTimePoint )
		{
			UnCacheData( _dTimePoint );
			m_vTimePoints.erase( m_vTimePoints.begin() + nIndex );
			m_Data.erase( m_Data.begin() + (nIndex - m_nCurrOffset) );
		}
	}
	m_bCacheCoherent = false;
	CorrectWinBoundary();
}

void CDenseDistr2D::RemoveTimePoints( double _dStart, double _dEnd )
{
	if( ( !m_vTimePoints.empty() ) && ( m_vTimePoints.front() == _dStart ) && ( m_vTimePoints.back() == _dEnd ) ) // remove all time points
	{
		RemoveAllTimePoints();
		return;
	}

	RemoveTimePoints( GetIndexesForInterval( _dStart, _dEnd ) );
}

void CDenseDistr2D::RemoveTimePoints(const std::vector<unsigned>& _vIndexes)
{
	if( ( !_vIndexes.empty() ) && ( !m_vTimePoints.empty() ) && ( m_vTimePoints.front() == m_vTimePoints[_vIndexes.front()] )
		&& ( m_vTimePoints.back() == m_vTimePoints[_vIndexes.back()] ) ) // remove all time points
	{
		RemoveAllTimePoints();
		return;
	}

	for ( unsigned i=0; i<_vIndexes.size(); i++ )
	{
		unsigned nIndex = _vIndexes[ _vIndexes.size() - i - 1 ];
		if ( nIndex < m_vTimePoints.size() )
		{
			UnCacheData( m_vTimePoints[nIndex] );
			m_Data.erase( m_Data.begin() + (nIndex - m_nCurrOffset) );
			m_vTimePoints.erase( m_vTimePoints.begin() + nIndex );
		}
	}
	m_bCacheCoherent = false;
	CorrectWinBoundary();
}

void CDenseDistr2D::RemoveAllDataAfter(double _dStartTime, bool _bIncludeStartTime)
{
	if (m_vTimePoints.empty()) return;

	size_t iStart = GetIndexByTime(_dStartTime);
	if (!_bIncludeStartTime && iStart < m_vTimePoints.size() && m_vTimePoints[iStart] == _dStartTime)
		iStart++;

	if (iStart >= m_vTimePoints.size())
		return;

	if (m_vTimePoints[iStart] == m_vTimePoints.front()) // delete all data
	{
		RemoveAllTimePoints();
	}
	else
	{
		UnCacheData(m_vTimePoints[iStart], m_vTimePoints.back());

		m_Data.erase(m_Data.begin() + (iStart - m_nCurrOffset), m_Data.end());
		m_vTimePoints.erase(m_vTimePoints.begin() + iStart, m_vTimePoints.end());

		m_bCacheCoherent = false;
		CorrectWinBoundary();
	}
}

void CDenseDistr2D::ChangeTimePoint( unsigned _nTimePointIndex, double _dNewValue )
{
	if ( _nTimePointIndex >= m_vTimePoints.size() ) return;
	if( _dNewValue < m_dCurrWinStart )
		UnCacheData( _dNewValue, m_dCurrWinStart );
	else if( _dNewValue > m_dCurrWinEnd )
		UnCacheData( m_dCurrWinEnd, _dNewValue );
	m_vTimePoints[ _nTimePointIndex ] = _dNewValue;
	if( m_dCurrWinStart == m_vTimePoints[ _nTimePointIndex ] )
		m_dCurrWinStart = _dNewValue;
	else if( m_dCurrWinEnd == m_vTimePoints[ _nTimePointIndex ] )
		m_dCurrWinEnd = _dNewValue;
	//m_nLastTimePos = _nTimePointIndex;
	m_bCacheCoherent = false;
}

void CDenseDistr2D::RemoveAllTimePoints()
{
	if( m_vTimePoints.empty() ) return;

	m_vTimePoints.clear();
	m_Data.clear();
	ClearCache();
}

size_t CDenseDistr2D::GetTimePointsNumber() const
{
	return m_vTimePoints.size();
}

std::vector<double> CDenseDistr2D::GetAllTimePoints() const
{
	return m_vTimePoints;
}

bool CDenseDistr2D::CopyFrom( CDenseDistr2D* _pSource, double _dStartTime, double _dEndTime )
{
	if ( m_nDimensions != _pSource->GetDimensionsNumber() ) // the number of dimensions does not corresponds to each other
		return false;

	std::vector<unsigned> vIndexes = _pSource->GetIndexesForInterval( _dStartTime, _dEndTime );
	for ( unsigned i=0; i < vIndexes.size(); i++ )
		this->SetValue( _pSource->GetTimeForIndex( vIndexes[i] ), _pSource->GetValueForIndex( vIndexes[i] ) );

	return true;
}

bool CDenseDistr2D::CopyFrom( CDenseDistr2D* _pSource, double _dTime )
{
	if ( m_nDimensions != _pSource->GetDimensionsNumber() ) // the number of dimensions does not corresponds to each other
		return false;

	AddTimePoint( _dTime );
	this->SetValue( _dTime, _pSource->GetValue( _dTime ) );

	return true;
}

bool CDenseDistr2D::CopyFromTimePoint( CDenseDistr2D* _pSource, double _dTimeSrc, double _dTimeDest )
{
	if ( m_nDimensions != _pSource->GetDimensionsNumber() ) // the number of dimensions does not corresponds to each other
		return false;

	AddTimePoint( _dTimeDest );
	this->SetValue( _dTimeDest, _pSource->GetValue( _dTimeSrc ) );

	return true;
}

void CDenseDistr2D::SetValue( unsigned _nTimeIndex, unsigned _nPropIndex, double _dNewValue )
{
	if ( m_vTimePoints.size() <= _nTimeIndex ) return;
	if ( m_nDimensions <= _nPropIndex ) return;
	UnCacheData( m_vTimePoints[_nTimeIndex] );
	m_Data[ _nTimeIndex - m_nCurrOffset ][ _nPropIndex ] = _dNewValue;
	m_bCacheCoherent = false;
	//m_nLastTimePos = _nTimeIndex;
}

void CDenseDistr2D::SetValue( double _dTime, unsigned _nPropIndex, double _dNewValue )
{
	if ( m_nDimensions <= _nPropIndex ) return;

	size_t index = GetIndexByTime( _dTime );
	UnCacheData( _dTime );
	if( index < m_vTimePoints.size() )
	{
		if( m_vTimePoints[index] == _dTime ) // overwrite existing element
			m_Data[ index - m_nCurrOffset ][ _nPropIndex ] = _dNewValue;
		else								// insert new element
		{
			m_Data.insert( m_Data.begin() + (index - m_nCurrOffset), std::vector<double>( m_nDimensions ) );
			if( ( index - m_nCurrOffset < m_Data.size() ) && ( index > 0 ) )
				m_Data[index - m_nCurrOffset] = m_Data[index-1 - m_nCurrOffset];
			m_Data[ index - m_nCurrOffset ][ _nPropIndex ] = _dNewValue;
			m_vTimePoints.insert( m_vTimePoints.begin() + index, _dTime );
		}
	}
	else // just add to the end of the vector
	{
		std::vector<double> vTemp( m_nDimensions );
		if( m_Data.size() != 0 )
			vTemp = m_Data.back();
		vTemp[_nPropIndex] = _dNewValue;
		m_Data.push_back( vTemp );
		m_vTimePoints.push_back( _dTime );
	}
	m_bCacheCoherent = false;
	CorrectWinBoundary();
	CheckCacheNeed();
}

void CDenseDistr2D::SetValue(double _dTime, const std::vector<double>& _newValue)
{
	if( _newValue.size() != m_nDimensions ) // if dimensions number is not corresponded
		return;

	size_t index = GetIndexByTime( _dTime );
	UnCacheData( _dTime );
	if( index < m_vTimePoints.size() )
	{
		if( m_vTimePoints[index] == _dTime ) // overwrite existing element
			m_Data[index - m_nCurrOffset] = _newValue;
		else								// insert new element
		{
			m_Data.insert( m_Data.begin() + (index - m_nCurrOffset), std::vector<double>( m_nDimensions ) );
			m_Data[index - m_nCurrOffset] = _newValue;
			m_vTimePoints.insert( m_vTimePoints.begin() + index, _dTime );
		}
	}
	else // just add to the end of the vector
	{
		m_Data.push_back( _newValue );
		m_vTimePoints.push_back( _dTime );
	}
	m_bCacheCoherent = false;
	CorrectWinBoundary();
	CheckCacheNeed();
}

//double CDenseDistr2D::GetValue( double _dTime, unsigned _nDimension ) const
//{
//	if (( _nDimension > m_nDimensions ) || m_vTimePoints.empty() )
//		return 0;
//
//	UnCacheData( _dTime );
//
//	if ( m_vTimePoints.size() == 1 ) // just one element - no interpolation will be done
//		//if( m_vTimePoints.front() == _dTime )
//		return m_Data[0][_nDimension];
//	//else
//	//return 0;
//
//
//	// find indexes where interpolation should be done
//	int nLeft = (int)m_vTimePoints.size()-1;
//	int nRight = 0;;
//	while ( nRight < (int)m_vTimePoints.size() )
//		if ( m_vTimePoints[ nRight ] >= _dTime )
//		{
//			nLeft = nRight - 1;
//			break;
//		}
//		else
//			nRight++;
//	if(( nLeft >= 0 ) && ( m_vTimePoints[nLeft] == _dTime ))
//		return m_Data[nLeft-m_nCurrOffset][_nDimension];
//	else if(( nRight < (int)m_vTimePoints.size() ) && ( m_vTimePoints[nRight] == _dTime ))
//		return m_Data[nRight-m_nCurrOffset][_nDimension];
//	else if( nRight == m_vTimePoints.size() ) // point is after the last - extrapolation
//	{
//		nRight--;
//		nLeft--;
//		return ( m_Data[nRight-m_nCurrOffset][_nDimension] - m_Data[nLeft-m_nCurrOffset][_nDimension] ) /
//			( m_vTimePoints[nRight] - m_vTimePoints[nLeft] ) *
//			( _dTime - m_vTimePoints[nLeft] ) + m_Data[nLeft-m_nCurrOffset][_nDimension];
//	}
//	else if( nLeft == -1 ) // point at the beginning - extrapolation
//	{
//		nRight++;
//		nLeft++;
//		double timePart = (m_vTimePoints[nRight] - _dTime) / (m_vTimePoints[nLeft] - _dTime);
//		return (timePart * m_Data[nLeft-m_nCurrOffset][_nDimension] - m_Data[nRight-m_nCurrOffset][_nDimension]) / (timePart - 1);
//	}
//	else // point inside - interpolation
//	{
//		return( m_Data[nRight-m_nCurrOffset][_nDimension] - m_Data[nLeft-m_nCurrOffset][_nDimension] ) /
//			( m_vTimePoints[nRight] - m_vTimePoints[nLeft] ) *
//			( _dTime - m_vTimePoints[nLeft] ) + m_Data[nLeft-m_nCurrOffset][_nDimension];
//	}
//}
//
//std::vector<double> CDenseDistr2D::GetValue( double _dTime ) const
//{
//	std::vector<double> vRes;
//
//	if ( m_vTimePoints.empty() )
//		return vRes;
//
//	UnCacheData( _dTime );
//
//	if ( m_vTimePoints.size() == 1 ) // just one element - no interpolation will be done
//		return m_Data[0];
//
//	// find indexes where interpolation should be done
//	int nLeft = (int)m_vTimePoints.size()-1;
//	int nRight = 0;;
//	while ( nRight < (int)m_vTimePoints.size() )
//		if ( m_vTimePoints[ nRight ] >= _dTime )
//		{
//			nLeft = nRight - 1;
//			break;
//		}
//		else
//			nRight++;
//	if(( nLeft >= 0 ) && ( m_vTimePoints[nLeft] == _dTime ))
//		return m_Data[nLeft-m_nCurrOffset];
//	else if(( nRight < (int)m_vTimePoints.size() ) && ( m_vTimePoints[nRight] == _dTime ))
//		return m_Data[nRight-m_nCurrOffset];
//	else if( nRight == m_vTimePoints.size() ) // point is after the last - extrapolation
//	{
//		nRight--;
//		nLeft--;
//		for( unsigned i=0; i<m_nDimensions; ++i )
//			vRes.push_back( ( m_Data[nRight-m_nCurrOffset][i] - m_Data[nLeft-m_nCurrOffset][i] ) /
//			( m_vTimePoints[nRight] - m_vTimePoints[nLeft] ) *
//			( _dTime - m_vTimePoints[nLeft] ) + m_Data[nLeft-m_nCurrOffset][i] );
//	}
//	else if( nLeft == -1 ) // point at the beginning - extrapolation
//	{
//		nRight++;
//		nLeft++;
//		double timePart = (m_vTimePoints[nRight] - _dTime) / (m_vTimePoints[nLeft] - _dTime);
//		for( unsigned i=0; i<m_nDimensions; ++i )
//			vRes.push_back( (timePart * m_Data[nLeft-m_nCurrOffset][i] - m_Data[nRight-m_nCurrOffset][i]) / (timePart - 1) );
//	}
//	else // point inside - interpolation
//	{
//		for( unsigned i=0; i<m_nDimensions; ++i )
//			vRes.push_back(( m_Data[nRight-m_nCurrOffset][i] - m_Data[nLeft-m_nCurrOffset][i] ) /
//			( m_vTimePoints[nRight] - m_vTimePoints[nLeft] ) *
//			( _dTime - m_vTimePoints[nLeft] ) + m_Data[nLeft-m_nCurrOffset][i] );
//	}
//
//	return vRes;
//}

double CDenseDistr2D::GetValue( double _dTime, size_t _nDimension ) const
{
	if (( _nDimension > m_nDimensions ) || m_vTimePoints.empty() )
		return 0;

	UnCacheData( _dTime );

	if ( m_vTimePoints.size() == 1 ) // just one element - no interpolation will be done
		//if( m_vTimePoints.front() == _dTime )
		return m_Data[0][_nDimension];
	//else
	//return 0;

	// find indexes where interpolation should be done
	int nLeft = (int)m_vTimePoints.size()-1;
	int nRight = 0;
	while ( nRight < (int)m_vTimePoints.size() )
		if ( m_vTimePoints[ nRight ] >= _dTime )
		{
			nLeft = nRight - 1;
			break;
		}
		else
			nRight++;
	if(( nLeft >= 0 ) && ( m_vTimePoints[nLeft] == _dTime ))
		return m_Data[nLeft-m_nCurrOffset][_nDimension];
	else if(( nRight < (int)m_vTimePoints.size() ) && ( m_vTimePoints[nRight] == _dTime ))
		return m_Data[nRight-m_nCurrOffset][_nDimension];
	else if( nRight == m_vTimePoints.size() ) // point is after the last - extrapolation
	{
		//nRight--;
		//nLeft--;
		//return ( m_Data[nRight-m_nCurrOffset][_nDimension] - m_Data[nLeft-m_nCurrOffset][_nDimension] ) /
		//	( m_vTimePoints[nRight] - m_vTimePoints[nLeft] ) *
		//	( _dTime - m_vTimePoints[nLeft] ) + m_Data[nLeft-m_nCurrOffset][_nDimension];
		return m_Data.back().at(_nDimension);
	}
	else if( nLeft == -1 ) // point at the beginning - extrapolation
	{
		/*nRight++;
		nLeft++;
		double timePart = (m_vTimePoints[nRight] - _dTime) / (m_vTimePoints[nLeft] - _dTime);
		return (timePart * m_Data[nLeft-m_nCurrOffset][_nDimension] - m_Data[nRight-m_nCurrOffset][_nDimension]) / (timePart - 1);*/
		return m_Data.front().at(_nDimension);
	}
	else // point inside - interpolation
	{
		/*return( m_Data[nRight-m_nCurrOffset][_nDimension] - m_Data[nLeft-m_nCurrOffset][_nDimension] ) /
			( m_vTimePoints[nRight] - m_vTimePoints[nLeft] ) *
			( _dTime - m_vTimePoints[nLeft] ) + m_Data[nLeft-m_nCurrOffset][_nDimension];*/
		//std::vector<double> vY, vX;
		//if( nLeft > 0 )
		//{
		//	UnCacheData( m_vTimePoints[nLeft-1] );
		//	vX.push_back( m_vTimePoints[nLeft-1] );
		//	vY.push_back( m_Data[nLeft-1-m_nCurrOffset][_nDimension] );
		//}
		//UnCacheData( _dTime );
		//vX.push_back( m_vTimePoints[nLeft] );
		//vY.push_back( m_Data[nLeft-m_nCurrOffset][_nDimension] );
		//vX.push_back( m_vTimePoints[nRight] );
		//vY.push_back( m_Data[nRight-m_nCurrOffset][_nDimension] );
		//if( nRight < m_vTimePoints.size() - 1 )
		//{
		//	UnCacheData( m_vTimePoints[nRight+1] );
		//	vX.push_back( m_vTimePoints[nRight+1] );
		//	vY.push_back( m_Data[nRight+1-m_nCurrOffset][_nDimension] );
		//}
		//return Interpolate( vY, vX, _dTime );
		return Interpolate( m_Data[nLeft-m_nCurrOffset][_nDimension], m_Data[nRight-m_nCurrOffset][_nDimension], m_vTimePoints[nLeft], m_vTimePoints[nRight], _dTime );
	}
}

std::vector<double> CDenseDistr2D::GetValues(std::vector<double> _vTimes, unsigned _nDimension) const
{
	if (_nDimension > m_nDimensions) return {};
	std::vector<double> res(_vTimes.size());
	for (size_t i = 0; i < _vTimes.size(); ++i)
		res[i] = GetValue(_vTimes[i], _nDimension);
	return res;
}

std::vector<double> CDenseDistr2D::GetValue(double _dTime) const
{
	std::vector<double> vRes;

	if (m_vTimePoints.empty())
		return std::vector<double>(m_nDimensions, 0);

	UnCacheData(_dTime);

	if (m_vTimePoints.size() == 1) // just one element - no interpolation will be done
		return m_Data[0];

	// find indexes where interpolation should be done
	int nLeft = static_cast<int>(m_vTimePoints.size()) - 1;
	int nRight = 0;
	while (nRight < static_cast<int>(m_vTimePoints.size()))
		if (m_vTimePoints[nRight] >= _dTime)
		{
			nLeft = nRight - 1;
			break;
		}
		else
			nRight++;
	if ((nLeft >= 0) && (m_vTimePoints[nLeft] == _dTime))
		return m_Data[nLeft - m_nCurrOffset];
	else if ((nRight < static_cast<int>(m_vTimePoints.size())) && (m_vTimePoints[nRight] == _dTime))
		return m_Data[nRight - m_nCurrOffset];
	else if (nRight == m_vTimePoints.size()) // point is after the last - extrapolation
		return m_Data.back();
	else if (nLeft == -1) // point at the beginning - extrapolation
		return m_Data.front();
	else // point inside - interpolation
		for (unsigned i = 0; i < m_nDimensions; ++i)
			vRes.push_back(Interpolate(m_Data[nLeft - m_nCurrOffset][i], m_Data[nRight - m_nCurrOffset][i], m_vTimePoints[nLeft], m_vTimePoints[nRight], _dTime));

	return vRes;
}

std::vector<unsigned> CDenseDistr2D::GetIndexesForInterval( double _dStartTime, double _dEndTime )
{
	std::vector<unsigned> vResult;
	for ( unsigned i=0; i < m_vTimePoints.size(); i++ )
		if (( m_vTimePoints[ i ] >= _dStartTime ) && ( m_vTimePoints[ i ] <= _dEndTime ) )
			vResult.push_back( i );
	return vResult;
}

std::vector<double> CDenseDistr2D::GetValueForIndex( unsigned _nIndex ) const
{
	if ( _nIndex >= m_vTimePoints.size() )
		return std::vector<double>( m_nDimensions );
	else
	{
		UnCacheData(m_vTimePoints[_nIndex]);
		return m_Data[ _nIndex - m_nCurrOffset ];
	}
}

std::vector<double> CDenseDistr2D::GetValues(unsigned _nDimension) const
{
	if (_nDimension > m_nDimensions) return {};
	std::vector<double> res(m_vTimePoints.size());
	for (size_t i = 0; i < m_vTimePoints.size(); ++i)
	{
		UnCacheData(m_vTimePoints[i]);
		res[i] = m_Data[i - m_nCurrOffset][_nDimension];
	}
	return res;
}

double CDenseDistr2D::GetTimeForIndex( unsigned _nIndex ) const
{
	if ( _nIndex >= m_vTimePoints.size() )
		return -1;
	else
		return m_vTimePoints[ _nIndex ];
}

void CDenseDistr2D::SaveToFile(CH5Handler& _h5File, const std::string& _sPath)
{
	if (!_h5File.IsValid())	return;

	// current version of save procedure
	_h5File.WriteAttribute(_sPath, StrConst::Distr2D_H5AttrSaveVersion, m_cnSaveVersion);

	/// save dimensions
	_h5File.WriteAttribute(_sPath, StrConst::Distr2D_H5AttrDimsNum, m_nDimensions);

	/// save time points
	_h5File.WriteData(_sPath, StrConst::Distr2D_H5TimePoints, m_vTimePoints);

	/// save data
	if (!m_vTimePoints.empty())
		UnCacheData(m_vTimePoints.front(), m_vTimePoints.back());
	if (!m_Data.empty())
	{
		bool bEqual = true;
		std::vector<std::vector<double>> vFirst;
		vFirst.push_back(m_Data.front());
		for (size_t i = 1; i < m_Data.size(); ++i)
			if (m_Data[i] != vFirst.front())
			{
				bEqual = false;
				break;
			}
		if (bEqual)
			_h5File.WriteData(_sPath, StrConst::Distr2D_H5Data, vFirst);
		else
			_h5File.WriteData(_sPath, StrConst::Distr2D_H5Data, m_Data);
	}

	CheckCacheNeed();
}

void CDenseDistr2D::LoadFromFile(CH5Handler& _h5File, const std::string& _sPath)
{
	Clear();

	if (!_h5File.IsValid())	return;

	/// load dimensions
	m_nDimensions = static_cast<unsigned>(_h5File.ReadAttribute(_sPath, StrConst::Distr2D_H5AttrDimsNum));
	m_vLabels.resize(m_nDimensions);

	/// load time points
	_h5File.ReadData(_sPath, StrConst::Distr2D_H5TimePoints, m_vTimePoints);

	/// load data
	_h5File.ReadData(_sPath, StrConst::Distr2D_H5Data, m_Data);
	if (m_Data.size() == 1 && m_Data.size() != m_vTimePoints.size())
		m_Data.resize(m_vTimePoints.size(), m_Data.front());

	if (!m_vTimePoints.empty())
	{
		m_dCurrWinStart = m_vTimePoints.front();
		m_dCurrWinEnd = m_vTimePoints.back();
	}
	else
	{
		m_dCurrWinStart = 0;
		m_dCurrWinEnd = 0;
	}
	m_nCurrOffset = 0;
	m_bCacheCoherent = false;
	CheckCacheNeed();
}

void CDenseDistr2D::SetDimensionsNumber( unsigned _nNewNumber )
{
	if ( m_nDimensions == _nNewNumber ) // old number was equal
		return;
	if ( _nNewNumber == 0 ) // delete all data
		Clear();

	if ( m_nDimensions > _nNewNumber ) // delete unnecessary entries
		for ( unsigned i=0; i < m_vTimePoints.size(); i++ )
		{
			UnCacheData(m_vTimePoints[i]);
			while ( m_Data[i-m_nCurrOffset].size() > _nNewNumber )
				m_Data[i-m_nCurrOffset].pop_back();
		}
	else // add new entries
		for ( unsigned i=0; i < m_vTimePoints.size(); i++ )
		{
			UnCacheData(m_vTimePoints[i]);
			while ( m_Data[i-m_nCurrOffset].size() < _nNewNumber )
				m_Data[i-m_nCurrOffset].push_back( 0 );
		}
	m_nDimensions = _nNewNumber;
	m_vLabels.resize( _nNewNumber );
	m_bCacheCoherent = false;
}

void CDenseDistr2D::SetDimensionsLabels( std::vector<std::string>& _vLabels )
{
	if( _vLabels.size() != m_nDimensions ) return;

	for( unsigned i=0; i<m_nDimensions; ++i )
		m_vLabels[i] = _vLabels[i];
}

void CDenseDistr2D::SetDimensionLabel( unsigned _nDimension, std::string _label )
{
	if( _nDimension < m_nDimensions )
		m_vLabels[_nDimension] = _label;
}

std::vector<std::string> CDenseDistr2D::GetLabels()
{
	return m_vLabels;
}

std::string CDenseDistr2D::GetLabel( unsigned _nDimension ) const
{
	return _nDimension < m_nDimensions ? m_vLabels[_nDimension] : "";
}

void CDenseDistr2D::AddDimension()
{
	for( unsigned i=0; i<m_vTimePoints.size(); i++ )
	{
		UnCacheData(m_vTimePoints[i]);
		while( m_Data[i-m_nCurrOffset].size() < m_nDimensions+1 )
			m_Data[i-m_nCurrOffset].push_back( 0 );
	}
	m_nDimensions++;
	m_vLabels.resize( m_nDimensions );
	m_bCacheCoherent = false;
}

void CDenseDistr2D::RemoveDimension( unsigned _nIndex )
{
	if ( _nIndex >= m_nDimensions ) return;

	for (unsigned i=0; i <m_vTimePoints.size(); i++ )
	{
		UnCacheData(m_vTimePoints[i]);
		m_Data[ i-m_nCurrOffset ].erase( m_Data[i-m_nCurrOffset].begin() + _nIndex );
	}

	m_vLabels.erase( m_vLabels.begin() + _nIndex );
	m_nDimensions--;
	m_bCacheCoherent = false;
}

void CDenseDistr2D::SetCacheParams( bool _bEnabled, unsigned _nWindow )
{
	m_bCacheEnabled = _bEnabled;
	m_nCacheWindow = _nWindow;
	if( ( _bEnabled ) && ( !m_sCachePath.empty() ) )
	{
		if( m_pCacheHandler == NULL )
		{
			m_pCacheHandler = new CDenseDistrCacher();
			m_pCacheHandler->SetChunk( m_nCacheWindow );
			m_pCacheHandler->SetDirPath( m_sCachePath );
			m_pCacheHandler->Initialize();
		}
		CheckCacheNeed();
	}
	if( m_sCachePath.empty() )
		m_bCacheEnabled = false;
}

void CDenseDistr2D::ExtrapolateToPoint( double _dT1, double _dT2, double _dTExtra )
{
	UnCacheData( _dT1, _dTExtra );

	std::vector<double> vRes( m_nDimensions );

	if( !m_vTimePoints.empty() )
	{
		if( m_vTimePoints.size() == 1 )
		{
			vRes = m_Data[0];
		}
		else
		{
			std::vector<double> v1 = GetValue( _dT1 );
			std::vector<double> v2 = GetValue( _dT2 );
			//double dTemp = ( _dT2 - _dT1 ) / ( _dTExtra - _dT1 );
			//for( unsigned i=0; i<m_nDimensions; ++i )
			//	vRes[i] = ( v2[i] - v1[i] ) / dTemp + v1[i];
			for( unsigned i=0; i<m_nDimensions; ++i )
			{
				//std::vector<double> vY, vX;
				//vX.push_back( _dT1 );
				//vY.push_back( v1[i] );
				//vX.push_back( _dT2 );
				//vY.push_back( v2[i] );
				//vRes[i] = Interpolate( vY, vX, _dTExtra );
				vRes[i] = Interpolate( v1[i], v2[i], _dT1, _dT2, _dTExtra );
			}
		}
	}

	SetValue( _dTExtra, vRes );

	CheckCacheNeed();
}

void CDenseDistr2D::ExtrapolateToPoint( double _dT0, double _dT1, double _dT2, double _dTExtra )
{
	UnCacheData( _dT0, _dTExtra );

	std::vector<double> vNewVal( m_nDimensions );

	if( !m_vTimePoints.empty() )
	{
		if( m_vTimePoints.size() > 2 )
		{
			std::vector<double> v0 = GetValue( _dT0 );
			std::vector<double> v1 = GetValue( _dT1 );
			std::vector<double> v2 = GetValue( _dT2 );
			for( unsigned i=0; i<m_nDimensions; ++i )
				vNewVal[i] = Extrapolate( v0[i], v1[i], v2[i], _dT0, _dT1, _dT2, _dTExtra );
		}
		else if( m_vTimePoints.size() == 2 )
			ExtrapolateToPoint( _dT0, _dT2, _dTExtra );
		else
			vNewVal = m_Data[0];
	}

	SetValue( _dTExtra, vNewVal );

	CheckCacheNeed();
}

void CDenseDistr2D::UnCacheData(double _dTP) const
{
	if( !m_bCacheEnabled ) return;

	if( ( m_vTimePoints.size() > 0 ) && ( m_vTimePoints.back() == m_dCurrWinEnd ) && ( _dTP > m_vTimePoints.back() ) )
	{
		m_dCurrWinEnd = m_vTimePoints[m_vTimePoints.size()-1];
		return;
	}
	//if( ( _dTP <= m_dCurrWinStart ) || ( _dTP >= m_dCurrWinEnd ) )
	if( ( _dTP < m_dCurrWinStart ) || ( _dTP > m_dCurrWinEnd ) )
	//if( ( ( _dTP <= m_dCurrWinStart ) && ( m_nCurrOffset != 0 ) ) || ( _dTP >= m_dCurrWinEnd ) )
	{
		FlushToCache();
		//m_CacheHandler.GetData(_dTP, m_Data, m_dCurrWinStart, m_dCurrWinEnd, m_nCurrOffset);
		m_pCacheHandler->ReadFromCache(_dTP, m_Data, m_dCurrWinStart, m_dCurrWinEnd, m_nCurrOffset);
		m_bCacheCoherent = true;
	}
}

void CDenseDistr2D::UnCacheData(double _dT1, double _dT2) const
{
	if( !m_bCacheEnabled ) return;

	if( ( m_vTimePoints.size() > 0 ) && ( m_vTimePoints.back() == m_dCurrWinEnd ) && ( _dT1 > m_dCurrWinStart ) && ( _dT2 > m_vTimePoints.back() ) )
	{
		m_dCurrWinEnd = m_vTimePoints[m_vTimePoints.size()-1];
		return;
	}
	//if( ( _dT1 <= m_dCurrWinStart ) || ( _dT2 >= m_dCurrWinEnd ) )
	if( ( _dT1 < m_dCurrWinStart ) || ( _dT2 > m_dCurrWinEnd ) )
	//if( ( ( _dT1 <= m_dCurrWinStart ) && ( m_nCurrOffset != 0 ) ) || ( _dT2 >= m_dCurrWinEnd ) )
	{
		FlushToCache();
		//m_CacheHandler.GetData(_dT1, _dT2, m_Data, m_dCurrWinStart, m_dCurrWinEnd, m_nCurrOffset);
		m_pCacheHandler->ReadFromCache(_dT1, _dT2, m_Data, m_dCurrWinStart, m_dCurrWinEnd, m_nCurrOffset);
		m_bCacheCoherent = true;
	}
}

void CDenseDistr2D::CheckCacheNeed()
{
	if( !m_bCacheEnabled ) return;

	while( m_Data.size() > m_nCacheWindow*2 )
		CacheData();
}

void CDenseDistr2D::FlushToCache() const
{
	if( !m_bCacheEnabled ) return;

	//m_CacheHandler.SaveData(m_Data, m_vTimePoints, m_nCurrOffset, m_Data.size() );
	if( m_nCurrOffset >= m_vTimePoints.size() ) return;
	m_pCacheHandler->WriteToCache(m_Data, m_vTimePoints, m_nCurrOffset, m_Data.size(), m_bCacheCoherent );
	m_nCurrOffset = m_vTimePoints.size();
	m_dCurrWinStart = m_dCurrWinEnd = 0;
	std::vector<std::vector<double>>(m_Data).swap(m_Data);
}

void CDenseDistr2D::CacheData()
{
	if( !m_bCacheEnabled ) return;

	//m_CacheHandler.SaveData( m_Data, m_vTimePoints, m_nCurrOffset, m_nCacheWindow );
	m_pCacheHandler->WriteToCache( m_Data, m_vTimePoints, m_nCurrOffset, m_nCacheWindow, m_bCacheCoherent );
	m_nCurrOffset += m_nCacheWindow;
	m_dCurrWinStart = m_vTimePoints[m_nCurrOffset];
	//m_dCurrWinEnd = m_vTimePoints[m_nCurrOffset+m_nCacheWindow];
	std::vector<std::vector<double>>(m_Data).swap(m_Data);
}

void CDenseDistr2D::CorrectWinBoundary()
{
	if( !m_bCacheEnabled ) return;

	if( ( m_vTimePoints.size() > 1 ) && ( m_dCurrWinEnd == m_vTimePoints[m_vTimePoints.size()-2] ) )
		m_dCurrWinEnd = m_vTimePoints.back();

	if( !m_vTimePoints.empty() )
	{
		/*if( m_dCurrWinEnd > m_vTimePoints[ m_nCurrOffset + m_Data.size() - 1 ] )
			m_dCurrWinEnd = m_vTimePoints[ m_nCurrOffset + m_Data.size() - 1 ];
		if( m_dCurrWinStart < m_vTimePoints[ m_nCurrOffset ] )
			m_dCurrWinStart = m_vTimePoints[ m_nCurrOffset ];*/

		if( m_nCurrOffset < m_vTimePoints.size() )
		{
			if( m_dCurrWinEnd > m_vTimePoints[ m_nCurrOffset + m_Data.size() - 1 ] )
				m_dCurrWinEnd = m_vTimePoints[ m_nCurrOffset + m_Data.size() - 1 ];
			if( m_dCurrWinStart < m_vTimePoints[ m_nCurrOffset ] )
				m_dCurrWinStart = m_vTimePoints[ m_nCurrOffset ];
		}
		else
		{
			m_dCurrWinStart = m_dCurrWinEnd = 0;
		}
	}
	else
	{
		m_dCurrWinStart = 0;
		m_dCurrWinEnd = 0;
		m_nCurrOffset = 0;
	}
}

void CDenseDistr2D::ClearCache() const
{
	if( !m_bCacheEnabled ) return;

	if( m_pCacheHandler )
		m_pCacheHandler->ClearData();
	m_dCurrWinStart = 0;
	m_dCurrWinEnd = 0;
	m_nCurrOffset = 0;
	m_bCacheCoherent = false;
}

void CDenseDistr2D::SetCachePath(const std::wstring& _sPath)
{
	m_sCachePath = _sPath;

	if( m_pCacheHandler )
		m_pCacheHandler->SetDirPath( m_sCachePath );
}

unsigned CDenseDistr2D::GetDimensionsNumber() const
{
	return m_nDimensions;
}

size_t CDenseDistr2D::GetIndexByTime(double _dTime)
{
	// binary search
	size_t nFirst = 0;
	size_t nLast = m_vTimePoints.size();
	size_t nMid = (size_t)(nFirst + nLast) >> 1;
	while( nFirst < nLast )
	{
		if ( _dTime <= m_vTimePoints[nMid] )
			nLast = nMid;
		else
			nFirst = nMid + 1;
		nMid = (size_t)(nFirst + nLast) >> 1;
	}
	return nLast;
}
