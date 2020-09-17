/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "StateVariable.h"

CStateVariable::CStateVariable(std::string _name, double _initValue, bool _trackHistory) :
	m_name{ std::move(_name) },
	m_value{ 0.0 },
	m_trackHistory{ _trackHistory },
	m_initValue{ _initValue },
	m_valueStored{ 0.0 }
{
}

void CStateVariable::Initialize()
{
	// set current value, clear history, initialize history with current value
	SetValue(0.0, m_initValue);
}

void CStateVariable::Clear()
{
	m_value = 0.0;
	m_valueStored = 0.0;
	m_data.clear();
}

std::string CStateVariable::GetName() const
{
	return m_name;
}

void CStateVariable::SetName(const std::string& _name)
{
	m_name = _name;
}

double CStateVariable::GetValue() const
{
	return m_value;
}

void CStateVariable::SetValue(double _value)
{
	m_value = _value;
}

void CStateVariable::SetValue(double _time, double _value)
{
	SetValue(_value);
	AddToHistory(_time, _value);
}

bool CStateVariable::IsTrackHistory() const
{
	return m_trackHistory;
}

void CStateVariable::SetTrackHistory(const bool _trackHistory)
{
	m_trackHistory = _trackHistory;
}

void CStateVariable::SaveState()
{
	m_valueStored = m_value;
}

void CStateVariable::LoadState()
{
	m_value = m_valueStored;
}

bool CStateVariable::HasHistory() const
{
	return m_data.size() > 1;
}

std::vector<STDValue> CStateVariable::GetHistory() const
{
	return m_data;
}

void CStateVariable::AddToHistory(double _time, double _value)
{
	if (_time < 0) return;
	if (m_data.empty() || m_data.back().time < _time)		// time is larger as all already stored
		m_data.emplace_back(_time , _value);				// add it to the end
	else if (std::abs(m_data.back().time - _time) <= m_eps)	// this time is the last stored
		m_data.back() = { _time, _value };					// replace it
	else													// there are larger time points
	{
		// clear larger time points
		m_data.erase(std::lower_bound(m_data.begin(), m_data.end(), STDValue{ _time, 0.0 }), m_data.end());
		// add value
		m_data.emplace_back(_time, _value);
	}
}
