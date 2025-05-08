/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "MDMatrix.h"
#include "ContainerFunctions.h"
#include "DyssolStringConstants.h"
#include "H5Handler.h"

#include <cmath>

CMDMatrix::CMDMatrix(const CMDMatrix& _other) :
	m_vDimensions{ _other.m_vDimensions },
	m_vClasses{ _other.m_vClasses },
	m_dMinFraction{ _other.m_dMinFraction },
	m_sCachePath{ _other.m_sCachePath },
	m_bCacheEnabled{ _other.m_bCacheEnabled },
	m_nCacheWindow{ _other.m_nCacheWindow }
{
	SetCachePath(_other.m_sCachePath);
	SetCacheParams(_other.m_bCacheEnabled, _other.m_nCacheWindow);
	if (!_other.m_vTimePoints.empty())
	{
		CopyFrom(_other, _other.m_vTimePoints.front(), _other.m_vTimePoints.back());
		CheckCacheNeed();
	}
}

CMDMatrix::~CMDMatrix()
{
	Clear();
}

void CMDMatrix::Clear()
{
	RemoveAllTimePoints();
	m_vDimensions.clear();
	m_vClasses.clear();
	ClearCache();
}

void CMDMatrix::AddDimension(unsigned _nDim, unsigned _nClasses)
{
	for( unsigned i=0; i<m_vDimensions.size(); ++i )
		if( m_vDimensions[i] == _nDim ) // dimension already exists
			return;

	if( !m_vTimePoints.empty() )
	{
		UnCacheData( m_vTimePoints.front(), m_vTimePoints.back() );
		ClearCache();
	}
	m_vDimensions.push_back(_nDim);
	m_vClasses.push_back(_nClasses);
	m_bCacheCoherent = false;
	CheckCacheNeed();
}

void CMDMatrix::DeleteDimension(unsigned _nDim)
{
	std::vector<unsigned> vDims;
	vDims.push_back( _nDim );
	DeleteDimensions( vDims );
}

void CMDMatrix::DeleteDimensions(const std::vector<unsigned>& _vDims)
{
	std::vector<unsigned> vDimsToKeep;
	std::vector<unsigned> vClassesToKeep;
	m_pSortMatr = new CMDMatrix();

	DeleteDimsWithSort( _vDims, vDimsToKeep, vClassesToKeep, *m_pSortMatr );

	SetDimensions( vDimsToKeep, vClassesToKeep );
	CopyFractionsRecursive( m_pSortMatr->m_data );
	delete m_pSortMatr;
	m_pSortMatr = nullptr;
}

std::vector<unsigned> CMDMatrix::GetDimensions() const
{
	return m_vDimensions;
}

std::vector<unsigned> CMDMatrix::GetClasses() const
{
	return m_vClasses;
}

void CMDMatrix::SetDimension(unsigned _nDim, unsigned _nClasses)
{
	if( !m_vDimensions.empty() )
		Clear();
	m_vDimensions.push_back( _nDim );
	m_vClasses.push_back( _nClasses );
}

void CMDMatrix::SetDimensions(const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vClasses)
{
	if( _vDims.size() != _vClasses.size() ) // wrong input data
		return;

	if( _vDims.size() == 0 ) // empty dimensions
		return;

	if( !CheckDuplicates( _vDims ) ) // repeating dimensions
		return;

	if( !m_vDimensions.empty() )
		Clear();

	m_vDimensions = _vDims;
	m_vClasses = _vClasses;
}

size_t CMDMatrix::GetDimensionsNumber() const
{
	return m_vDimensions.size();
}

int CMDMatrix::GetDimensionTypeByIndex(unsigned _nIndex) const
{
	if ( _nIndex >= m_vDimensions.size() )
		return -1;

	return m_vDimensions[_nIndex];
}

int CMDMatrix::GetDimensionSizeByIndex(unsigned _nIndex) const
{
	if ( _nIndex >= m_vClasses.size() )
		return -1;

	return m_vClasses[_nIndex];
}

unsigned CMDMatrix::GetDimensionSizeByType(unsigned _nDim) const
{
	for( unsigned i=0; i<m_vDimensions.size(); ++i )
		if( m_vDimensions[i] == _nDim )
			return m_vClasses[i];
	return 0;
}

void CMDMatrix::UpdateDimensions(const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vClasses)
{
	if( _vDims.size() != _vClasses.size() )
		return;

	//bool bLastCacheState = m_bCacheEnabled;
	//if( !m_vTimePoints.empty() )
	//	UnCacheData( m_vTimePoints.front(), m_vTimePoints.back() );
	//m_bCacheEnabled = false;

	if( m_vDimensions.size() == 0 ) // no dimensions at all
		SetDimensions( _vDims, _vClasses );
	else
	{
		//// check current
		if( m_vDimensions.size() == _vDims.size() )
		{
			unsigned cnt = 0;
			for( unsigned i=0; i<m_vDimensions.size(); ++i )
				for( unsigned j=0; j<_vDims.size(); ++j )
					if( ( m_vDimensions[i] == _vDims[j] ) && ( m_vClasses[i] == _vClasses[j] ))
						cnt++;
			if( cnt == m_vDimensions.size() ) // nothing to change
			{
				//m_bCacheEnabled = bLastCacheState;
				//CheckCacheNeed();
				return;
			}
		}

		//// delete dimensions

		// get types to delete
		std::vector<unsigned> vDimsToDelete;
		for( unsigned i=0; i<m_vDimensions.size(); ++i )
		{
			unsigned j=0;
			for( j=0; j<_vDims.size(); ++j )
				if( _vDims[j] == m_vDimensions[i] )
					break;
			if( j == _vDims.size() )
				vDimsToDelete.push_back( m_vDimensions[i] );
		}

		// delete dimensions
		std::vector<unsigned> vDimsToKeep;
		std::vector<unsigned> vClassesToKeep;
		m_pSortMatr = new CMDMatrix();
		DeleteDimsWithSort( vDimsToDelete, vDimsToKeep, vClassesToKeep, *m_pSortMatr );
		// find new number of classes
		for( unsigned i=0; i<vDimsToKeep.size(); ++i )
			for( unsigned j=0; j<_vDims.size(); ++j )
				if( vDimsToKeep[i] == _vDims[j] )
					vClassesToKeep[i] = _vClasses[j];
		// set new dimensions
		SetDimensions( vDimsToKeep, vClassesToKeep );

		//// rewrite data

		for( unsigned j=0; j<m_pSortMatr->m_vTimePoints.size(); ++j )
			AddTimePoint( m_pSortMatr->m_vTimePoints[j] );
		std::vector<unsigned> vCoords;
		std::vector<unsigned> vDyms;
		std::vector<unsigned> vClasses;
		for( unsigned i=0; i<m_vDimensions.size(); ++i ) // for each dimension
		{
			vDyms.push_back( m_vDimensions[i] );
			vClasses.push_back( m_vClasses[i] );

			bool bRes;
			do
			{
				for( unsigned j=0; j<m_pSortMatr->m_vTimePoints.size(); ++j )
				{
					double dTime = m_pSortMatr->m_vTimePoints[j];
					std::vector<double> vVal;
					if( m_pSortMatr->GetVectorValue( dTime, vDyms, vCoords, vVal ) )
					{
						if( m_vClasses[i] > m_pSortMatr->m_vClasses[i] ) // add zeros
						{
							std::vector<double> vAddVal( m_vClasses[i] - m_pSortMatr->m_vClasses[i], 0 );
							vVal.insert( vVal.end(), vAddVal.begin(), vAddVal.end() );
						}
						else if( m_vClasses[i] < m_pSortMatr->m_vClasses[i] ) // remove excess
						{
							vVal.erase( vVal.begin(), vVal.begin() + m_pSortMatr->m_vClasses[i] - m_vClasses[i] );
						}

						/* bool setRes =  */SetVectorValue( dTime, vDyms, vCoords, vVal );
					}
				}

				bRes = IncrementCoords( vCoords, vClasses );
			}
			while( bRes );

			// go to next dimension
			std::fill( vCoords.begin(), vCoords.end(), 0 );
			vCoords.push_back( 0 );
		}

		//// add dimensions
		for( unsigned index=0; index<_vDims.size(); ++index )
		{
			unsigned j;
			for( j=0; j<vDimsToKeep.size(); ++j  )
				if( _vDims[index] == vDimsToKeep[j] )
					break;
			if( j == vDimsToKeep.size() )
			{
				AddDimension( _vDims[index], _vClasses[index] );
				std::vector<double> vNewVals( _vClasses[index], 1./_vClasses[index] );
				for( unsigned k=0; k<m_vTimePoints.size(); ++k )
					SetVectorValue( m_vTimePoints[k], _vDims[index], vNewVals );
			}
		}
	}

	//m_bCacheEnabled = bLastCacheState;
	//CheckCacheNeed();
	if( m_pSortMatr )
		m_pSortMatr->Clear();
	else
		m_pSortMatr = new CMDMatrix();

	m_pSortMatr->SetDimensions( m_vDimensions, m_vClasses );
	m_pSortMatr->m_data = CopyFractionsRecursive( m_data );
	m_pSortMatr->m_vTimePoints = m_vTimePoints;
	Clear();
	SetDimensions( _vDims, _vClasses );
	m_pSortMatr->SortMatrix( *this );
	m_vTimePoints = m_pSortMatr->m_vTimePoints;
	delete m_pSortMatr;
	m_pSortMatr = nullptr;
}

void CMDMatrix::AddClass(unsigned _nDim)
{
	for( unsigned i=0; i<m_vDimensions.size(); ++i )
	{
		if( m_vDimensions[i] == _nDim )
		{
			if( !m_vTimePoints.empty() )
				UnCacheData(m_vTimePoints.front(), m_vTimePoints.back());
			m_data = AddClassRecursive( m_data, i );
			m_vClasses[i]++;
			break;
		}
	}
	m_bCacheCoherent = false;
	CheckCacheNeed();
}

void CMDMatrix::RemoveClass(unsigned _nDim, size_t _nClassIndex)
{
	const size_t iDim = VectorFind(m_vDimensions, _nDim);
	if (iDim == static_cast<size_t>(-1)) return;
	if (_nClassIndex >= m_vClasses[iDim]) return;
	for( unsigned i=0; i<m_vDimensions.size(); ++i )
	{
		if( m_vDimensions[i] == _nDim )
		{
			if( !m_vTimePoints.empty() )
				UnCacheData(m_vTimePoints.front(), m_vTimePoints.back());
			m_data = RemoveClassRecursive( m_data, i, _nClassIndex );
			m_vClasses[i]--;
			break;
		}
	}
	m_bCacheCoherent = false;
	CheckCacheNeed();
}

bool CMDMatrix::CompareDims(const CMDMatrix& _matr) const
{
	if ( m_vDimensions.size() != _matr.m_vDimensions.size() ) // the number of dimensions does not corresponds to each other
		return false;

	for( unsigned i=0; i<m_vDimensions.size(); ++i )
		if( ( m_vDimensions[i] != _matr.m_vDimensions[i] ) || ( m_vClasses[i] != _matr.m_vClasses[i] ) ) // wrong dimensions
			return false;

	return true;
}

void CMDMatrix::AddTimePoint(double _dTime, double _dSrcTimePoint /*= -1 */)
{
	unsigned index = GetTimeIndex( _dTime, false ); // get new index to insert
	if( (unsigned)index < m_vTimePoints.size() )
		if( m_vTimePoints[index] == _dTime ) // time point already exists
			return;

	if( _dSrcTimePoint != -1 )
	{
		if( _dTime < _dSrcTimePoint )
			UnCacheData(_dTime, _dSrcTimePoint);
		else
			UnCacheData(_dSrcTimePoint,_dTime);
	}
	else if( index > 0 )
		UnCacheData( m_vTimePoints[index-1], _dTime );
	else if( ( index == 0 ) && ( index < (int)m_vTimePoints.size() ) )
		UnCacheData( m_vTimePoints[index], _dTime );
	m_dTempT1 = _dTime;
	m_dTempT2 = _dSrcTimePoint;
	AddTimePointRecursive( m_data );

	m_vTimePoints.insert( m_vTimePoints.begin() + index, _dTime );

	m_nNonCachedTPNum++;
	m_bCacheCoherent = false;
	CorrectWinBoundary();
	CheckCacheNeed();
}

void CMDMatrix::ChangeTimePoint(unsigned _nTimePointIndex, double _dNewTime)
{
	if ( _nTimePointIndex >= m_vTimePoints.size() )
		return;

	if( m_vTimePoints.size() > 1 )
	{
		if( ( _nTimePointIndex != 0 ) && ( m_vTimePoints[_nTimePointIndex-1] >= _dNewTime ) ) // new time value is less than the previous time point
			return;
		if( ( _nTimePointIndex != m_vTimePoints.size()-1 ) && ( m_vTimePoints[_nTimePointIndex+1] <= _dNewTime ) ) // new time value is bigger than the next time point
			return;
	}

	m_dTempT1 = GetTimeForIndex( _nTimePointIndex );
	UnCacheData(m_dTempT1);
	if( _dNewTime < m_dCurrWinStart )
		UnCacheData( _dNewTime, m_dCurrWinStart );
	else if( _dNewTime > m_dCurrWinEnd )
		UnCacheData( m_dCurrWinEnd, _dNewTime );
	m_dTempT2 = _dNewTime;
	ChangeTimePointRecursive( m_data );
	m_vTimePoints[_nTimePointIndex] = _dNewTime;
	if( m_dCurrWinStart == m_vTimePoints[ _nTimePointIndex ] )
		m_dCurrWinStart = _dNewTime;
	else if( m_dCurrWinEnd == m_vTimePoints[ _nTimePointIndex ] )
		m_dCurrWinEnd = _dNewTime;

	m_bCacheCoherent = false;
}

void CMDMatrix::RemoveTimePoint(double _dTime)
{
	unsigned index = GetTimeIndex( _dTime );
	if( index == -1 ) // no such time point
		return;

	UnCacheData(_dTime);
	m_dTempT1 = _dTime;
	m_dTempT2 = -1;
	m_data = RemoveTimePointsRecursive( m_data );
	m_vTimePoints.erase( m_vTimePoints.begin() + index );
	m_nNonCachedTPNum--;

	if( m_vTimePoints.empty() )
		m_data = RemoveFractionsRecursive( m_data );
	CorrectWinBoundary();

	m_bCacheCoherent = false;
}

void CMDMatrix::RemoveTimePoints(double _dStart, double _dEnd, bool _inclusive/* = true*/)
{
	if( _dStart > _dEnd ) // wrong interval
		return;

	if( m_vTimePoints.empty() ) // nothing to remove
		return;

	if( ( m_vTimePoints.front() == _dStart ) && ( m_vTimePoints.back() == _dEnd ) && _inclusive) // remove all time points
	{
		RemoveAllTimePoints();
		return;
	}

	unsigned iLast = GetTimeIndex( _dEnd, false );
	unsigned iFirst = GetTimeIndex( _dStart, false );

	if( ( iLast >= m_vTimePoints.size() ) || ( m_vTimePoints[iLast] != _dEnd ) )
		iLast--;

	if (!_inclusive)
	{
		if (iFirst < m_vTimePoints.size() - 1 && m_vTimePoints[iFirst] == _dStart)		iFirst++;
		if (iLast < m_vTimePoints.size() && iLast > 0 && m_vTimePoints[iLast] == _dEnd)	iLast--;
		if (m_vTimePoints[iFirst] > m_vTimePoints[iLast]) return;
		_dStart = m_vTimePoints[iFirst];
		_dEnd = m_vTimePoints[iLast];
	}

	UnCacheData(_dStart,_dEnd);
	m_dTempT1 = _dStart;
	m_dTempT2 = _dEnd;
	m_data = RemoveTimePointsRecursive( m_data );
	m_vTimePoints.erase( m_vTimePoints.begin() + iFirst, m_vTimePoints.begin() + iLast + 1 );
	m_nNonCachedTPNum -= ( iLast - iFirst + 1 );

	if( m_vTimePoints.empty() )
		m_data = RemoveFractionsRecursive( m_data );
	CorrectWinBoundary();

	m_bCacheCoherent = false;
}

void CMDMatrix::RemoveTimePointsAfter(double _dTime, bool _bIncludeTime /*= false */)
{
	if( m_vTimePoints.empty() ) // nothing to remove
		return;

	unsigned i=0;
	if( _bIncludeTime )
		while( i < m_vTimePoints.size() )
			if( m_vTimePoints[i] >= _dTime )
				break;
			else
				i++;
	else
		while( i < m_vTimePoints.size() )
			if( m_vTimePoints[i] > _dTime )
				break;
			else
				i++;

	if( i < m_vTimePoints.size() )
		RemoveTimePoints( m_vTimePoints[i], m_vTimePoints.back() );
}

void CMDMatrix::RemoveAllTimePoints()
{
	if( !m_vTimePoints.empty() )
	{
		m_dTempT1 = m_vTimePoints.front();
		m_dTempT2 = m_vTimePoints.back();
		m_data = RemoveTimePointsRecursive( m_data );
		m_vTimePoints.clear();
	}

	m_data = RemoveFractionsRecursive( m_data );
	ClearCache();
}

std::vector<double> CMDMatrix::GetTimePoints(double _dStart, double _dEnd) const
{
	std::vector<double> vTimePoints;

	if( m_vTimePoints.empty() ) // nothing to return
		return vTimePoints;

	unsigned iFirst = GetTimeIndex( _dStart, false );
	unsigned iLast = GetTimeIndex( _dEnd, false );

	if(iLast && ( iLast >= m_vTimePoints.size()  ||  m_vTimePoints[iLast] != _dEnd ) ) // correction
		iLast--;

	for(size_t i=iFirst; i<=iLast; ++i )
		vTimePoints.push_back( m_vTimePoints[i] );

	return vTimePoints;
}

std::vector<double> CMDMatrix::GetAllTimePoints() const
{
	return m_vTimePoints;
}

double CMDMatrix::GetTimeForIndex(unsigned _nIndex) const
{
	if ( _nIndex >= m_vTimePoints.size() )
		return -1;

	return m_vTimePoints[_nIndex];
}

size_t CMDMatrix::GetTimePointsNumber() const
{
	return m_vTimePoints.size();
}

double CMDMatrix::GetMinimalFraction() const
{
	return m_dMinFraction;
}

void CMDMatrix::SetMinimalFraction(double _dValue)	// TODO: compress according to new min fraction
{
	if( _dValue < 0 )
		m_dMinFraction = 0;
	else
		m_dMinFraction = _dValue;
}

double CMDMatrix::GetValue(size_t _nTimeIndex, unsigned _nDim, unsigned _nCoord) const
{
	if( _nTimeIndex >= m_vTimePoints.size() )
		return 0;

	std::vector<unsigned> vDims(1);
	vDims[0] = _nDim;
	std::vector<unsigned> vCoords(1);
	vCoords[0] = _nCoord;
	return GetValue( m_vTimePoints[_nTimeIndex], vDims, vCoords );
}

double CMDMatrix::GetValue(double _dTime, unsigned _nDim, unsigned _nCoord) const
{
	std::vector<unsigned> vDims(1);
	vDims[0] = _nDim;
	std::vector<unsigned> vCoords(1);
	vCoords[0] = _nCoord;
	return GetValue( _dTime, vDims, vCoords );
}

double CMDMatrix::GetValue(double _dTime, unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2) const
{
	std::vector<unsigned> vDims(2);
	vDims[0] = _nDim1;
	vDims[1] = _nDim2;
	std::vector<unsigned> vCoords(2);
	vCoords[0] = _nCoord1;
	vCoords[1] = _nCoord2;
	return GetValue( _dTime, vDims, vCoords );
}

double CMDMatrix::GetValue(double _dTime, unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nDim3, unsigned _nCoord3) const
{
	std::vector<unsigned> vDims(3);
	vDims[0] = _nDim1;
	vDims[1] = _nDim2;
	vDims[2] = _nDim3;
	std::vector<unsigned> vCoords(3);
	vCoords[0] = _nCoord1;
	vCoords[1] = _nCoord2;
	vCoords[2] = _nCoord3;
	return GetValue( _dTime, vDims, vCoords );
}

double CMDMatrix::GetValue(double _dTime, const std::vector<unsigned>& _vCoords) const
{
	return GetValue( _dTime, m_vDimensions, _vCoords );
}

double CMDMatrix::GetValue(double _dTime, const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords) const
{
	if( m_vTimePoints.empty() )
		return 0;

	if( _vDims.size() > m_vDimensions.size() ) // _vDims has wrong size
		return 0;

	if( _vDims.size() != _vCoords.size() ) // _vCoords has wrong size
		return 0;

	UnCacheData(_dTime);
	m_dTempT1 = _dTime;
	m_vTempDims = _vDims;
	m_vTempCoords = _vCoords;

	double dValue = GetValueRecursive( m_data );
	return dValue >= m_dMinFraction ? dValue : 0;
}

bool CMDMatrix::SetValue(unsigned _nTimeIndex, unsigned _nDim, unsigned _nCoord, double _dValue, bool _bExternal /*= true*/)
{
	if( _nTimeIndex >= m_vTimePoints.size() )
		return false;

	std::vector<unsigned> vDims(1);
	vDims[0] = _nDim;
	std::vector<unsigned> vCoords(1);
	vCoords[0] = _nCoord;
	return SetValue( m_vTimePoints[_nTimeIndex], vDims, vCoords, _dValue, _bExternal );
}

bool CMDMatrix::SetValue(double _dTime, unsigned _nDim, unsigned _nCoord, double _dValue, bool _bExternal /*= true*/)
{
	std::vector<unsigned> vDims(1);
	vDims[0] = _nDim;
	std::vector<unsigned> vCoords(1);
	vCoords[0] = _nCoord;
	return SetValue( _dTime, vDims, vCoords, _dValue, _bExternal );
}

bool CMDMatrix::SetValue(double _dTime, unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, double _dValue, bool _bExternal /*= true*/)
{
	std::vector<unsigned> vDims(2);
	vDims[0] = _nDim1;
	vDims[1] = _nDim2;
	std::vector<unsigned> vCoords(2);
	vCoords[0] = _nCoord1;
	vCoords[1] = _nCoord2;
	return SetValue( _dTime, vDims, vCoords, _dValue, _bExternal );
}

bool CMDMatrix::SetValue(double _dTime, unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nDim3, unsigned _nCoord3, double _dValue, bool _bExternal /*= true*/)
{
	std::vector<unsigned> vDims(3);
	vDims[0] = _nDim1;
	vDims[1] = _nDim2;
	vDims[2] = _nDim3;
	std::vector<unsigned> vCoords(3);
	vCoords[0] = _nCoord1;
	vCoords[1] = _nCoord2;
	vCoords[2] = _nCoord3;
	return SetValue( _dTime, vDims, vCoords, _dValue, _bExternal );
}

bool CMDMatrix::SetValue( double _dTime, const std::vector<unsigned>& _vCoords, double _dValue, bool _bExternal /*= true*/ )
{
	return SetValue( _dTime, m_vDimensions, _vCoords, _dValue, _bExternal );
}

bool CMDMatrix::SetValue(double _dTime, const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, double _dValue, bool _bExternal /*= true*/)
{
	//int index;
	if( /* ( index = */ GetTimeIndex( _dTime ) /* ) */ == -1 ) // time point doesn't exist
		return false;

	if( _vDims.size() > m_vDimensions.size() ) // _vDims has wrong size
		return false;

	if( _vDims.size() != _vCoords.size() ) // _vCoords has wrong size
		return false;

	m_dTempValue = _dValue > m_dMinFraction ? _dValue : 0;
	m_dTempT1 = _dTime;
	m_vTempDims = _vDims;
	m_vTempCoords = _vCoords;

	// initialize first dimension if it is NULL
	if( m_data == NULL )
		m_data = IitialiseDimension( m_vClasses.front() );

	UnCacheData(_dTime);
	m_bCacheCoherent = false;
	return SetValueRecursive( m_data, _bExternal );
}

std::vector<double> CMDMatrix::GetVectorValue(unsigned _nTimeIndex, unsigned _nDim) const
{
	if (_nTimeIndex >= m_vTimePoints.size()) return {};
	return GetVectorValue(m_vTimePoints[_nTimeIndex], std::vector<unsigned>{_nDim}, std::vector<unsigned>{});
}

bool CMDMatrix::GetVectorValue(unsigned _nTimeIndex, unsigned _nDim, std::vector<double>& _vResult) const
{
	if( _nTimeIndex >= m_vTimePoints.size() )
		return false;

	std::vector<unsigned> vDims(1);
	vDims[0] = _nDim;
	std::vector<unsigned> vCoords;
	return GetVectorValue( m_vTimePoints[_nTimeIndex], vDims, vCoords, _vResult );
}

std::vector<double> CMDMatrix::GetVectorValue(double _dTime, unsigned _nDim) const
{
	return GetVectorValue(_dTime, std::vector<unsigned>{_nDim}, std::vector<unsigned>{});
}

bool CMDMatrix::GetVectorValue(double _dTime, unsigned _nDim, std::vector<double>& _vResult) const
{
	std::vector<unsigned> vDims(1);
	vDims[0] = _nDim;
	std::vector<unsigned> vCoords;
	return GetVectorValue( _dTime, vDims, vCoords, _vResult );
}

std::vector<double> CMDMatrix::GetVectorValue(double _dTime, unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2) const
{
	return GetVectorValue(_dTime, { _nDim1 , _nDim2 }, {_nCoord1});
}

bool CMDMatrix::GetVectorValue(double _dTime, unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, std::vector<double>& _vResult) const
{
	std::vector<unsigned> vDims(2);
	vDims[0] = _nDim1;
	vDims[1] = _nDim2;
	std::vector<unsigned> vCoords(1);
	vCoords[0] = _nCoord1;
	return GetVectorValue( _dTime, vDims, vCoords, _vResult );
}

std::vector<double> CMDMatrix::GetVectorValue(double _dTime, const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords) const
{
	if (m_vTimePoints.empty())
		return {};

	if (_vDims.size() > m_vDimensions.size()) // _vDims has wrong size
		return {};

	if (_vDims.size() != _vCoords.size() + 1) // _vCoords has wrong size
		return {};

	m_dTempT1 = _dTime;
	m_vTempDims = _vDims;
	m_vTempCoords = _vCoords;

	// find index of last element in _vDims
	size_t iDims;
	for (iDims = 0; iDims < m_vDimensions.size(); ++iDims)
		if (_vDims.back() == m_vDimensions[iDims])
			break;
	if (iDims == m_vDimensions.size()) // not found
		return {};

	bool bIsDimsConsecutive = true; // the sequence of dym types in _vDims corresponds to a sequence in m_vDimensions
	for (size_t i = 0; i < _vDims.size(); ++i)
		if (_vDims[i] != m_vDimensions[i])
		{
			bIsDimsConsecutive = false;
			break;
		}

	UnCacheData(_dTime);

	std::vector<double> res;

	if (bIsDimsConsecutive)
	{
		res.reserve(m_vClasses[iDims]);
		if (!GetVectorValueRecursive(m_data, res)) // get whole vector
			return std::vector<double>(m_vClasses[iDims], 0);
	}
	else
	{
		res.resize(m_vClasses[iDims]);
		m_vTempCoords.push_back(0);

		// get data by values
		for (size_t i = 0; i < m_vClasses[iDims]; ++i)
		{
			res[i] = GetValueRecursive(m_data);
			m_vTempCoords.back()++;
		}
	}

	int cnt = 0;
	for (size_t i = 0; i<res.size(); ++i)
	{
		if (res[i] == 0)
			cnt++;
		else if (res[i] < m_dMinFraction)
		{
			res[i] = 0;
			cnt++;
		}
	}

	return res;
}

bool CMDMatrix::GetVectorValue(double _dTime, const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, std::vector<double>& _vResult) const
{
	if( m_vTimePoints.empty() )
		return false;

	if( _vDims.size() > m_vDimensions.size() ) // _vDims has wrong size
		return false;

	if( _vDims.size() != _vCoords.size() + 1 ) // _vCoords has wrong size
		return false;

	_vResult.clear();

	m_dTempT1 = _dTime;
	m_vTempDims = _vDims;
	m_vTempCoords = _vCoords;

	// find index of last element in _vDims
	unsigned iDims;
	for( iDims=0; iDims<m_vDimensions.size(); ++iDims )
		if ( _vDims.back() == m_vDimensions[iDims] )
			break;
	if( iDims == m_vDimensions.size() ) // not found
		return false;

	_vResult.reserve( m_vClasses[iDims] );

	bool bIsDimsConsecutive = true; // the sequence of dym types in _vDims corresponds to a sequence in m_vDimensions
	for( unsigned i=0; i<_vDims.size(); ++i )
		if( _vDims[i] != m_vDimensions[i] )
		{
			bIsDimsConsecutive = false;
			break;
		}

	UnCacheData(_dTime);
	//bool bReturnVal = true;
	if( bIsDimsConsecutive )
	{
		// get whole vector
		if ( !GetVectorValueRecursive( m_data, _vResult ) )
		{
			_vResult.assign( m_vClasses[iDims], 0 );
			return false;
		}
	}
	else
	{
		m_vTempCoords.push_back(0);

		// get data by values
		for( unsigned i=0; i<m_vClasses[iDims]; ++i )
		{
			_vResult.push_back( GetValueRecursive( m_data ) );
			m_vTempCoords.back()++;
		}
	}

	int cnt = 0;
	for( unsigned i=0; i<_vResult.size(); ++i )
	{
		if( _vResult[i] == 0 )
			cnt++;
		else if( _vResult[i] < m_dMinFraction )
		{
			_vResult[i] = 0;
			cnt++;
		}
	}

	return cnt == _vResult.size() ? false : true;
}

std::vector<double> CMDMatrix::GetValues(unsigned _nDim, unsigned _nCoord) const
{
	if (_nDim >= m_vDimensions.size()) return {};
	std::vector<double> res(m_vTimePoints.size());
	for (size_t i = 0; i < m_vTimePoints.size(); ++i)
		res[i] = GetValue(i, _nDim, _nCoord);
	return res;
}

bool CMDMatrix::SetVectorValue(unsigned _nTimeIndex, unsigned _nDim, const std::vector<double>& _vValue, bool _bExternal /*= false*/ )
{
	if( _nTimeIndex >= m_vTimePoints.size() )
		return false;

	std::vector<unsigned> vDims(1);
	vDims[0] = _nDim;
	std::vector<unsigned> vCoords;
	return SetVectorValue( m_vTimePoints[_nTimeIndex], vDims, vCoords, _vValue, _bExternal );
}

bool CMDMatrix::SetVectorValue(double _dTime, unsigned _nDim, const std::vector<double>& _vValue, bool _bExternal /*= false*/ )
{
	std::vector<unsigned> vDims(1);
	vDims[0] = _nDim;
	std::vector<unsigned> vCoords;
	return SetVectorValue( _dTime, vDims, vCoords, _vValue, _bExternal );
}

bool CMDMatrix::SetVectorValue(double _dTime, unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, const std::vector<double>& _vValue, bool _bExternal /*= false*/ )
{
	std::vector<unsigned> vDims(2);
	vDims[0] = _nDim1;
	vDims[1] = _nDim2;
	std::vector<unsigned> vCoords(1);
	vCoords[0] = _nCoord1;
	return SetVectorValue( _dTime, vDims, vCoords, _vValue, _bExternal );
}

bool CMDMatrix::SetVectorValue(double _dTime, const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, const std::vector<double>& _vValue, bool _bExternal /*= false*/ )
{
	if( m_vTimePoints.empty() )
		return false;

	//int index;
	if( /* ( index = */ GetTimeIndex( _dTime ) /* ) */ == -1 ) // time point doesn't exist
		return false;

	if( _vDims.size() > m_vDimensions.size() ) // _vDims has wrong size
		return false;

	if( _vDims.size() != _vCoords.size() + 1 ) // _vCoords has wrong size
		return false;

	// find index of last element in _vDims
	unsigned iDims;
	for ( iDims=0; iDims<m_vDimensions.size(); ++iDims )
		if ( _vDims.back() == m_vDimensions[iDims] )
			break;
	if( iDims == m_vDimensions.size() ) // not found
		return false;

	if( m_vClasses[iDims] != _vValue.size() ) // wrong _vValue size
		return false;

	bool bIsDimsConsecutive = true; // the sequence of dim types in _vDims corresponds to a sequence in m_vDimensions
	for( unsigned i=0; i<_vDims.size(); ++i )
	{
		if( _vDims[i] != m_vDimensions[i] )
		{
			bIsDimsConsecutive = false;
			break;
		}
	}

	m_dTempT1 = _dTime;
	m_vTempDims = _vDims;
	m_vTempCoords = _vCoords;

	// initialize first dimension if it is NULL
	if( m_data == NULL )
		m_data = IitialiseDimension( m_vClasses.front() );

	UnCacheData(_dTime);
	m_bCacheCoherent = false;
	if( ( bIsDimsConsecutive ) && ( !_bExternal ) )
	{
		m_vTempValues.resize( _vValue.size() );
		for( unsigned i=0; i<_vValue.size(); ++i )
		{
			if( _vValue[i] < m_dMinFraction )
				m_vTempValues[i] = 0;
			else
				m_vTempValues[i] = _vValue[i];
		}

		return SetVectorValueRecursive( m_data );
	}
	else
	{
		m_vTempCoords.push_back(0);

		for( unsigned i=0; i<_vValue.size(); ++i )
		{
			m_dTempValue = _vValue[i] > m_dMinFraction ? _vValue[i] : 0;
			SetValueRecursive( m_data, _bExternal );
			m_vTempCoords.back()++;
		}
	}

	return true;
}

bool CMDMatrix::GetMatrixValue(double _dTime, const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, std::vector<std::vector<double>>& _vResult)
{
	if( m_vTimePoints.empty() )
		return false;

	if( _vDims.size() > m_vDimensions.size() ) // _vDims has wrong size
		return false;

	if( ( _vDims.size() != _vCoords.size() + 1 ) && ( _vDims.size() != _vCoords.size() + 2 ) ) // _vCoords has wrong size
		return false;

	_vResult.clear();
	std::vector<unsigned> vBufCoords = _vCoords;
	if( _vDims.size() == _vCoords.size() + 2 )
		vBufCoords.push_back(0);

	std::vector<unsigned> vTmpClasses;;
	for( unsigned i=0; i<_vDims.size(); ++i )
		for( unsigned j=0; j<m_vDimensions.size(); ++j )
			if( _vDims[i] == m_vDimensions[j] )
			{
				vTmpClasses.push_back( m_vClasses[j] );
				break;
			}

	//if( vTmpClasses.size() < 2 )
	//	return false;

	//bool bRes = false;
	//do
	//{
	//	std::vector<double> vRes;
	//	GetVectorValue( _dTime, _vDims, vBufCoords, vRes );
	//	_vResult.push_back( vRes );
	//	if( _vDims.size() != _vCoords.size() + 1 )
	//	{
	//		vBufCoords.back()++;
	//		bRes = vBufCoords.back() < vTmpClasses[vTmpClasses.size()-2];
	//	}
	//}
	//while( bRes );

	//return true;

	if( vTmpClasses.size() < 2 )
	{
		std::vector<double> vRes;
		GetVectorValue( _dTime, _vDims, vBufCoords, vRes );
		_vResult.push_back( vRes );
	}
	else
	{
		bool bRes = false;
		do
		{
			std::vector<double> vRes;
			GetVectorValue( _dTime, _vDims, vBufCoords, vRes );
			_vResult.push_back( vRes );
			if( _vDims.size() != _vCoords.size() + 1 )
			{
				vBufCoords.back()++;
				bRes = vBufCoords.back() < vTmpClasses[vTmpClasses.size()-2];
			}
		}
		while( bRes );
	}

	return true;
}

std::vector<std::vector<double>> CMDMatrix::GetMatrixValue(double _dTime, const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords)
{
	if (m_vTimePoints.empty()) return {};
	if (_vDims.size() > m_vDimensions.size()) return {}; // _vDims has wrong size
	if (_vDims.size() != _vCoords.size() + 1 && _vDims.size() != _vCoords.size() + 2) return {}; // _vCoords has wrong size

	std::vector<unsigned> vFullCoords = _vCoords;
	if (_vDims.size() == _vCoords.size() + 2)
		vFullCoords.push_back(0);

	std::vector<unsigned> vClasses;;
	for (size_t i = 0; i < _vDims.size(); ++i)
		for (size_t j = 0; j < m_vDimensions.size(); ++j)
			if (_vDims[i] == m_vDimensions[j])
			{
				vClasses.push_back(m_vClasses[j]);
				break;
			}

	std::vector<std::vector<double>> res;
	if (vClasses.size() < 2)
		res.push_back(GetVectorValue(_dTime, _vDims, vFullCoords));
	else
	{
		bool bRes = false;
		do
		{
			res.push_back(GetVectorValue(_dTime, _vDims, vFullCoords));
			if (_vDims.size() != _vCoords.size() + 1)
			{
				vFullCoords.back()++;
				bRes = vFullCoords.back() < vClasses[vClasses.size() - 2];
			}
		} while (bRes);
	}

	return res;
}

bool CMDMatrix::SetMatrixValue(double _dTime, const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, const std::vector<std::vector<double>>& _vValue)
{
	if( m_vTimePoints.empty() )
		return false;

	if( _vDims.size() > m_vDimensions.size() ) // _vDims has wrong size
		return false;

	if( ( _vDims.size() != _vCoords.size() + 1 ) && ( _vDims.size() != _vCoords.size() + 2 ) ) // _vCoords has wrong size
		return false;

	std::vector<unsigned> vBufCoords = _vCoords;
	if( _vDims.size() == _vCoords.size() + 2 )
		vBufCoords.push_back(0);

	//CDenseMDMatrix distr;
	//if( !GetDistribution( _dTime, _vDims, distr ) )
	//	return false;
	//for( unsigned i=0; i<_vValue.size(); ++i )
	//{
	//	std::vector<double> vCurrVal = _vValue[i];
	//	distr.SetVectorValue( _vDims, vBufCoords, vCurrVal );
	//	if( _vDims.size() != _vCoords.size() + 1 )
	//		vBufCoords.back()++;
	//}
	//return SetDistribution( _dTime, distr );
	if( ( _vDims.size() > 1 ) && ( _vValue.size() > 1 ) && ( _vValue.front().size() > 1 ) && ( _vDims.size() == _vCoords.size() + 2 ) )
	{
		int i1 = -1;
		int	i2 = -1;
		for( unsigned i=0; i<m_vDimensions.size(); ++i )
		{
			if( _vDims[_vDims.size()-2] == m_vDimensions[i] )
				i1 = i;
			if( _vDims[_vDims.size()-1] == m_vDimensions[i] )
				i2 = i;
		}
		std::vector<double> vPreDimData;
		std::vector<unsigned> vPreDimDims;
		if( i1 < i2 )
		{
			vPreDimData.resize( _vValue.size() );
			vPreDimDims = _vDims;
			vPreDimDims.pop_back();
			for( unsigned i=0; i<_vValue.size(); ++i )
			{
				double dSum = 0;
				for( unsigned j=0; j<_vValue[i].size(); ++j )
					dSum += _vValue[i][j];
				vPreDimData[i] = dSum;
			}
		}
		else
		{
			vPreDimData.resize( _vValue.front().size() );
			vPreDimDims = _vDims;
			vPreDimDims.erase( vPreDimDims.begin() + ( vPreDimDims.size() - 2 ) );
			for( unsigned i=0; i<_vValue.front().size(); ++i )
			{
				double dSum = 0;
				for( unsigned j=0; j<_vValue.size(); ++j )
					dSum += _vValue[j][i];
				vPreDimData[i] = dSum;
			}
		}
		SetVectorValue( _dTime, vPreDimDims, _vCoords, vPreDimData, true );
	}

	for( unsigned i=0; i<_vValue.size(); ++i )
	{
		std::vector<double> vCurrVal = _vValue[i];
		SetVectorValue( _dTime, _vDims, vBufCoords, vCurrVal, true );
		if( _vDims.size() != _vCoords.size() + 1 )
			vBufCoords.back()++;
	}

	NormalizeMatrix( _dTime );
	return true;
}

CDenseMDMatrix CMDMatrix::GetDistribution(double _dTime) const
{
	if (m_vTimePoints.empty())
		return {};

	// initialize result matrix
	CDenseMDMatrix res(m_vDimensions, m_vClasses);

	// get local copy of coordinates
	std::vector<unsigned> vCoords(m_vDimensions.size() - 1, 0);

	// main loop
	do
	{
		res.SetVectorValue(m_vDimensions, vCoords, GetVectorValue(_dTime, m_vDimensions, vCoords));
	} while (IncrementCoords(vCoords, m_vClasses));

	return res;
}

bool CMDMatrix::GetDistribution(double _dTime, CDenseMDMatrix& _Result) const
{
	if( m_vTimePoints.empty() )
		return false;

	// initialize result matrix
	if( !_Result.SetDimensions( m_vDimensions, m_vClasses ) ) // cannot set dimensions
		return false;

	// get local copy of coordinates
	std::vector<unsigned> vCoords( m_vDimensions.size()-1, 0 );

	// main loop
	bool bRes;
	std::vector<double> vResult;
	do
	{
		if( GetVectorValue( _dTime, m_vDimensions, vCoords, vResult ) )
			_Result.SetVectorValue( m_vDimensions, vCoords, vResult );
		bRes = IncrementCoords( vCoords, m_vClasses );
	}
	while( bRes );

	return true;
}

bool CMDMatrix::GetDistribution(double _dTime, unsigned _nDim, std::vector<double>& _vResult) const
{
	if( m_vTimePoints.empty() )
		return false;

	// make a vector of dimensions
	unsigned i;
	for( i=0; i<m_vDimensions.size(); ++i )
		if( _nDim == m_vDimensions[i] )
			break;
	if( i == m_vDimensions.size() )
		return false;

	// initialize result vector
	_vResult.clear();

	return GetVectorValue( _dTime, _nDim, _vResult );
}

std::vector<double> CMDMatrix::GetDistribution(double _dTime, unsigned _nDim) const
{
	if (m_vTimePoints.empty()) return {};

	// make a vector of dimensions
	unsigned i;
	for (i = 0; i<m_vDimensions.size(); ++i)
		if (_nDim == m_vDimensions[i])
			break;
	if (i == m_vDimensions.size()) return {};

	return GetVectorValue(_dTime, _nDim);
}

bool CMDMatrix::GetDistribution(double _dTime, unsigned _nDim1, unsigned _nDim2, CMatrix2D& _Result) const
{
	if( m_vTimePoints.empty() )
		return false;

	if( _nDim1 == _nDim2 )
		return false;

	// make a vector of dimensions
	unsigned index1 = 0, index2 = 0;
	std::vector<unsigned> vDims;
	for( unsigned i=0; i<m_vDimensions.size(); ++i )
		if ( _nDim1 == m_vDimensions[i] )
		{
			vDims.push_back( _nDim1 );
			index1 = i;
			break;
		}
	for( unsigned i=0; i<m_vDimensions.size(); ++i )
		if ( _nDim2 == m_vDimensions[i] )
		{
			vDims.push_back( _nDim2 );
			index2 = i;
			break;
		}
	if( vDims.size() != 2 ) // wrong dimension type
		return false;

	// initialize result matrix
	_Result.Resize( m_vClasses[index1], m_vClasses[index2] );

	// main loop
	std::vector<unsigned> vCoords( 1, 0 );
	std::vector<double> vResult;
	for( unsigned i=0; i<m_vClasses[index1]; ++i )
	{
		if( GetVectorValue( _dTime, vDims, vCoords, vResult ) )
			_Result.SetRow( i, vResult );
		vCoords.front()++;
	}

	return true;
}

CMatrix2D CMDMatrix::GetDistribution(double _dTime, unsigned _nDim1, unsigned _nDim2) const
{
	if (m_vTimePoints.empty()) return {};
	if (_nDim1 == _nDim2) return {};

	// make a vector of dimensions
	size_t index1 = 0, index2 = 0;
	std::vector<unsigned> vDims;
	for (size_t i = 0; i < m_vDimensions.size(); ++i)
		if (_nDim1 == m_vDimensions[i])
		{
			vDims.push_back(_nDim1);
			index1 = i;
			break;
		}
	for (size_t i = 0; i < m_vDimensions.size(); ++i)
		if (_nDim2 == m_vDimensions[i])
		{
			vDims.push_back(_nDim2);
			index2 = i;
			break;
		}
	if (vDims.size() != 2) return {}; // wrong dimension type

	// initialize result matrix
	CMatrix2D res(m_vClasses[index1], m_vClasses[index2]);

	// main loop
	std::vector<unsigned> vCoords(1, 0);
	std::vector<double> vResult;
	for (size_t i = 0; i < m_vClasses[index1]; ++i)
	{
		if (GetVectorValue(_dTime, vDims, vCoords, vResult))
			res.SetRow(i, vResult);
		vCoords.front()++;
	}

	return res;
}

bool CMDMatrix::GetDistribution(double _dTime, unsigned _nDim1, unsigned _nDim2, unsigned _nDim3, CDenseMDMatrix& _Result) const
{
	std::vector<unsigned> vDims;
	vDims.push_back( _nDim1 );
	vDims.push_back( _nDim2 );
	vDims.push_back( _nDim3 );
	return GetDistribution( _dTime, vDims, _Result );
}

CDenseMDMatrix CMDMatrix::GetDistribution(double _dTime, unsigned _nDim1, unsigned _nDim2, unsigned _nDim3) const
{
	return GetDistribution(_dTime, std::vector<unsigned>{ _nDim1, _nDim2, _nDim3 });
}

bool CMDMatrix::GetDistribution(double _dTime, const std::vector<unsigned>& _vDims, CDenseMDMatrix& _Result) const
{
	if( m_vTimePoints.empty() )
		return false;

	if( _vDims.size() > m_vDimensions.size() ) // wrong size of _vDims
		return false;
	if( _vDims.size() == 0 )
		return false;

	// get local copy of vector of dimensions and sizes according to their order in _vDims
	std::vector<unsigned> vDims;
	std::vector<unsigned> vSizes;
	for( unsigned i=0; i<_vDims.size(); ++i )
		for ( unsigned j=0; j<m_vDimensions.size(); ++j )
			if ( _vDims[i] == m_vDimensions[j] )
			{
				vDims.push_back( m_vDimensions[j] );
				vSizes.push_back( m_vClasses[j] );
				break;
			}
	if( vDims.size() != _vDims.size() ) // wrong dimensions types
		return false;

	// initialize result matrix
	if( !_Result.SetDimensions( vDims, vSizes ) ) // cannot set dimensions
		return false;

	// get local copy of coordinates
	std::vector<unsigned> vCoords( vDims.size()-1, 0 );

	// main loop
	bool bRes;
	std::vector<double> vResult;
	do
	{
		if( GetVectorValue( _dTime, vDims, vCoords, vResult ) )
			_Result.SetVectorValue( vDims, vCoords, vResult );
		bRes = IncrementCoords( vCoords, vSizes );
	}
	while( bRes );

	return true;
}

CDenseMDMatrix CMDMatrix::GetDistribution(double _dTime, const std::vector<unsigned>& _vDims) const
{
	if (m_vTimePoints.empty()) return {};
	if (_vDims.size() > m_vDimensions.size()) return {};  // wrong size of _vDims
	if (_vDims.empty()) return {};

	// get local copy of vector of dimensions and sizes according to their order in _vDims
	std::vector<unsigned> vDims;
	std::vector<unsigned> vSizes;
	for (unsigned dim : _vDims)
		for (size_t j = 0; j < m_vDimensions.size(); ++j)
			if (dim == m_vDimensions[j])
			{
				vDims.push_back(m_vDimensions[j]);
				vSizes.push_back(m_vClasses[j]);
				break;
			}
	if (vDims.size() != _vDims.size()) return {}; // wrong dimensions types

	// initialize result matrix
	CDenseMDMatrix res(vDims, vSizes);

	// get local copy of coordinates
	std::vector<unsigned> vCoords(vDims.size() - 1, 0);

	// main loop
	do
	{
		res.SetVectorValue(vDims, vCoords, GetVectorValue(_dTime, vDims, vCoords));
	} while (IncrementCoords(vCoords, vSizes));

	return res;
}

bool CMDMatrix::SetDistribution(double _dTime, unsigned _nDim, const std::vector<double>& _vDistr)
{
	if(GetTimeIndex( _dTime ) == -1) // time point doesn't exist
		return false;

	unsigned iDim;
	for( iDim=0; iDim<m_vDimensions.size(); ++iDim )
		if( m_vDimensions[iDim] == _nDim )
			break;
	if( iDim == m_vDimensions.size() ) // wrong dimension type
		return false;

	if( m_vClasses[iDim] != _vDistr.size() ) // wrong distribution size
		return false;

	bool bRes = SetVectorValue( _dTime, _nDim, _vDistr );
	NormalizeMatrix(_dTime);
	return bRes;
}

bool CMDMatrix::SetDistribution(double _dTime, unsigned _nDim1, unsigned _nDim2, const CMatrix2D& _Distr)
{
	//int index;
	if( /* ( index =  */GetTimeIndex( _dTime ) /* ) */ == -1 ) // time point doesn't exist
		return false;

	// get sequence of dimensions
	std::vector<unsigned> vDims;
	unsigned iDim;
	for( iDim=0; iDim<m_vDimensions.size(); ++iDim )
		if( m_vDimensions[iDim] == _nDim1 )
			vDims.push_back( m_vDimensions[iDim] );
		else if( m_vDimensions[iDim] == _nDim2 )
			vDims.push_back( m_vDimensions[iDim] );
	if( vDims.size() != 2 ) // wrong dimensions type
		return false;

	if( vDims.front() == _nDim1 )
	{
		// get distribution for top level
		std::vector<double> vTopLevelValues( _Distr.Rows() );
		std::vector<std::vector<double>> vBottomLevelValues(_Distr.Rows(), std::vector<double>(_Distr.Cols()));
		for( unsigned i=0; i<_Distr.Rows(); ++i )
		{
			vBottomLevelValues[i] = _Distr.GetRow(i);
			for( unsigned j=0; j<_Distr.Cols(); ++j )
				vTopLevelValues[i] += vBottomLevelValues[i][j];
		}
		// set distribution for top level
		if( !SetVectorValue( _dTime, _nDim1, vTopLevelValues ) )
			return false;

		// set distribution for bottom level
		for( unsigned i=0; i<vTopLevelValues.size(); ++i )
			if( !SetVectorValue( _dTime, _nDim1, i, _nDim2, vBottomLevelValues[i] ) )
				return false;
	}
	else
	{
		// get distribution for top level
		std::vector<double> vTopLevelValues( _Distr.Cols() );
		std::vector<std::vector<double>> vBottomLevelValues(_Distr.Cols(), std::vector<double>(_Distr.Rows()));
		for( unsigned i=0; i<_Distr.Cols(); ++i )
		{
			vBottomLevelValues[i] = _Distr.GetCol(i);
			for( unsigned j=0; j<_Distr.Rows(); ++j )
				vTopLevelValues[i] += vBottomLevelValues[i][j];
		}
		// set distribution for top level
		if( !SetVectorValue( _dTime, _nDim2, vTopLevelValues ))
			return false;

		// set distribution for bottom level
		for( unsigned i=0; i<vTopLevelValues.size(); ++i )
			if( !SetVectorValue( _dTime, _nDim2, i, _nDim1, vBottomLevelValues[i] ) )
				return false;
	}

	NormalizeMatrix(_dTime);
	return true;
}

bool CMDMatrix::SetDistribution(double _dTime, const CDenseMDMatrix& _Distr)
{
	//int index;
	if( /* ( index = */ GetTimeIndex( _dTime ) /* ) */ == -1 ) // time point doesn't exist
		return false;

	std::vector<unsigned> vDistrDims = _Distr.GetDimensions();
	std::vector<unsigned> vDistrClasses = _Distr.GetClasses();

	// get dimensions types in right order
	std::vector<unsigned> vDims;
	std::vector<unsigned> vClasses;
	for( unsigned i=0; i<m_vDimensions.size(); ++i )
		for( unsigned j=0; j<vDistrDims.size(); ++j )
			if( ( vDistrDims[j] == m_vDimensions[i] ) && ( vDistrClasses[j] == m_vClasses[i] ) )
			{
				vDims.push_back( m_vDimensions[i] );
				vClasses.push_back( m_vClasses[i] );
			}
	if( vDims.size() != vDistrDims.size() ) // wrong dimensions
		return false;

	std::vector<unsigned> vCurrDims;
	std::vector<unsigned> vCurrClasses;
	std::vector<unsigned> vCurrCoords;
	for( unsigned i=0; i<vDims.size(); ++i )
	{
		vCurrDims.push_back( vDims[i] );
		vCurrClasses.push_back( vClasses[i] );
		bool bRes;
		do
		{
			std::vector<double> vRes;
			_Distr.GetVectorValue( vCurrDims, vCurrCoords, vRes );
			SetVectorValue( _dTime, vCurrDims, vCurrCoords, vRes );

			bRes = IncrementCoords( vCurrCoords, vCurrClasses );
		}
		while( bRes );

		std::fill( vCurrCoords.begin(), vCurrCoords.end(), 0 );
		vCurrCoords.push_back( 0 );
	}

	NormalizeMatrix(_dTime);
	return true;
}

bool CMDMatrix::Transform(double _dTime, const CTransformMatrix& _TMatrix)
{
	std::vector<unsigned> vTDims = _TMatrix.GetDimensions();
	std::vector<unsigned> vTClasses = _TMatrix.GetClasses();
	std::vector<unsigned> vNewDims;
	std::vector<unsigned> vNewClasses;

	if( vTDims.size() == 0 ) return false;

	// get new sequence of dimensions for sorting
	for( unsigned i=0; i<vTDims.size(); ++i )
	{
		unsigned j=0;
		for( j=0; j<m_vDimensions.size(); ++j )
		{
			if( ( vTDims[i] == m_vDimensions[j] ) && ( vTClasses[i] == m_vClasses[j] ) )
			{
				vNewDims.push_back( vTDims[i] );
				vNewClasses.push_back( vTClasses[i] );
				break;
			}
		}
		if( j == m_vDimensions.size() ) // dimensions are not similar in types or numbers of classes
			return false;
	}
	if( vNewDims.size() != m_vDimensions.size() ) // add missing dimensions
		for( unsigned i=0; i<m_vDimensions.size(); ++i )
		{
			unsigned j=0;
			for( j=0; j<vNewDims.size(); ++j )
				if( vNewDims[j] == m_vDimensions[i] )
					break;
			if( j == vNewDims.size() )
			{
				vNewDims.push_back( m_vDimensions[i] );
				vNewClasses.push_back( m_vClasses[i] );
			}
		}

	// sorting according to vNewDims
	m_pSortMatr = new CMDMatrix();
	m_pSortMatr->SetDimensions( vNewDims, vNewClasses );
	m_pSortMatr->AddTimePoint( 0 );
	m_pSortMatr->AddTimePoint( 1 );
	m_pSortMatr->SetMinimalFraction(m_dMinFraction);
	bool bRes = SortMatrix( _dTime, 0, *m_pSortMatr );

	TransformRecurcive( _TMatrix );

	// transformation of lower dimensions
	if( vTDims.size() != m_vDimensions.size() )
	{
		std::vector<unsigned> vFirstCoord( vTDims.size(), 0 );
		std::vector<unsigned> vSecondCoord;
		std::vector<unsigned> vFullLowerCoord( vFirstCoord.size(), 0 );
		std::vector<unsigned> vFullLowerDims = vTDims;
		std::vector<unsigned> vSecondClasses;
		for( unsigned i=0; i<m_vDimensions.size() - vTDims.size(); ++i ) // for each lower dimension
		{
			vFullLowerDims.push_back( vNewDims[vTDims.size()+i] ); // add next dimension
			vSecondClasses.push_back( vNewClasses[vTDims.size()+i] ); // add next classes

			bool bSecondRes;
			do // for each class in current dimension
			{
				// get vector of values
				std::vector<std::vector<double>> vvLowerValues;
				vvLowerValues.reserve( vTClasses.back() );
				std::vector<bool> vIsNotEmpty;
				vIsNotEmpty.reserve( vTClasses.back() );
				bool bFirstRes;
				bool bIsEmpty = true;
				for( unsigned j=0; j<vSecondCoord.size(); ++j )
					vFullLowerCoord[vFirstCoord.size()+j] = vSecondCoord[j];
				do
				{
					for( unsigned j=0; j<vFirstCoord.size(); ++j )
						vFullLowerCoord[j] = vFirstCoord[j];
					std::vector<double> vRes;
					bool bRes = m_pSortMatr->GetVectorValue( 0, vFullLowerDims, vFullLowerCoord, vRes );
					vvLowerValues.push_back( vRes );
					vIsNotEmpty.push_back( bRes );
					if( bIsEmpty ) // check if values contains only zeros
					{
						unsigned index;
						for( index=0; index<vRes.size(); ++index )
							if( vRes[index] != 0 )
								break;
						if( index != vRes.size() )
							bIsEmpty = false;
					}
					bFirstRes = IncrementCoords( vFirstCoord, vTClasses );
				}
				while( bFirstRes );

				if( !bIsEmpty )
				{
					std::vector<unsigned> vSrcCoord( vTDims.size()-1, 0 );
					std::vector<unsigned> vDestCoord( vTDims.size(), 0 );

					// for each value in vLowerValues
					bool bResDest;
					do
					{
						std::fill( vSrcCoord.begin(), vSrcCoord.end(), 0 ); // zeroing coordinates
						std::vector<double> vNewValue( vvLowerValues.front().size(), 0 );
						size_t shift = 0;
						bool bTransRes;
						do
						{
							std::vector<double> vTFactors;
							_TMatrix.GetVectorValue( vSrcCoord, vDestCoord, vTFactors );
							for( unsigned j=0; j<vTFactors.size(); ++j )
								if( ( vTFactors[j] != 0 ) && ( vIsNotEmpty[j + shift] ) )
									for( unsigned k=0; k<vvLowerValues.front().size(); ++k )
										vNewValue[k] += vvLowerValues[j + shift][k] * vTFactors[j];
							shift += vTFactors.size();
							bTransRes = IncrementCoords( vSrcCoord, vTClasses );
						}
						while( bTransRes );

						// set value
						std::vector<unsigned> vFullSetCoord;
						vFullSetCoord.insert( vFullSetCoord.end(), vDestCoord.begin(), vDestCoord.end() );
						vFullSetCoord.insert( vFullSetCoord.end(), vSecondCoord.begin(), vSecondCoord.end() );
						m_pSortMatr->SetVectorValue( 1, vFullLowerDims, vFullSetCoord, vNewValue ); // set new value

						// prepare to the next iteration
						bResDest = IncrementCoords( vDestCoord, vTClasses );
					}
					while( bResDest );
				}

				// prepare to next iteration
				std::fill( vFirstCoord.begin(), vFirstCoord.end(), 0 );
				vvLowerValues.clear();

				bSecondRes = IncrementCoords( vSecondCoord, vSecondClasses );
			}
			while( bSecondRes );

			// zeroing coordinates
			std::fill( vSecondCoord.begin(), vSecondCoord.end(), 0 );
			vSecondCoord.push_back( 0 );
			vFullLowerCoord.push_back( 0 );
			//vFullLowerDims.clear();
		}
	}

	// reverse sorting
	/* bRes =  */m_pSortMatr->SortMatrix( 1, _dTime, *this );

	delete m_pSortMatr;
	m_pSortMatr = nullptr;

	return true;
}

void CMDMatrix::NormalizeMatrix(double _dTime)
{
	unsigned index = GetTimeIndex( _dTime );
	if( index != -1 )
	{
		m_vTempValues.clear();
		m_vTempValues.push_back( m_vTimePoints[index] );
		UnCacheData(_dTime);
		m_bCacheCoherent = false;
		NormalizeWrapper( m_vTempValues );
		//NormalizeMatrixRecursive( m_data );
	}
}

void CMDMatrix::NormalizeMatrix(double _dStart, double _dEnd)
{
	if( m_vTimePoints.size() == 0 ) // nothing to normalize
		return;

	size_t iLast = GetTimeIndex( _dEnd, false );
	size_t iFirst = GetTimeIndex( _dStart, false );
	if((iLast == -1) || (iLast >= m_vTimePoints.size()))
		return;

	if( m_vTimePoints[iLast] != _dEnd )
		iLast--;

	m_vTempValues.clear();
	for(size_t i=iFirst; i<=iLast; ++i )
		m_vTempValues.push_back( m_vTimePoints[i] );

	UnCacheData(_dStart,_dEnd);
	m_bCacheCoherent = false;
	//NormalizeMatrixRecursive( m_data );
	NormalizeWrapper( m_vTempValues );
	CheckCacheNeed();
}

void CMDMatrix::NormalizeMatrix()
{
	m_vTempValues = m_vTimePoints;
	if( !m_vTimePoints.empty() )
		UnCacheData(m_vTimePoints.front(),m_vTimePoints.back());
	m_bCacheCoherent = false;
	//NormalizeMatrixRecursive( m_data );
	NormalizeWrapper( m_vTempValues );
	CheckCacheNeed();
}

bool CMDMatrix::IsNormalized(double _dTime)
{
	//for( unsigned i=0; i<m_vDimensions.size(); ++i )
	//{
	//	std::vector<double> vVal;
	//	GetDistribution( _dTime, (unsigned)GetDimensionTypeByIndex(i), vVal );
	//	double dSum = 0;
	//	for( unsigned j=0; j<vVal.size(); ++j )
	//		dSum += vVal[j];

	//	double dCompValue = m_dMinFraction == 0 ? DEFAULT_MIN_FRACTION*10 : m_dMinFraction*10;
	//	if( ( dSum != 0 ) && ( std::fabs(1 - dSum) >= dCompValue ) )
	//		return false;
	//}
	//return true;

	unsigned index = GetTimeIndex( _dTime );
	if( index != -1 )
	{
		m_vTempValues.clear();
		m_vTempValues.push_back( m_vTimePoints[index] );
		UnCacheData(_dTime);
		return CheckNormalizationRecursive( m_data );
	}
	else
		return false;
}

bool CMDMatrix::CopyFrom(const CMDMatrix& _Source, double _dTime)
{
	return CopyFrom( _Source, _dTime, _dTime );
}

bool CMDMatrix::CopyFrom(const CMDMatrix& _Source, double _dStart, double _dEnd)
{
	if( !CompareDims( _Source ) )
		return false;

	//bool bLastCacheState = m_bCacheEnabled;
	//UnCacheData( _dStart, _dEnd );
	//_Source.UnCacheData(_dStart,_dEnd);
	//m_bCacheEnabled = false;

	std::vector<double> vTimePoints = _Source.GetTimePoints( _dStart, _dEnd );
	if( ( vTimePoints.size() == 0 ) && ( _dStart == _dEnd ) )
		vTimePoints.push_back( _dStart );
	if (vTimePoints.empty()) return true;
	for( unsigned i=0; i<vTimePoints.size(); ++i )
		AddTimePoint( vTimePoints[i] );

	m_dTempT1 = vTimePoints.front();
	m_dTempT2 = vTimePoints.back();
	UnCacheData(_dStart,_dEnd);
	_Source.UnCacheData(_dStart,_dEnd);
	m_bCacheCoherent = false;
	m_data = CopyFromRecursive( m_data, _Source.m_data );

	//m_bCacheEnabled = bLastCacheState;
	//CheckCacheNeed();
	//_Source.CheckCacheNeed();

	return true;
}

bool CMDMatrix::CopyFromTimePoint(const CMDMatrix& _Source, double _dTimeSrc, double _dTimeDest)
{
	if( !CompareDims( _Source ) )
		return false;

	AddTimePoint( _dTimeDest );
	m_dTempT1 = _dTimeSrc;
	m_dTempT2 = _dTimeDest;
	UnCacheData(_dTimeDest);
	_Source.UnCacheData(_dTimeSrc);
	m_bCacheCoherent = false;
	m_data = CopyFromTimePointRecursive( m_data, _Source.m_data );

	return true;
}

//void CMDMatrix::AddMatrix(CMDMatrix& _srcMatr, double _dFactorDst, double _dFactorSrc, double _dTime)
//{
//	std::vector<double> vFactors1;
//	vFactors1.push_back(_dFactorDst);
//	std::vector<double> vFactors2;
//	vFactors2.push_back(_dFactorSrc);
//	std::vector<double> vTimes;
//	vTimes.push_back(_dTime);
//	AddMatrix( _srcMatr, vFactors1, vFactors2, vTimes );
//}
//
//void CMDMatrix::AddMatrix(CMDMatrix& _srcMatr, const std::vector<double>& _vFactorsDst, const std::vector<double>& _vFactorsSrc, const std::vector<double>& _vTimePoints)
//{
//	if( !CompareDims( this, _srcMatr ) )
//		return false;
//
//	if( _vFactorsDst.size() != _vFactorsSrc.size() )
//		return;
//
//	std::vector<CDenseMDMatrix> vBufDst( _vTimePoints.size() );
//	std::vector<CDenseMDMatrix> vBufSrc( _vTimePoints.size() );
//	for( unsigned i=0; i<_vTimePoints.size(); ++i )
//	{
//		GetDistribution( _vTimePoints[i], m_vDimensions, vBufDst[i] );
//		_srcMatr.GetDistribution( _vTimePoints[i], m_vDimensions, vBufSrc[i] );
//	}
//	for( unsigned i=0; i<_vTimePoints.size(); ++i )
//		SetDistribution( _vTimePoints[i], vBufDst[i]*_vFactorsDst[i] + vBufSrc[i]*_vFactorsSrc[i] );
//}

void CMDMatrix::SaveToFile(CH5Handler& _h5File, const std::string& _sPath) const
{
	if (!_h5File.IsValid())
		return;

	// current version of save procedure
	_h5File.WriteAttribute(_sPath, StrConst::MDM_H5AttrSaveVersion, m_cnSaveVersion);

	/// save dimensions
	_h5File.WriteData(_sPath, StrConst::MDM_H5Dimensions, m_vDimensions);

	/// save classes
	_h5File.WriteData(_sPath, StrConst::MDM_H5Classes, m_vClasses);

	/// save time points
	_h5File.WriteData(_sPath, StrConst::MDM_H5TimePoints, m_vTimePoints);

	/// calculate total length of 1d array to store one matrix
	size_t cnt = 0, mul = 1;
	for (size_t j = 0; j < m_vDimensions.size(); ++j)
	{
		mul *= m_vClasses[j];
		cnt += mul;
	}
	std::vector<std::vector<double>> vvBufData(cnt);

	/// save multidimensional data
	unsigned iCnt;
	unsigned nBlocksNum = static_cast<unsigned>(m_vTimePoints.size() / DATA_SAVE_BLOCK);
	for (iCnt = 0; iCnt < nBlocksNum; ++iCnt)
		SaveMDBlockToFile(_h5File, _sPath, iCnt * DATA_SAVE_BLOCK, iCnt * DATA_SAVE_BLOCK + DATA_SAVE_BLOCK - 1, vvBufData);
	unsigned nModulo = m_vTimePoints.size() % DATA_SAVE_BLOCK;
	if (nModulo != 0)
		SaveMDBlockToFile(_h5File, _sPath, iCnt * DATA_SAVE_BLOCK, iCnt * DATA_SAVE_BLOCK + nModulo - 1, vvBufData);

	CheckCacheNeed();
}

void CMDMatrix::SaveMDBlockToFile(CH5Handler& _h5File, const std::string& _sPath, unsigned _iFirst, unsigned _iLast, std::vector<std::vector<double>>& _vvBuf) const
{
	UnCacheData(m_vTimePoints[_iFirst], m_vTimePoints[_iLast]);
	m_vTempValues.assign(m_vTimePoints.begin() + _iFirst, m_vTimePoints.begin() + _iLast + 1);
	m_nCounter = 0;
	GetDataForSaveRecursive(m_data, _vvBuf);
	_h5File.WriteData(_sPath, StrConst::MDM_H5Data + std::to_string(static_cast<unsigned>(_iFirst / DATA_SAVE_BLOCK)), _vvBuf);
}

void CMDMatrix::LoadFromFile(const CH5Handler& _h5File, const std::string& _sPath)
{
	Clear();

	if (!_h5File.IsValid())
		return;

	std::vector<unsigned> vDims;
	std::vector<unsigned> vClasses;

	/// load dimensions
	_h5File.ReadData(_sPath, StrConst::MDM_H5Dimensions, vDims);

	/// load classes
	_h5File.ReadData(_sPath, StrConst::MDM_H5Classes, vClasses);

	SetDimensions(vDims, vClasses);

	/// load time points
	_h5File.ReadData(_sPath, StrConst::MDM_H5TimePoints, m_vTimePoints);

	m_dCurrWinStart = 0;
	m_nCurrOffset = 0;
	unsigned iCnt;
	unsigned nBlocksNum = static_cast<unsigned>(m_vTimePoints.size() / DATA_SAVE_BLOCK);
	std::vector<std::vector<double>> vvBufData;
	for (iCnt = 0; iCnt < nBlocksNum; ++iCnt)
		LoadMDBlockFromFile(_h5File, _sPath, iCnt * DATA_SAVE_BLOCK, iCnt * DATA_SAVE_BLOCK + DATA_SAVE_BLOCK - 1, vvBufData);
	unsigned nModulo = m_vTimePoints.size() % DATA_SAVE_BLOCK;
	if (nModulo != 0)
		LoadMDBlockFromFile(_h5File, _sPath, iCnt * DATA_SAVE_BLOCK, iCnt * DATA_SAVE_BLOCK + nModulo - 1, vvBufData);
}

void CMDMatrix::LoadMDBlockFromFile(const CH5Handler& _h5File, const std::string& _sPath, unsigned _iFirst, unsigned _iLast, std::vector<std::vector<double>>& vvBuf)
{
	_h5File.ReadData(_sPath, StrConst::MDM_H5Data + std::to_string(static_cast<unsigned>(_iFirst / DATA_SAVE_BLOCK)), vvBuf);
	m_vTempValues.assign(m_vTimePoints.begin() + _iFirst, m_vTimePoints.begin() + _iLast + 1);
	m_nCounter = 0;
	m_data = SetDataForLoadRecursive(m_data, vvBuf);
	/// set cache
	m_nNonCachedTPNum += (_iLast - _iFirst + 1);
	m_dCurrWinEnd = m_vTimePoints[_iLast];
	m_bCacheCoherent = false;
	CheckCacheNeed();
}

void CMDMatrix::SetCacheParams(bool _bEnabled, size_t _nWindow)
{
	m_bCacheEnabled = _bEnabled;
	m_nCacheWindow = _nWindow;
	if( ( _bEnabled ) && ( !m_sCachePath.empty() ) )
	{
		m_cacheHandler.SetChunk( m_nCacheWindow );
		m_cacheHandler.SetDirPath( m_sCachePath );
		m_cacheHandler.Initialize();
		CheckCacheNeed();
	}
	if( m_sCachePath.empty() )
		m_bCacheEnabled = false;
}

void CMDMatrix::CompressData( double _dStartTime, double _dEndTime, double _dATol, double _dRTol )
{
	if( _dStartTime < _dEndTime )
	{
		m_dTempT1 = _dStartTime;
		m_dTempT2 = _dEndTime;
		UnCacheData(_dStartTime,_dEndTime);
		m_bCacheCoherent = false;
		CompressDataRecursive( m_data, _dATol, _dRTol );
		CheckCacheNeed();
	}
}

void CMDMatrix::ExtrapolateToPoint( double _dT1, double _dT2, double _dTExtra )
{
	UnCacheData(_dT1,_dTExtra);

	m_dTempT1 = _dT1;
	m_dTempT2 = _dT2;
	m_dTempValue = _dTExtra;

	Extrapolate2ToPointRecursive( m_data );
	NormalizeMatrix( _dTExtra );

	CheckCacheNeed();
}

void CMDMatrix::ExtrapolateToPoint( double _dT0, double _dT1, double _dT2, double _dTExtra )
{
	UnCacheData(_dT0,_dTExtra);

	m_vTempValues.resize( 4 );
	m_vTempValues[0] = _dT0;
	m_vTempValues[1] = _dT1;
	m_vTempValues[2] = _dT2;
	m_vTempValues[3] = _dTExtra;

	Extrapolate3ToPointRecursive( m_data );
	NormalizeMatrix( _dTExtra );

	CheckCacheNeed();
}

bool CMDMatrix::CheckDuplicates(const std::vector<unsigned>& _vVec) const
{
	for( unsigned i=0; i<_vVec.size()-1; ++i )
		for( unsigned j=i+1; j<_vVec.size(); ++j )
			if( _vVec[i] == _vVec[j] )
				return false;
	return true;
}

unsigned CMDMatrix::GetTimeIndex(double _dTime, bool _bIsStrict /*= true */) const
{
	// check if empty
	if( m_vTimePoints.empty() )
	{
		if( _bIsStrict )
			return -1;
		else
			return 0;
	}

	// check the boundaries
	if( m_vTimePoints.front() > _dTime )
	{
		if( _bIsStrict )
			return -1;
		else
			return 0;
	}
	if( m_vTimePoints.back() < _dTime )
	{
		if( _bIsStrict )
			return -1;
		else
			return static_cast<unsigned>(m_vTimePoints.size());
	}

	// binary search
	unsigned nFirst = 0;
	unsigned nLast = static_cast<unsigned>(m_vTimePoints.size());
	unsigned nMid = (unsigned)(nFirst + nLast) >> 1;
	while ( nFirst < nLast )
	{
		if ( _dTime <= m_vTimePoints.at(nMid) )
			nLast = nMid;
		else
			nFirst = nMid + 1;
		nMid = (unsigned)(nFirst + nLast) >> 1;
	}
	if( ( m_vTimePoints[nLast] == _dTime ) || ( !_bIsStrict ) )
		return nLast;
	else
		return -1;
}

sFraction* CMDMatrix::IitialiseDimension(unsigned _nSize) const
{
	sFraction *pFraction = new sFraction[_nSize];
	for( unsigned i=0; i<_nSize; ++i )
		pFraction[i].pNext = NULL;

	return pFraction;
}

bool CMDMatrix::IncrementCoords(std::vector<unsigned>& _vCoords, const std::vector<unsigned>& _vSizes) const
{
	if( _vCoords.size() == 0 )
		return false;

	unsigned nFactor = 0;
	if( _vCoords.size()+1 == _vSizes.size() )
		nFactor = 2;
	else if( _vCoords.size() == _vSizes.size() )
		nFactor = 1;
	else
		return false;

	unsigned iCoord = (unsigned)_vCoords.size();
	unsigned iDims = (unsigned)_vSizes.size();
	iCoord--;
	iDims -= nFactor;

	while( true )
	{
		_vCoords[iCoord]++;
		if( _vCoords[iCoord] >= _vSizes[iDims] )
		{
			if( iCoord == 0 )
				return false;
			_vCoords[iCoord] = 0;
			iCoord--;
			iDims--;
		}
		else
			return true;
	}
}

bool CMDMatrix::SortMatrix(double _dSrcTime, double _dDstTime, CMDMatrix& _dstMatrix)
{
	if( m_vDimensions.size() != _dstMatrix.m_vDimensions.size() ) // dimensions are not similar in length
		return false;

	for( unsigned i=0; i<m_vDimensions.size(); ++i )
	{
		unsigned j=0;
		for( unsigned j=0; j<_dstMatrix.m_vDimensions.size(); ++j )
			if( ( m_vDimensions[i] == _dstMatrix.m_vDimensions[j] ) && ( m_vClasses[i] == _dstMatrix.m_vClasses[j] ) )
				break;
		if( j == _dstMatrix.m_vDimensions.size() ) // dimensions are not similar in types or numbers of classes
			return false;
	}

	// main loop of sorting
	bool bRes;
	std::vector<unsigned> vDims;
	std::vector<unsigned> vClasses;
	std::vector<unsigned> vCoords;
	std::vector<double> vRes;
	for( unsigned i=0; i<_dstMatrix.m_vDimensions.size(); ++i )
	{
		vDims.push_back( _dstMatrix.m_vDimensions[i] );
		vClasses.push_back( _dstMatrix.m_vClasses[i] );
		do
		{
			this->GetVectorValue( _dSrcTime, vDims, vCoords, vRes );
			_dstMatrix.SetVectorValue( _dDstTime, vDims, vCoords, vRes );
			bRes = IncrementCoords( vCoords, vClasses );
		}
		while( bRes );
		std::fill( vCoords.begin(), vCoords.end(), 0 ); // zeroize coords
		vCoords.push_back( 0 );
	}
	return true;
}

bool CMDMatrix::SortMatrix( CMDMatrix& _dstMatrix )
{
	if( m_vDimensions.size() != _dstMatrix.m_vDimensions.size() ) // dimensions are not similar in length
		return false;

	for( unsigned i=0; i<m_vDimensions.size(); ++i )
	{
		unsigned j=0;
		for( unsigned j=0; j<_dstMatrix.m_vDimensions.size(); ++j )
			if( ( m_vDimensions[i] == _dstMatrix.m_vDimensions[j] ) && ( m_vClasses[i] == _dstMatrix.m_vClasses[j] ) )
				break;
		if( j == _dstMatrix.m_vDimensions.size() ) // dimensions are not similar in types or numbers of classes
			return false;
	}

	// main loop of sorting
	for( unsigned k=0; k<m_vTimePoints.size(); ++k )
	{
		double dTime = m_vTimePoints[k];
		_dstMatrix.AddTimePoint( dTime );
		bool bRes;
		std::vector<unsigned> vDims;
		std::vector<unsigned> vClasses;
		std::vector<unsigned> vCoords;
		std::vector<double> vRes;
		for( unsigned i=0; i<_dstMatrix.m_vDimensions.size(); ++i )
		{
			vDims.push_back( _dstMatrix.m_vDimensions[i] );
			vClasses.push_back( _dstMatrix.m_vClasses[i] );
			do
			{
				if( this->GetVectorValue( dTime, vDims, vCoords, vRes ) )
					_dstMatrix.SetVectorValue( dTime, vDims, vCoords, vRes );
				bRes = IncrementCoords( vCoords, vClasses );
			}
			while( bRes );
			std::fill( vCoords.begin(), vCoords.end(), 0 ); // zeroize coords
			vCoords.push_back( 0 );
		}
	}
	return true;
}

void CMDMatrix::DeleteDimsWithSort(const std::vector<unsigned>& _vDims, std::vector<unsigned>& _vNewDims, std::vector<unsigned>& _vNewClasses, CMDMatrix& _sortMatr)
{
	// get new sequence of dimensions for sorting
	std::vector<unsigned> vDims;
	std::vector<unsigned> vClasses;
	std::vector<unsigned> vDimsToDelete;
	for( unsigned i=0; i<m_vDimensions.size(); ++i )
	{
		unsigned j;
		for( j=0; j<_vDims.size(); ++j )
			if( m_vDimensions[i] == _vDims[j] )
			{
				vDimsToDelete.push_back( m_vDimensions[i] );
				break;
			}
			if( j == _vDims.size() )
			{
				_vNewDims.push_back( m_vDimensions[i] );
				_vNewClasses.push_back( m_vClasses[i] );
				vDims.push_back( m_vDimensions[i] );
				vClasses.push_back( m_vClasses[i] );
			}
	}
	if( vDims.size() != m_vDimensions.size() ) // add missing dimensions
		for( unsigned i=0; i<m_vDimensions.size(); ++i )
		{
			unsigned j=0;
			for( j=0; j<_vDims.size(); ++j )
				if( _vDims[j] == m_vDimensions[i] )
				{
					vDims.push_back( m_vDimensions[i] );
					vClasses.push_back( m_vClasses[i] );
					break;
				}
		}

	//bool bLastCacheState = m_bCacheEnabled;
	//bool bLastCacheStateSort = _sortMatr.m_bCacheEnabled;
	//if( !m_vTimePoints.empty() )
	//	UnCacheData( m_vTimePoints.front(), m_vTimePoints.back() );
	//if( !_sortMatr.m_vTimePoints.empty() )
	//	_sortMatr.UnCacheData( _sortMatr.m_vTimePoints.front(), _sortMatr.m_vTimePoints.back() );
	//m_bCacheEnabled = false;
	//_sortMatr.m_bCacheEnabled = false;

	// sorting according to vDims
	_sortMatr.SetDimensions( vDims, vClasses );
	/* bool bResSort = */ SortMatrix( _sortMatr );

	std::vector<unsigned> vCoord( _vNewDims.size(), 0 );
	bool bRes;
	do
	{
		sFraction *pFrac = _sortMatr.m_data;
		for( unsigned i=0; i<vCoord.size(); ++i )
			if( pFrac != NULL )
				pFrac = pFrac[ vCoord[i] ].pNext;
		if( pFrac != NULL )
		{
			if( !m_vTimePoints.empty() )
				UnCacheData(m_vTimePoints.front(), m_vTimePoints.back());
			_sortMatr.RemoveFractionsRecursive( pFrac, (unsigned)_vNewDims.size() );
			CheckCacheNeed();
		}

		bRes = _sortMatr.IncrementCoords( vCoord, _vNewClasses );
	}
	while( bRes );

	for( unsigned j=0; j<vDimsToDelete.size(); ++j )
		for( unsigned i=0; i<_sortMatr.m_vDimensions.size(); ++i )
			if( _sortMatr.m_vDimensions[i] == vDimsToDelete[j] ) // remove dimension from m_vDimensions
			{
				_sortMatr.m_vDimensions.erase( _sortMatr.m_vDimensions.begin() + i );
				break;
			}

	//m_bCacheEnabled = bLastCacheState;
	//_sortMatr.m_bCacheEnabled = bLastCacheStateSort;
	//CheckCacheNeed();
	//_sortMatr.CheckCacheNeed();
}

void CMDMatrix::NormalizeWrapper( const std::vector<double>& _vTimes )
{
	if( m_vDimensions.empty() )
		return;

	NormalizeMatrixBySumRecursive( m_data );

	//m_vTempDims.clear();
	//m_vTempDims.push_back( m_vDimensions.front() );
	//m_vTempCoords.clear();
	for( unsigned i=0; i<_vTimes.size(); ++i )
	{
		m_dTempT1 = _vTimes[i];
		bool bAllAreZeroes = NormalizeMatrixRecursive( m_data );
		if( bAllAreZeroes )
			for( unsigned j=0; j<m_vClasses.front(); ++j )
				m_data[j].tdArray.SetValue( m_dTempT1, 1/m_vClasses.front() );
	}
}

sFraction* CMDMatrix::CopyFractionsRecursive(sFraction *_pSrc, unsigned _nNesting /*= 0 */)
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pSrc == NULL ) )
		return NULL;

	sFraction *pDst = IitialiseDimension( m_vClasses[_nNesting] );
	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		if( _pSrc[i].pNext != NULL )
			pDst[i].pNext = CopyFractionsRecursive( _pSrc[i].pNext, _nNesting+1 );
		else
			pDst[i].pNext = NULL;
		pDst[i].tdArray = _pSrc[i].tdArray;
	}
	return pDst;
}

sFraction* CMDMatrix::RemoveFractionsRecursive( sFraction *_pFraction, unsigned _nNesting /*= 0 */)
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pFraction == NULL ) )
		return NULL;

	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
		if( ( &_pFraction[i] != NULL ) && ( _pFraction[i].pNext != NULL ) )
			_pFraction[i].pNext = RemoveFractionsRecursive( _pFraction[i].pNext, _nNesting+1 );

	delete[] _pFraction;
	_pFraction = NULL;

	return _pFraction;
}

void CMDMatrix::AddTimePointRecursive(sFraction *_pFraction, unsigned _nNesting /*= 0 */)
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pFraction == NULL ) )
		return;

	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		_pFraction[i].tdArray.AddTimePoint( m_dTempT1, m_dTempT2 ); // add time point
		if( _pFraction[i].pNext != NULL ) // go to the next dimension
			AddTimePointRecursive( _pFraction[i].pNext, _nNesting+1 );
	}
}

void CMDMatrix::ChangeTimePointRecursive(sFraction *_pFraction, unsigned _nNesting /*= 0 */)
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pFraction == NULL ) )
		return;

	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		_pFraction[i].tdArray.ChangeTimePoint( m_dTempT1, m_dTempT2 ); // change time point
		if( _pFraction[i].pNext != NULL ) // go to the next dimension
			ChangeTimePointRecursive( _pFraction[i].pNext, _nNesting+1 );
	}
}

sFraction* CMDMatrix::RemoveTimePointsRecursive(sFraction *_pFraction, unsigned _nNesting )
{
	if(( _nNesting < m_vDimensions.size() ) && ( _pFraction != NULL ))
	{
		unsigned cnt = 0;
		for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
		{
			if( _pFraction[i].pNext != NULL ) // go to the next dimension
				_pFraction[i].pNext = RemoveTimePointsRecursive( _pFraction[i].pNext, _nNesting+1 );

			// remove time points itself
			if( m_dTempT2 == -1 )
				_pFraction[i].tdArray.RemoveTimePoint( m_dTempT1 );
			else
				_pFraction[i].tdArray.RemoveTimePoints( m_dTempT1, m_dTempT2 );

			if( _pFraction[i].tdArray.IsEmpty() )
				cnt++;
		}

		//if( cnt == m_vClasses[_nNesting] ) // delete this fraction and all lower at all
		//	if( _pFraction != NULL )
		//		_pFraction = RemoveFractionsRecursive( _pFraction, _nNesting );
	}
	return _pFraction;
}

double CMDMatrix::GetValueRecursive(sFraction *_pFraction, unsigned _nLevel /*= 1*/, unsigned _nNesting /*= 0 */) const
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pFraction == NULL ) )
		return 0;

	// find current index of dimension
	unsigned index;
	for( index=0; index<m_vTempDims.size(); ++index )
		if( m_vTempDims[index] == m_vDimensions[_nNesting] )
			break;

	if( index == m_vTempDims.size() ) // skip this dimension
	{
		if( _nNesting+1 == m_vDimensions.size() ) // last level
			return 0; // no such coordinates

		double dResult = 0;
		for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
			if( _pFraction[i].pNext != NULL ) // go to the next dimension
				dResult += _pFraction[i].tdArray.GetValue( m_dTempT1 ) * GetValueRecursive( _pFraction[i].pNext, _nLevel, _nNesting+1 );
		return dResult;
	}

	// check out of bounds
	if( m_vTempCoords[index] >= m_vClasses[_nNesting] )
		return 0;

	unsigned iClassIndex = m_vTempCoords[index];

	if( _nLevel == m_vTempDims.size() ) // the search is over
		return _pFraction[iClassIndex].tdArray.GetValue( m_dTempT1 );
	else
	{
		if( _nNesting+1 == m_vDimensions.size() ) // last level
			return 0; // no such coordinates

		// go to the next dimension
		if( _pFraction[iClassIndex].pNext != NULL )
			return _pFraction[iClassIndex].tdArray.GetValue( m_dTempT1 ) * GetValueRecursive( _pFraction[iClassIndex].pNext, _nLevel+1, _nNesting+1 );
		else
			return 0;
	}
}

bool CMDMatrix::SetValueRecursive(sFraction *_pFraction, bool _bExternal /*= true*/, unsigned _nLevel /*= 1*/, unsigned _nNesting /*= 0 */ )
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pFraction == NULL ) )
		return false;

	// find current index of dimension
	unsigned index;
	for ( index=0; index<m_vTempDims.size(); ++index )
		if ( m_vTempDims[index] == m_vDimensions[_nNesting] )
			break;

	if( index == m_vTempDims.size() ) // skip this dimension
	{
		if( _nNesting+1 == m_vDimensions.size() ) // last level
			return false; // no such coordinates

		bool bFlag = false;
		for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
		{
			// create next dimension
			if( _pFraction[i].pNext == NULL )
			{
				if( _pFraction[i].tdArray.GetValue( m_dTempT1 ) != 0 ) // cannot set value if previous dimension is not set
					_pFraction[i].pNext = IitialiseDimension( m_vClasses[_nNesting+1] );
				else
					continue;
			}
			// go to the next dimension
			if( !SetValueRecursive( _pFraction[i].pNext, _bExternal, _nLevel, _nNesting+1 ) )
				return false;
			else
				bFlag = true;
		}
		return bFlag;
	}

	// check out of bounds
	if( m_vTempCoords[index] >= m_vClasses[_nNesting] )
		return false;

	unsigned iClassIndex = m_vTempCoords[index];

	if( _nLevel == m_vTempDims.size() ) // the search is over
	{
		double dOldValue = _pFraction[iClassIndex].tdArray.GetValue( m_dTempT1 );

		if( ( _bExternal ) && ( dOldValue == 0 ) && ( m_dTempValue != 0 ) && ( _nNesting < m_vDimensions.size() - 1 ) )
		{
			std::vector<double> vLowDistr( m_vClasses[_nNesting+1] );
			double dSum = 0;
			for( unsigned j=0; j<m_vClasses[_nNesting]; ++j )
			{
				for( unsigned k=0; k<m_vClasses[_nNesting+1]; ++k )
				{
					if( _pFraction[j].pNext != NULL )
					{
						double dCurr = _pFraction[j].tdArray.GetValue( m_dTempT1 ) * _pFraction[j].pNext[k].tdArray.GetValue( m_dTempT1 );
						vLowDistr[k] += dCurr;
						dSum += dCurr;
					}
				}
			}
			if( dSum != 0 )
			{
				_pFraction[iClassIndex].tdArray.SetValue( m_dTempT1, m_dTempValue );
				if( _pFraction[iClassIndex].pNext == NULL )
					_pFraction[iClassIndex].pNext = IitialiseDimension( m_vClasses[_nNesting+1] );
				// set disrtibution to lower fraction
				for( unsigned j=0; j<m_vClasses[_nNesting+1]; ++j )
					_pFraction[iClassIndex].pNext[j].tdArray.SetValue( m_dTempT1, vLowDistr[j]/dSum );
				if( _nNesting+1 < m_vClasses.size() - 1 )
					for( unsigned j=0; j<m_vClasses[_nNesting+1]; ++j )
						_pFraction[iClassIndex].pNext[j].pNext = SetToOneRecursive( _pFraction[iClassIndex].pNext[j].pNext, _nNesting+2 );
			}
			else
			{
				_pFraction[iClassIndex].tdArray.SetValue( m_dTempT1, m_dTempValue );
				_pFraction[iClassIndex].pNext = SetToOneRecursive( _pFraction[iClassIndex].pNext, _nNesting+1 );
			}
		}
		else if( ( _bExternal ) && ( dOldValue != 0 ) && ( m_dTempValue == 0 ) && ( _nNesting < m_vDimensions.size() - 1 ) )
		{
			std::vector<double> vLowDistr( m_vClasses[_nNesting+1] );
			//double dSum = 0;
			for( unsigned j=0; j<m_vClasses[_nNesting]; ++j )
			{
				for( unsigned k=0; k<m_vClasses[_nNesting+1]; ++k )
				{
					if( _pFraction[j].pNext != NULL )
					{
						double dCurr = _pFraction[j].tdArray.GetValue( m_dTempT1 ) * _pFraction[j].pNext[k].tdArray.GetValue( m_dTempT1 );
						vLowDistr[k] += dCurr;
						//dSum += dCurr;
					}
				}
			}
			for( unsigned j=0; j<m_vClasses[_nNesting]; ++j )
			{
				if( j != iClassIndex )
				{
					if( _pFraction[j].pNext == NULL )
						_pFraction[j].pNext = IitialiseDimension( m_vClasses[_nNesting+1] );
					for( unsigned k=0; k<m_vClasses[_nNesting+1]; ++k )
						_pFraction[j].pNext[k].tdArray.SetValue( m_dTempT1, vLowDistr[k] );
				}
			}
			_pFraction[iClassIndex].tdArray.SetValue( m_dTempT1, m_dTempValue );
			_pFraction[iClassIndex].pNext = SetToZeroRecursive( _pFraction[iClassIndex].pNext, _nNesting+1 );
		}
		else
		{
			if( _nNesting != 0 ) // not compounds
			{
				if( _nNesting < m_vDimensions.size() - 1 )
				{
					std::vector<double> vLowDistr( m_vClasses[_nNesting+1] );
					for( unsigned j=0; j<m_vClasses[_nNesting]; ++j )
					{
						for( unsigned k=0; k<m_vClasses[_nNesting+1]; ++k )
						{
							if( _pFraction[j].pNext != NULL )
							{
								double dCurr = _pFraction[j].tdArray.GetValue( m_dTempT1 ) * _pFraction[j].pNext[k].tdArray.GetValue( m_dTempT1 );
								vLowDistr[k] += dCurr;
							}
						}
					}
					m_vTempDims.push_back( m_vDimensions[_nNesting+1] );
					m_vTempCoords.push_back( 0 );
					double dTemp = m_dTempValue;
					for( unsigned j=0; j<m_vClasses[_nNesting]; ++j )
					{
						for( unsigned k=0; k<m_vClasses[_nNesting+1]; ++k )
							if( _pFraction[j].pNext != NULL )
							{
								m_vTempCoords.back() = k;
								if( _pFraction[j].pNext[k].tdArray.GetValue( m_dTempT1 ) != 0 )
								{
									m_dTempValue = vLowDistr[k];
									SetValueRecursive( _pFraction[j].pNext, true, (unsigned)m_vTempDims.size(), _nNesting+1 );
								}
									//_pFraction[j].pNext[k].tdArray.SetValue( m_dTempT1, vLowDistr[k] );
							}
					}
					m_vTempDims.pop_back();
					m_vTempCoords.pop_back();
					m_dTempValue = dTemp;
				}
			}
			_pFraction[iClassIndex].tdArray.SetValue( m_dTempT1, m_dTempValue );
		}
	}
	else
	{
		if( _nNesting+1 == m_vDimensions.size() ) // last level
			return false; // no such coordinates

		// create next dimension
		if( _pFraction[iClassIndex].pNext == NULL )
		{
			if( _pFraction[iClassIndex].tdArray.GetValue( m_dTempT1 ) != 0 ) // cannot set value if previous dimension is not set
				_pFraction[iClassIndex].pNext = IitialiseDimension( m_vClasses[_nNesting+1] );
			else
				return false;
		}
		// go to the next dimension
		double dFactor = _pFraction[iClassIndex].tdArray.GetValue( m_dTempT1 );
		if( dFactor != 0 )
			m_dTempValue /= dFactor; // correcting data
		if( !SetValueRecursive( _pFraction[iClassIndex].pNext, _bExternal, _nLevel+1, _nNesting+1 ) )
			return false;
	}
	return true;
}

bool CMDMatrix::GetVectorValueRecursive(sFraction *_pFraction, std::vector<double>& _vRes, unsigned _nNesting /*= 0 */) const
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pFraction == NULL ) )
		return false;

	// find current index of dimension
	unsigned index;
	for ( index=0; index<m_vTempDims.size(); ++index )
		if ( m_vTempDims[index] == m_vDimensions[_nNesting] )
			break;

	if( index == m_vTempDims.size() ) // skip this dimension
		return false;


	if( _nNesting != m_vTempDims.size()-1 ) // go deeper
	{
		// check out of bounds
		if( m_vTempCoords[index] >= m_vClasses[_nNesting] )
			return false;

		unsigned iClassIndex = m_vTempCoords[index];

		if( _nNesting+1 == m_vDimensions.size() ) // last level
			return false; // no such coordinates

		// go to the next dimension
		if( _pFraction[iClassIndex].pNext != NULL )
		{
			bool bRes = GetVectorValueRecursive( _pFraction[iClassIndex].pNext, _vRes, _nNesting+1 );
			if( bRes )
				for( unsigned i=0; i<_vRes.size(); ++i )
					_vRes[i] *= _pFraction[iClassIndex].tdArray.GetValue( m_dTempT1 );
			return bRes;
		}
		else
			return false;
	}
	else // the search is over
	{
		for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
			_vRes.push_back( _pFraction[i].tdArray.GetValue( m_dTempT1 ) );
		return true;
	}
}

bool CMDMatrix::SetVectorValueRecursive(sFraction *_pFraction, unsigned _nNesting /*= 0 */)
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pFraction == NULL ) )
		return false;

	// find current index of dimension
	unsigned index;
	for ( index=0; index<m_vTempDims.size(); ++index )
		if ( m_vTempDims[index] == m_vDimensions[_nNesting] )
			break;

	if( index == m_vTempDims.size() ) // skip this dimension
		return false;


	if( _nNesting != m_vTempDims.size()-1 )
	{
		// check out of bounds
		if( m_vTempCoords[index] >= m_vClasses[_nNesting] )
			return false;

		unsigned iClassIndex = m_vTempCoords[index];

		if( _nNesting+1 == m_vDimensions.size() ) // last level
			return false; // no such coordinates

		// create next dimension
		if( _pFraction[iClassIndex].pNext == NULL )
		{
			if( _pFraction[iClassIndex].tdArray.GetValue( m_dTempT1 ) != 0 ) // cannot set value if previous dimension is not set
				_pFraction[iClassIndex].pNext = IitialiseDimension( m_vClasses[_nNesting+1] );
			else
				return false;
		}
		// go to the next dimension
		double dFactor = _pFraction[iClassIndex].tdArray.GetValue( m_dTempT1 );
		if( dFactor != 0 )
			for( unsigned i=0; i<m_vTempValues.size(); ++i ) // correcting data
				m_vTempValues[i] /= dFactor;
		return SetVectorValueRecursive( _pFraction[iClassIndex].pNext, _nNesting+1 );
	}
	else // the search is over
		for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
			_pFraction[i].tdArray.SetValue( m_dTempT1, m_vTempValues[i] );

	return true;
}

void CMDMatrix::TransformRecurcive(const CTransformMatrix& _TMatr)
{
	std::vector<unsigned> vDims = _TMatr.GetDimensions();
	std::vector<unsigned> vClasses = _TMatr.GetClasses();

	// transformation of required dimension
	std::vector<unsigned> vSrcCoords( vDims.size()-1, 0 );
	std::vector<unsigned> vDestCoords( vDims.size(), 0 );

	// get transforming values
	std::vector<double> vCurrValues;
	bool bIsEmpty = true;
	bool bRes;
	do
	{
		std::vector<double> vRes;
		//if( m_pSortMatr->GetVectorValue( 0, vDims, vSrcCoords, vRes ) )
		m_pSortMatr->GetVectorValue(0, vDims, vSrcCoords, vRes);
		vCurrValues.insert( vCurrValues.end(), vRes.begin(), vRes.end() );
		if( bIsEmpty ) // check if values contains only zeros
		{
			unsigned index;
			for( index=0; index<vRes.size(); ++index )
				if( vRes[index] != 0 )
					break;
			if( index != vRes.size() )
				bIsEmpty = false;
		}
		bRes = IncrementCoords( vSrcCoords, vClasses );
	}
	while( bRes );

	if( !bIsEmpty )
	{
		bool bResDest;
		do // for each value in vCurrValues
		{
			std::fill( vSrcCoords.begin(), vSrcCoords.end(), 0 ); // set coords	to zero

			std::vector<double> vTFactors;
			std::vector<double> vBufTFactors;
			double dNewValue = 0;
			bool bTransRes;
			do
			{
				_TMatr.GetVectorValue( vDims, vSrcCoords, vDims, vDestCoords, vBufTFactors ); // get current factors
				vTFactors.insert( vTFactors.end(), vBufTFactors.begin(), vBufTFactors.end() );
				bTransRes = IncrementCoords( vSrcCoords, vClasses );
			}
			while( bTransRes );

			for (unsigned i = 0; i < vCurrValues.size(); ++i)
			{
				if ((vTFactors[i] != 0) && (vCurrValues[i] != 0))
				{
					dNewValue += vCurrValues[i] * vTFactors[i];
				}
			}

			m_pSortMatr->SetValue( 1, vDims, vDestCoords, dNewValue, false ); // set new value

			// prepare to the next iteration
			bResDest = IncrementCoords( vDestCoords, vClasses );
		}
		while( bResDest );
	}

	if( vDims.size() > 1 ) // transformation of upper dimensions
	{
		CTransformMatrix newTMatr;
		_TMatr.ReduceLastDim( newTMatr );
		TransformRecurcive( newTMatr );
	}
}

void CMDMatrix::NormalizeMatrixBySumRecursive(sFraction *_pFraction, unsigned _nNesting)
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pFraction == NULL ) )
		return;

	// get vectors of values
	std::vector<std::vector<double>> vValues( m_vClasses[_nNesting], std::vector<double>( m_vTempValues.size() ) );
	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		if( _pFraction[i].pNext != NULL )
			NormalizeMatrixBySumRecursive( _pFraction[i].pNext, _nNesting+1 );
		_pFraction[i].tdArray.GetVectorValue( m_vTempValues, vValues[i] );
	}


	for( unsigned i=0; i<m_vTempValues.size(); ++i )
	{
		// get sum of elements in level
		double dSum = 0;
		for( unsigned j=0; j<m_vClasses[_nNesting]; ++j )
			dSum += vValues[j][i];

		if( ( dSum != 0 ) && ( dSum != 1 ) ) // normalize
			for( unsigned j=0; j<m_vClasses[_nNesting]; ++j )
				_pFraction[j].tdArray.SetValue( m_vTempValues[i], vValues[j][i] / dSum );
	}
}


/// m_dTempT1 = time
/// m_vTempDims = m_vDimensions[0]
/// m_vTempCoords - empty
bool CMDMatrix::NormalizeMatrixRecursive(sFraction *_pFraction, unsigned _nNesting)
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pFraction == NULL ) )
		return false;

	//if( _nNesting < m_vClasses.size() - 1 )
	//	m_vTempDims.push_back( m_vDimensions[_nNesting+1] );
	//m_vTempCoords.push_back( 0 );

	std::vector<double> vValues;
	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
		vValues.push_back( _pFraction[i].tdArray.GetValue( m_dTempT1 ) );

	//bool bAllBelowAreZero = false;
	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		//m_vTempCoords.back() = i;
		bool bLowerZero = false;
		if( _pFraction[i].pNext != NULL )
			bLowerZero = NormalizeMatrixRecursive( _pFraction[i].pNext, _nNesting+1 );
		else if( _nNesting < m_vClasses.size() - 1 )
			bLowerZero = true;

		if( ( bLowerZero ) && ( vValues[i] != 0 ) ) // lower fractions all are zero AND this cell is not zero
		{
			// get lower distribution
			std::vector<double> vLowDistr( m_vClasses[_nNesting+1] );
			double dSum = 0;
			for( unsigned j=0; j<m_vClasses[_nNesting]; ++j )
			{
				for( unsigned k=0; k<m_vClasses[_nNesting+1]; ++k )
				{
					if( _pFraction[j].pNext != NULL )
					{
						double dCurr = _pFraction[j].tdArray.GetValue( m_dTempT1 ) * _pFraction[j].pNext[k].tdArray.GetValue( m_dTempT1 );
						vLowDistr[k] += dCurr;
						dSum += dCurr;
					}
				}
			}
			//GetVectorValueRecursive( m_data, vLowDistr );


			//if( dSum != 0 ) // not all are zeros
			//{
			//	if( _pFraction[i].pNext == NULL )
			//		_pFraction[i].pNext = IitialiseDimension( m_vClasses[_nNesting+1] );
			//	// set distribution to lower fraction
			//	for( unsigned j=0; j<m_vClasses[_nNesting+1]; ++j )
			//		_pFraction[i].pNext[j].tdArray.SetValue( m_dTempT1, vLowDistr[j]/dSum );
			//	if( _nNesting+1 < m_vClasses.size() - 1 )
			//		for( unsigned j=0; j<m_vClasses[_nNesting+1]; ++j )
			//			_pFraction[i].pNext[j].pNext = SetToOneRecursive( _pFraction[i].pNext[j].pNext, _nNesting+2 );
			//}
			//else // lower distribution is empty
			//{
			//	//for( unsigned j=0; j<m_vClasses[_nNesting+1]; ++j )
			//	_pFraction[i].pNext = SetToOneRecursive( _pFraction[i].pNext, _nNesting+1 );
			//}

			if( dSum != 0 ) // not all are zeros
			{
				if( _pFraction[i].pNext == NULL )
					_pFraction[i].pNext = IitialiseDimension( m_vClasses[_nNesting+1] );
				// set distribution to lower fraction
				for( unsigned j=0; j<m_vClasses[_nNesting+1]; ++j )
					_pFraction[i].pNext[j].tdArray.SetValue( m_dTempT1, vLowDistr[j]/dSum );
				if( _nNesting+1 < m_vClasses.size() - 1 )
					for( unsigned j=0; j<m_vClasses[_nNesting+1]; ++j )
						_pFraction[i].pNext[j].pNext = SetToOneRecursive( _pFraction[i].pNext[j].pNext, _nNesting+2 );
			}
			else // lower distribution is empty
			{
				_pFraction[i].pNext = SetToZeroRecursive( _pFraction[i].pNext, _nNesting+1 );
			}
		}
		else if( ( !bLowerZero ) && ( vValues[i] == 0 ) ) // lower fractions are not zero AND this cell is zero
		{
			//for( unsigned j=0; j<m_vClasses[_nNesting+1]; ++j )
			_pFraction[i].pNext = SetToZeroRecursive( _pFraction[i].pNext, _nNesting+1 );
		}
	}

	double dSum = 0;
	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		dSum += vValues[i];
		if( dSum != 0 )
			break;
	}

	//if( ( dSum != 0 ) && ( dSum != 1 ) ) // normalize
	//	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	//		_pFraction[i].tdArray.SetValue( m_dTempT1, vValues[i] / dSum );

	//if( _nNesting < m_vClasses.size() - 1 )
	//	m_vTempDims.pop_back();
	//m_vTempCoords.pop_back();

	return dSum == 0;
}

bool CMDMatrix::CheckNormalizationRecursive( sFraction *_pFraction, unsigned _nNesting /*= 0*/ )
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pFraction == NULL ) )
		return true;

	// get vectors of values
	std::vector<std::vector<double>> vValues( m_vClasses[_nNesting], std::vector<double>( m_vTempValues.size() ) );
	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		if( _pFraction[i].pNext != NULL )
			if( !CheckNormalizationRecursive( _pFraction[i].pNext, _nNesting+1 ) )
				return false;
		_pFraction[i].tdArray.GetVectorValue( m_vTempValues, vValues[i] );
	}


	for( unsigned i=0; i<m_vTempValues.size(); ++i )
	{
		// get sum of elements in level
		double dSum = 0;
		for( unsigned j=0; j<m_vClasses[_nNesting]; ++j )
			dSum += vValues[j][i];

		const double dCompValue = m_dMinFraction == 0.0 ? 1e-8 : m_dMinFraction * 10;
		if (dSum != 0.0 && std::fabs(1 - dSum) > dCompValue)
			return false;
	}

	return true;
}

sFraction* CMDMatrix::CopyFromRecursive(sFraction *_pDest, sFraction *_pSource, unsigned _nNesting)
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pSource == NULL ) )
		return _pDest;

	sFraction *pDest;
	if( _pDest == NULL )
		pDest = IitialiseDimension( m_vClasses[_nNesting] );
	else
		pDest = _pDest;
	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		if( _pSource[i].pNext != NULL )
			pDest[i].pNext = CopyFromRecursive( pDest[i].pNext, _pSource[i].pNext, _nNesting+1 );
		else if (pDest[i].pNext) // no values in source, but something in destination
			SetToZero2Recursive(pDest[i].pNext, _nNesting + 1);
		pDest[i].tdArray.CopyFrom( _pSource[i].tdArray, m_dTempT1, m_dTempT2 );
	}
	return pDest;
}

sFraction* CMDMatrix::CopyFromTimePointRecursive(sFraction *_pDest, sFraction *_pSource, unsigned _nNesting)
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pSource == NULL ) )
		return _pDest;

	sFraction *pDest;
	if( _pDest == NULL )
		pDest = IitialiseDimension( m_vClasses[_nNesting] );
	else
		pDest = _pDest;
	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		if( _pSource[i].pNext != NULL )
			pDest[i].pNext = CopyFromTimePointRecursive( pDest[i].pNext, _pSource[i].pNext, _nNesting+1 );
			//pDest[i].pNext = CopyFromRecursive( pDest[i].pNext, _pSource[i].pNext, _nNesting+1 );
		else if (pDest[i].pNext) // no values in source, but something in destination
			SetToZero2Recursive(pDest[i].pNext, _nNesting + 1);
		pDest[i].tdArray.CopyFromTimePoint( _pSource[i].tdArray, m_dTempT1, m_dTempT2 );
	}
	return pDest;
}

sFraction* CMDMatrix::AddClassRecursive(sFraction *_pFraction, unsigned _nDimIndex, unsigned _nNesting /*= 0 */)
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pFraction == NULL ) )
		return NULL;

	if( _nNesting == _nDimIndex )
	{
		sFraction *pNewFraction = IitialiseDimension( m_vClasses[_nDimIndex]+1 );
		for( unsigned i=0; i<m_vClasses[_nDimIndex]; ++i )
		{
			pNewFraction[i].tdArray = _pFraction[i].tdArray;
			pNewFraction[i].pNext = _pFraction[i].pNext;
		}
		delete[] _pFraction;
		_pFraction = NULL;
		return pNewFraction;
	}
	else
	{
		for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
			if( _pFraction[i].pNext != NULL ) // go to the next dimension
				_pFraction[i].pNext = AddClassRecursive( _pFraction[i].pNext, _nDimIndex, _nNesting+1 );
	}

	return _pFraction;
}

sFraction* CMDMatrix::RemoveClassRecursive(sFraction *_pFraction, unsigned _nDimIndex, size_t _nClassIndex, unsigned _nNesting /*= 0 */)
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pFraction == NULL ) )
		return NULL;

	if( _nNesting == _nDimIndex )
	{
		sFraction *pNewFraction = IitialiseDimension( m_vClasses[_nDimIndex]-1 );
		size_t index = 0;
		for( unsigned i=0; i<m_vClasses[_nDimIndex]; ++i )
		{
			if( index == _nClassIndex )
			{
				_pFraction[index].pNext = RemoveFractionsRecursive( _pFraction[i].pNext, _nNesting + 1);
			}
			else
			{
				pNewFraction[index].tdArray = _pFraction[i].tdArray;
				pNewFraction[index].pNext = _pFraction[i].pNext;
				index++;
			}
		}
		delete[] _pFraction;
		_pFraction = NULL;
		return pNewFraction;
	}
	else
	{
		for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
			if( _pFraction[i].pNext != NULL ) // go to the next dimension
				_pFraction[i].pNext = RemoveClassRecursive( _pFraction[i].pNext, _nDimIndex, _nClassIndex, _nNesting+1 );
	}

	return _pFraction;
}

void CMDMatrix::CompressDataRecursive( sFraction *_pFraction, double _dATol, double _dRTol, unsigned _nNesting )
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pFraction == NULL ) )
		return;

	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		_pFraction[i].tdArray.CompressData( m_dTempT1, m_dTempT2, _dATol, _dRTol ); // compress data
		if( _pFraction[i].pNext != NULL ) // go to the next dimension
			CompressDataRecursive( _pFraction[i].pNext, _dATol, _dRTol, _nNesting+1 );
	}
}

//void CMDMatrix::NormalizeToOneRecursive( sFraction *_pFraction, unsigned _nNesting /*= 0*/ )
//{
//	if( _nNesting >= m_vDimensions.size() )
//		return;
//
//	if( _pFraction == NULL )
//	{
//		_pFraction = CreateWithOneRecursive( _pFraction, _nNesting );
//		return;
//	}
//
//	_pFraction[0].tdArray.SetValue( m_dTempT1, 1 );
//	//double dSum = 0;
//	//for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
//	//{
//	//	dSum += _pFraction[i].tdArray.GetValue( m_dTempT1 );
//	//	if( dSum > 0 )
//	//		break;
//	//}
//
//	//if( dSum == 0 )
//	//{
//
//	//}
//
//	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
//		NormalizeToOneRecursive( _pFraction[i].pNext, _nNesting+1 );
//}
//
//void CMDMatrix::NormalizeToZeroRecursive( sFraction *_pFraction, unsigned _nNesting /*= 0*/ )
//{
//	if( ( _nNesting >= m_vDimensions.size() ) || ( _pFraction == NULL ) )
//		return;
//
//	_pFraction[0].tdArray.SetValue( m_dTempT1, 1 );
//	//double dSum = 0;
//	//for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
//	//{
//	//	dSum += _pFraction[i].tdArray.GetValue( m_dTempT1 );
//	//	if( dSum > 0 )
//	//		break;
//	//}
//
//	//if( dSum == 0 )
//	//{
//
//	//}
//
//	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
//		_pFraction[i].pNext = NormalizeToZeroRecursive( _pFraction[i].pNext, _nNesting+1 );
//}
//
//sFraction* CMDMatrix::CreateWithOneRecursive( sFraction *_pFraction, unsigned _nNesting /*= 0*/ )
//{
//	if( _nNesting >= m_vDimensions.size() )
//		return NULL;
//	if( _pFraction != NULL )
//		return _pFraction;
//
//	_pFraction = IitialiseDimension( m_vClasses[_nNesting] );
//	_pFraction[0].tdArray.SetValue( m_dTempT1, 1 );
//	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
//		_pFraction[i].pNext = CreateWithOneRecursive( _pFraction[i].pNext, _nNesting+1 );
//	return _pFraction;
//}

sFraction* CMDMatrix::SetToZeroRecursive( sFraction *_pFraction, unsigned _nNesting /*= 0*/ )
{
	if( _nNesting >= m_vDimensions.size() )
		return NULL;

	if( _pFraction == NULL )
		return NULL;
		//_pFraction = IitialiseDimension( m_vClasses[_nNesting] );

	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		_pFraction[i].pNext = SetToZeroRecursive( _pFraction[i].pNext, _nNesting+1 );
		_pFraction[i].tdArray.SetValue( m_dTempT1, 0 );
	}

	return _pFraction;
}

void CMDMatrix::SetToZero2Recursive(sFraction* _pFraction, unsigned _nNesting /*= 0*/)
{
	if (!_pFraction || _nNesting >= m_vDimensions.size()) return;

	for (unsigned i = 0; i < m_vClasses[_nNesting]; ++i)
	{
		SetToZero2Recursive(_pFraction[i].pNext, _nNesting + 1);
		_pFraction[i].tdArray.RemoveTimePoints(m_dTempT1, m_dTempT2);
		_pFraction[i].tdArray.SetValue(m_dTempT1, 0.0);
		_pFraction[i].tdArray.SetValue(m_dTempT2, 0.0);
	}
}

sFraction* CMDMatrix::SetToOneRecursive( sFraction *_pFraction, unsigned _nNesting /*= 0*/ )
{
	if( _nNesting >= m_vDimensions.size() )
		return NULL;

	if( _pFraction == NULL )
		_pFraction = IitialiseDimension( m_vClasses[_nNesting] );

	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		 _pFraction[i].pNext = SetToOneRecursive( _pFraction[i].pNext, _nNesting+1 );
		_pFraction[i].tdArray.SetValue( m_dTempT1, 1./m_vClasses[_nNesting] );
	}

	return _pFraction;
}

void CMDMatrix::Extrapolate2ToPointRecursive( sFraction *_pFraction, unsigned _nNesting /*= 0*/ )
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pFraction == NULL ) )
		return;

	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		if( _pFraction[i].pNext != NULL )
			Extrapolate2ToPointRecursive( _pFraction[i].pNext, _nNesting+1 );
		_pFraction[i].tdArray.ExtrapolateToPoint( m_dTempT1, m_dTempT2, m_dTempValue );
	}
}

void CMDMatrix::Extrapolate3ToPointRecursive( sFraction *_pFraction, unsigned _nNesting /*= 0*/ )
{
	if( ( _nNesting >= m_vDimensions.size() ) || ( _pFraction == NULL ) )
		return;

	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		if( _pFraction[i].pNext != NULL )
			Extrapolate3ToPointRecursive( _pFraction[i].pNext, _nNesting+1 );
		_pFraction[i].tdArray.ExtrapolateToPoint( m_vTempValues[0], m_vTempValues[1], m_vTempValues[2], m_vTempValues[3] );
	}
}

void CMDMatrix::UnCacheData(double _dTP) const
{
	if( !m_bCacheEnabled ) return;

	if( ( m_nNonCachedTPNum == 0 ) && ( m_nCurrOffset == 0 ) ) return;

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
		//std::vector<std::vector<double>> *vvBuf = new std::vector<std::vector<double>>();
		std::vector<std::vector<double>> vvBuf;
		//m_CacheHandler.GetData( _dTP, m_vTempValues, *vvBuf, &m_dCurrWinStart, &m_dCurrWinEnd, &m_nCurrOffset );
		m_cacheHandler.ReadFromCache( _dTP, m_vTempValues, vvBuf, m_dCurrWinStart, m_dCurrWinEnd, m_nCurrOffset );
		m_nCounter = 0;
		m_data = UnCacheDataRecursive( m_data, vvBuf );
		m_nNonCachedTPNum = (unsigned)m_vTempValues.size();
		m_bCacheCoherent = true;
		m_vTempValues.clear();
		//vvBuf->clear();
		//delete vvBuf;
	}
}

void CMDMatrix::UnCacheData(double _dT1, double _dT2) const
{
	if( !m_bCacheEnabled ) return;

	if( ( m_nNonCachedTPNum == 0 ) && ( m_nCurrOffset == 0 ) ) return;

	if( ( m_vTimePoints.size() > 0 ) && ( m_vTimePoints.back() == m_dCurrWinEnd ) && ( ( _dT1 > m_dCurrWinStart ) || ( _dT1 == m_vTimePoints.front() ) ) && ( _dT2 > m_vTimePoints.back() ) )
	{
		m_dCurrWinEnd = m_vTimePoints[m_vTimePoints.size()-1];
		return;
	}
	//if( ( _dT1 <= m_dCurrWinStart ) || ( _dT2 >= m_dCurrWinEnd ) )
	if( ( _dT1 < m_dCurrWinStart ) || ( _dT2 > m_dCurrWinEnd ) )
	//if( ( ( _dT1 <= m_dCurrWinStart ) && ( m_nCurrOffset != 0 ) ) || ( _dT2 >= m_dCurrWinEnd ) )
	{
		FlushToCache();
		//std::vector<std::vector<double>> *vvBuf = new std::vector<std::vector<double>>();
		std::vector<std::vector<double>> vvBuf;
		//m_CacheHandler.GetData( _dT1, _dT2, m_vTempValues, *vvBuf, &m_dCurrWinStart, &m_dCurrWinEnd, &m_nCurrOffset );
		m_cacheHandler.ReadFromCache( _dT1, _dT2, m_vTempValues, vvBuf, m_dCurrWinStart, m_dCurrWinEnd, m_nCurrOffset );
		m_nCounter = 0;
		m_data = UnCacheDataRecursive( m_data, vvBuf );
		m_nNonCachedTPNum = (unsigned)m_vTempValues.size();
		m_bCacheCoherent = true;
		m_vTempValues.clear();
		//vvBuf->clear();
		//delete vvBuf;
	}
}

void CMDMatrix::CheckCacheNeed() const
{
	if( !m_bCacheEnabled ) return;

	while( m_nNonCachedTPNum > m_nCacheWindow*2 )
		CacheData();
}

void CMDMatrix::FlushToCache() const
{
	if( !m_bCacheEnabled ) return;

	if( m_nNonCachedTPNum == 0 ) return;
	//std::vector<std::vector<double>> *vvBufData = new std::vector<std::vector<double>>();
	std::vector<std::vector<double>> vvBufData;
	unsigned cnt = 0, mul = 1;
	for( unsigned j=0; j<m_vDimensions.size(); ++j )
	{
		mul *= m_vClasses[j];
		cnt += mul;
	}
	vvBufData.resize( cnt );
	m_vTempValues.assign( m_vTimePoints.begin() + m_nCurrOffset, m_vTimePoints.begin() + m_nCurrOffset + m_nNonCachedTPNum );
	//std::copy( m_vTimePoints.begin() + m_nCurrOffset, m_vTimePoints.end(), m_vTempValues );
	m_nCounter = 0;
	CacheDataRecursive( m_data, vvBufData );

	//m_CacheHandler.SaveData( m_vTempValues, *vvBufData );
	m_cacheHandler.WriteToCache( m_vTempValues, vvBufData, m_bCacheCoherent );
	m_nCurrOffset = m_vTimePoints.size();
	m_dCurrWinStart = m_dCurrWinEnd = 0;
	m_nNonCachedTPNum = 0;
	//vvBufData->clear();
	//delete vvBufData;
}

void CMDMatrix::CacheData() const
{
	if( !m_bCacheEnabled ) return;

	//std::vector<std::vector<double>> *vvBufData = new std::vector<std::vector<double>>();
	std::vector<std::vector<double>> vvBufData;
	unsigned cnt = 0, mul = 1;
	for( unsigned j=0; j<m_vDimensions.size(); ++j )
	{
		mul *= m_vClasses[j];
		cnt += mul;
	}
	vvBufData.resize( cnt );
	m_vTempValues.assign( m_vTimePoints.begin() + m_nCurrOffset, m_vTimePoints.begin() + m_nCurrOffset + m_nCacheWindow );
	//std::copy( m_vTimePoints.begin() + m_nCurrOffset, m_vTimePoints.begin() + m_nCurrOffset + m_nCacheWindow, m_vTempValues );
	m_nCounter = 0;
	CacheDataRecursive( m_data, vvBufData );

	//m_CacheHandler.SaveData( m_vTempValues, *vvBufData );
	m_cacheHandler.WriteToCache( m_vTempValues, vvBufData, m_bCacheCoherent );
	m_nCurrOffset += m_nCacheWindow;
	m_nNonCachedTPNum -= (unsigned)m_vTempValues.size();
	m_dCurrWinStart = m_vTimePoints[m_nCurrOffset];
	//m_dCurrWinEnd = m_vTimePoints[m_nCurrOffset+m_vTempValues.size()];
	//vvBufData->clear();
	//delete vvBufData;
}

void CMDMatrix::CorrectWinBoundary() const
{
	if( !m_bCacheEnabled ) return;

	if (m_vTimePoints.size() == 1 && m_nNonCachedTPNum == 1)
	{
		m_dCurrWinStart = m_dCurrWinEnd = m_vTimePoints[0];
		return;
	}

	if( ( m_vTimePoints.size() > 1 ) && ( m_dCurrWinEnd == m_vTimePoints[m_vTimePoints.size()-2] ) )
		m_dCurrWinEnd = m_vTimePoints.back();

	if( !m_vTimePoints.empty() )
	{
		if( m_nNonCachedTPNum != 0 )
		{
			if( m_dCurrWinEnd > m_vTimePoints[ m_nCurrOffset - 1 + m_nNonCachedTPNum ] )
				m_dCurrWinEnd = m_vTimePoints[ m_nCurrOffset - 1 + m_nNonCachedTPNum ];
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
		m_nNonCachedTPNum = 0;
		m_nCurrOffset = 0;
	}
}

void CMDMatrix::ClearCache() const
{
	if( !m_bCacheEnabled ) return;

	m_cacheHandler.ClearData();
	m_nNonCachedTPNum = 0;
	m_dCurrWinStart = 0;
	m_dCurrWinEnd = 0;
	m_nCurrOffset = 0;
	m_bCacheCoherent = false;
}

sFraction* CMDMatrix::UnCacheDataRecursive( sFraction *_pFraction, std::vector<std::vector<double>>& _vData, unsigned _nNesting /*= 0*/ ) const
{
	if( _nNesting >= m_vDimensions.size() )
		return NULL;

	bool bFlag = false;
	if( _pFraction == NULL )
	{
		_pFraction = IitialiseDimension( m_vClasses[_nNesting] );
		bFlag = true;
	}

	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		if( !_pFraction[i].tdArray.SetCacheArray( m_vTempValues, _vData[m_nCounter] ) )
		{
			unsigned cnt = 0;
			unsigned mul = 1;
			for( unsigned j=_nNesting; j<m_vDimensions.size(); ++j )
			{
				mul *= m_vClasses[j];
				cnt += mul;
			}
			m_nCounter += cnt;
			if( i == 0 && bFlag )
			{
				delete[] _pFraction;
				return NULL;
			}
			else
				return _pFraction;
		}
		m_nCounter++;
		if( _nNesting+1 != m_vDimensions.size() )
			_pFraction[i].pNext = UnCacheDataRecursive( _pFraction[i].pNext, _vData, _nNesting+1 );
	}
	return _pFraction;
}

void CMDMatrix::CacheDataRecursive( sFraction *_pFraction, std::vector<std::vector<double>>& _vData, unsigned _nNesting /*= 0*/ ) const
{
	if( _nNesting >= m_vDimensions.size() )
		return;

	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		if( _pFraction == NULL )
		{
			unsigned cnt = 0;
			unsigned mul = 1;
			for( unsigned j=_nNesting; j<m_vDimensions.size(); ++j )
			{
				mul *= m_vClasses[j];
				cnt += mul;
			}
			std::vector<double> vTemp( m_vTempValues.size(), -1 );
			for( unsigned j=0; j<cnt; ++j )
				_vData[m_nCounter+j] = vTemp;
			m_nCounter += cnt;
			return;
		}

		_pFraction[i].tdArray.GetCacheArray( m_vTempValues, _vData[m_nCounter] );
		m_nCounter++;
		if( _nNesting+1 != m_vDimensions.size() ) // go to the next dimension
			CacheDataRecursive( _pFraction[i].pNext, _vData, _nNesting+1 );
	}
}

void CMDMatrix::GetDataForSaveRecursive(sFraction *_pFraction, std::vector<std::vector<double>>& _vData, unsigned _nNesting /*= 0 */) const
{
	if( _nNesting >= m_vDimensions.size() )
		return;

	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		if( _pFraction == NULL )
		{
			unsigned cnt = 0;
			unsigned mul = 1;
			for( unsigned j=_nNesting; j<m_vDimensions.size(); ++j )
			{
				mul *= m_vClasses[j];
				cnt += mul;
			}
			std::vector<double> vTemp( 1, -1 );
			for( unsigned j=0; j<cnt; ++j )
				_vData[m_nCounter+j] = vTemp;
			m_nCounter += cnt;
			return;
		}

		_pFraction[i].tdArray.GetDataForSave( m_vTempValues, _vData[m_nCounter] );
		m_nCounter++;
		if( _nNesting+1 != m_vDimensions.size() ) // go to the next dimension
			GetDataForSaveRecursive( _pFraction[i].pNext, _vData, _nNesting+1 );
	}
}

sFraction* CMDMatrix::SetDataForLoadRecursive(sFraction *_pFraction, std::vector<std::vector<double>>& _vData, unsigned _nNesting /*= 0 */) const
{
	if( _nNesting >= m_vDimensions.size() )
		return NULL;

	bool bFlag = false;
	if( _pFraction == NULL )
	{
		_pFraction = IitialiseDimension( m_vClasses[_nNesting] );
		bFlag = true;
	}

	bool bAllZero = true;
	for( unsigned i=0; i<m_vClasses[_nNesting]; ++i )
	{
		if( !_pFraction[i].tdArray.SetDataForLoad( m_vTempValues, _vData[m_nCounter] ) )
		{
			unsigned cnt = 0;
			unsigned mul = 1;
			for( unsigned j=_nNesting+1; j<m_vDimensions.size(); ++j )
			{
				mul *= m_vClasses[j];
				cnt += mul;
			}
			m_nCounter += cnt;
			m_nCounter++;
		}
		else
		{
			bAllZero = false;
			m_nCounter++;
			if( _nNesting+1 != m_vDimensions.size() )
				_pFraction[i].pNext = SetDataForLoadRecursive( _pFraction[i].pNext, _vData, _nNesting+1 );
		}
	}
	if( bFlag && bAllZero )
	{
		delete[] _pFraction;
		_pFraction = NULL;
	}
	return _pFraction;
}

void CMDMatrix::SetCachePath(const std::wstring& _sPath)
{
	m_sCachePath = _sPath;

	m_cacheHandler.SetDirPath( m_sCachePath );
}
