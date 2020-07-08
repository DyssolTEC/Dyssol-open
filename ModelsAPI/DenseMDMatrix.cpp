/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DenseMDMatrix.h"
#include "DyssolUtilities.h"
#include <functional>

CDenseMDMatrix::CDenseMDMatrix()
{
}

CDenseMDMatrix::CDenseMDMatrix(const std::vector<unsigned>& _vTypes, const std::vector<unsigned>& _vClasses)
{
	SetDimensions(_vTypes, _vClasses);
}

bool CDenseMDMatrix::SetDimensions(unsigned _nType, unsigned _nClasses)
{
	return SetDimensions(std::vector<unsigned>{_nType}, std::vector<unsigned>{_nClasses});
}

bool CDenseMDMatrix::SetDimensions(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2)
{
	return SetDimensions(std::vector<unsigned>{_nType1, _nType2}, std::vector<unsigned>{_nClasses1, _nClasses2});
}

bool CDenseMDMatrix::SetDimensions(unsigned _nType1, unsigned _nClasses1, unsigned _nType2, unsigned _nClasses2, unsigned _nType3, unsigned _nClasses3)
{
	return SetDimensions(std::vector<unsigned>{_nType1, _nType2, _nType3}, std::vector<unsigned>{_nClasses1, _nClasses2, _nClasses3});
}

bool CDenseMDMatrix::SetDimensions(const std::vector<unsigned>& _vTypes, const std::vector<unsigned>& _vClasses)
{
	if (_vTypes.empty() || _vClasses.empty())	return false;	// wrong size of _vDimensions or _vClasses
	if (!CheckDuplicates(_vTypes))				return false;	// duplicates in dimensions
	if (_vTypes.size() != _vClasses.size())		return false;	// wrong size of _vDimensions or _vClasses
	for (size_t i = 0; i < _vClasses.size(); ++i) // wrong number of classes
		if (_vClasses[i] == 0)
			return false;

	if (!m_vDimensions.empty())	// remove old data
		Clear();

	// save dimension parameters
	m_vDimensions = _vTypes;
	m_vClasses = _vClasses;

	// allocate memory
	size_t nSize = 1;
	for (size_t i = 0; i < _vClasses.size(); ++i)
		nSize *= _vClasses[i];
	m_vData.resize(nSize, 0);

	return true;
}

std::vector<unsigned> CDenseMDMatrix::GetDimensions() const
{
	return m_vDimensions;
}

std::vector<unsigned> CDenseMDMatrix::GetClasses() const
{
	return m_vClasses;
}

size_t CDenseMDMatrix::GetDimensionsNumber() const
{
	return m_vDimensions.size();
}

void CDenseMDMatrix::Clear()
{
	m_vDimensions.clear();
	m_vClasses.clear();
	m_vData.clear();
}

void CDenseMDMatrix::ClearData()
{
	std::fill(m_vData.begin(), m_vData.end(), 0);
}

const double* CDenseMDMatrix::GetDataPtr() const
{
	return m_vData.data();
}

double* CDenseMDMatrix::GetDataPtr()
{
	return m_vData.data();
}

size_t CDenseMDMatrix::GetDataLength() const
{
	return m_vData.size();
}

double CDenseMDMatrix::GetValue(unsigned _nDim, unsigned _nCoord) const
{
	return GetValue(std::vector<unsigned>{_nDim}, std::vector<unsigned>{_nCoord});
}

double CDenseMDMatrix::GetValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2) const
{
	return GetValue(std::vector<unsigned>{_nDim1, _nDim2}, std::vector<unsigned>{_nCoord1, _nCoord2});
}

double CDenseMDMatrix::GetValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nDim3, unsigned _nCoord3) const
{
	return GetValue(std::vector<unsigned>{_nDim1, _nDim2, _nDim3}, std::vector<unsigned>{_nCoord1, _nCoord2, _nCoord3});
}

double CDenseMDMatrix::GetValue(const std::vector<unsigned>& _vCoords) const
{
	return GetValue( m_vDimensions, _vCoords );
}

double CDenseMDMatrix::GetValue(const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords) const
{
	if (_vDims.size() > m_vDimensions.size())	return -1; // _vDims has wrong size
	if (_vDims.size() != _vCoords.size())		return -1; // wrong size of _vDims or _vCoords
	return GetValueRecursive(_vDims, _vCoords);
}

std::vector<double> CDenseMDMatrix::GetVectorValue(unsigned _nDim) const
{
	std::vector<double> vRes;
	GetVectorValue(std::vector<unsigned>{_nDim}, std::vector<unsigned>(), vRes);
	return vRes;
}

bool CDenseMDMatrix::GetVectorValue(unsigned _nDim, std::vector<double>& _vResult) const
{
	return GetVectorValue(std::vector<unsigned>{_nDim}, std::vector<unsigned>(), _vResult);
}

bool CDenseMDMatrix::GetVectorValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, std::vector<double>& _vResult) const
{
	return GetVectorValue(std::vector<unsigned>{_nDim1, _nDim2}, std::vector<unsigned>{_nCoord1}, _vResult);
}

bool CDenseMDMatrix::GetVectorValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nDim3, std::vector<double>& _vResult) const
{
	return GetVectorValue(std::vector<unsigned>{_nDim1, _nDim2, _nDim3}, std::vector<unsigned>{_nCoord1, _nCoord2}, _vResult);
}

std::vector<double> CDenseMDMatrix::GetVectorValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nDim3) const
{
	return GetVectorValue(std::vector<unsigned>{_nDim1, _nDim2, _nDim3}, std::vector<unsigned>{_nCoord1, _nCoord2});
}

bool CDenseMDMatrix::GetVectorValue(const std::vector<unsigned>& _vCoords, std::vector<double>& _vResult) const
{
	return GetVectorValue(m_vDimensions, _vCoords, _vResult);
}

bool CDenseMDMatrix::GetVectorValue(const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, std::vector<double>& _vResult) const
{
	if( _vDims.size() > m_vDimensions.size() )	return false;	// _vDimType has wrong size
	if (_vDims.size() != _vCoords.size() + 1)	return false;	// _vCoord has wrong size
	return GetVectorValueRecursive(_vDims, _vCoords, _vResult);
}

std::vector<double> CDenseMDMatrix::GetVectorValue(const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords) const
{
	if (_vDims.size() > m_vDimensions.size())	return {};	// _vDimType has wrong size
	if (_vDims.size() != _vCoords.size() + 1)	return {};	// _vCoord has wrong size
	std::vector<double> res;
	const bool success = GetVectorValueRecursive(_vDims, _vCoords, res);
	if (!success) return {};
	return res;
}

bool CDenseMDMatrix::SetValue(unsigned _nCoord, double _dValue)
{
	if (m_vDimensions.size() != 1) return false;
	return SetValue(m_vDimensions, std::vector<unsigned>{_nCoord}, _dValue);
}

bool CDenseMDMatrix::SetValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nCoord2, double _dValue)
{
	if (m_vDimensions.size() != 2) return false;

	std::vector<unsigned> vDims{ _nDim1 , 0 };
	if (_nDim1 == m_vDimensions[0])	vDims[1] = m_vDimensions[1];
	else							vDims[1] = m_vDimensions[0];

	return SetValue(vDims, std::vector<unsigned>{_nCoord1, _nCoord2}, _dValue);
}

bool CDenseMDMatrix::SetValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, unsigned _nCoord3, double _dValue)
{
	if (m_vDimensions.size() != 3)	return false;
	if (_nDim1 == _nDim2)			return false;

	std::vector<unsigned> vDims{ _nDim1 , _nDim2 , 0 };
	if (_nDim1 == m_vDimensions[0])
		if (_nDim2 == m_vDimensions[1])		vDims[2] = m_vDimensions[2];
		else								vDims[2] = m_vDimensions[1];
	else if (_nDim1 == m_vDimensions[1])
		if (_nDim2 == m_vDimensions[0])		vDims[2] = m_vDimensions[2];
		else								vDims[2] = m_vDimensions[0];
	else
		if (_nDim2 == m_vDimensions[0])		vDims[2] = m_vDimensions[1];
		else								vDims[2] = m_vDimensions[0];

	return SetValue(vDims, std::vector<unsigned>{_nCoord1, _nCoord2, _nCoord3}, _dValue);
}

bool CDenseMDMatrix::SetValue(const std::vector<unsigned>& _vCoords, double _dValue)
{
	return SetValue(m_vDimensions, _vCoords, _dValue);
}

bool CDenseMDMatrix::SetValue(const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, double _dValue)
{
	size_t index = GetIndex(_vDims, _vCoords);
	if ((index == -1) || (index >= m_vData.size()))
		return false;
	m_vData[index] = _dValue;
	return true;
}

bool CDenseMDMatrix::SetVectorValue(const std::vector<double>& _vValue)
{
	if (m_vDimensions.size() != 1) return false;
	return SetVectorValue( m_vDimensions, std::vector<unsigned>(), _vValue );
}

bool CDenseMDMatrix::SetVectorValue(unsigned _nDim, unsigned _nCoord, const std::vector<double>& _vValue)
{
	if (m_vDimensions.size() != 2) return false;

	std::vector<unsigned> vDims{ _nDim , 0 };
	if (_nDim == m_vDimensions[0])	vDims[1] = m_vDimensions[1];
	else							vDims[1] = m_vDimensions[0];

	return SetVectorValue(vDims, std::vector<unsigned>{_nCoord}, _vValue);
}

bool CDenseMDMatrix::SetVectorValue(unsigned _nDim1, unsigned _nCoord1, unsigned _nDim2, unsigned _nCoord2, const std::vector<double>& _vValue)
{
	if (m_vDimensions.size() != 3)	return false;
	if (_nDim1 == _nDim2)			return false;

	std::vector<unsigned> vDims{ _nDim1 , _nDim2 , 0 };
	if (_nDim1 == m_vDimensions[0])
		if (_nDim2 == m_vDimensions[1])		vDims[2] = m_vDimensions[2];
		else								vDims[2] = m_vDimensions[1];
	else if (_nDim1 == m_vDimensions[1])
		if (_nDim2 == m_vDimensions[0])		vDims[2] = m_vDimensions[2];
		else								vDims[2] = m_vDimensions[0];
	else
		if (_nDim2 == m_vDimensions[0])		vDims[2] = m_vDimensions[1];
		else								vDims[2] = m_vDimensions[0];

	return SetVectorValue(vDims, std::vector<unsigned>{_nCoord1, _nCoord2}, _vValue);
}

bool CDenseMDMatrix::SetVectorValue(const std::vector<unsigned>& _vCoords, const std::vector<double>& _vValue)
{
	return SetVectorValue(m_vDimensions, _vCoords, _vValue);
}

bool CDenseMDMatrix::SetVectorValue(const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, const std::vector<double>& _vValue)
{
	size_t index;
	size_t step;
	size_t vecLength;
	if (!GetIndexAndStep(_vDims, _vCoords, &index, &step, &vecLength))
		return false;

	if (vecLength != _vValue.size()) // wrong size of values vector
		return false;

	for(size_t i=0; i<vecLength; ++i, index += step)
		if( index < m_vData.size() )
			m_vData[index] = _vValue[i];
	return true;
}

bool CDenseMDMatrix::IsNormalized()
{
	return (std::accumulate(m_vData.begin(), m_vData.end(), 0.) == 1.);
}

void CDenseMDMatrix::Normalize()
{
	VectorNormalize(m_vData);
}

bool CDenseMDMatrix::CheckDuplicates(const std::vector<unsigned>& _vDims) const
{
	for (size_t i = 0; i < _vDims.size() - 1; ++i)
		for (size_t j = i + 1; j < _vDims.size(); ++j)
			if (_vDims[i] == _vDims[j])
				return false;
	return true;
}

size_t CDenseMDMatrix::GetIndex(const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords) const
{
	if (_vDims.size() != m_vDimensions.size())	return -1; // wrong number of dimensions
	if (_vDims.size() != _vCoords.size())		return -1; // wrong number of coordinates
	if (_vDims.size() == 0)						return -1; // empty dimensions set

	std::vector<unsigned> vSizes;
	std::vector<unsigned> vCoords;
	for (size_t i = 0; i < m_vDimensions.size(); ++i)
	{
		unsigned iDim;
		for (iDim = 0; iDim < _vDims.size(); ++iDim)
			if (m_vDimensions[i] == _vDims[iDim])
			{
				if (_vCoords[iDim] >= m_vClasses[i]) // wrong coordinate
					return -1;
				vSizes.push_back(m_vClasses[i]);
				vCoords.push_back(_vCoords[iDim]);
				break;
			}
		if (iDim == _vDims.size()) // dimension #i wasn't defined in _vDims
			return -1;
	}

	std::vector<unsigned>::reverse_iterator iterCoord = vCoords.rbegin();
	std::vector<unsigned>::reverse_iterator iterSize = vSizes.rbegin();
	unsigned index = *iterCoord;
	++iterCoord;
	++iterSize;
	for (size_t i = 0; i < _vDims.size() - 1; ++i, ++iterSize, ++iterCoord)
		index = index * *iterSize + *iterCoord;

	return index;
}

bool CDenseMDMatrix::GetIndexAndStep(const std::vector<unsigned>& _vDims, const std::vector<unsigned>& _vCoords, size_t* _pResIndex, size_t* _pResStep, size_t* _pResVecLength) const
{
	if (_vDims.size() != m_vDimensions.size())	return false; // wrong number of dimensions
	if (_vDims.size() != _vCoords.size() + 1)	return false; // wrong number of coordinates
	if (_vDims.size() == 0)						return false; // empty dimensions set

	std::vector<size_t> vSizes;
	std::vector<size_t> vCoords;
	size_t iVect = 0;
	size_t nStep = 1;
	bool bFlagOk = true;
	for (size_t i = 0; i < m_vDimensions.size(); ++i)
	{
		unsigned iDim;
		for (iDim = 0; iDim < _vDims.size(); ++iDim)
		{
			if (m_vDimensions[i] == _vDims[iDim])
			{
				vSizes.push_back(m_vClasses[i]);
				if (iDim < _vCoords.size())
				{
					if (m_vClasses[i] <= _vCoords[iDim]) // wrong coordinate
						return false;
					vCoords.push_back(_vCoords[iDim]);
				}
				if (bFlagOk)
				{
					if (_vDims[iDim] != _vDims.back())
						nStep *= m_vClasses[i];
					else
					{
						bFlagOk = false;
						iVect = i;
					}
				}
				break;
			}
		}
		if (iDim == _vDims.size())
			return false;
	}

	vCoords.insert(vCoords.begin() + iVect, 0);
	std::vector<size_t>::reverse_iterator iterCoord = vCoords.rbegin();
	std::vector<size_t>::reverse_iterator iterSize = vSizes.rbegin();
	size_t index = *iterCoord;
	++iterCoord;
	++iterSize;
	for (size_t i = 0; i < _vDims.size() - 1; ++i, ++iterSize, ++iterCoord)
		index = index * *iterSize + *iterCoord;

	*_pResIndex = index;
	*_pResStep = nStep;
	*_pResVecLength = vSizes.at(iVect);
	return true;
}

double CDenseMDMatrix::GetValueRecursive(std::vector<unsigned> _vDims, std::vector<unsigned> _vCoords) const
{
	if (_vDims.size() < m_vDimensions.size() - 1) // add dimension and call GetValueRecursive for each class recursively
	{
		size_t index = 0;
		for (size_t i = 0; i < m_vDimensions.size(); ++i)
		{
			std::vector<unsigned>::iterator res = std::find(_vDims.begin(), _vDims.end(), m_vDimensions.at(i));
			if (res == _vDims.end())
			{
				_vDims.push_back(m_vDimensions[i]);
				index = i;
				break;
			}
		}
		double dRes = 0;
		_vCoords.push_back(0);
		for (unsigned i = 0; i < m_vClasses[index]; ++i)
		{
			dRes += GetValueRecursive(_vDims, _vCoords);
			_vCoords.back()++;
		}
		return dRes;
	}
	else if (_vDims.size() == m_vDimensions.size() - 1) // add dim and return sum of the vector
	{
		for (size_t i = 0; i < m_vDimensions.size(); ++i)
		{
			std::vector<unsigned>::iterator res = std::find(_vDims.begin(), _vDims.end(), m_vDimensions[i]);
			if (res == _vDims.end())
			{
				_vDims.push_back(m_vDimensions[i]);
				break;
			}
		}
		std::vector<double> vRes;
		if (!GetVectorValue(_vDims, _vCoords, vRes))
			return 0;
		double dRes = 0;
		for (size_t i = 0; i < vRes.size(); ++i)
			dRes += vRes[i];
		return dRes;
	}
	else if (_vDims.size() == m_vDimensions.size()) // just return the value
	{
		size_t index = GetIndex(_vDims, _vCoords);
		if ((index != -1) && (index < m_vData.size()))
			return m_vData[index];
	}
	return 0;
}

bool CDenseMDMatrix::GetVectorValueRecursive(std::vector<unsigned> _vDims, std::vector<unsigned> _vCoords, std::vector<double>& _pResult) const
{
	if( _vDims.size() < m_vDimensions.size() ) // add dimension and call GetVectorValueRecursive for each class recursively
	{
		// find index of dimension that corresponds to a last element of _vDims
		size_t index = 0;
		for(size_t i=0; i<m_vDimensions.size(); ++i )
			if( m_vDimensions[i] == _vDims.back() )
			{
				index = i;
				break;
			}

		for(size_t i=0; i<m_vDimensions.size(); ++i )
		{
			std::vector<unsigned>::iterator res = std::find( _vDims.begin(), _vDims.end(), m_vDimensions[i] );
			if( res == _vDims.end() )
			{
				_vDims.push_back( m_vDimensions[i] );
				break;
			}
		}
		_vCoords.push_back( 0 );
		_pResult.clear();
		for(size_t i=0; i<m_vClasses[index]; ++i )
		{
			std::vector<double> vTemp;
			_pResult.push_back( 0 );
			if( GetVectorValueRecursive( _vDims, _vCoords, vTemp ) )
				for(size_t j=0; j<vTemp.size(); ++j )
					_pResult.back() += vTemp[j];
			_vCoords.back()++;
		}
		return true;
	}
	else if( _vDims.size() == m_vDimensions.size() ) // just return the vector value
	{
		size_t index;
		size_t nStep;
		size_t nVecLength;
		if( !GetIndexAndStep( _vDims, _vCoords, &index, &nStep, &nVecLength ) )
			return false;
		_pResult.resize( nVecLength );
		for (size_t i = 0; i < nVecLength; ++i)
			if( index < m_vData.size() )
			{
				_pResult[i] = m_vData[index];
				index += nStep;
			}
		return true;
	}
	return false;
}

CDenseMDMatrix CDenseMDMatrix::operator+(const CDenseMDMatrix& _matrix)
{
	if (m_vDimensions.size() != _matrix.m_vDimensions.size())
		return CDenseMDMatrix();
	for (size_t i = 0; i < m_vDimensions.size(); ++i)
		if ((m_vDimensions[i] != _matrix.m_vDimensions[i]) || (m_vClasses[i] != _matrix.m_vClasses[i]))
			return CDenseMDMatrix();

	CDenseMDMatrix sum;
	sum.SetDimensions(m_vDimensions, m_vClasses);
	std::transform(m_vData.begin(), m_vData.end(), _matrix.m_vData.begin(), sum.m_vData.begin(), std::plus<>());
	return sum;
}

CDenseMDMatrix CDenseMDMatrix::operator-(const CDenseMDMatrix& _matrix)
{
	if (m_vDimensions.size() != _matrix.m_vDimensions.size())
		return CDenseMDMatrix();
	for (unsigned i = 0; i < m_vDimensions.size(); ++i)
		if ((m_vDimensions[i] != _matrix.m_vDimensions[i]) || (m_vClasses[i] != _matrix.m_vClasses[i]))
			return CDenseMDMatrix();

	CDenseMDMatrix diff;
	diff.SetDimensions(m_vDimensions, m_vClasses);
	std::transform(m_vData.begin(), m_vData.end(), _matrix.m_vData.begin(), diff.m_vData.begin(), std::minus<>());
	return diff;
}

CDenseMDMatrix CDenseMDMatrix::operator*(double _dFactor)
{
	CDenseMDMatrix prod;
	prod.SetDimensions(m_vDimensions, m_vClasses);
	std::transform(m_vData.begin(), m_vData.end(), prod.m_vData.begin(), [_dFactor](const double v) { return v * _dFactor; });
	return prod;
}
