/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <string>
#include <vector>

constexpr auto DEFAULT_CHUNK_SIZE  = 100;
constexpr auto MAX_CACHE_FILE_SIZE = 1024 * 1024 * 2000;

class CBaseCacheHandler
{
protected:
	struct SDescriptor
	{
		bool valid;
		size_t fileNumber;
		size_t descriptorNumber;
		double timeStart;
		double timeEnd;
		std::streamoff filePosition;
		SDescriptor() : valid(true), fileNumber(0), descriptorNumber(0), timeStart(0), timeEnd(0), filePosition(0) {}
		SDescriptor(bool _valid, size_t _fileNumber, size_t _descriptorNumber, double _timeStart, double _timeEnd, std::streamoff _filePosition)
			: valid(_valid), fileNumber(_fileNumber), descriptorNumber(_descriptorNumber), timeStart(_timeStart), timeEnd(_timeEnd), filePosition(_filePosition) {}
	};

	std::wstring m_dirPath;			// Path where to store cache file.
	std::wstring m_fileNamePrefix;	// Prefix of the file name.
	size_t m_chunk;					// Chunk size.

	mutable std::vector<SDescriptor> m_descriptors;	// List of file descriptors.

private:
	std::wstring m_fileExt;
	std::wstring m_fileName;

public:
	CBaseCacheHandler();
	virtual ~CBaseCacheHandler() = 0;
	CBaseCacheHandler(const CBaseCacheHandler& _other) = default;
	CBaseCacheHandler(CBaseCacheHandler&& _other) noexcept = default;
	CBaseCacheHandler& operator=(const CBaseCacheHandler& _other) = default;
	CBaseCacheHandler& operator=(CBaseCacheHandler&& _other) noexcept = default;

	void ClearData() const;

	void SetChunk(size_t _chunk);
	void SetDirPath(const std::wstring& _dirPath);
	void Initialize();

protected:
	size_t* GetIndexToRead(double _t) const;
	size_t* GetIndexToRead(double _t1, double _t2) const;
	void GetIndexToWrite(const std::vector<double>& _tp, size_t _tStart, size_t& _index, bool& _bInsert) const;

	void CreateFile();
	std::ifstream* OpenFileToRead(size_t _dataIndex) const;
	std::fstream* OpenFileToWrite(SDescriptor& _currDescriptor, bool _bInsert, size_t _entitiesNum, uint64_t _bytesToWrite) const;

	void RemoveUnusedBlocks() const;
};

