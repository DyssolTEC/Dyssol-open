/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DependentValues.h"
#include "ContainerFunctions.h"
#include "DyssolUtilities.h"
#include "StringFunctions.h"
#include <ostream>
#include <sstream>
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
	return ::Interpolate(m_params, m_values, _param);	// return interpolation otherwise
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

std::ostream& operator<<(std::ostream& _s, const CDependentValues& _obj)
{
	for (size_t i = 0; i < _obj.Size(); ++i)
		_s << " " << _obj.GetParamAt(i) << " " << _obj.GetValueAt(i);
	return _s;
}

std::istream& operator>>(std::istream& _s, CDependentValues& _obj)
{
	_obj.Clear();

	const auto timeOrValue = StringFunctions::GetValueFromStream<double>(_s);
	if (_s.eof())	// special treatment for single constant value
		_obj.SetValue(0.0, timeOrValue);
	else			// usual dependent values
	{
		_obj.SetValue(timeOrValue, StringFunctions::GetValueFromStream<double>(_s));
		while (!_s.eof())
		{
			// NB: order is important, do not put both calls directly into insert function
			const auto param = StringFunctions::GetValueFromStream<double>(_s);
			const auto value = StringFunctions::GetValueFromStream<double>(_s);
			_obj.SetValue(param, value);
		}
	}
	return _s;
}
