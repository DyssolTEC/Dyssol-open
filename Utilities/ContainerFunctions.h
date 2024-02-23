/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolTypes.h"
#include <algorithm>
#include <map>
#include <set>
#include <vector>

// Checks whether the vector contains a specified element.
template<typename T>
bool VectorContains(const std::vector<T>& _vec, T _value)
{
	return std::find(_vec.begin(), _vec.end(), _value) != _vec.end();
}

// Checks whether the sorted vector contains a specified element.
template<typename T>
bool VectorContainsSorted(const std::vector<T>& _vec, T _value)
{
	return std::binary_search(_vec.begin(), _vec.end(), _value);
}

// Checks whether the vector contains a specified element.
template<typename T, typename FUN>
bool VectorContains(const std::vector<T>& _vec, const FUN& _fun)
{
	return std::find_if(_vec.begin(), _vec.end(), _fun) != _vec.end();
}

// Returns index of a specified element or -1 if it does not exist.
template<typename T>
size_t VectorFind(const std::vector<T>& _vec, T _val)
{
	const auto it = std::find(_vec.begin(), _vec.end(), _val);
	if (it == _vec.end()) return -1;
	return std::distance(_vec.begin(), it);
}

// Returns index of a specified element or -1 if it does not exist.
template<typename T, typename FUN>
size_t VectorFind(const std::vector<T>& _vec, const FUN& _fun)
{
	const auto it = std::find_if(_vec.begin(), _vec.end(), _fun);
	if (it == _vec.end()) return -1;
	return std::distance(_vec.begin(), it);
}

// Return the lowest element in vector.
template<typename T>
T VectorMin(const std::vector<T>& _vec)
{
	if (_vec.empty()) return T{};
	return *std::min_element(_vec.begin(), _vec.end());
}

// Return the greatest element in vector.
template<typename T>
T VectorMax(const std::vector<T>& _vec)
{
	if (_vec.empty()) return T{};
	return *std::max_element(_vec.begin(), _vec.end());
}

// Removes all elements of the vector that satisfy the given predicate.
template<typename T, typename FUN>
void VectorDelete(std::vector<T>& _vec, const FUN& _fun)
{
	_vec.erase(std::remove_if(_vec.begin(), _vec.end(), _fun), _vec.end());
}

// Removes an element with the given index if it does exist.
template<typename T>
void VectorDelete(std::vector<T>& _vec, T _value)
{
	_vec.erase(std::remove(_vec.begin(), _vec.end(), _value), _vec.end());
}

// Removes an element with the given index if it does exist.
template<typename T>
void VectorDelete(std::vector<T>& _vec, size_t _index)
{
	if (_index < _vec.size())
		_vec.erase(_vec.begin() + _index);
}

// Moves an element with the given index upwards/downwards in the vector.
template<typename T>
void VectorShift(std::vector<T>& _vec, size_t _index, EDirection _direction)
{
	switch (_direction)
	{
	case EDirection::UP:
		if (_index < _vec.size() && _index != 0)
			std::iter_swap(_vec.begin() + _index, _vec.begin() + _index - 1);
		break;
	case EDirection::DOWN:
		if (_index < _vec.size() && _index != _vec.size() - 1)
			std::iter_swap(_vec.begin() + _index, _vec.begin() + _index + 1);
		break;
	}
}

// Checks if the vector contains only unique elements.
template<typename T>
bool VectorUnique(const std::vector<T>& _vec)
{
	std::set<T> unique(_vec.begin(), _vec.end());
	return unique.size() == _vec.size();
}

// Checks if the sorted vector contains only unique elements.
template<typename T>
bool VectorUniqueSorted(const std::vector<T>& _vec)
{
	return std::adjacent_find(_vec.begin(), _vec.end()) == _vec.end();
}


// Checks if the element is unique in vector, returns indizes of duplicates
template<typename T>
std::vector<size_t> VectorGetDuplicates(const std::vector<T*>& _vec, size_t _index)
{
	T* elem = _vec[_index];
	std::vector<size_t> indizes;
	for (size_t i = 0; i < _vec.size(); i++)
		if (_index != i && *elem == *_vec[i])
			indizes.push_back(i);
	return indizes;
}

// Checks if the element is unique in vector, returns indizes of duplicates
template<typename T>
std::vector<size_t> VectorGetDuplicates(const std::vector<T>& _vec, size_t _index)
{
	T elem = _vec[_index];
	std::vector<size_t> indizes;
	for (size_t i = 0; i < _vec.size(); i++)
		if (_index != i && elem == _vec[i])
			indizes.push_back(i);
	return indizes;
}

// Adds two equally sized vectors element-wise and writes results to the third one.
template<typename T>
void AddVectors(const std::vector<T>& _vec1, const std::vector<T>& _vec2, std::vector<T>& _res)
{
	if (_vec1.size() == _vec2.size())
		std::transform(_vec1.begin(), _vec1.end(), _vec2.begin(), _res.begin(), std::plus<>());
	else if (!_vec1.empty() && _vec2.empty())
		_res = _vec1;
	else if (_vec1.empty() && !_vec2.empty())
		_res = _vec2;
}

// Adds two equally sized vectors element-wise and returns the result.
template<typename T>
std::vector<T> AddVectors(const std::vector<T>& _vec1, const std::vector<T>& _vec2)
{
	std::vector<T> res(_vec1.size());
	if (_vec1.size() == _vec2.size())
		std::transform(_vec1.begin(), _vec1.end(), _vec2.begin(), res.begin(), std::plus<>());
	else if (!_vec1.empty() && _vec2.empty())
		res = _vec1;
	else if (_vec1.empty() && !_vec2.empty())
		res = _vec2;
	return res;
}

// Multiplies all values of the vector with value and returns the result.
template<typename T>
std::vector<T> MultVector(const std::vector<T>& _vec, const T& _val)
{
	std::vector<T> res(_vec.size());
	std::transform(_vec.begin(), _vec.end(), res.begin(), [&](const T& el) { return el * _val; });
	return res;
}

// Checks whether the map contains a specified key.
template<typename K, typename V>
bool MapContainsKey(const std::map<K, V>& _map, K _key)
{
	return _map.find(_key) != _map.end();
}

// Checks whether the multimap contains a specified key.
template<typename K, typename V>
bool MapContainsKey(const std::multimap<K, V>& _map, K _key)
{
	return _map.find(_key) != _map.end();
}

// Checks whether the map contains a specified value.
template<typename K, typename V>
bool MapContainsValue(const std::map<K, V>& _map, V _value)
{
	for (const auto& p : _map)
		if (p.second == _value)
			return true;
	return false;
}

// Returns all keys defined in the map.
template<typename K, typename V>
std::vector<K> MapKeys(const std::map<K, V>& _map)
{
	std::vector<K> keys;
	keys.reserve(_map.size());
	for (auto const& entry : _map)
		keys.push_back(entry.first);
	return keys;
}

// Returns all values defined in the map.
template<typename K, typename V>
std::vector<V> MapValues(const std::map<K, V>& _map)
{
	std::vector<V> values;
	values.reserve(_map.size());
	for (auto const& entry : _map)
		values.push_back(entry.second);
	return values;
}

// Returns a sorted copy of the vector.
template<typename T>
std::vector<T> VectorSort(const std::vector<T>& _v)
{
	std::vector<T> res = _v;
	std::sort(res.begin(), res.end());
	return res;
}

// Calculates a sorted union of two sorted vectors.
template<typename T>
void VectorsUnionSorted(const std::vector<T>& _v1, const std::vector<T>& _v2, std::vector<T>& _res)
{
	_res.resize(_v1.size() + _v2.size());
	_res.resize(std::set_union(_v1.begin(), _v1.end(), _v2.begin(), _v2.end(), _res.begin()) - _res.begin());
}

// Calculates and returns a sorted union of two sorted vectors.
template<typename T>
std::vector<T> VectorsUnionSorted(const std::vector<T>& _v1, const std::vector<T>& _v2)
{
	std::vector<T> res;
	VectorsUnionSorted(_v1, _v2, res);
	return res;
}

// Calculates union of two unsorted vectors.
template<typename T>
void VectorsUnionUnsorted(const std::vector<T>& _v1, const std::vector<T>& _v2, std::vector<T>& _res)
{
	auto s1 = VectorSort(_v1);
	auto s2 = VectorSort(_v2);
	_res.resize(s1.size() + s2.size());
	_res.resize(std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), _res.begin()) - _res.begin());
}

// Calculates and returns union of two unsorted vectors.
template<typename T>
std::vector<T> VectorsUnionUnsorted(const std::vector<T>& _v1, const std::vector<T>& _v2)
{
	std::vector<T> res;
	VectorsUnionUnsorted(_v1, _v2, res);
	return res;
}

// Returns a vector with reserved size.
template<typename T> std::vector<T> ReservedVector(size_t _size)
{
	std::vector<T> res;
	res.reserve(_size);
	return res;
}

// Returns vector of elements of _v1, which are not found in _v2. Both vectors are unsorted.
template<typename T> std::vector<T> VectorDifference(const std::vector<T>& _v1, const std::vector<T>& _v2)
{
	std::vector<T> res;
	std::copy_if(_v1.begin(), _v1.end(), std::back_inserter(res), [&_v2](const T& val) { return !VectorContains(_v2, val); });
	return res;
}

/**
 * \brief Returns the values at the specified indices in the data array.
 * \details Does not perform any out-of-boundary checks.
 * \param _data Input array of data.
 * \param _ind List of indices of data to be extracted to the slice.
 * \return Vector of data with given indices.
 */
inline std::vector<double> Slice(const double* const _data, const std::vector<size_t>& _ind)
{
	std::vector<double> res(_ind.size());
	if (_ind.empty()) return res;
	if (std::adjacent_find(_ind.begin(), _ind.end(), [](size_t i1, size_t i2) { return i2 != i1 + 1; }) == _ind.end())
		std::copy(_data + _ind.front(), _data + _ind.back() + 1, res.begin());
	else
		for (size_t i = 0; i < _ind.size(); ++i)
			res[i] = _data[i];
	return res;
}

/**
 * \brief Returns the values at the specified indices in the data vector.
 * \details Does not perform any out-of-boundary checks.
 * \param _data Input vector of data.
 * \param _ind List of indices of data to be extracted to the slice.
 * \return Vector of data with given indices.
 */
inline std::vector<double> Slice(const std::vector<double>& _data, const std::vector<size_t>& _ind)
{
	return Slice(_data.data(), _ind);
}