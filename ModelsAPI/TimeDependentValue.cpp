/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "TimeDependentValue.h"
#include "DyssolStringConstants.h"
#include "DyssolUtilities.h"
#include <utility>

CTimeDependentValue::CTimeDependentValue(std::string _name, std::string _units) :
	m_name{std::move(_name)},
	m_units{std::move(_units)}
{
}

void CTimeDependentValue::SetName(const std::string& _name)
{
	m_name = _name;
}

std::string CTimeDependentValue::GetName() const
{
	return m_name;
}

void CTimeDependentValue::SetUnits(const std::string& _units)
{
	m_units = _units;
}

std::string CTimeDependentValue::GetUnits() const
{
	return m_units;
}

void CTimeDependentValue::AddTimePoint(double _time)
{
	if (HasTime(_time)) return;
	CopyTimePoint(_time, PreviousTime(_time));
}

void CTimeDependentValue::CopyTimePoint(double _timeDst, double _timeSrc)
{
	if (_timeDst < 0) return;
	SetValue(_timeDst, GetValue(_timeSrc));
}

void CTimeDependentValue::RemoveTimePoint(double _time)
{
	RemoveTimePoints(_time, _time);
}

void CTimeDependentValue::RemoveTimePoints(double _timeBeg, double _timeEnd)
{
	if (m_data.empty()) return;
	if (_timeBeg > _timeEnd) return;
	const auto [beg, end] = Interval(_timeBeg, _timeEnd);
	if (beg == m_data.end()) return;
	m_data.erase(beg, end);
}

void CTimeDependentValue::RemoveAllTimePoints()
{
	m_data.clear();
}

size_t CTimeDependentValue::GetTimePointsNumber() const
{
	return m_data.size();
}

void CTimeDependentValue::SetValue(double _time, double _value)
{
	if (_time < 0) return;
	const auto pos = std::lower_bound(m_data.begin(), m_data.end(), STDValue{ _time, 0.0 });
	if (pos == m_data.end())							// all existing times are smaller
		m_data.emplace_back(_time, _value);
	else if (std::abs((*pos).time - _time) <= m_eps)	// this time already exists
		*pos = { _time, _value };
	else												// insert to the right position
		m_data.insert(pos, { _time, _value });
}

double CTimeDependentValue::GetValue(double _time) const
{
	if (m_data.empty()) return {};							// return zero, if there are no data at all
	if (m_data.size() == 1) return m_data.front().value;	// return const value, if there is only a single value defined
	return Interpolate(_time);								// return interpolation otherwise
}

void CTimeDependentValue::SetRawData(const std::vector<std::vector<double>>& _data)
{
	m_data.clear();
	m_data.resize(_data.front().size());
	for (size_t i = 0; i < _data.front().size(); ++i)
		m_data[i] = { _data[0][i], _data[1][i] };
}

std::vector<std::vector<double>> CTimeDependentValue::GetRawData() const
{
	std::vector<std::vector<double>> res(2, std::vector<double>(m_data.size()));
	for (size_t i = 0; i < m_data.size(); ++i)
	{
		res[0][i] = m_data[i].time;
		res[1][i] = m_data[i].value;
	}
	return res;
}

void CTimeDependentValue::CopyFrom(double _time, const CTimeDependentValue& _source)
{
	SetValue(_time, _source.GetValue(_time));
}

void CTimeDependentValue::CopyFrom(double _timeDst, const CTimeDependentValue& _source, double _timeSrc)
{
	SetValue(_timeDst, _source.GetValue(_timeSrc));
}

void CTimeDependentValue::CopyFrom(double _timeBeg, double _timeEnd, const CTimeDependentValue& _source)
{
	RemoveTimePoints(_timeBeg, _timeEnd);
	const auto [beg, end] = _source.Interval(_timeBeg, _timeEnd);
	if (beg == _source.m_data.end()) return;
	for (auto it = beg; it != end; ++it)
		SetValue(it->time, it->value);
}

void CTimeDependentValue::Extrapolate(double _timeExtra, double _time)
{
	CopyTimePoint(_timeExtra, _time);
}

void CTimeDependentValue::Extrapolate(double _timeExtra, double _time1, double _time2)
{
	const double v1 = GetValue(_time1);
	const double v2 = GetValue(_time2);
	const double res = ::Interpolate(v1, v2, _time1, _time2, _timeExtra);
	SetValue(_timeExtra, res);
}

void CTimeDependentValue::Extrapolate(double _timeExtra, double _time1, double _time2, double _time3)
{
	const double v1 = GetValue(_time1);
	const double v2 = GetValue(_time2);
	const double v3 = GetValue(_time3);
	const double res = ::Extrapolate(v1, v2, v3, _time1, _time2, _time3, _timeExtra);
	SetValue(_timeExtra, res);
}

void CTimeDependentValue::SetCacheSettings(const SCacheSettings& _cache)
{
	// TODO: implement caching
}

void CTimeDependentValue::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	if (!_h5File.IsValid())	return;

	_h5File.WriteAttribute(_path, StrConst::H5AttrSaveVersion, m_saveVersion);
	_h5File.WriteData(_path, StrConst::TDV_H5Name, m_name);
	_h5File.WriteData(_path, StrConst::TDV_H5Units, m_units);

	// TODO: check constness of this and rewrite.
	auto data = GetRawData();
	_h5File.WriteData(_path, StrConst::TDV_H5Data, data);
}

void CTimeDependentValue::LoadFromFile(CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid())	return;

	//const int version = _h5File.ReadAttribute(_path, StrConst::H5AttrSaveVersion);
	_h5File.ReadData(_path, StrConst::TDV_H5Name, m_name);
	_h5File.ReadData(_path, StrConst::TDV_H5Units, m_units);

	std::vector<std::vector<double>> data;
	_h5File.ReadData(_path, StrConst::TDV_H5Data, data);
	SetRawData(data);
}

double CTimeDependentValue::Interpolate(double _time) const
{
	if (m_data.empty()) return {};
	if (m_data.size() == 1) return m_data.front().value;

	auto u = std::upper_bound(m_data.begin(), m_data.end(), STDValue{ _time, 0.0 });
	if (u == m_data.end())		return (--u)->value;	// nearest-neighbor extrapolation to the right
	if (u == m_data.begin())	return u->value;		// nearest-neighbor extrapolation to the left

	const auto l = u - 1;

	if (std::abs(u->time - _time) <= m_eps)	return u->value;		// exact value found
	if (std::abs(l->time - _time) <= m_eps)	return l->value;		// exact value found

	return (u->value - l->value) / (u->time - l->time) * (_time - l->time) + l->value; // linearly interpolated value
}

bool CTimeDependentValue::HasTime(double _time) const
{
	if (m_data.empty()) return false;
	const auto pos = std::lower_bound(m_data.begin(), m_data.end(), STDValue{ _time, 0.0 });
	if (pos == m_data.end()) return false;
	return std::abs((*pos).time - _time) <= m_eps;
}

double CTimeDependentValue::PreviousTime(double _time) const
{
	if (m_data.empty()) return {};
	auto pos = std::lower_bound(m_data.begin(), m_data.end(), STDValue{ _time, 0.0 });
	if (pos == m_data.begin()) return {};
	return (--pos)->time;
}

std::pair<std::vector<STDValue>::iterator, std::vector<STDValue>::iterator> CTimeDependentValue::Interval(double _timeBeg, double _timeEnd)
{
	auto end = std::upper_bound(m_data.begin(), m_data.end(), STDValue{ _timeEnd, 0.0 });
	if (end == m_data.begin()) return { m_data.end(), m_data.end() };
	auto beg = std::lower_bound(m_data.begin(), m_data.end(), STDValue{ _timeBeg, 0.0 });
	return { beg, end };
}

std::pair<std::vector<STDValue>::const_iterator, std::vector<STDValue>::const_iterator> CTimeDependentValue::Interval(double _timeBeg, double _timeEnd) const
{
	auto end = std::upper_bound(m_data.begin(), m_data.end(), STDValue{ _timeEnd, 0.0 });
	if (end == m_data.begin()) return { m_data.end(), m_data.end() };
	auto beg = std::lower_bound(m_data.begin(), m_data.end(), STDValue{ _timeBeg, 0.0 });
	return { beg, end };
}
