/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DependentValues.h"
#include "ContainerFunctions.h"
#include <ostream>
#include <numeric>

CDependentValues::CDependentValues(const std::vector<double>& _params, const std::vector<double>& _values)
{
	SetValues(_params, _values);
}

bool CDependentValues::IsEmpty() const
{
	return m_values.empty();
}

size_t CDependentValues::Size() const
{
	return m_values.size();
}

double CDependentValues::GetValue(double _param) const
{
	if (m_values.empty()) return 0;						// return zero, if there are no data at all
	if (m_values.size() == 1) return m_values.front();	// return const value, if there is only a single value defined
	return Interpolate(_param);							// return interpolation otherwise
}

void CDependentValues::SetValue(double _param, double _value)
{
	const auto pos = std::lower_bound(m_params.begin(), m_params.end(), _param);
	if (pos == m_params.end())					// all existing parameters are smaller
	{
		m_values.emplace_back(_value);
		m_params.emplace_back(_param);
	}
	else if (std::abs(*pos - _param) <= m_eps)	// this parameter already exists
	{
		m_values[std::distance(m_params.begin(), pos)] = _value;
		*pos = _param;
	}
	else										// insert to the right position
	{
		m_values.insert(m_values.begin() + std::distance(m_params.begin(), pos), _value);
		m_params.insert(pos, _param);
	}
}

void CDependentValues::SetValues(const std::vector<double>& _params, const std::vector<double>& _values)
{
	if (m_params.size() != m_values.size()) return;
	if (IsEmpty())
	{
		m_params = _params;
		m_values = _values;
	}
	else
		for (size_t i = 0; i < _params.size(); ++i)
			SetValue(_params[i], _values[i]);
}

void CDependentValues::RemoveValue(double _param)
{
	const auto pos = std::lower_bound(m_params.begin(), m_params.end(), _param);
	if (pos != m_params.end() && std::abs(*pos - _param) <= m_eps)
	{
		m_values.erase(m_values.begin() + std::distance(m_params.begin(), pos));
		m_params.erase(pos);
	}
}

double CDependentValues::GetParamAt(size_t _index) const
{
	if (_index >= m_params.size()) return {};
	return m_params[_index];
}

double CDependentValues::GetValueAt(size_t _index) const
{
	if (_index >= m_values.size()) return {};
	return m_values[_index];
}

std::pair<double, double> CDependentValues::GetPairAt(size_t _index) const
{
	if (_index >= m_values.size()) return {};
	return { m_params[_index], m_values[_index] };
}

void CDependentValues::RemovePairAt(size_t _index)
{
	if (_index >= m_values.size()) return;
	m_params.erase(m_params.begin() + _index);
	m_values.erase(m_values.begin() + _index);
}

std::vector<double> CDependentValues::GetParamsList() const
{
	return m_params;
}

std::vector<double> CDependentValues::GetValuesList() const
{
	return m_values;
}

void CDependentValues::SetParamsList(const std::vector<double>& _params)
{
	if (_params.size() == m_params.size())
		m_params = VectorSort(_params);
}

void CDependentValues::SetValuesList(const std::vector<double>& _values)
{
	if (_values.size() == m_values.size())
		m_values = _values;
}

bool CDependentValues::HasParam(double _param) const
{
	return VectorContainsSorted(m_params, _param);
}

bool CDependentValues::IsConst() const
{
	if (m_values.empty()) return true;
	return std::adjacent_find(m_values.begin(), m_values.end(), std::not_equal_to<>()) == m_values.end();
}

void CDependentValues::Clear()
{
	m_params.clear();
	m_values.clear();
}

bool CDependentValues::operator==(const CDependentValues& _v) const
{
	return m_params == _v.m_params && m_values == _v.m_values;
}

double CDependentValues::Interpolate(double _param) const
{
	const auto upper = std::upper_bound(m_params.begin(), m_params.end(), _param);
	if (upper == m_params.end())	return m_values.back();		// nearest-neighbor extrapolation to the right
	if (upper == m_params.begin())	return m_values.front();	// nearest-neighbor extrapolation to the left

	auto lower = upper;
	--lower;

	const double upar = *upper;
	const double lpar = *lower;
	const double uval = m_values[std::distance(m_params.begin(), upper)];
	const double lval = m_values[std::distance(m_params.begin(), lower)];

	if (std::abs(upar - _param) <= m_eps)	return uval;	// exact value found
	if (std::abs(lpar - _param) <= m_eps)	return lval;	// exact value found

	return (uval - lval) / (upar - lpar) * (_param - lpar) + lval; // linearly interpolated value
}

std::ostream& operator<<(std::ostream& _os, const CDependentValues& _obj)
{
	if (!_obj.m_values.empty())
		_os << _obj.m_params[0] << " " << _obj.m_values[0];
	for (size_t i = 1; i < _obj.m_values.size(); ++i)
		_os << " " << _obj.m_params[i] << " " << _obj.m_values[i];
	return _os;
}
