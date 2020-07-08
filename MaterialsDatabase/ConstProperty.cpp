/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ConstProperty.h"

CConstProperty::CConstProperty(unsigned _nProperty, const std::string& _sName, const std::wstring& _sUnits, double _defaultValue)
	: CBaseProperty(_nProperty, _sName, _sUnits),
	m_defaultValue{ _defaultValue },
	m_dValue{ _defaultValue }
{
}

double CConstProperty::GetValue() const
{
	return m_dValue;
}

void CConstProperty::SetValue(double _dValue)
{
	m_dValue = _dValue;
}

bool CConstProperty::IsDefaultValue() const
{
	return m_dValue == m_defaultValue;
}
