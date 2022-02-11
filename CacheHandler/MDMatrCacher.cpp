/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "MDMatrCacher.h"
#include <fstream>

CMDMatrCacher::CMDMatrCacher(void)
{
	m_fileNamePrefix = L"MD_";
	//CreateFile();
}

CMDMatrCacher::~CMDMatrCacher(void)
{
}

void CMDMatrCacher::ReadFromCache(double _dT, std::vector<double>& _vTP, std::vector<std::vector<double>>& _vvData,
	double& _dCurrWinStart, double& _dCurrWinEnd, size_t& _nCurrOffset) const
{
	if( m_descriptors.size() == 0 )
		return;

	size_t *index = GetIndexToRead( _dT );

	_vTP.clear();
	_vvData.clear();

	for(size_t i=index[0]; i<=index[1]; ++i )
	{
		if( m_descriptors[i].valid )
			ReadFromFile( i, _vTP, _vvData );
		m_descriptors[i].valid = false;
	}

	_nCurrOffset = 0;
	for(size_t i=0; i<index[0]; ++i )
		_nCurrOffset += m_descriptors[i].descriptorNumber;
	_dCurrWinStart = m_descriptors[index[0]].timeStart;
	_dCurrWinEnd = m_descriptors[index[1]].timeEnd;

	delete[] index;
}

void CMDMatrCacher::ReadFromCache(double _dT1, double _dT2, std::vector<double>& _vTP, std::vector<std::vector<double>>& _vvData,
	double& _dCurrWinStart, double& _dCurrWinEnd, size_t& _nCurrOffset) const
{
	if( _dT1 == _dT2 )
	{
		ReadFromCache( _dT1, _vTP, _vvData, _dCurrWinStart, _dCurrWinEnd, _nCurrOffset );
		return;
	}

	if( m_descriptors.size() == 0 )
		return;

	size_t *index = GetIndexToRead( _dT1, _dT2 );

	_vTP.clear();
	_vvData.clear();

	for(size_t i=index[0]; i<=index[1]; ++i )
	{
		if( m_descriptors[i].valid )
			ReadFromFile( i, _vTP, _vvData );
		m_descriptors[i].valid = false;
	}

	_nCurrOffset = 0;
	for(size_t i=0; i<index[0]; ++i )
		_nCurrOffset += m_descriptors[i].descriptorNumber;
	_dCurrWinStart = m_descriptors[index[0]].timeStart;
	_dCurrWinEnd = m_descriptors[index[1]].timeEnd;

	delete[] index;
}

void CMDMatrCacher::WriteToCache(const std::vector<double>& _vTP, std::vector<std::vector<double>>& _vvData, bool _bCoherent)
{
	bool bInsertNewDescr;
	size_t index;
	GetIndexToWrite( _vTP, 0, index, bInsertNewDescr );

	if( _bCoherent && !bInsertNewDescr ) // no changes in cached data -> just set descriptors to 'valid'
	{
		size_t nNumber = _vTP.size();
		while( nNumber > 0 )
		{
			m_descriptors[index].valid = true;
			nNumber -= m_descriptors[index].descriptorNumber;
			index++;
		}
		return;
	}

	if( _vTP.size() != 0 )
	{
		size_t nNumber = _vTP.size();
		size_t nStartTP = 0;
		while( nNumber >= m_chunk*2 )
		{
			WriteData( index, _vvData, _vTP, nStartTP, m_chunk, bInsertNewDescr );
			index++;
			nStartTP += m_chunk;
			nNumber -= m_chunk;
		}
		WriteData( index, _vvData, _vTP, nStartTP, nNumber, bInsertNewDescr );
		index++;
	}

	RemoveUnusedBlocks();
	if( ( index < (int)m_descriptors.size() ) && ( !m_descriptors[index].valid ) )
		while( ( index < m_descriptors.size() ) && ( !m_descriptors[index].valid ) )
		{
			m_descriptors.erase( m_descriptors.begin() + index );
			//index++;
		}
}

void CMDMatrCacher::WriteData(size_t _nIndex, std::vector<std::vector<double>>& _vvData, const std::vector<double>& _vTP, size_t _nStartTP, size_t _nSize, bool _bInsert)
{
	bool bInsert = false;
	SDescriptor newDescr( true, 0, _nSize, _vTP[_nStartTP], _vTP[_nStartTP+_nSize-1], 0 );
	if (_bInsert || _nIndex < m_descriptors.size() && m_descriptors[_nIndex].valid || _nIndex >= m_descriptors.size() || _nSize > m_descriptors[_nIndex].descriptorNumber)
		bInsert = true;
	else
	{
		newDescr.fileNumber = m_descriptors[_nIndex].fileNumber;
		newDescr.filePosition = m_descriptors[_nIndex].filePosition;
	}

	WriteToFile( newDescr, bInsert, _nSize, _vTP, _vvData, _nStartTP );
	if( bInsert )
		m_descriptors.insert( m_descriptors.begin() + _nIndex, newDescr );
	else
		m_descriptors[_nIndex] = newDescr;
}

void CMDMatrCacher::ReadFromFile(size_t _nIndex, std::vector<double>& _vTP, std::vector<std::vector<double>>& _vvData) const
{
	std::ifstream *pFile = OpenFileToRead( _nIndex );

	size_t nNumber, nDataLen;
	pFile->read( (char*)&nNumber, sizeof(nNumber) );
	pFile->read( (char*)&nDataLen, sizeof(nDataLen) );

	if( nNumber != 0 )
	{
		double *bufferTP = new double[ nNumber ];
		double *bufferData = new double[ nNumber*nDataLen ];
		pFile->read( (char*)bufferTP, sizeof(double)*nNumber );
		pFile->read( (char*)bufferData, sizeof(double)*nNumber*nDataLen );

		size_t nLastSize = _vTP.size();
		_vTP.resize( _vTP.size() + nNumber );
		if( _vvData.size() != nDataLen )
			_vvData.resize( nDataLen );
		for(size_t i=0; i<_vvData.size(); ++i )
			_vvData[i].resize( nLastSize + nNumber );

		for(size_t i=0; i<nNumber; ++i )
			_vTP[i+nLastSize] = bufferTP[i];

		for(size_t i=0; i<nDataLen; ++i )
			for(size_t j=0; j<nNumber; ++j )
				_vvData[i][j+nLastSize] = bufferData[nNumber*i + j];

		delete[] bufferTP;
		delete[] bufferData;
	}

	pFile->close();
	delete pFile;
}

void CMDMatrCacher::WriteToFile(SDescriptor& _currDescr, bool _bInsert, size_t _nNumber, const std::vector<double>& _vTimePoints, const std::vector<std::vector<double>>& _vvData, size_t _nOffset) const
{
	size_t nDataLen = _vvData.size()*_nNumber;
	size_t nDataDims = _vvData.size();
	uint64_t nBytesToWritwe = sizeof(_nNumber) + sizeof(nDataDims) + sizeof(double)*_nNumber + sizeof(double)*nDataLen;
	std::fstream *pFile = OpenFileToWrite( _currDescr, _bInsert, _nNumber, nBytesToWritwe );

	double *buffer = new double[ nDataLen ];
	for(size_t i=0; i<_vvData.size(); ++i )
		for(size_t j=0; j<_nNumber; ++j )
			buffer[_nNumber*i+j] = _vvData[i][j+_nOffset];

	pFile->write( (char*)&_nNumber, sizeof(_nNumber) );
	pFile->write( (char*)&nDataDims, sizeof(nDataDims) );
	pFile->write( (char*)&(_vTimePoints[_nOffset]), sizeof(double)*_nNumber );
	pFile->write( (char*)buffer, sizeof(double)*nDataLen );

	delete[] buffer;

	pFile->close();
	delete pFile;
}
