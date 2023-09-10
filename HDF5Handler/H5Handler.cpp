/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "H5Handler.h"
#include "PacketTable.h"
#include "DyssolStringConstants.h"
#include "FileSystem.h"

using namespace H5;

CH5Handler::CH5Handler() :
	m_sFileName(""),
	m_bFileValid(false),
	m_ph5File(nullptr)
{
	Exception::dontPrint();
}

CH5Handler::~CH5Handler()
{
	Close();
}

void CH5Handler::Create(const std::filesystem::path& _sFileName, bool _bSingleFile /*= true*/)
{
	Exception::dontPrint();

	OpenH5File(_sFileName, false, _bSingleFile);
}

void CH5Handler::Open(const std::filesystem::path& _sFileName)
{
	Exception::dontPrint();

	OpenH5File(_sFileName, true, true);
	if (!m_bFileValid)
		OpenH5File(_sFileName, true, false);
}

void CH5Handler::Close()
{
	if (!m_ph5File) return;

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
	if (!m_bFileValid) return "";

	std::string sPath = _sPath + "/" + _sGroupName;
	m_ph5File->createGroup(sPath);

	return sPath;
}

void CH5Handler::WriteAttribute(const std::string& _sPath, const std::string& _sAttrName, int _nValue) const
{
	if (!m_bFileValid) return;

	DataSpace h5Dataspace(H5S_SCALAR);
	Group h5Group(m_ph5File->openGroup(_sPath));
	Attribute h5Attribute(h5Group.createAttribute(_sAttrName, PredType::NATIVE_INT, h5Dataspace, PropList::DEFAULT));

	h5Attribute.write(PredType::NATIVE_INT, &_nValue);

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
		Group h5Group(m_ph5File->openGroup(_sPath));
		if (h5Group.getId() == -1) return 0;

		Attribute h5Attribute(h5Group.openAttribute(_sAttrName));
		h5Attribute.read(PredType::NATIVE_INT, &nAttrValue);

		h5Attribute.close();
		h5Group.close();
	}
	catch (...)
	{
		nAttrValue = 0;
	}

	return nAttrValue;
}

void CH5Handler::WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::string& _sData) const
{
	const char* pCString = _sData.c_str();
	WriteValue(_sPath, _sDatasetName, 1, h5String_type(), &pCString);
}

void CH5Handler::WriteData(const std::string& _sPath, const std::string& _sDatasetName, double _dData) const
{
	WriteValue(_sPath, _sDatasetName, 1, PredType::NATIVE_DOUBLE, &_dData);
}

void CH5Handler::WriteData(const std::string& _sPath, const std::string& _sDatasetName, uint32_t _nData) const
{
	WriteValue(_sPath, _sDatasetName, 1, PredType::NATIVE_UINT32, &_nData);
}

void CH5Handler::WriteData(const std::string& _sPath, const std::string& _sDatasetName, uint64_t _nData) const
{
	WriteValue(_sPath, _sDatasetName, 1, PredType::NATIVE_UINT64, &_nData);
}

void CH5Handler::WriteData(const std::string& _sPath, const std::string& _sDatasetName, int64_t _nData) const
{
	WriteValue(_sPath, _sDatasetName, 1, PredType::NATIVE_INT64, &_nData);
}

void CH5Handler::WriteData(const std::string& _sPath, const std::string& _sDatasetName, bool _bData) const
{
	WriteValue(_sPath, _sDatasetName, 1, PredType::NATIVE_HBOOL, &_bData);
}

void CH5Handler::WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::vector<std::string>& _vData) const
{
	if (_vData.empty()) return;

	std::vector<const char*> vCStrings(_vData.size());
	for (size_t i = 0; i < _vData.size(); ++i)
		vCStrings[i] = _vData[i].c_str();

	WriteValue(_sPath, _sDatasetName, _vData.size(), h5String_type(), &vCStrings.front());
}

void CH5Handler::WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::vector<uint32_t>& _vData) const
{
	if (_vData.empty()) return;
	WriteValue(_sPath, _sDatasetName, _vData.size(), PredType::NATIVE_UINT32, &_vData.front());
}

void CH5Handler::WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::vector<uint64_t>& _vData) const
{
	if (_vData.empty()) return;
	WriteValue(_sPath, _sDatasetName, _vData.size(), PredType::NATIVE_UINT64, &_vData.front());
}

void CH5Handler::WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::vector<int32_t>& _vData) const
{
	if (_vData.empty()) return;
	WriteValue(_sPath, _sDatasetName, _vData.size(), PredType::NATIVE_INT32, &_vData.front());
}

void CH5Handler::WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::vector<int64_t>& _vData) const
{
	if (_vData.empty()) return;
	WriteValue(_sPath, _sDatasetName, _vData.size(), PredType::NATIVE_INT64, &_vData.front());
}

void CH5Handler::WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::vector<double>& _vData) const
{
	if (_vData.empty()) return;
	WriteValue(_sPath, _sDatasetName, _vData.size(), PredType::NATIVE_DOUBLE, &_vData.front());
}

void CH5Handler::WriteData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<std::vector<double>>& _vvData) const
{
	if (!m_bFileValid) return;
	if (_vvData.empty()) return;

	const hsize_t size{ _vvData.size() };

	Group h5Group(m_ph5File->openGroup(_sPath));
	DataSpace h5Dataspace(1, &size);
	auto itemtype = H5::PredType::NATIVE_DOUBLE;
	auto h5Varlentype = H5::VarLenType(&itemtype);
	DataSet h5Dataset = h5Group.createDataSet(_sDatasetName, h5Varlentype, h5Dataspace);

	auto* buffer = new hvl_t[static_cast<size_t>(size)];
	for (size_t i = 0; i < size; ++i)
	{
		buffer[i].len = _vvData[i].size();
		buffer[i].p = !_vvData[i].empty() ? &_vvData[i].front() : nullptr;
	}
	h5Dataset.write(buffer, h5Varlentype);
	delete[] buffer;

	h5Dataset.close();
	h5Varlentype.close();
	h5Dataspace.close();
	h5Group.close();
}

void CH5Handler::WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::vector<STDValue>& _data) const
{
	WriteValue(_sPath, _sDatasetName, _data.size(), h5STDValue_type(), _data.data());
}

void CH5Handler::WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::vector<CPoint>& _data) const
{
	WriteValue(_sPath, _sDatasetName, _data.size(), h5CPoint_type(), _data.data());
}

void CH5Handler::ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::string& _sData) const
{
	char* buf{ nullptr };
	if (ReadValue(_sPath, _sDatasetName, h5String_type(), &buf))
		_sData = buf;
	else
		_sData.clear();
	free(buf);		// use free() since malloc is used internally by HDF5
}

void CH5Handler::ReadData(const std::string& _sPath, const std::string& _sDatasetName, double& _dData) const
{
	(void)ReadValue(_sPath, _sDatasetName, PredType::NATIVE_DOUBLE, &_dData);
}

void CH5Handler::ReadData(const std::string& _sPath, const std::string& _sDatasetName, uint32_t& _nData) const
{
	(void)ReadValue(_sPath, _sDatasetName, PredType::NATIVE_UINT32, &_nData);
}

void CH5Handler::ReadData(const std::string& _sPath, const std::string& _sDatasetName, uint64_t& _nData) const
{
	(void)ReadValue(_sPath, _sDatasetName, PredType::NATIVE_UINT64, &_nData);
}

void CH5Handler::ReadData(const std::string& _sPath, const std::string& _sDatasetName, int64_t& _nData) const
{
	(void)ReadValue(_sPath, _sDatasetName, PredType::NATIVE_INT64, &_nData);
}

void CH5Handler::ReadData(const std::string& _sPath, const std::string& _sDatasetName, bool& _bData) const
{
	(void)ReadValue(_sPath, _sDatasetName, PredType::NATIVE_HBOOL, &_bData);
}

void CH5Handler::ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<std::string>& _vData) const
{
	_vData.resize(ReadSize(_sPath, _sDatasetName));
	if (_vData.empty()) return;
	std::vector<char*> buf(_vData.size(), nullptr);
	if (ReadValue(_sPath, _sDatasetName, h5String_type(), buf.data()))
		std::copy(buf.begin(), buf.end(), _vData.begin());
	else
		_vData.clear();

	for (void* v : buf)
		free(v);	// use free() since malloc is used internally by HDF5
}

void CH5Handler::ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<uint32_t>& _vData) const
{
	_vData.resize(ReadSize(_sPath, _sDatasetName));
	if (!_vData.empty())
		if (!ReadValue(_sPath, _sDatasetName, PredType::NATIVE_UINT32, &_vData.front()))
			_vData.clear();
}

void CH5Handler::ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<uint64_t>& _vData) const
{
	_vData.resize(ReadSize(_sPath, _sDatasetName));
	if (!_vData.empty())
		if (!ReadValue(_sPath, _sDatasetName, PredType::NATIVE_UINT64, &_vData.front()))
			_vData.clear();
}

void CH5Handler::ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<int32_t>& _vData) const
{
	_vData.resize(ReadSize(_sPath, _sDatasetName));
	if (!_vData.empty())
		if (!ReadValue(_sPath, _sDatasetName, PredType::NATIVE_INT32, &_vData.front()))
			_vData.clear();
}

void CH5Handler::ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<int64_t>& _vData) const
{
	_vData.resize(ReadSize(_sPath, _sDatasetName));
	if (!_vData.empty())
		if (!ReadValue(_sPath, _sDatasetName, PredType::NATIVE_INT64, &_vData.front()))
			_vData.clear();
}

void CH5Handler::ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<double>& _vData) const
{
	_vData.resize(ReadSize(_sPath, _sDatasetName));
	if (!_vData.empty())
		if (!ReadValue(_sPath, _sDatasetName, PredType::NATIVE_DOUBLE, &_vData.front()))
			_vData.clear();
}

void CH5Handler::ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<std::vector<double>>& _vvData) const
{
	if (!m_bFileValid) return;

	try
	{
		Group h5Group(m_ph5File->openGroup(_sPath));
		DataSet h5Dataset = h5Group.openDataSet(_sDatasetName);
		DataSpace h5Dataspace = h5Dataset.getSpace();
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
		Group h5Group(m_ph5File->openGroup(_sPath));
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

void CH5Handler::ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<STDValue>& _data) const
{
	_data.resize(ReadSize(_sPath, _sDatasetName));
	if (_data.empty()) return;
	(void)ReadValue(_sPath, _sDatasetName, h5STDValue_type(), _data.data());
}

void CH5Handler::ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<CPoint>& _data) const
{
	_data.resize(ReadSize(_sPath, _sDatasetName));
	if (_data.empty()) return;

	(void)ReadValue(_sPath, _sDatasetName, h5CPoint_type(), _data.data());
}

bool CH5Handler::IsValid() const
{
	return m_bFileValid;
}

void CH5Handler::WriteValue(const std::string& _sPath, const std::string& _sDatasetName, const hsize_t _size, const DataType& _type, const void* _pValue) const
{
	if (!m_bFileValid) return;

	Group h5Group(m_ph5File->openGroup(_sPath));
	DataSpace h5Dataspace(1, &_size);
	DataSet h5Dataset = h5Group.createDataSet(_sDatasetName, _type, h5Dataspace);

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
		Group h5Group(m_ph5File->openGroup(_sPath));
		if (!h5Group.nameExists(_sDatasetName))	return 0;

		DataSet h5Dataset = h5Group.openDataSet(_sDatasetName);
		DataSpace h5Dataspace = h5Dataset.getSpace();

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
		Group h5Group(m_ph5File->openGroup(_sPath));
		DataSet h5Dataset = h5Group.openDataSet(_sDatasetName);

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

	FileAccPropList h5AccPropList = CreateFileAccPropList(_bSingleFile);
	try
	{
		m_ph5File = new H5File(_sFileName.string(), _bOpen ? H5F_ACC_RDONLY : H5F_ACC_TRUNC, H5P_DEFAULT, h5AccPropList);
	}
	catch (...)
	{
		m_ph5File = nullptr;
	}
	h5AccPropList.close();

	if (m_ph5File && m_ph5File->getId() != -1)
		m_bFileValid = true;
}

FileAccPropList CH5Handler::CreateFileAccPropList(bool _bSingleFile)
{
	const hsize_t cChunkSize = 500;
	const hsize_t cCacheSize = 1024 * 1024 * 50;		// in bytes
	const hsize_t cMaxH5FileSize = 1024 * 1024 * 2000;	// in bytes

	FileAccPropList h5AccPropList;
	if (!_bSingleFile)
		h5AccPropList.setFamily(cMaxH5FileSize, H5P_DEFAULT);
	h5AccPropList.setFcloseDegree(H5F_CLOSE_STRONG);
	h5AccPropList.setCache(cChunkSize, cChunkSize, cCacheSize, 0.5);
	return h5AccPropList;
}

CompType& CH5Handler::h5CPoint_type()
{
	static std::unique_ptr<CompType> type{};

	if (!type)
	{
		type = std::make_unique<CompType>(sizeof(CPoint));
		type->insertMember("x", HOFFSET(CPoint, x), PredType::NATIVE_DOUBLE);
		type->insertMember("y", HOFFSET(CPoint, y), PredType::NATIVE_DOUBLE);
	}
	return *type;
}

H5::CompType& CH5Handler::h5STDValue_type()
{
	static std::unique_ptr<CompType> type{};

	if (!type)
	{
		type = std::make_unique<CompType>(sizeof(STDValue));
		type->insertMember("time" , HOFFSET(STDValue, time ), PredType::NATIVE_DOUBLE);
		type->insertMember("value", HOFFSET(STDValue, value), PredType::NATIVE_DOUBLE);
	}
	return *type;
}

H5::StrType& CH5Handler::h5String_type()
{
	static std::unique_ptr<StrType> type{};

	if (!type)
	{
		type = std::make_unique<StrType>(PredType::C_S1, H5T_VARIABLE);
	}
	return *type;
}

std::filesystem::path CH5Handler::ConvertFileName(const std::filesystem::path& _sFileName, bool _bOpen, bool _bSingleFile)
{
	if (_bSingleFile) return _sFileName;
	else if (_bOpen)  return MultiFileReadName(_sFileName);
	else			  return MultiFileWriteName(_sFileName);
}

std::filesystem::path CH5Handler::MultiFileReadName(const std::filesystem::path& _sFileName)
{
	if (_sFileName.empty()) return {};
	if (FindSuffix(_sFileName, StrConst::HDF5H_FileExtFinalRegex).size() == 2)       // already in proper multi-file format, i.e. contains [[%d]]
		return _sFileName;
	const std::smatch m = FindSuffix(_sFileName, StrConst::HDF5H_FileExtInitRegex);	 // apply a regex search of a multi-file suffix in form [[N]], where N is any number
	if (m.size() < 2) return _sFileName;									         // no valid multi-file suffix found
	return ReplaceMultiFileSuffix(_sFileName, m.position(1), m[1].length());         // replace existing digits with a multi-file suffix %d
}

std::filesystem::path CH5Handler::MultiFileWriteName(std::filesystem::path _sFileName)
{
	if (_sFileName.empty()) return {};
	if (!StringFunctions::CompareCaseInsensitive(_sFileName.extension().string(), StrConst::HDF5H_DotFileExt)) // no proper extension
		_sFileName += StrConst::HDF5H_DotFileExt;                                                              // add proper extension
	if (FindSuffix(_sFileName, StrConst::HDF5H_FileExtFinalRegex).size() == 2)                                 // already in proper multi-file format, i.e. contains [[%d]]
		return _sFileName;
	const std::smatch m = FindSuffix(_sFileName, StrConst::HDF5H_FileExtInitRegex);                            // apply a regex search of a multi-file suffix in form [[N]], where N is any number
	if (m.size() == 2)                                                                                         // found valid multi-file suffix in form [[N]], where N is any number
		return ReplaceMultiFileSuffix(_sFileName, m.position(1), m[1].length());                               // replace existing digits with a multi-file suffix %d
	std::string copy = _sFileName.string();
	const std::size_t dotPos = copy.rfind('.');                                                                // find a dot before extension
	copy.insert(dotPos, StrConst::HDF5H_DotFileExtMult);                                                       // insert a multi-file suffix .[[%d]]
	return copy;
}

std::smatch CH5Handler::FindSuffix(const std::filesystem::path& _str, const std::string& _regexStr)
{
	const std::regex r(_regexStr);							// regex expression
	std::smatch m;											// results of regex search
	std::string copy = _str.string();
	std::sregex_iterator it(copy.begin(), copy.end(), r);	// apply regex
	for (; it != std::sregex_iterator(); ++it) m = *it;		// find the last occurrence
	return m;
}

std::filesystem::path CH5Handler::ReplaceMultiFileSuffix(std::filesystem::path _str, size_t _pos, size_t _len)
{
	std::string copy = _str.string();
	copy.erase(_pos, _len);				            // remove digits
	copy.insert(_pos, StrConst::HDF5H_FileExtSpec); // insert multi-file extension
	return copy;
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
