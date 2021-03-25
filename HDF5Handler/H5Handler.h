/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolTypes.h"
#include <vector>
#include "H5Cpp.h"
#include <regex>


/**
 *	IO with HDF5 files. Two modes:
 *	1. Single file: all data stored in a single file.
 *	2. Multi file: the file is split into parts of 2000 Mb each.
 */
class CH5Handler
{
	std::wstring m_sFileName;
	bool m_bFileValid;
	H5::H5File* m_ph5File;

public:
	CH5Handler();
	~CH5Handler();

	void Create(const std::wstring& _sFileName, bool _bSingleFile = true);	/// Create new file with truncation.
	void Open(const std::wstring& _sFileName);								/// Open existing file.
	void Close();															/// Close current file.
	std::wstring FileName() const;											/// Returns current file name.

	std::string CreateGroup(const std::string& _sPath, const std::string& _sGroupName) const;

	void WriteAttribute(const std::string& _sPath, const std::string& _sAttrName, int _nValue) const;
	int ReadAttribute(const std::string& _sPath, const std::string& _sAttrName) const;

	void WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::string& _sData) const;
	void WriteData(const std::string& _sPath, const std::string& _sDatasetName, double _dData) const;
	void WriteData(const std::string& _sPath, const std::string& _sDatasetName, uint32_t _nData) const;
	void WriteData(const std::string& _sPath, const std::string& _sDatasetName, uint64_t _nData) const;
	void WriteData(const std::string& _sPath, const std::string& _sDatasetName, int64_t _nData) const;
	void WriteData(const std::string& _sPath, const std::string& _sDatasetName, bool _bData) const;
	void WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::vector<std::string>& _vData) const;
	void WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::vector<unsigned>& _vData) const;
	void WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::vector<double>& _vData) const;
	void WriteData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<std::vector<double>>& _vvData) const;
	void WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::vector<STDValue>& _data) const;
	void WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::vector<CPoint>& _data) const;

	void ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::string& _sData) const;
	void ReadData(const std::string& _sPath, const std::string& _sDatasetName, double& _dData) const;
	void ReadData(const std::string& _sPath, const std::string& _sDatasetName, uint32_t& _nData) const;
	void ReadData(const std::string& _sPath, const std::string& _sDatasetName, uint64_t& _nData) const;
	void ReadData(const std::string& _sPath, const std::string& _sDatasetName, int64_t& _nData) const;
	void ReadData(const std::string& _sPath, const std::string& _sDatasetName, bool& _bData) const;
	void ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<std::string>& _vData) const;
	void ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<unsigned>& _vData) const;
	void ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<double>& _vData) const;
	void ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<std::vector<double>>& _vvData) const;
	void ReadDataOld(const std::string& _sPath, const std::string& _sDatasetName, std::vector<std::vector<double>>& _vvData) const;
	void ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<STDValue>& _data) const;
	void ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<CPoint>& _data) const;

	bool IsValid() const;

	static std::wstring DisplayFileName(std::wstring _fileName);	        // Returns displayable file name in form "path/FileName.dflw", removing all [[%d]] and [[N]] from it

private:
	void WriteValue(const std::string& _sPath, const std::string& _sDatasetName, hsize_t _size, const H5::DataType& _type, const void* _pValue) const;
	size_t ReadSize(const std::string& _sPath, const std::string& _sDatasetName) const;
	bool ReadValue(const std::string& _sPath, const std::string& _sDatasetName, const H5::DataType& _type, void* _pRes) const;
	bool ReadStrings(const std::string& _sPath, const std::string& _sDatasetName, char** _pRes) const;

	void OpenH5File(const std::wstring& _sFileName, bool _bOpen, bool _bSingleFile);
	static H5::FileAccPropList CreateFileAccPropList(bool _bSingleFile);

	static H5::CompType& h5CPoint_type();	// Lazily initializes HDF5 type for CPoint and returns it.
	static H5::CompType& h5STDValue_type(); // Lazily initializes HDF5 type for STDValue and returns it.

	static std::wstring ConvertFileName(const std::wstring& _sFileName, bool _bOpen, bool _bSingleFile);	// Converts the file name to a Dyssol format.
	static std::wstring MultiFileReadName(const std::wstring& _sFileName);									// Transforms the file name to the form needed to read from multi-file.
	static std::wstring MultiFileWriteName(std::wstring _sFileName);										// Transforms the file name to the form needed to write to multi-file.
	static std::wsmatch FindSuffix(const std::wstring& _str, const std::wstring& _regexStr);				// Returns results of the regex search.
	static std::wstring ReplaceMultiFileSuffix(std::wstring _str, size_t _pos, size_t _len);				// Replaces _len symbols starting from _pos in _str with a multi-file suffix.
};
