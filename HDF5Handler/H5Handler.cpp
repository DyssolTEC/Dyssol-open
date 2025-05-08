/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "H5Handler.h"
#include "PacketTable.h"
#include "DyssolStringConstants.h"
#include "FileSystem.h"

#include <regex>

namespace
{
	H5::FileAccPropList CreateFileAccPropList(bool _bSingleFile)
	{
		const hsize_t cChunkSize = 500;
		const hsize_t cCacheSize = 1024 * 1024 * 50;		// in bytes
		const hsize_t cMaxH5FileSize = 1024 * 1024 * 2000;	// in bytes

		H5::FileAccPropList h5AccPropList;
		if (!_bSingleFile)
			h5AccPropList.setFamily(cMaxH5FileSize, H5P_DEFAULT);
		h5AccPropList.setFcloseDegree(H5F_CLOSE_STRONG);
		h5AccPropList.setCache(cChunkSize, cChunkSize, cCacheSize, 0.5);

		return h5AccPropList;
	}

	// Returns results of the regex search.
	std::smatch FindSuffix(const std::filesystem::path& _str, const std::string_view& _regexStr)
	{
		const auto& str = _str.string();                       // Convert path to string
		const std::regex r(_regexStr.begin(), _regexStr.end()); // Compile regex expression

		std::smatch m;                                         // Results of regex search
		std::regex_search(str, m, r);                          // Perform regex search

		return m;
	}

	// Replaces _len symbols starting from _pos in _str with a multi-file suffix.
	std::filesystem::path ReplaceMultiFileSuffix(const std::filesystem::path& _str, size_t _pos, size_t _len)
	{
		std::string copy = _str.string();
		copy.replace(_pos, _len, StrConst::HDF5H_FileExtSpec); // Replace existing digits with multi-file extension

		return copy;
	}

	// Transforms the file name to the form needed to read from multi-file.
	std::filesystem::path MultiFileReadName(const std::filesystem::path& _sFileName)
	{
		if (_sFileName.empty())
			return {};

		// Already in proper multi-file format, i.e., contains [[%d]]
		if (FindSuffix(_sFileName, StrConst::HDF5H_FileExtFinalRegex).size() == 2)
			return _sFileName;

		// Search for a multi-file suffix in the form [[N]], where N is any number
		const auto& m = FindSuffix(_sFileName, StrConst::HDF5H_FileExtInitRegex);
		if (m.size() < 2)
			return _sFileName;	// No valid multi-file suffix found

		// Replace existing digits with a multi-file suffix %d
		return ReplaceMultiFileSuffix(_sFileName, m.position(1), m[1].length());
	}

	// Transforms the file name to the form needed to write to multi-file.
	std::filesystem::path MultiFileWriteName(std::filesystem::path _sFileName)
	{
		if (_sFileName.empty())
			return {};

		// Check if the extension is missing or incorrect
		if (!StringFunctions::CompareCaseInsensitive(_sFileName.extension().string(), StrConst::HDF5H_DotFileExt))
			_sFileName += StrConst::HDF5H_DotFileExt; // Add proper extension

		// Already in proper multi-file format, i.e., contains [[%d]]
		if (FindSuffix(_sFileName, StrConst::HDF5H_FileExtFinalRegex).size() == 2)
			return _sFileName;

		// Search for a multi-file suffix in the form [[N]], where N is any number
		const auto& m = FindSuffix(_sFileName, StrConst::HDF5H_FileExtInitRegex);
		// Found valid multi-file suffix
		if (m.size() == 2)
			return ReplaceMultiFileSuffix(_sFileName, m.position(1), m[1].length()); // Replace existing digits with a multi-file suffix %d

		std::string copy = _sFileName.string();
		const size_t dotPos = copy.rfind('.');					// Find a dot before extension
		copy.insert(dotPos, StrConst::HDF5H_DotFileExtMult);	// Insert a multi-file suffix .[[%d]]

		return copy;
	}

	// Converts the file name to a Dyssol format.
	std::filesystem::path ConvertFileName(const std::filesystem::path& _sFileName, bool _bOpen, bool _bSingleFile)
	{
		if (_bSingleFile)
			return _sFileName;

		return (_bOpen) ? MultiFileReadName(_sFileName) : MultiFileWriteName(_sFileName);
	}
}

CH5Handler::CH5Handler()
	: m_sFileName("")
	, m_bFileValid(false)
	, m_ph5File(nullptr)
{
	H5::Exception::dontPrint();
}

CH5Handler::~CH5Handler()
{
	Close();
}

void CH5Handler::Create(const std::filesystem::path& _sFileName, bool _bSingleFile /*= true*/)
{
	H5::Exception::dontPrint();

	OpenH5File(_sFileName, false, _bSingleFile);
}

void CH5Handler::Open(const std::filesystem::path& _sFileName)
{
	H5::Exception::dontPrint();

	OpenH5File(_sFileName, true, !m_bFileValid);
}

void CH5Handler::Close()
{
	if (!m_ph5File)
		return;

	m_ph5File->close();
	delete m_ph5File;
	m_ph5File = nullptr;
	m_bFileValid = false;
}

std::filesystem::path CH5Handler::FileName() const
{
	return m_sFileName;
}

std::string CH5Handler::CreateGroup(const std::string& _sPath, const std::string& _sGroupName) const
{
	if (!m_bFileValid)
		return "";

	const std::string& sPath = _sPath + "/" + _sGroupName;
	try
	{
		m_ph5File->createGroup(sPath);
	}
	catch (...)
	{
		return "";
	}

	return sPath;
}

std::string CH5Handler::OpenGroup(const std::string& _sPath, const std::string& _sGroupName) const
{
	if (!m_bFileValid)
		return "";

	const std::string& sPath = _sPath + "/" + _sGroupName;
	try
	{
		m_ph5File->openGroup(sPath);
	}
	catch (...)
	{
		return "";
	}

	return sPath;
}

void CH5Handler::WriteAttribute(const std::string& _sPath, const std::string& _sAttrName, int _nValue) const
{
	if (!m_bFileValid) return;

	H5::DataSpace h5Dataspace(H5S_SCALAR);
	H5::Group h5Group(m_ph5File->openGroup(_sPath));
	H5::Attribute h5Attribute(h5Group.createAttribute(_sAttrName, H5::PredType::NATIVE_INT, h5Dataspace, H5::PropList::DEFAULT));

	h5Attribute.write(H5::PredType::NATIVE_INT, &_nValue);

	h5Attribute.close();
	h5Group.close();
	h5Dataspace.close();
}

int CH5Handler::ReadAttribute(const std::string& _sPath, const std::string& _sAttrName) const
{
	if (!m_bFileValid) return 0;

	int nAttrValue;
	try
	{
		H5::Group h5Group(m_ph5File->openGroup(_sPath));
		if (h5Group.getId() == -1) return 0;

		H5::Attribute h5Attribute(h5Group.openAttribute(_sAttrName));
		h5Attribute.read(H5::PredType::NATIVE_INT, &nAttrValue);

		h5Attribute.close();
		h5Group.close();
	}
	catch (...)
	{
		nAttrValue = 0;
	}

	return nAttrValue;
}

void CH5Handler::WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::vector<std::vector<double>>& _vvData) const
{
	if (!m_bFileValid) return;
	if (_vvData.empty()) return;

	const hsize_t size{ _vvData.size() };

	H5::Group h5Group(m_ph5File->openGroup(_sPath));
	H5::DataSpace h5Dataspace(1, &size);

	auto itemtype = H5::PredType::NATIVE_DOUBLE;
	auto h5Varlentype = H5::VarLenType(&itemtype);
	H5::DataSet h5Dataset = h5Group.createDataSet(_sDatasetName, h5Varlentype, h5Dataspace);

	std::vector<hvl_t> buffer(size);
	for (size_t i = 0; i < size; ++i)
	{
		buffer[i].len = _vvData[i].size();
		buffer[i].p = !_vvData[i].empty() ? const_cast<double*>(&_vvData[i].front()) : nullptr;
	}
	h5Dataset.write(buffer.data(), h5Varlentype);

	h5Dataset.close();
	h5Varlentype.close();
	h5Dataspace.close();
	h5Group.close();
}

void CH5Handler::ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<std::vector<double>>& _vvData) const
{
	if (!m_bFileValid) return;

	try
	{
		H5::Group h5Group(m_ph5File->openGroup(_sPath));
		H5::DataSet h5Dataset = h5Group.openDataSet(_sDatasetName);
		H5::DataSpace h5Dataspace = h5Dataset.getSpace();
		auto itemtype = H5::PredType::NATIVE_DOUBLE;
		auto h5Varlentype = H5::VarLenType(&itemtype);
		const auto nRows = static_cast<size_t>(h5Dataspace.getSimpleExtentNpoints());

		if (nRows != 0)
		{
			auto* buffer = new hvl_t[nRows];
			h5Dataset.read(buffer, h5Varlentype);

			_vvData.resize(nRows);
			for (size_t i = 0; i < nRows; ++i)
			{
				const size_t nCols = buffer[i].len;
				_vvData[i].resize(buffer[i].len);
				auto* pTemp = static_cast<double*>(buffer[i].p);
				std::copy(&pTemp[0], &pTemp[nCols], _vvData[i].begin());
				free(buffer[i].p);
			}
			delete[] buffer;
		}

		h5Varlentype.close();
		h5Dataspace.close();
		h5Dataset.close();
		h5Group.close();
	}
	catch (...)
	{
		ReadDataOld(_sPath, _sDatasetName, _vvData);
	}
}

void CH5Handler::ReadDataOld(const std::string& _sPath, const std::string& _sDatasetName, std::vector<std::vector<double>>& _vvData) const
{
	if (!m_bFileValid) return;

	try
	{
		H5::Group h5Group(m_ph5File->openGroup(_sPath));
		CH5PacketTable h5PacketTable(h5Group, _sDatasetName);
		if (h5PacketTable.getId() == -1)
		{
			_vvData.clear();
			h5PacketTable.close();
			h5Group.close();
			return;
		}

		const auto nLen = static_cast<size_t>(h5PacketTable.GetPacketCount());
		if (nLen != 0)
		{
			auto* buffer = new hvl_t[nLen];
			h5PacketTable.GetPackets(0, nLen - 1, buffer);

			_vvData.resize(nLen);
			const size_t nDimensions = buffer[0].len;

			for (auto& v : _vvData)
				v.resize(nDimensions);

			for (size_t i = 0; i < nLen; ++i)
			{
				auto* pTemp = static_cast<double*>(buffer[i].p);
				std::copy(&pTemp[0], &pTemp[nDimensions], _vvData[i].begin());
			}

			for (size_t i = 0; i < nLen; ++i)
				free(buffer[i].p);
			delete[] buffer;
		}

		h5PacketTable.close();
		h5Group.close();
	}
	catch (...)
	{
		_vvData.clear();
	}
}

bool CH5Handler::IsValid() const
{
	return m_bFileValid;
}

void CH5Handler::WriteValue(const std::string& _sPath, const std::string& _sDatasetName, size_t _size, const H5::DataType& _type, const void* _pValue) const
{
	if (!m_bFileValid) return;

	H5::Group h5Group(m_ph5File->openGroup(_sPath));
	H5::DataSpace h5Dataspace(1, &static_cast<const hsize_t&>(_size));
	H5::DataSet h5Dataset = h5Group.createDataSet(_sDatasetName, _type, h5Dataspace);

	h5Dataset.write(_pValue, _type);

	h5Dataset.close();
	h5Dataspace.close();
	h5Group.close();
}

size_t CH5Handler::ReadSize(const std::string& _sPath, const std::string& _sDatasetName) const
{
	if (!m_bFileValid) return 0;

	try
	{
		H5::Group h5Group(m_ph5File->openGroup(_sPath));
#if H5_VERSION_GE(1, 10, 1) == true
		if (!h5Group.nameExists(_sDatasetName))
			return 0;
#endif

		H5::DataSet h5Dataset = h5Group.openDataSet(_sDatasetName);
		H5::DataSpace h5Dataspace = h5Dataset.getSpace();

		const auto nElemNum = static_cast<size_t>(h5Dataspace.getSimpleExtentNpoints());

		h5Dataset.close();
		h5Dataspace.close();
		h5Group.close();

		return nElemNum;
	}
	catch (...)
	{
		return 0;
	}
}

bool CH5Handler::ReadValue(const std::string& _sPath, const std::string& _sDatasetName, const H5::DataType& _type, void* _pRes) const
{
	if (!m_bFileValid) return false;

	try
	{
		H5::Group h5Group(m_ph5File->openGroup(_sPath));
		H5::DataSet h5Dataset = h5Group.openDataSet(_sDatasetName);

		h5Dataset.read(_pRes, _type);

		h5Dataset.close();
		h5Group.close();

		return true;
	}
	catch (...)
	{
		return false;
	}
}

void CH5Handler::OpenH5File(const std::filesystem::path& _sFileName, bool _bOpen, bool _bSingleFile)
{
	Close();

	m_sFileName = ConvertFileName(_sFileName, _bOpen, _bSingleFile);
	if (m_ph5File || m_sFileName.empty()) return;

	H5::FileAccPropList h5AccPropList = CreateFileAccPropList(_bSingleFile);
	try
	{
		m_ph5File = new H5::H5File(_sFileName.string(), _bOpen ? H5F_ACC_RDONLY : H5F_ACC_TRUNC , H5P_DEFAULT, h5AccPropList);
	}
	catch (...)
	{
		m_ph5File = nullptr;
	}
	h5AccPropList.close();

	if (m_ph5File && m_ph5File->getId() != -1)
		m_bFileValid = true;
}

std::filesystem::path CH5Handler::DisplayFileName(std::filesystem::path _fileName)
{
	if (_fileName.empty()) return {};
	std::smatch m = FindSuffix(_fileName, StrConst::HDF5H_FileExtFinalRegex);  // apply a regex search of a multi-file suffix in form [[%d]]
	if (m.size() < 2)                                                          // no such suffix found
		m = FindSuffix(_fileName, StrConst::HDF5H_FileExtInitRegex);           // apply a regex search of a multi-file suffix in form [[N]], where N is any number
	if (m.size() < 2) return _fileName;				                           // no valid multi-file suffix found
	const size_t pos = m.position(0);                                          // position of a multi-file suffix
	std::string copy = _fileName.string();
	copy.erase(m.position(0), m[0].length());                                  // remove multi-file suffix
	if(pos - 1 < copy.size() && copy[pos - 1] == '.')                          // a dot before multi-file suffix
		copy.erase(pos - 1, 1);						                           // remove dot
	return copy;
}
