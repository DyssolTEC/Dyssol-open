/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "H5Handler.h"

#include "PacketTable.h"
#include "StringFunctions.h"

#include <regex>

namespace
{
	const std::string FILE_EXT             = "dflw";
	const std::string DOT_FILE_EXT         = "." + FILE_EXT;
	const std::string FILE_EXT_SPEC        = "%d";
	const std::string FILE_EXT_INIT_REGEX  = R"(\[\[([0-9]+)\]\])";
	const std::string FILE_EXT_FINAL_REGEX = R"(\[\[(%d)\]\])";
	const std::string FILE_EXT_MULT        = "[[" + FILE_EXT_SPEC + "]]";
	const std::string DOT_FILE_EXT_MULT    = "." + FILE_EXT_MULT;

	H5::FileAccPropList CreateFileAccPropList(bool _isSingleFile)
	{
		constexpr hsize_t CHUNK_SIZE = 500;
		constexpr hsize_t CACHE_SIZE = 1024 * 1024 * 50;		// in bytes
		constexpr hsize_t MAX_H5FILE_SIZE = 1024 * 1024 * 2000;	// in bytes

		H5::FileAccPropList h5AccPropList;
		if (!_isSingleFile)
			h5AccPropList.setFamily(MAX_H5FILE_SIZE, H5P_DEFAULT);
		h5AccPropList.setFcloseDegree(H5F_CLOSE_STRONG);
		h5AccPropList.setCache(CHUNK_SIZE, CHUNK_SIZE, CACHE_SIZE, 0.5);

		return h5AccPropList;
	}

	// Returns results of the regex search.
	std::smatch FindSuffix(const std::filesystem::path& _str, const std::string_view& _regexStr)
	{
		const auto& str = _str.string();                        // Convert path to string
		const std::regex r(_regexStr.begin(), _regexStr.end()); // Compile regex expression
		std::smatch m;                                          // Results of regex search
		std::regex_search(str, m, r);                           // Perform regex search
		return m;
	}

	// Replaces _len symbols starting from _pos in _str with a multi-file suffix.
	std::filesystem::path ReplaceMultiFileSuffix(const std::filesystem::path& _str, size_t _pos, size_t _len)
	{
		std::string copy = _str.string();
		copy.replace(_pos, _len, FILE_EXT_SPEC); // Replace existing digits with multi-file extension
		return copy;
	}

	// Transforms the file name to the form needed to read from multi-file.
	std::filesystem::path MultiFileReadName(const std::filesystem::path& _fileName)
	{
		if (_fileName.empty())
			return {};

		// Already in proper multi-file format, i.e., contains [[%d]]
		if (FindSuffix(_fileName, FILE_EXT_FINAL_REGEX).size() == 2)
			return _fileName;

		// Search for a multi-file suffix in the form [[N]], where N is any number
		const auto& m = FindSuffix(_fileName, FILE_EXT_INIT_REGEX);
		if (m.size() < 2)
			return _fileName;	// No valid multi-file suffix found

		// Replace existing digits with a multi-file suffix %d
		return ReplaceMultiFileSuffix(_fileName, m.position(1), m[1].length());
	}

	// Transforms the file name to the form needed to write to multi-file.
	std::filesystem::path MultiFileWriteName(std::filesystem::path _fileName)
	{
		if (_fileName.empty())
			return {};

		// Check if the extension is missing or incorrect
		if (!StringFunctions::CompareCaseInsensitive(_fileName.extension().string(), DOT_FILE_EXT))
			_fileName += DOT_FILE_EXT; // Add proper extension

		// Already in proper multi-file format, i.e., contains [[%d]]
		if (FindSuffix(_fileName, FILE_EXT_FINAL_REGEX).size() == 2)
			return _fileName;

		// Search for a multi-file suffix in the form [[N]], where N is any number
		const auto& m = FindSuffix(_fileName, FILE_EXT_INIT_REGEX);
		// Found valid multi-file suffix
		if (m.size() == 2)
			return ReplaceMultiFileSuffix(_fileName, m.position(1), m[1].length()); // Replace existing digits with a multi-file suffix %d

		std::string copy = _fileName.string();
		const size_t dotPos = copy.rfind('.');	// Find a dot before extension
		copy.insert(dotPos, DOT_FILE_EXT_MULT);	// Insert a multi-file suffix .[[%d]]

		return copy;
	}

	// Converts the file name to a Dyssol format.
	std::filesystem::path ConvertFileName(const std::filesystem::path& _fileName, bool _open, bool _isSingleFile)
	{
		if (_isSingleFile)
			return _fileName;
		return (_open) ? MultiFileReadName(_fileName) : MultiFileWriteName(_fileName);
	}
}

CH5Handler::CH5Handler()
{
	H5::Exception::dontPrint();
}

CH5Handler::~CH5Handler()
{
	Close();
}

void CH5Handler::Create(const std::filesystem::path& _fileName, bool _isSingleFile /*= true*/)
{
	H5::Exception::dontPrint();

	OpenH5File(_fileName, false, _isSingleFile);
}

void CH5Handler::Open(const std::filesystem::path& _fileName)
{
	H5::Exception::dontPrint();

	OpenH5File(_fileName, true, !m_isFileValid);
}

void CH5Handler::Close()
{
	if (!m_h5File)
		return;

	m_h5File->close();
	delete m_h5File;
	m_h5File = nullptr;
	m_isFileValid = false;
}

std::filesystem::path CH5Handler::FileName() const
{
	return m_fileName;
}

std::string CH5Handler::CreateGroup(const std::string& _path, const std::string& _groupName) const
{
	if (!m_isFileValid)
		return "";

	const std::string& path = _path + "/" + _groupName;
	try
	{
		m_h5File->createGroup(path);
	}
	catch (...)
	{
		return "";
	}

	return path;
}

std::string CH5Handler::OpenGroup(const std::string& _path, const std::string& _groupName) const
{
	if (!m_isFileValid)
		return "";

	const std::string& path = _path + "/" + _groupName;
	try
	{
		m_h5File->openGroup(path);
	}
	catch (...)
	{
		return "";
	}

	return path;
}

void CH5Handler::WriteAttribute(const std::string& _path, const std::string& _attrName, int _value) const
{
	if (!m_isFileValid) return;

	H5::DataSpace h5Dataspace(H5S_SCALAR);
	H5::Group h5Group(m_h5File->openGroup(_path));
	H5::Attribute h5Attribute(h5Group.createAttribute(_attrName, H5::PredType::NATIVE_INT, h5Dataspace, H5::PropList::DEFAULT));

	h5Attribute.write(H5::PredType::NATIVE_INT, &_value);

	h5Attribute.close();
	h5Group.close();
	h5Dataspace.close();
}

int CH5Handler::ReadAttribute(const std::string& _path, const std::string& _attrName) const
{
	if (!m_isFileValid) return 0;

	int attrValue;
	try
	{
		H5::Group h5Group(m_h5File->openGroup(_path));
		if (h5Group.getId() == -1) return 0;

		H5::Attribute h5Attribute(h5Group.openAttribute(_attrName));
		h5Attribute.read(H5::PredType::NATIVE_INT, &attrValue);

		h5Attribute.close();
		h5Group.close();
	}
	catch (...)
	{
		attrValue = 0;
	}

	return attrValue;
}

void CH5Handler::WriteData(const std::string& _path, const std::string& _dataset, const std::vector<std::vector<double>>& _data) const
{
	if (!m_isFileValid) return;
	if (_data.empty()) return;

	const hsize_t size{ _data.size() };

	H5::Group h5Group(m_h5File->openGroup(_path));
	H5::DataSpace h5Dataspace(1, &size);

	auto itemtype = H5::PredType::NATIVE_DOUBLE;
	auto h5Varlentype = H5::VarLenType(&itemtype);
	H5::DataSet h5Dataset = h5Group.createDataSet(_dataset, h5Varlentype, h5Dataspace);

	std::vector<hvl_t> buffer(size);
	for (size_t i = 0; i < size; ++i)
	{
		buffer[i].len = _data[i].size();
		buffer[i].p = !_data[i].empty() ? const_cast<double*>(&_data[i].front()) : nullptr;
	}
	h5Dataset.write(buffer.data(), h5Varlentype);

	h5Dataset.close();
	h5Varlentype.close();
	h5Dataspace.close();
	h5Group.close();
}

void CH5Handler::ReadData(const std::string& _path, const std::string& _dataset, std::vector<std::vector<double>>& _data) const
{
	if (!m_isFileValid) return;

	try
	{
		H5::Group h5Group(m_h5File->openGroup(_path));
		H5::DataSet h5Dataset = h5Group.openDataSet(_dataset);
		H5::DataSpace h5Dataspace = h5Dataset.getSpace();
		auto itemtype = H5::PredType::NATIVE_DOUBLE;
		auto h5Varlentype = H5::VarLenType(&itemtype);
		const auto nRows = static_cast<size_t>(h5Dataspace.getSimpleExtentNpoints());

		if (nRows != 0)
		{
			auto* buffer = new hvl_t[nRows];
			h5Dataset.read(buffer, h5Varlentype);

			_data.resize(nRows);
			for (size_t i = 0; i < nRows; ++i)
			{
				const size_t nCols = buffer[i].len;
				_data[i].resize(buffer[i].len);
				auto* pTemp = static_cast<double*>(buffer[i].p);
				std::copy(&pTemp[0], &pTemp[nCols], _data[i].begin());
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
		ReadDataOld(_path, _dataset, _data);
	}
}

void CH5Handler::ReadDataOld(const std::string& _path, const std::string& _dataset, std::vector<std::vector<double>>& _data) const
{
	if (!m_isFileValid) return;

	try
	{
		H5::Group h5Group(m_h5File->openGroup(_path));
		CH5PacketTable h5PacketTable(h5Group, _dataset);
		if (h5PacketTable.getId() == -1)
		{
			_data.clear();
			h5PacketTable.close();
			h5Group.close();
			return;
		}

		const auto nLen = static_cast<size_t>(h5PacketTable.GetPacketCount());
		if (nLen != 0)
		{
			auto* buffer = new hvl_t[nLen];
			h5PacketTable.GetPackets(0, nLen - 1, buffer);

			_data.resize(nLen);
			const size_t nDimensions = buffer[0].len;

			for (auto& v : _data)
				v.resize(nDimensions);

			for (size_t i = 0; i < nLen; ++i)
			{
				auto* pTemp = static_cast<double*>(buffer[i].p);
				std::copy(&pTemp[0], &pTemp[nDimensions], _data[i].begin());
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
		_data.clear();
	}
}

bool CH5Handler::IsValid() const
{
	return m_isFileValid;
}

void CH5Handler::WriteValue(const std::string& _path, const std::string& _dataset, size_t _size, const H5::DataType& _type, const void* _value) const
{
	if (!m_isFileValid) return;

	H5::Group h5Group(m_h5File->openGroup(_path));
	H5::DataSpace h5Dataspace(1, &static_cast<const hsize_t&>(_size));
	H5::DataSet h5Dataset = h5Group.createDataSet(_dataset, _type, h5Dataspace);

	h5Dataset.write(_value, _type);

	h5Dataset.close();
	h5Dataspace.close();
	h5Group.close();
}

size_t CH5Handler::ReadSize(const std::string& _path, const std::string& _dataset) const
{
	if (!m_isFileValid) return 0;

	try
	{
		H5::Group h5Group(m_h5File->openGroup(_path));
#if H5_VERSION_GE(1, 10, 1) == true
		if (!h5Group.nameExists(_dataset))
			return 0;
#endif

		H5::DataSet h5Dataset = h5Group.openDataSet(_dataset);
		H5::DataSpace h5Dataspace = h5Dataset.getSpace();

		const auto elemNum = static_cast<size_t>(h5Dataspace.getSimpleExtentNpoints());

		h5Dataset.close();
		h5Dataspace.close();
		h5Group.close();

		return elemNum;
	}
	catch (...)
	{
		return 0;
	}
}

bool CH5Handler::ReadValue(const std::string& _path, const std::string& _dataset, const H5::DataType& _type, void* _value) const
{
	if (!m_isFileValid) return false;

	try
	{
		H5::Group h5Group(m_h5File->openGroup(_path));
		H5::DataSet h5Dataset = h5Group.openDataSet(_dataset);

		h5Dataset.read(_value, _type);

		h5Dataset.close();
		h5Group.close();

		return true;
	}
	catch (...)
	{
		return false;
	}
}

void CH5Handler::OpenH5File(const std::filesystem::path& _fileName, bool _isOpen, bool _isSingleFile)
{
	Close();

	m_fileName = ConvertFileName(_fileName, _isOpen, _isSingleFile);
	if (m_h5File || m_fileName.empty()) return;

	H5::FileAccPropList h5AccPropList = CreateFileAccPropList(_isSingleFile);
	try
	{
		m_h5File = new H5::H5File(_fileName.string(), _isOpen ? H5F_ACC_RDONLY : H5F_ACC_TRUNC, H5P_DEFAULT, h5AccPropList);
	}
	catch (...)
	{
		m_h5File = nullptr;
	}
	h5AccPropList.close();

	if (m_h5File && m_h5File->getId() != -1)
		m_isFileValid = true;
}

std::filesystem::path CH5Handler::DisplayFileName(std::filesystem::path _fileName)
{
	if (_fileName.empty()) return {};
	std::smatch m = FindSuffix(_fileName, FILE_EXT_FINAL_REGEX);  // apply a regex search of a multi-file suffix in form [[%d]]
	if (m.size() < 2)                                             // no such suffix found
		m = FindSuffix(_fileName, FILE_EXT_INIT_REGEX);           // apply a regex search of a multi-file suffix in form [[N]], where N is any number
	if (m.size() < 2) return _fileName;				              // no valid multi-file suffix found
	const size_t pos = m.position(0);                             // position of a multi-file suffix
	std::string copy = _fileName.string();
	copy.erase(m.position(0), m[0].length());                     // remove multi-file suffix
	if (pos - 1 < copy.size() && copy[pos - 1] == '.')            // a dot before multi-file suffix
		copy.erase(pos - 1, 1);						              // remove dot
	return copy;
}
