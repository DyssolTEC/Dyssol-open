/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DependentValues.h"
#include <ostream>

bool CDependentValues::empty() const
{
	return m_data.empty();
}

size_t CDependentValues::size() const
{
	return m_data.size();
}

std::map<double, double>::iterator CDependentValues::begin()
{
	return m_data.begin();
}

std::map<double, double>::const_iterator CDependentValues::begin() const
{
	return m_data.begin();
}

std::map<double, double>::iterator CDependentValues::end()
{
	return m_data.end();
}

std::map<double, double>::const_iterator CDependentValues::end() const
{
	return m_data.end();
}

std::pair<double, double> CDependentValues::front()
{
	return *m_data.begin();
}

std::pair<double, double> CDependentValues::front() const
{
	return *m_data.begin();
}

std::pair<double, double> CDependentValues::back()
{
	return *m_data.rbegin();
}

std::pair<double, double> CDependentValues::back() const
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

std::pair<double, double> CDependentValues::GetPairAt(size_t _index) const
{
	if (_index >= m_data.size()) return { 0, 0 };
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
	std::vector<double> res;
	for (const auto& pair : m_data)
		res.push_back(pair.first);
	return res;
}

std::vector<double> CDependentValues::GetValuesList() const
{
	std::vector<double> res;
	for (const auto& pair : m_data)
		res.push_back(pair.second);
	return res;
}

bool CDependentValues::IsDefined(double _param)
{
	return m_data.find(_param) != m_data.end();
}

void CDependentValues::clear()
{
	m_data.clear();
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

std::ostream& operator<<(std::ostream& _os, const CDependentValues& _obj)
{
	if (!_obj.m_data.empty())
		_os << _obj.GetParamAt(0) << " " << _obj.GetValueAt(0);
	for (size_t i = 1; i < _obj.m_data.size(); ++i)
		_os << " " << _obj.GetParamAt(i) << " " << _obj.GetValueAt(i);
	return  _os;
}
