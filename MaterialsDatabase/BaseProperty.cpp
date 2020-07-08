/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "BaseProperty.h"
#include <utility>

CBaseProperty::CBaseProperty(unsigned _nType, std::string _sName, std::wstring _sUnits) :
	m_nType(_nType),
	m_sName(std::move(_sName)),
	m_sUnits(std::move(_sUnits))
{
}

CBaseProperty::~CBaseProperty() = default;

unsigned CBaseProperty::GetType() const
{
	return m_nType;
}

void CBaseProperty::SetType(unsigned _nType)
{
	m_nType = _nType;
}

std::string CBaseProperty::GetName() const
{
	return m_sName;
}

void CBaseProperty::SetName(const std::string& _sName)
{
	m_sName = _sName;
}

std::wstring CBaseProperty::GetUnits() const
{
	return m_sUnits;
}

void CBaseProperty::SetUnits(const std::wstring& _sUnits)
{
	m_sUnits = _sUnits;
}
