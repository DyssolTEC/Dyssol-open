/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolTypes.h"
#include "DyssolFilesystem.h"

#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>

namespace H5
{
	class H5File;
	class DataType;
}

template<typename T>
const H5::DataType& GetType()
{
	static_assert(sizeof(T) == 0, "No specialization found");
	return {};
}

template<> const H5::DataType& GetType<double>();
template<> const H5::DataType& GetType<uint32_t>();
template<> const H5::DataType& GetType<uint64_t>();
template<> const H5::DataType& GetType<int32_t>();
template<> const H5::DataType& GetType<int64_t>();
template<> const H5::DataType& GetType<bool>();
template<> const H5::DataType& GetType<std::string>();
template<> const H5::DataType& GetType<CPoint>();
template<> const H5::DataType& GetType<STDValue>();

/**
 *	IO with HDF5 files. Two modes:
 *	1. Single file: all data stored in a single file.
 *	2. Multi file: the file is split into parts of 2000 Mb each.
 */
class CH5Handler
{
	std::filesystem::path m_sFileName;
	bool m_bFileValid;
	H5::H5File* m_ph5File;

	template <typename T>
	struct is_vector : std::false_type {};

	template <typename T>
	struct is_vector<std::vector<T>> : std::true_type {};

	template <typename T>
	struct is_map : std::false_type {};

	template <typename K, typename V>
	struct is_map<std::map<K, V>> : std::true_type {};

	template <typename K, typename V>
	struct is_map<std::unordered_map<K, V>> : std::true_type {};

	// C++20 std::type_identity
	template <typename T>
	struct type_identity { using type = T; };

	template <typename T>
	struct type_identity<std::vector<T>> { using type = T; };

public:
	CH5Handler();
	~CH5Handler();

	void Create(const std::filesystem::path& _sFileName, bool _bSingleFile = true);	/// Create new file with truncation.
	void Open(const std::filesystem::path& _sFileName);								/// Open existing file.
	void Close();																	/// Close current file.
	std::filesystem::path FileName() const;											/// Returns current file name.

	std::string CreateGroup(const std::string& _sPath, const std::string& _sGroupName) const;
	std::string OpenGroup(const std::string& _sPath, const std::string& _sGroupName) const;

	void WriteAttribute(const std::string& _sPath, const std::string& _sAttrName, int _nValue) const;
	int ReadAttribute(const std::string& _sPath, const std::string& _sAttrName) const;

	template<typename T>
	void WriteData(const std::string& _sPath, const std::string& _sDatasetName, const T& _data) const
	{
		if constexpr (std::is_same_v<T, std::string>)
		{
			const char* ptr = _data.c_str();
			WriteValue(_sPath, _sDatasetName, 1, GetType<std::string>(), &ptr);
		}
		else if constexpr (std::is_enum_v<T>)
		{
			auto value = static_cast<int64_t>(_data);
			WriteValue(_sPath, _sDatasetName, 1, GetType<decltype(value)>(), &value);
		}
		else
		{
			WriteValue(_sPath, _sDatasetName, 1, GetType<T>(), &_data);
		}
	}

	template<typename T, typename = std::enable_if_t<!is_vector<T>::value>>
	void WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::vector<T>& _data) const
	{
		if (_data.empty())
			return;

		if constexpr (std::is_same_v<T, std::string>)
		{
			std::vector<const char*> vCStrings(_data.size());
			std::transform(_data.begin(), _data.end(), vCStrings.begin(), [](const auto& _str) { return _str.c_str(); });

			WriteValue(_sPath, _sDatasetName, vCStrings.size(), GetType<std::string>(), &vCStrings.front());
		}
		else
		{
			WriteValue(_sPath, _sDatasetName, _data.size(), GetType<T>(), &_data.front());
		}
	}

	template <typename M, typename = std::enable_if_t<is_map<M>::value>>
	void WriteData(const std::string& _sPath, const std::string& _sDatasetName, const M& _data)
	{
		using K = typename M::key_type;
		using V = typename M::mapped_type;

		std::vector<K> keys;
		std::vector<typename type_identity<V>::type> values;
		std::vector<size_t> sizes;

		for (const auto& [key, value] : _data)
		{
			keys.emplace_back(key);

			if constexpr (is_vector<V>::value)
			{
				values.insert(values.end(), value.begin(), value.end());
				sizes.emplace_back(value.size());
			}
			else
			{
				values.emplace_back(value);
			}
		}

		WriteData(_sPath, _sDatasetName + "K", keys);
		WriteData(_sPath, _sDatasetName + "V", values);
		WriteData(_sPath, _sDatasetName + "S", sizes);
	}

	void WriteData(const std::string& _sPath, const std::string& _sDatasetName, const std::vector<std::vector<double>>& _vvData) const;

	template<typename T>
	void ReadData(const std::string& _sPath, const std::string& _sDatasetName, T& _data) const
	{
		if constexpr (std::is_same_v<T, std::string>)
		{
			_data.clear();

			char* buffer{ nullptr };
			if (ReadValue(_sPath, _sDatasetName, GetType<std::string>(), &buffer))
				_data = buffer;

			free(buffer);	// use free() since malloc is used internally by HDF5
		}
		else if constexpr (std::is_enum_v<T>)
		{
			int64_t value{};
			ReadValue(_sPath, _sDatasetName, GetType<decltype(value)>(), &value);
			_data = static_cast<T>(value);
		}
		else
		{
			ReadValue(_sPath, _sDatasetName, GetType<T>(), &_data);
		}
	}

	template<typename T, typename = std::enable_if_t<!is_vector<T>::value>>
	void ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<T>& _data) const
	{
		_data.clear();

		_data.resize(ReadSize(_sPath, _sDatasetName));

		if constexpr (std::is_same_v<T, std::string>)
		{
			std::vector<char*> buffer(_data.size(), nullptr);
			if (ReadValue(_sPath, _sDatasetName, GetType<std::string>(), buffer.data()))
				std::copy(buffer.begin(), buffer.end(), _data.begin());

			// use free() since malloc is used internally by HDF5
			std::for_each(buffer.begin(), buffer.end(), [](auto* _ptr) { free(_ptr); });
		}
		else
		{
			if(!_data.empty())
				ReadValue(_sPath, _sDatasetName, GetType<T>(), &_data.front());
		}
	}

	template <typename M, typename = std::enable_if_t<is_map<M>::value>>
	void ReadData(const std::string& _sPath, const std::string& _sDatasetName, M& _data)
	{
		using K = typename M::key_type;
		using V = typename M::mapped_type;

		_data.clear();

		std::vector<K> keys;
		std::vector<typename type_identity<V>::type> values;
		std::vector<size_t> sizes;

		ReadData(_sPath, _sDatasetName + "K", keys);
		ReadData(_sPath, _sDatasetName + "V", values);
		ReadData(_sPath, _sDatasetName + "S", sizes);

		auto it = values.begin();
		for (size_t i = 0; i < keys.size(); ++i)
		{
			if constexpr (is_vector<V>::value)
			{
				_data[keys[i]] = V(it, it + sizes[i]);
				it += sizes[i];
			}
			else
			{
				_data[keys[i]] = *it;
				++it;
			}
		}
	}

	void ReadData(const std::string& _sPath, const std::string& _sDatasetName, std::vector<std::vector<double>>& _vvData) const;
	void ReadDataOld(const std::string& _sPath, const std::string& _sDatasetName, std::vector<std::vector<double>>& _vvData) const;

	bool IsValid() const;

	// Returns displayable file name in form "path/FileName.dflw", removing all [[%d]] and [[N]] from it
	static std::filesystem::path DisplayFileName(std::filesystem::path _fileName);

private:
	void WriteValue(const std::string& _sPath, const std::string& _sDatasetName, size_t _size, const H5::DataType& _type, const void* _pValue) const;
	// Read element count
	size_t ReadSize(const std::string& _sPath, const std::string& _sDatasetName) const;
	bool ReadValue(const std::string& _sPath, const std::string& _sDatasetName, const H5::DataType& _type, void* _pRes) const;

	void OpenH5File(const std::filesystem::path& _sFileName, bool _bOpen, bool _bSingleFile);
};
