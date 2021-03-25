/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "StringFunctions.h"
#include "BaseCacheHandler.h"
#include "FileSystem.h"
#include <fstream>

CBaseCacheHandler::CBaseCacheHandler() :
	m_dirPath(L"cache"),
	m_fileNamePrefix(L""),
	m_chunk(DEFAULT_CHUNK_SIZE),
	m_fileExt(L".cache"),
	m_fileName(L"")
{}

CBaseCacheHandler::~CBaseCacheHandler()
{
	for (size_t i = 0; ; ++i)
	{
		const std::wstring sBufName = m_fileName + std::to_wstring(i) + m_fileExt;
		if (FileSystem::FileExists(StringFunctions::UnicodePath(sBufName)))
			FileSystem::RemoveFile(StringFunctions::UnicodePath(sBufName));
		else
			break;
	}
}

void CBaseCacheHandler::ClearData() const
{
	std::wstring sBufName = m_fileName + std::to_wstring(0) + m_fileExt;
	std::ofstream file(StringFunctions::UnicodePath(sBufName), std::ios::out | std::ios::trunc | std::ios::binary);
	file.close();

	for (size_t i = 1; ; ++i)
	{
		sBufName = m_fileName + std::to_wstring(i) + m_fileExt;
		if (FileSystem::FileExists(StringFunctions::UnicodePath(sBufName)))
		{
			std::ofstream file(StringFunctions::UnicodePath(sBufName), std::ios::out | std::ios::trunc | std::ios::binary);
			file.close();
		}
		else
			break;
	}
	m_descriptors.clear();
}

void CBaseCacheHandler::SetChunk(size_t _chunk)
{
	if (_chunk != 0)
		m_chunk = _chunk;
}

void CBaseCacheHandler::SetDirPath(const std::wstring& _dirPath)
{
	m_dirPath = _dirPath;
}

void CBaseCacheHandler::Initialize()
{
	CreateFile();
}

size_t* CBaseCacheHandler::GetIndexToRead(double _t) const
{
	auto* index = new size_t[2]();
	if (_t < m_descriptors.front().timeEnd)
		index[0] = index[1] = 0;
	else if (_t == m_descriptors.front().timeEnd && _t == m_descriptors.front().timeStart)
		index[0] = index[1] = 0;
	else if (_t > m_descriptors.back().timeStart)
		index[0] = index[1] = m_descriptors.size() - 1;
	else
		for (index[0] = 1; index[0] < m_descriptors.size(); ++index[0])
			if (_t > m_descriptors[index[0]].timeStart && _t < m_descriptors[index[0]].timeEnd)
			{
				index[1] = index[0];
				break;
			}
			else if (_t >= m_descriptors[index[0] - 1].timeEnd && _t <= m_descriptors[index[0]].timeStart)
			{
				index[1] = index[0];
				index[0] -= 1;
				break;
			}
	return index;
}

size_t* CBaseCacheHandler::GetIndexToRead(double _t1, double _t2) const
{
	auto* index = new size_t[2]();

	if (_t2 < _t1)
		std::swap(_t1, _t2);

	if (_t1 < m_descriptors.front().timeEnd)
		index[0] = 0;
	else if (_t1 > m_descriptors.back().timeStart)
		index[0] = m_descriptors.size() - 1;
	else
		for (index[0] = 1; index[0] < m_descriptors.size(); ++index[0])
			if (_t1 > m_descriptors[index[0]].timeStart && _t1 < m_descriptors[index[0]].timeEnd)
				break;
			else if (_t1 >= m_descriptors[index[0] - 1].timeEnd && _t1 <= m_descriptors[index[0]].timeStart)
			{
				index[0] -= 1;
				break;
			}
	if (index[0] >= m_descriptors.size())
		index[0] = m_descriptors.size() - 1;

	if (_t2 < m_descriptors.front().timeEnd)
		index[1] = 0;
	else if (_t2 > m_descriptors.back().timeStart)
		index[1] = m_descriptors.size() - 1;
	else
		for (index[1] = index[0]; index[1] < m_descriptors.size(); ++index[1])
			if (_t2 > m_descriptors[index[1]].timeStart && _t2 < m_descriptors[index[1]].timeEnd)
				break;
			else if (index[1] > 0 && _t2 >= m_descriptors[index[1] - 1].timeEnd && _t2 <= m_descriptors[index[1]].timeStart)
				break;
	if (index[1] >= m_descriptors.size())
		index[1] = m_descriptors.size() - 1;
	return index;
}

void CBaseCacheHandler::GetIndexToWrite(const std::vector<double>& _tp, size_t _tStart, size_t& _index, bool& _bInsert) const
{
	_bInsert = false;

	if (m_descriptors.empty())
	{
		_index = 0;
		return;
	}

	for (_index = 0; _index < m_descriptors.size(); ++_index)
		if (!m_descriptors[_index].valid)
		{
			for (size_t i = _index + 1; i < m_descriptors.size(); ++i)
				if (m_descriptors[i].valid)
				{
					_bInsert = true;
					break;
				}
			break;
		}

	if (_tp[_tStart] < m_descriptors.back().timeEnd && _index == m_descriptors.size()) // no invalid blocks ==> search by times
	{
		_bInsert = true;
		for (_index = 0; _index < m_descriptors.size(); ++_index)
			if (_tp[_tStart] < m_descriptors[_index].timeStart)
				break;
	}
}

void CBaseCacheHandler::CreateFile()
{
	if (!FileSystem::DirExists(StringFunctions::UnicodePath(m_dirPath)))
		FileSystem::CreateDir(StringFunctions::UnicodePath(m_dirPath));

	m_fileName = m_dirPath + L"/" + m_fileNamePrefix + StringFunctions::String2WString(StringFunctions::GenerateRandomKey());
	std::wstring sBufName = m_fileName + L"0" + m_fileExt;
	while (FileSystem::FileExists(StringFunctions::UnicodePath(sBufName)))
	{
		m_fileName = m_dirPath + L"/" + m_fileNamePrefix + StringFunctions::String2WString(StringFunctions::GenerateRandomKey());
		sBufName = m_fileName + L"0" + m_fileExt;
	}

	std::ofstream file(StringFunctions::UnicodePath(sBufName), std::ios::out | std::ios::trunc | std::ios::binary);
	file.close();
}

std::ifstream* CBaseCacheHandler::OpenFileToRead(size_t _dataIndex) const
{
	const std::wstring bufName = m_fileName + std::to_wstring(m_descriptors[_dataIndex].fileNumber) + m_fileExt;
	std::ifstream *pFile = new std::ifstream(StringFunctions::UnicodePath(bufName), std::ios::out | std::ios::binary);
	pFile->seekg(static_cast<std::streamoff>(m_descriptors[_dataIndex].filePosition), std::ifstream::beg);
	return pFile;
}

std::fstream* CBaseCacheHandler::OpenFileToWrite(SDescriptor& _currDescriptor, bool _bInsert, size_t _entitiesNum, uint64_t _bytesToWrite) const
{
	std::fstream *pFile;

	if (!_bInsert && _currDescriptor.descriptorNumber <= _entitiesNum)
	{
		const std::wstring bufName = m_fileName + std::to_wstring(_currDescriptor.fileNumber) + m_fileExt;
		pFile = new std::fstream(StringFunctions::UnicodePath(bufName), std::ios::in | std::ios::out | std::ios::binary);
		pFile->seekp(static_cast<std::streamoff>(_currDescriptor.filePosition));
	}
	else
	{
		for (size_t iFile = 0; ; ++iFile)
		{
			const std::wstring bufName = m_fileName + std::to_wstring(iFile) + m_fileExt;
			const uint64_t currFileSize = FileSystem::FileSize(StringFunctions::UnicodePath(bufName));
			if (currFileSize == static_cast<uint64_t>(-1)) // file not exists
			{
				pFile = new std::fstream(StringFunctions::UnicodePath(bufName), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
				_currDescriptor.fileNumber = iFile;
				_currDescriptor.filePosition = 0;
				break;
			}
			else if (currFileSize + _bytesToWrite < MAX_CACHE_FILE_SIZE)
			{
				pFile = new std::fstream(StringFunctions::UnicodePath(bufName), std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
				_currDescriptor.fileNumber = iFile;
				_currDescriptor.filePosition = currFileSize;
				break;
			}
		}
	}
	return pFile;
}

void CBaseCacheHandler::RemoveUnusedBlocks() const
{
	for (size_t iFile = 0; ; ++iFile)
	{
		std::wstring bufName = m_fileName + std::to_wstring(iFile) + m_fileExt;
		if (FileSystem::FileExists(StringFunctions::UnicodePath(bufName)))
		{
			std::streamoff minInvalidOffset = -1;
			for (auto& descriptor : m_descriptors)
				if (!descriptor.valid && descriptor.fileNumber == iFile)
				{
					minInvalidOffset = descriptor.filePosition;
					break;
				}
			if (minInvalidOffset == -1)
				continue;

			std::streamoff maxValidOffset = 0;
			for (auto& descriptor : m_descriptors)
			{
				if (descriptor.fileNumber == iFile)
				{
					if (descriptor.valid)
					{
						if (descriptor.filePosition > maxValidOffset)
							maxValidOffset = descriptor.filePosition;
					}
					else
					{
						if (descriptor.filePosition < minInvalidOffset)
							minInvalidOffset = descriptor.filePosition;
					}
				}
			}

			if (maxValidOffset < minInvalidOffset)
				FileSystem::ChangeFileSize(StringFunctions::UnicodePath(bufName), minInvalidOffset);
		}
		else
			break;
	}
}
