/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "TransformMatrix.h"
#include "DyssolUtilities.h"
#include <cstring>

CTransformMatrix::CTransformMatrix(void)
{
	m_pData = NULL;
	m_nSize = 0;
}

CTransformMatrix::CTransformMatrix( unsigned _nType, unsigned _nClasses )
{
	m_pData = NULL;
	m_nSize = 0;
	SetDimensions( _nType, _nClasses );
}

CTransformMatrix::CTransformMatrix(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2)
{
	m_pData = nullptr;
	m_nSize = 0;
	SetDimensions( _nType1, _nClasses1, _nType2, _nClasses2 );
}

CTransformMatrix::CTransformMatrix( const std::vector<unsigned> &_vTypes, const std::vector<unsigned> &_vClasses )
{
	m_pData = NULL;
	m_nSize = 0;
	SetDimensions( _vTypes, _vClasses );
}

CTransformMatrix::~CTransformMatrix(void)
{
	Clear();
}

bool CTransformMatrix::SetDimensions( unsigned _nType, unsigned _nClasses )
{
	std::vector<unsigned> vDims;
	std::vector<unsigned> vClasses;
	vDims.push_back( _nType );
	vClasses.push_back( _nClasses );
	return SetDimensions( vDims, vClasses );
}

bool CTransformMatrix::SetDimensions( unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2 )
{
	std::vector<unsigned> vDims;
	std::vector<unsigned> vClasses;
	vDims.push_back( _nType1 );
	vDims.push_back( _nType2 );
	vClasses.push_back( _nClasses1 );
	vClasses.push_back( _nClasses2 );
	return SetDimensions( vDims, vClasses );
}

bool CTransformMatrix::SetDimensions( unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2, unsigned _nType3, unsigned _nClasses3 )
{
	std::vector<unsigned> vDims;
	std::vector<unsigned> vClasses;
	vDims.push_back( _nType1 );
	vDims.push_back( _nType2 );
	vDims.push_back( _nType3 );
	vClasses.push_back( _nClasses1 );
	vClasses.push_back( _nClasses2 );
	vClasses.push_back( _nClasses3 );
	return SetDimensions( vDims, vClasses );
}

bool CTransformMatrix::SetDimensions(const std::vector<unsigned>& _vTypes, const std::vector<unsigned>& _vClasses)
{
	if( ( _vTypes.size() == 0 ) || ( _vClasses.size() == 0 ) )	// wrong size of _vDimensions or _vClasses
		return false;

	if( !CheckDuplicates( _vTypes ) )	// duplicates in dimensions
		return false;

	if( _vTypes.size() != _vClasses.size() )	// wrong size of _vDimensions or _vClasses
		return false;

	for( unsigned i=0; i<_vClasses.size(); ++i ) // wrong number of classes
		if( _vClasses[i] == 0 )
			return false;

	if( !m_vDimensions.empty() )	// remove old data
		Clear();

	// save dimension parameters
	m_vDimensions = _vTypes;
	m_vClasses = _vClasses;

	// allocate memory
	m_nSize = 1;
	for (const unsigned count : _vClasses)
		m_nSize *= static_cast<size_t>(count) * static_cast<size_t>(count);
	m_pData = new double[m_nSize]();

	return true;
}

std::vector<unsigned> CTransformMatrix::GetDimensions() const
{
	return m_vDimensions;
}

std::vector<unsigned> CTransformMatrix::GetClasses() const
{
	return m_vClasses;
}

size_t CTransformMatrix::GetDimensionsNumber() const
{
	return m_vDimensions.size();
}

void CTransformMatrix::Clear()
{
	if( m_pData )
		delete[] m_pData;
	m_pData = NULL;
	m_nSize = 0;
	m_vDimensions.clear();
	m_vClasses.clear();
}

void CTransformMatrix::ClearData()
{
	if( m_pData != NULL )
		for( unsigned i=0; i<m_nSize; ++i )
			m_pData[i] = 0;
}

void CTransformMatrix::Normalize()
{
	if (m_vDimensions.empty()) return;

	std::vector<unsigned> vSrcCoord(m_vDimensions.size());
	std::vector<unsigned> vDstCoord(m_vDimensions.size());
	std::vector<double> vValues;
	bool bSrc;
	do
	{
		// get sum
		std::fill(vDstCoord.begin(), vDstCoord.end(), 0);
		vValues.clear();
		double dSum = 0;
		bool bDstGet;
		do
		{
			double dVal = GetValue(vSrcCoord, vDstCoord);
			vValues.push_back(dVal);
			dSum += dVal;
			bDstGet = IncrementCoordsOld(vDstCoord, m_vClasses);
		}
		while (bDstGet);

		// set normalized values
		if ((dSum != 1) && (dSum != 0))
		{
			std::fill(vDstCoord.begin(), vDstCoord.end(), 0);
			unsigned ind = 0;
			bool bDstSet;
			do
			{
				SetValue(vSrcCoord, vDstCoord, vValues[ind] / dSum);
				bDstSet = IncrementCoordsOld(vDstCoord, m_vClasses);
				ind++;
			}
			while (bDstSet);
		}

		bSrc = IncrementCoordsOld(vSrcCoord, m_vClasses);
	}
	while (bSrc);
}

double CTransformMatrix::GetValue( unsigned _nCoordSrc, unsigned _nCoordDst ) const
{
	std::vector<unsigned> vCoordsSrc( 1, _nCoordSrc );
	std::vector<unsigned> vCoordsDst( 1, _nCoordDst );
	return GetValue( vCoordsSrc, vCoordsDst );
}

double CTransformMatrix::GetValue(unsigned _nCoordSrc1, unsigned _nCoordSrc2, unsigned _nCoordDst1, unsigned _nCoordDst2) const
{
	std::vector<unsigned> vCoordsSrc(2);
	vCoordsSrc[0] = _nCoordSrc1;
	vCoordsSrc[1] = _nCoordSrc2;
	std::vector<unsigned> vCoordsDst(2);
	vCoordsDst[0] = _nCoordDst1;
	vCoordsDst[1] = _nCoordDst2;
	return GetValue(vCoordsSrc, vCoordsDst);
}

double CTransformMatrix::GetValue(const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst) const
{
	if( ( _vCoordsSrc.size() != m_vDimensions.size() ) || ( _vCoordsDst.size() != m_vDimensions.size() ) )	// wrong coordinates size
		return -1;

	size_t index = GetIndex( m_vDimensions, _vCoordsSrc, m_vDimensions, _vCoordsDst );
	if( ( index != static_cast<size_t>(-1) ) && ( index < m_nSize ) )
		return m_pData[index];
	else
		return -1;
}

double CTransformMatrix::GetValue(const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
	const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst) const
{
	if( ( _vDimsSrc.size() != m_vDimensions.size() ) || ( _vDimsDst.size() != m_vDimensions.size() ) )	// wrong dimensions size
		return -1;
	if( ( _vCoordsSrc.size() != m_vDimensions.size() ) || ( _vCoordsDst.size() != m_vDimensions.size() ) )	// wrong coordinates size
		return -1;

	size_t index = GetIndex( _vDimsSrc, _vCoordsSrc, _vDimsDst, _vCoordsDst );
	if( ( index != static_cast<size_t>(-1) ) && ( index < m_nSize ) )
		return m_pData[index];
	else
		return -1;
}

bool CTransformMatrix::GetVectorValue(const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst, std::vector<double>& _vResult) const
{
	if (_vCoordsSrc.size() + _vCoordsDst.size() + 1 != m_vDimensions.size() * 2)  // wrong coordinates size
		return false;

	size_t index;
	size_t nStep;
	size_t nVecLength;
	if (!GetIndexAndStep(m_vDimensions, _vCoordsSrc, m_vDimensions, _vCoordsDst, index, nStep, nVecLength))
		return false;

	_vResult.resize(nVecLength);
	for (unsigned i = 0; i < nVecLength; ++i)
		if (index < m_nSize)
		{
			_vResult[i] = m_pData[index];
			index += nStep;
		}
	return true;
}

std::vector<double> CTransformMatrix::GetVectorValue(const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst) const
{
	if (_vCoordsSrc.size() + _vCoordsDst.size() + 1 != m_vDimensions.size() * 2) return {}; // wrong coordinates size

	size_t index;
	size_t step;
	size_t vecLength;
	if (!GetIndexAndStep(m_vDimensions, _vCoordsSrc, m_vDimensions, _vCoordsDst, index, step, vecLength)) return {};

	std::vector<double> res(vecLength);
	for (size_t i = 0; i < vecLength; ++i)
		if (index < m_nSize)
		{
			res[i] = m_pData[index];
			index += step;
		}
	return res;
}

bool CTransformMatrix::GetVectorValue(const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
									  const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst, std::vector<double>& _vResult) const
{
	if ((_vDimsSrc.size() != m_vDimensions.size()) || (_vDimsDst.size() != m_vDimensions.size()))	// wrong dimensions size
		return false;
	if (_vDimsSrc.size() + _vDimsDst.size() != _vCoordsSrc.size() + _vCoordsDst.size() + 1)  // wrong number of coordinates
		return false;

	size_t index;
	size_t nStep;
	size_t nVecLength;
	if (!GetIndexAndStep(_vDimsSrc, _vCoordsSrc, _vDimsDst, _vCoordsDst, index, nStep, nVecLength))
		return false;

	_vResult.resize(nVecLength);
	for (unsigned i = 0; i < nVecLength; ++i)
		if (index < m_nSize)
		{
			_vResult[i] = m_pData[index];
			index += nStep;
		}
	return true;
}

bool CTransformMatrix::SetValue( unsigned _nCoordSrc, unsigned _nCoordDst, double _dValue )
{
	std::vector<unsigned> vCoordsSrc( 1, _nCoordSrc );
	std::vector<unsigned> vCoordsDst( 1, _nCoordDst );
	return SetValue( vCoordsSrc, vCoordsDst, _dValue );
}

bool CTransformMatrix::SetValue( unsigned _nCoordSrc1, unsigned _nCoordSrc2, unsigned _nCoordDst1, unsigned _nCoordDst2, double _dValue )
{
	std::vector<unsigned> vCoordsSrc(2);
	vCoordsSrc[0] = _nCoordSrc1;
	vCoordsSrc[1] = _nCoordSrc2;
	std::vector<unsigned> vCoordsDst(2);
	vCoordsDst[0] = _nCoordDst1;
	vCoordsDst[1] = _nCoordDst2;
	return SetValue( vCoordsSrc, vCoordsDst, _dValue );
}

bool CTransformMatrix::SetValue(const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst, double _dValue)
{
	if( ( _vCoordsSrc.size() != m_vDimensions.size() ) || ( _vCoordsDst.size() != m_vDimensions.size() ) )	// wrong coordinates size
		return false;

	size_t index = GetIndex( m_vDimensions, _vCoordsSrc, m_vDimensions, _vCoordsDst );
	if( ( index == static_cast<size_t>(-1) ) || ( index >= m_nSize ) ) // wrong index
		return false;

	m_pData[index] = _dValue;

	return true;
}

bool CTransformMatrix::SetValue(const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
	const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst, double _dValue)
{
	if( ( _vDimsSrc.size() != m_vDimensions.size() ) || ( _vDimsDst.size() != m_vDimensions.size() ) )	// wrong dimensions size
		return false;
	if( ( _vCoordsSrc.size() != m_vDimensions.size() ) || ( _vCoordsDst.size() != m_vDimensions.size() ) )	// wrong coordinates size
		return false;

	size_t index = GetIndex( _vDimsSrc, _vCoordsSrc, _vDimsDst, _vCoordsDst );
	if( ( index == static_cast<size_t>(-1) ) || ( index >= m_nSize ) ) // wrong index
		return false;

	m_pData[index] = _dValue;

	return true;
}

bool CTransformMatrix::SetVectorValue(const std::vector<unsigned>& _vCoordsSrc, const std::vector<unsigned>& _vCoordsDst, const std::vector<double>& _vValue)
{
	if (m_vDimensions.size() + m_vDimensions.size() != _vCoordsSrc.size() + _vCoordsDst.size() + 1) // wrong number of coordinates
		return false;

	size_t index;
	size_t nStep;
	size_t nVecLength;
	if (!GetIndexAndStep(m_vDimensions, _vCoordsSrc, m_vDimensions, _vCoordsDst, index, nStep, nVecLength))
		return false;

	if (nVecLength != _vValue.size())  // wrong size of values vector
		return false;
	for (unsigned i = 0; i < nVecLength; ++i)
		if (index < m_nSize)
		{
			m_pData[index] = _vValue[i];
			index += nStep;
		}
	return true;
}

bool CTransformMatrix::SetVectorValue(const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
									  const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst, const std::vector<double>& _vValue)
{
	if ((_vDimsSrc.size() != m_vDimensions.size()) || (_vDimsDst.size() != m_vDimensions.size()))	// wrong coordinates size
		return false;
	if (_vDimsSrc.size() + _vDimsDst.size() != _vCoordsSrc.size() + _vCoordsDst.size() + 1) // wrong number of coordinates
		return false;

	size_t index;
	size_t nStep;
	size_t nVecLength;
	if (!GetIndexAndStep(_vDimsSrc, _vCoordsSrc, _vDimsDst, _vCoordsDst, index, nStep, nVecLength))
		return false;

	if (nVecLength != _vValue.size())  // wrong size of values vector
		return false;
	for (unsigned i = 0; i < nVecLength; ++i)
		if (index < m_nSize)
		{
			m_pData[index] = _vValue[i];
			index += nStep;
		}
	return true;
}

CMatrix2D CTransformMatrix::GetMatrix() const
{
	if (m_vDimensions.size() != 1) return {};
	CMatrix2D res(m_vClasses[0], m_vClasses[0]);
	for (size_t i = 0; i < m_vClasses[0]; ++i)
		res.SetCol(i, std::vector<double>{m_pData + i * m_vClasses[0], m_pData + i * m_vClasses[0] + m_vClasses[0]});
	return res;
}

bool CTransformMatrix::SetMatrix(const CMatrix2D& _matrix)
{
	if (m_vDimensions.size() != 1) return false;
	if (m_vClasses[0] != _matrix.Rows() || m_vClasses[0] != _matrix.Cols()) return false;
	for (size_t i = 0; i < _matrix.Cols(); ++i)
		std::memcpy(m_pData + i * m_vClasses[0], _matrix.GetCol(i).data(), sizeof(double)*m_vClasses[0]);
	return true;
}

void CTransformMatrix::ReduceLastDim( CTransformMatrix& _newTMatr ) const
{
	if (m_vDimensions.size() < 2) return; // nothing to reduce

	// setup new matrix
	std::vector<unsigned> vNewDims = m_vDimensions;
	vNewDims.pop_back();
	std::vector<unsigned> vNewClasses = m_vClasses;
	vNewClasses.pop_back();
	_newTMatr.SetDimensions(vNewDims, vNewClasses);

	// prepare all parameters
	const unsigned nLastDimSize = m_vClasses.back();
	std::vector<unsigned> vSrcCoord(vNewDims.size());
	std::vector<unsigned> vDstCoord(vNewDims.size());
	std::vector<unsigned> vSrcCoordFull(m_vDimensions.size());
	std::vector<unsigned> vDstCoordFull(m_vDimensions.size());

	bool bSrc;
	do
	{
		bool bDst;
		do
		{
			double dVal = 0;
			std::copy(vSrcCoord.begin(), vSrcCoord.end(), vSrcCoordFull.begin());
			for (size_t i = 0; i < nLastDimSize; ++i)	// for src
			{
				vSrcCoordFull.back() = static_cast<unsigned>(i);
				std::copy(vDstCoord.begin(), vDstCoord.end(), vDstCoordFull.begin());
				const std::vector<unsigned> vDstCoordPart(vDstCoordFull.begin(), vDstCoordFull.begin() + vDstCoordFull.size() - 1);
				std::vector<double> vals = GetVectorValue(vSrcCoordFull, vDstCoordPart);
				dVal += VectorSum(vals);
			}
			dVal /= nLastDimSize;
			_newTMatr.SetValue(vSrcCoord, vDstCoord, dVal);

			bDst = IncrementCoordsOld(vDstCoord, vNewClasses);
		}
		while (bDst);

		std::fill(vDstCoord.begin(), vDstCoord.end(), 0);
		bSrc = IncrementCoordsOld(vSrcCoord, vNewClasses);
	}
	while (bSrc);
}

bool CTransformMatrix::CheckDuplicates(const std::vector<unsigned>& _vDims) const
{
	for( unsigned i=0; i<_vDims.size()-1; ++i )
		for( unsigned j=i+1; j<_vDims.size(); ++j )
			if( _vDims[i] == _vDims[j] )
				return false;
	return true;
}

size_t CTransformMatrix::GetIndex(const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
	const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst) const
{
	std::vector<unsigned> vSizes;
	std::vector<unsigned> vCoords;
	// for source
	if( !MakeSizeAndCoord( vSizes, vCoords, _vDimsSrc, _vCoordsSrc ) )
		return -1;
	// for destination
	if( !MakeSizeAndCoord( vSizes, vCoords, _vDimsDst, _vCoordsDst ) )
		return -1;

	// get index
	size_t index;
	std::vector<unsigned>::reverse_iterator iterCoord = vCoords.rbegin();
	std::vector<unsigned>::reverse_iterator iterSize = vSizes.rbegin();
	iterSize++;
	index = *iterCoord;
	iterCoord++;
	for( unsigned i=0; i<m_vDimensions.size()*2-1; ++i)
	{
		index *= *iterSize;
		index += *iterCoord;
		iterSize++;
		iterCoord++;
	}

	return index;
}

bool CTransformMatrix::GetIndexAndStep(const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc,
	const std::vector<unsigned>& _vDimsDst, const std::vector<unsigned>& _vCoordsDst,
	size_t& _nResIndex, size_t& _nResStep, size_t& _nResVecLength) const
{
	std::vector<unsigned> vSizes;
	std::vector<unsigned> vCoords;
	unsigned iVect;
	unsigned nStep = 1;
	bool bFlagOk = true;

	// for source
	if (!MakeSizeAndCoord(vSizes, vCoords, nStep, iVect, bFlagOk, _vDimsSrc, _vCoordsSrc))
		return false;
	// for destination
	if (!MakeSizeAndCoord(vSizes, vCoords, nStep, iVect, bFlagOk, _vDimsDst, _vCoordsDst))
		return false;
	if (_vCoordsSrc.size() > _vCoordsDst.size())
	{
		iVect = iVect * 2 + 1;
		for (size_t i = vSizes.size() / 2; i < vSizes.size(); ++i)
			nStep *= vSizes[i];
	}

	size_t index;
	vCoords.insert(vCoords.begin() + iVect, 0);
	std::vector<unsigned>::reverse_iterator iterCoord = vCoords.rbegin();
	std::vector<unsigned>::reverse_iterator iterSize = vSizes.rbegin();
	iterSize++;
	index = *iterCoord;
	iterCoord++;
	for (unsigned i = 0; i < m_vDimensions.size() * 2 - 1; ++i)
	{
		index *= *iterSize;
		index += *iterCoord;
		iterSize++;
		iterCoord++;
	}

	_nResIndex = index;
	_nResStep = nStep;
	_nResVecLength = vSizes.at(iVect);
	return true;
}

bool CTransformMatrix::MakeSizeAndCoord(std::vector<unsigned>& _vResSizes, std::vector<unsigned>& _vResCoords,
	const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc) const
{
	for( unsigned i=0; i<m_vDimensions.size(); ++i )
	{
		unsigned iDim;
		for( iDim=0; iDim<_vDimsSrc.size(); ++iDim )
		{
			if( m_vDimensions[i] == _vDimsSrc[iDim] )
			{
				if( _vCoordsSrc[iDim] >= m_vClasses[i] ) // wrong coordinate
					return false;
				_vResSizes.push_back( m_vClasses[i] );
				_vResCoords.push_back( _vCoordsSrc[iDim] );
				break;
			}
		}
		if( iDim == _vDimsSrc.size() )
			return false;
	}
	return true;
}

bool CTransformMatrix::MakeSizeAndCoord(std::vector<unsigned>& _vResSizes, std::vector<unsigned>& _vResCoords,
	uint32_t& _nResStep, uint32_t& _nResIVec, bool& _bResFlagOk,
	const std::vector<unsigned>& _vDimsSrc, const std::vector<unsigned>& _vCoordsSrc) const
{
	for( unsigned i=0; i<m_vDimensions.size(); ++i )
	{
		unsigned iDim;
		for( iDim=0; iDim<_vDimsSrc.size(); ++iDim )
		{
			if( m_vDimensions[i] == _vDimsSrc[iDim] )
			{
				_vResSizes.push_back( m_vClasses[i] );
				if( iDim < _vCoordsSrc.size() )
				{
					if( m_vClasses[i] <= _vCoordsSrc[iDim] ) // wrong coordinate
						return false;
					_vResCoords.push_back( _vCoordsSrc[iDim] );
				}
				if( ( _vCoordsSrc.size() != _vDimsSrc.size() ) && ( _bResFlagOk ) )
				{
					if( _vDimsSrc.at(iDim) != _vDimsSrc.back() )
						_nResStep *= m_vClasses[i];
					else
					{
						_bResFlagOk = false;
						_nResIVec = i;
					}
				}
				break;
			}
		}
		if( iDim == _vDimsSrc.size() )
			return false;
	}
	return true;
}
