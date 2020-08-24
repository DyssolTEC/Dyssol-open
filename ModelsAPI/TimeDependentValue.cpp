#include "TimeDependentValue.h"

#include <utility>

const uint16_t CTimeDependentValue::m_saveVersion = 0;

CTimeDependentValue::CTimeDependentValue(std::string _name, std::string _units) :
	m_name{std::move(_name)},
	m_units{std::move(_units)}
{
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
	auto end = std::upper_bound(m_data.begin(), m_data.end(), STDValue{ _timeEnd, 0.0 });
	if (end == m_data.begin()) return;
	const auto beg = std::lower_bound(m_data.begin(), m_data.end(), STDValue{ _timeBeg, 0.0 });
	m_data.erase(beg, --end);
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

double CTimeDependentValue::Interpolate(double _time) const
{
	if (m_data.empty()) return {};
	if (m_data.size() == 1) return m_data.front().value;

	auto u = std::upper_bound(m_data.begin(), m_data.end(), STDValue{ _time, 0.0 });
	if (u == m_data.end())		return (--u)->value;	// nearest-neighbor extrapolation to the right
	if (u == m_data.begin())	return u->value;		// nearest-neighbor extrapolation to the left

	const auto l = --u;

	if (std::abs(u->time - _time) <= m_eps)	return u->value;		// exact value found
	if (std::abs(l->time - _time) <= m_eps)	return l->value;		// exact value found

	return (u->value - l->value) / (u->time - l->time) * (_time - l->time) + l->value; // linearly interpolated value
}

bool CTimeDependentValue::HasTime(double _time) const
{
	const auto pos = std::lower_bound(m_data.begin(), m_data.end(), STDValue{ _time, 0.0 });
	return std::abs((*pos).time - _time) <= m_eps;
}

double CTimeDependentValue::PreviousTime(double _time) const
{
	if (m_data.empty()) return {};
	auto pos = std::lower_bound(m_data.begin(), m_data.end(), STDValue{ _time, 0.0 });
	if (pos == m_data.begin()) return {};
	return (--pos)->time;
}
