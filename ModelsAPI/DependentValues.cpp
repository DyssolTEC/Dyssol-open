/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DependentValues.h"
#include "ContainerFunctions.h"
#include "StringFunctions.h"
#include <ostream>
#include <sstream>
#include <numeric>

bool CDependentValues::IsEmpty() const
{
	return m_data.empty();
}

size_t CDependentValues::Size() const
{
	return m_data.size();
}

CDependentValues::iterator CDependentValues::begin()
{
	return m_data.begin();
}

CDependentValues::const_iterator CDependentValues::begin() const
{
	return m_data.begin();
}

CDependentValues::const_iterator CDependentValues::cbegin() const
{
	return m_data.cbegin();
}

CDependentValues::iterator CDependentValues::end()
{
	return m_data.end();
}

CDependentValues::const_iterator CDependentValues::end() const
{
	return m_data.end();
}

CDependentValues::const_iterator CDependentValues::cend() const
{
	return m_data.cend();
}

CDependentValues::value_type CDependentValues::front()
{
	return *m_data.begin();
}

CDependentValues::value_type CDependentValues::front() const
{
	return *m_data.begin();
}

CDependentValues::value_type CDependentValues::back()
{
	return *m_data.rbegin();
}

CDependentValues::value_type CDependentValues::back() const
{
	return *m_data.rbegin();
}

double CDependentValues::GetValue(double _param) const
{
	if (m_data.empty()) return 0;							// return zero, if there are no data at all
	if (m_data.size() == 1) return m_data.begin()->second;	// return const value, if there is only a single value defined
	return Interpolate(_param);								// return interpolation otherwise
}

void CDependentValues::SetValue(double _param, double _value)
{
	m_data.insert_or_assign(_param, _value);
}

void CDependentValues::RemoveValue(double _param)
{
	m_data.erase(_param);
}

double CDependentValues::GetParamAt(size_t _index) const
{
	return GetPairAt(_index).first;
}

double CDependentValues::GetValueAt(size_t _index) const
{
	return GetPairAt(_index).second;
}

CDependentValues::value_type CDependentValues::GetPairAt(size_t _index) const
{
	if (_index >= m_data.size()) return {};
	auto it = m_data.begin();
	std::advance(it, _index);
	return *it;
}

void CDependentValues::RemovePairAt(size_t _index)
{
	if (_index >= m_data.size()) return;
	auto it = m_data.begin();
	std::advance(it, _index);
	m_data.erase(it);
}

std::vector<double> CDependentValues::GetParamsList() const
{
	auto res = ReservedVector<double>(m_data.size());
	for (const auto& [param, value] : m_data)
		res.push_back(param);
	return res;
}

std::vector<double> CDependentValues::GetValuesList() const
{
	auto res = ReservedVector<double>(m_data.size());
	for (const auto& [param, value] : m_data)
		res.push_back(value);
	return res;
}

bool CDependentValues::HasParam(double _param) const
{
	return m_data.find(_param) != m_data.end();
}

bool CDependentValues::IsConst() const
{
	if (m_data.empty()) return true;
	const auto val0 = GetValueAt(0);
	return std::all_of(m_data.begin(), m_data.end(), [&](const auto& pair) { return pair.second == val0; });
}

void CDependentValues::Clear()
{
	m_data.clear();
}

CDependentValues CDependentValues::Unique(const CDependentValues& _table)
{
	auto params = _table.GetParamsList();
	auto values = _table.GetValuesList();
	// initial index locations
	std::vector<size_t> iparams(params.size());
	std::vector<size_t> ivalues(values.size());
	std::iota(iparams.begin(), iparams.end(), 0);
	std::iota(ivalues.begin(), ivalues.end(), 0);
	// sort indices based on comparing values in vectors
	std::stable_sort(iparams.begin(), iparams.end(), [&params](size_t i1, size_t i2) { return params[i1] < params[i2]; });
	std::stable_sort(ivalues.begin(), ivalues.end(), [&values](size_t i1, size_t i2) { return values[i1] < values[i2]; });
	// indices of elements to be deleted
	std::set<size_t> todel;
	// find all but first repeating params
	for (size_t i = 1; i < iparams.size(); ++i)
		if (params[iparams[i]] == params[iparams[i - 1]])
			todel.insert(iparams[i]);
	// find all but first repeating values
	for (size_t i = 1; i < ivalues.size(); ++i)
		if (values[ivalues[i]] == values[ivalues[i - 1]])
			todel.insert(ivalues[i]);
	// create a copy of the table
	CDependentValues res = _table;
	// remove not unique
	for (const auto& i : todel)
		res.RemoveValue(params[i]);
	return res;
}

bool CDependentValues::operator==(const CDependentValues& _v) const
{
	return m_data == _v.m_data;
}

double CDependentValues::Interpolate(double _param) const
{
	auto upper = m_data.upper_bound(_param);
	if (upper == m_data.end())		return (--upper)->second;	// nearest-neighbor extrapolation to the right
	if (upper == m_data.begin())	return upper->second;		// nearest-neighbor extrapolation to the left

	auto lower = upper;
	--lower;

	if (upper->first == _param)	return upper->second;		// exact value found
	if (lower->first == _param)	return lower->second;		// exact value found

	return (upper->second - lower->second) / (upper->first - lower->first) * (_param - lower->first) + lower->second; // linearly interpolated value
}

std::ostream& operator<<(std::ostream& _s, const CDependentValues& _obj)
{
	_s << _obj.Size();
	for (const auto& [param, value] : _obj.m_data)
		_s << " " << param << " " << value;
	return _s;
}

std::istream& operator>>(std::istream& _s, CDependentValues& _obj)
{
	_obj.m_data.clear();

	std::stringstream numberOrValue{ StringFunctions::GetValueFromStream<std::string>(_s) };
	if (_s.eof())	// special treatment for single constant value
	{
		const auto value = StringFunctions::GetValueFromStream<double>(numberOrValue);
		_obj.m_data.insert({ 0.0, value });
	}
	else			// usual dependent values
	{
		const auto number = StringFunctions::GetValueFromStream<size_t>(numberOrValue);
		for (size_t i = 0; i < number; ++i)
		{
			// NB: order is important, therefore it is not possible to put both calls directly into insert function.
			const auto param = StringFunctions::GetValueFromStream<double>(_s);
			const auto value = StringFunctions::GetValueFromStream<double>(_s);
			_obj.m_data.insert({ param, value });
		}
	}
	return _s;
}
