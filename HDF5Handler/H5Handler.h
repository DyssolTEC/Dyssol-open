/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "H5Cpp.h"

#include "DyssolTypes.h"
#include "DyssolFilesystem.h"

#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>

template<typename T>
const H5::DataType& GetType()
{
	static_assert(sizeof(T) == 0, "No specialization found");
	return H5::PredType::PREDTYPE_CONST;
}

template<> inline const H5::DataType& GetType<double>()   { return H5::PredType::NATIVE_DOUBLE; }
template<> inline const H5::DataType& GetType<uint32_t>() { return H5::PredType::NATIVE_UINT32; }
template<> inline const H5::DataType& GetType<uint64_t>() { return H5::PredType::NATIVE_UINT64; }
template<> inline const H5::DataType& GetType<int32_t>()  { return H5::PredType::NATIVE_INT32; };
template<> inline const H5::DataType& GetType<int64_t>()  { return H5::PredType::NATIVE_INT64; }
template<> inline const H5::DataType& GetType<bool>()     { return H5::PredType::NATIVE_HBOOL; }

template<>
inline const H5::DataType& GetType<CPoint>()
{
	static std::unique_ptr<H5::CompType> type{};

	if (!type)
	{
		type = std::make_unique<H5::CompType>(sizeof(CPoint));
		type->insertMember("x", HOFFSET(CPoint, x), H5::PredType::NATIVE_DOUBLE);
		type->insertMember("y", HOFFSET(CPoint, y), H5::PredType::NATIVE_DOUBLE);
	}
	return *type;
}

template<>
inline const H5::DataType& GetType<STDValue>()
{
	static std::unique_ptr<H5::CompType> type{};

	if (!type)
	{
		type = std::make_unique<H5::CompType>(sizeof(STDValue));
		type->insertMember("time", HOFFSET(STDValue, time), H5::PredType::NATIVE_DOUBLE);
		type->insertMember("value", HOFFSET(STDValue, value), H5::PredType::NATIVE_DOUBLE);
	}
	return *type;
}

template<>
inline const H5::DataType& GetType<SInterval>()
{
	static std::unique_ptr<H5::CompType> type{};

	if (!type)
	{
		type = std::make_unique<H5::CompType>(sizeof(SInterval));
		type->insertMember("min", HOFFSET(SInterval, min), H5::PredType::NATIVE_DOUBLE);
		type->insertMember("max", HOFFSET(SInterval, max), H5::PredType::NATIVE_DOUBLE);
	}
	return *type;
}

template<>
inline const H5::DataType& GetType<std::string>()
{
	static H5::StrType type{ H5::PredType::C_S1, H5T_VARIABLE };

	return type;
}

/**
 *	IO with HDF5 files. Two modes:
 *	1. Single file: all data stored in a single file.
 *	2. Multi file: the file is split into parts of 2000 Mb each.
 */
class CH5Handler
{
	std::filesystem::path m_fileName{};
	bool m_isFileValid{ false };
	H5::H5File* m_h5File{ nullptr };

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

	void Create(const std::filesystem::path& _fileName, bool _isSingleFile = true);	///< Create new file with truncation.
	void Open(const std::filesystem::path& _fileName);								///< Open existing file.
	void Close();																	///< Close current file.
	[[nodiscard]] std::filesystem::path FileName() const;							///< Returns current file name.

	[[nodiscard]] std::string CreateGroup(const std::string& _path, const std::string& _groupName) const;
	[[nodiscard]] std::string OpenGroup(const std::string& _path, const std::string& _groupName) const;

	void WriteAttribute(const std::string& _path, const std::string& _attrName, int _value) const;
	[[nodiscard]] int ReadAttribute(const std::string& _path, const std::string& _attrName) const;

	template<typename T>
	void WriteData(const std::string& _path, const std::string& _dataset, const T& _data) const
	{
		if constexpr (std::is_convertible_v<T, std::string_view>)
		{
			const char* value = _data.c_str();
			WriteValue(_path, _dataset, 1, GetType<std::string>(), &value);
		}
		else if constexpr (std::is_enum_v<T>)
		{
			auto value = static_cast<int64_t>(_data);
			WriteValue(_path, _dataset, 1, GetType<decltype(value)>(), &value);
		}
		else
		{
			WriteValue(_path, _dataset, 1, GetType<T>(), &_data);
		}
	}

	template<typename T, typename = std::enable_if_t<!is_vector<T>::value>>
	void WriteData(const std::string& _path, const std::string& _dataset, const std::vector<T>& _data) const
	{
		if (_data.empty())
			return;

		if constexpr (std::is_convertible_v<T, std::string_view>)
		{
			std::vector<const char*> strings(_data.size());
			std::transform(_data.begin(), _data.end(), strings.begin(), [](const auto& _str) { return _str.c_str(); });
			WriteValue(_path, _dataset, strings.size(), GetType<std::string>(), &strings.front());
		}
		else
		{
			WriteValue(_path, _dataset, _data.size(), GetType<T>(), &_data.front());
		}
	}

	template <typename M, typename = std::enable_if_t<is_map<M>::value>>
	void WriteData(const std::string& _path, const std::string& _dataset, const M& _data)
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

		WriteData(_path, _dataset + "K", keys);
		WriteData(_path, _dataset + "V", values);

		if (!sizes.empty())
			WriteData(_path, _dataset + "S", sizes);
	}

	void WriteData(const std::string& _path, const std::string& _dataset, const std::vector<std::vector<double>>& _data) const;

	template<typename T>
	void ReadData(const std::string& _path, const std::string& _dataset, T& _data) const
	{
		if constexpr (std::is_same_v<T, std::string>)
		{
			_data.clear();

			char* buffer{ nullptr };
			if (ReadValue(_path, _dataset, GetType<std::string>(), &buffer))
				_data = buffer;

			free(buffer); // use free() since malloc is used internally by HDF5
		}
		else if constexpr (std::is_enum_v<T>)
		{
			int64_t value{ ~0 };
			ReadValue(_path, _dataset, GetType<decltype(value)>(), &value);
			_data = static_cast<T>(value);
		}
		else if constexpr (is_map<T>::value)
		{
			using K = typename T::key_type;
			using V = typename T::mapped_type;

			_data.clear();

			std::vector<K> keys;
			std::vector<typename type_identity<V>::type> values;
			std::vector<size_t> sizes;

			ReadData(_path, _dataset + "K", keys);
			ReadData(_path, _dataset + "V", values);

			const auto& datasetNameSize = _dataset + "S";
			if (ReadSize(_path, datasetNameSize) > 0)
				ReadData(_path, datasetNameSize, sizes);

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
		else
		{
			ReadValue(_path, _dataset, GetType<T>(), &_data);
		}
	}

	template<typename T, typename = std::enable_if_t<!is_vector<T>::value>>
	void ReadData(const std::string& _path, const std::string& _dataset, std::vector<T>& _data) const
	{
		_data.clear();
		_data.resize(ReadSize(_path, _dataset));

		if constexpr (std::is_same_v<T, std::string>)
		{
			std::vector<char*> buffer(_data.size(), nullptr);
			if (ReadValue(_path, _dataset, GetType<std::string>(), buffer.data()))
				std::copy(buffer.begin(), buffer.end(), _data.begin());

			// use free() since malloc is used internally by HDF5
			std::for_each(buffer.begin(), buffer.end(), [](auto* _ptr) { free(_ptr); });
		}
		else
		{
			if (!_data.empty())
				ReadValue(_path, _dataset, GetType<T>(), &_data.front());
		}
	}

	void ReadData(const std::string& _path, const std::string& _dataset, std::vector<std::vector<double>>& _data) const;
	// TODO: Remove
	void ReadDataOld(const std::string& _path, const std::string& _dataset, std::vector<std::vector<double>>& _data) const;

	[[nodiscard]] bool IsValid() const;

	// Returns displayable file name in form "path/FileName.dflw", removing all [[%d]] and [[N]] from it
	static std::filesystem::path DisplayFileName(std::filesystem::path _fileName);

private:
	void WriteValue(const std::string& _path, const std::string& _dataset, size_t _size, const H5::DataType& _type, const void* _value) const;
	// Read element count
	[[nodiscard]] size_t ReadSize(const std::string& _path, const std::string& _dataset) const;
	bool ReadValue(const std::string& _path, const std::string& _dataset, const H5::DataType& _type, void* _value) const;

	void OpenH5File(const std::filesystem::path& _fileName, bool _isOpen, bool _isSingleFile);
};
