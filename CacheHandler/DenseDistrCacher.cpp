/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DenseDistrCacher.h"
#include <fstream>

CDenseDistrCacher::CDenseDistrCacher(void)
{
	m_fileNamePrefix = L"DD_";
	//CreateFile();
}

CDenseDistrCacher::~CDenseDistrCacher(void)
{
}

void CDenseDistrCacher::ReadFromCache(double _dT, std::vector<std::vector<double>>& _vvData, double& _dCurrWinStart, double& _dCurrWinEnd, size_t& _nCurrOffset) const
{
	if( m_descriptors.size() == 0 )
		return;

	size_t *index = GetIndexToRead( _dT );

	for(size_t i=index[0]; i<=index[1]; ++i )
	{
		if( m_descriptors[i].valid )
			ReadFromFile( i, _vvData );
		m_descriptors[i].valid = false;
	}

	_nCurrOffset = 0;
	for(size_t i=0; i<index[0]; ++i )
		_nCurrOffset += m_descriptors[i].descriptorNumber;
	_dCurrWinStart = m_descriptors[index[0]].timeStart;
	_dCurrWinEnd = m_descriptors[index[1]].timeEnd;

	delete[] index;
}

void CDenseDistrCacher::ReadFromCache(double _dT1, double _dT2, std::vector<std::vector<double>>& _vvData, double& _dCurrWinStart, double& _dCurrWinEnd, size_t& _nCurrOffset) const
{
	if( _dT1 == _dT2 )
	{
		ReadFromCache( _dT1, _vvData, _dCurrWinStart, _dCurrWinEnd, _nCurrOffset);
		return;
	}

	if( m_descriptors.size() == 0 )
		return;

	size_t *index = GetIndexToRead( _dT1, _dT2 );

	for(size_t i=index[0]; i<=index[1]; ++i )
	{
		if( m_descriptors[i].valid )
			ReadFromFile( i, _vvData );
		m_descriptors[i].valid = false;
	}

	_nCurrOffset = 0;
	for(size_t i=0; i<index[0]; ++i )
		_nCurrOffset += m_descriptors[i].descriptorNumber;
	_dCurrWinStart = m_descriptors[index[0]].timeStart;
	_dCurrWinEnd = m_descriptors[index[1]].timeEnd;

	delete[] index;
}

void CDenseDistrCacher::WriteToCache(std::vector<std::vector<double>>& _vvData, const std::vector<double>& _vTP, size_t _nStartTP, size_t _nNumber, bool _bCoherent)
{
	bool bInsertNewDescr;
	size_t index;
	GetIndexToWrite( _vTP, _nStartTP, index, bInsertNewDescr );

	if( _bCoherent && !bInsertNewDescr ) // no changes in cached data -> just set descriptors to 'valid'
	{
		size_t nNumber = _nNumber;
		while( nNumber > 0 )
		{
			m_descriptors[index].valid = true;
			nNumber -= m_descriptors[index].descriptorNumber;
			_vvData.erase( _vvData.begin(), _vvData.begin() + m_descriptors[index].descriptorNumber );
			index++;
		}
		return;
	}

	if( _vvData.size() != 0)
	{
		while( _nNumber >= m_chunk*2 )
		{
			WriteData( index, _vvData, _vTP, _nStartTP, m_chunk, bInsertNewDescr );
			index++;
			_nStartTP += m_chunk;
			_nNumber -= m_chunk;
		}

		WriteData( index, _vvData, _vTP, _nStartTP, _nNumber, bInsertNewDescr );
		index++;
	}

	RemoveUnusedBlocks();
	if( ( index < m_descriptors.size() ) && ( !m_descriptors[index].valid ) )
		while( ( index < m_descriptors.size() ) && ( !m_descriptors[index].valid ) )
		{
			m_descriptors.erase( m_descriptors.begin() + index );
			//index++;
		}
}

void CDenseDistrCacher::WriteData(size_t _nIndex, std::vector<std::vector<double>>& _vvData, const std::vector<double>& _vTP, size_t _nStartTP, size_t _nSize, bool _bInsert)
{
	bool bInsert = false;
	SDescriptor newDescr( true, 0, _nSize, _vTP[_nStartTP], _vTP[_nStartTP+_nSize-1], 0 );
	if( ( _bInsert ) || ( ( _nIndex < m_descriptors.size() ) && ( m_descriptors[_nIndex].valid ) ) || ( _nIndex >= m_descriptors.size() ) )
		bInsert = true;
	else
	{
		newDescr.fileNumber = m_descriptors[_nIndex].fileNumber;
		newDescr.filePosition = m_descriptors[_nIndex].filePosition;
	}

	WriteToFile( newDescr, bInsert, _nSize, _vvData );
	if( bInsert )
		m_descriptors.insert( m_descriptors.begin() + _nIndex, newDescr );
	else
		m_descriptors[_nIndex] = newDescr;

	_vvData.erase( _vvData.begin(), _vvData.begin() + _nSize );
}

void CDenseDistrCacher::ReadFromFile(size_t _nIndex, std::vector<std::vector<double>>& _vvData) const
{
	std::ifstream *pFile = OpenFileToRead( _nIndex );

	size_t nLen, nDims;
	pFile->read( (char*)&nDims, sizeof(nDims) );
	pFile->read( (char*)&nLen, sizeof(nLen) );

	if( nLen != 0 )
	{
		double *buffer = new double[ nDims*nLen ];
		pFile->read( (char*)buffer, sizeof(double)*nDims*nLen );

		size_t nLastSize = _vvData.size();
		_vvData.resize( nLastSize + nLen );
		for(size_t i=nLastSize; i<_vvData.size(); ++i )
			_vvData[i].resize( nDims );

		for(size_t i=0; i<nLen; ++i )
			for(size_t j=0; j<nDims; ++j )
				_vvData[i+nLastSize][j] = buffer[nDims*i + j];

		delete[] buffer;
	}

	pFile->close();
	delete pFile;
}

void CDenseDistrCacher::WriteToFile(SDescriptor& _currDescr, bool _bInsert, size_t _nNumber, const std::vector<std::vector<double>>& _vvData) const
{
	size_t nDims = _vvData.front().size();
	uint64_t nBytesToWritwe = sizeof(nDims) + sizeof(_nNumber) + sizeof(double)*nDims*_nNumber;
	std::fstream *pFile = OpenFileToWrite( _currDescr, _bInsert, _nNumber, nBytesToWritwe );

	double *buffer = new double[ nDims*_nNumber ];
	for(size_t i=0; i<_nNumber; ++i )
		for(size_t j=0; j<nDims; ++j )
			buffer[nDims*i + j] = _vvData[i][j];

	pFile->write( (char*)&nDims, sizeof(nDims) );
	pFile->write( (char*)&_nNumber, sizeof(_nNumber) );
	pFile->write( (char*)buffer, sizeof(double)*nDims*_nNumber );

	delete[] buffer;

	pFile->close();
	delete pFile;
}

//void CDenseDistrCacher::ResaveFile( unsigned _nIndex )
//{
//if( _nIndex >= m_descriptors.size() )
//	return;

//unsigned nCurrFile = m_descriptors[_nIndex].fileNumber;
//long long nMaxValid;
//for( unsigned i=0; i<_nIndex; ++i )
//{
//	if()
//}
//if( ( index < m_descriptors.size() ) && ( !m_descriptors[index].valid ) )
//	while( ( index < m_descriptors.size() ) && ( !m_descriptors[index].valid ) )
//	{
//		m_descriptors.erase( m_descriptors.begin() + index );
//		index++;
//	}
//}
