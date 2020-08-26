/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Compound.h"
#include "StringFunctions.h"

CCompound::CCompound(const MDBDescriptors::constDescr& _constDescrs, const MDBDescriptors::tpdepDescr& _tpdDescrs, const std::string& _sUniqueCompoundKey /*= ""*/)
{
	m_sUniqueKey = _sUniqueCompoundKey.empty() ? StringFunctions::GenerateRandomKey() : _sUniqueCompoundKey;
	m_sName = "New compound";

	// add all const properties
	for (const auto& c : _constDescrs)
		m_vConstProperties.emplace_back(c.first, c.second.name, c.second.units, c.second.defaultValue);
	// add all TP-properties
	for (const auto& tp : _tpdDescrs)
		m_vTPProperties.emplace_back(tp.first, tp.second.name, tp.second.units, CCorrelation{ tp.second.defuaultType, tp.second.defaultParameters });
}

std::string CCompound::GetName() const
{
	return m_sName;
}

void CCompound::SetName(const std::string& _sName)
{
	m_sName = _sName;
}

std::string CCompound::GetKey() const
{
	return m_sUniqueKey;
}

void CCompound::SetKey(const std::string& _sKey)
{
	m_sUniqueKey = _sKey;
}

size_t CCompound::ConstPropertiesNumber() const
{
	return m_vConstProperties.size();
}

size_t CCompound::TPPropertiesNumber() const
{
	return m_vTPProperties.size();
}

bool CCompound::HasConstProperty(ECompoundConstProperties _nType) const
{
	for (const auto& c : m_vConstProperties)
		if (c.GetType() == _nType)
			return true;
	return false;
}

bool CCompound::HasTPProperty(ECompoundTPProperties _nType) const
{
	for (const auto& tp : m_vTPProperties)
		if (tp.GetType() == _nType)
			return true;
	return false;
}

bool CCompound::HasProperty(unsigned _nType) const
{
	return HasConstProperty(static_cast<ECompoundConstProperties>(_nType)) || HasTPProperty(static_cast<ECompoundTPProperties>(_nType));
}

void CCompound::AddConstProperty(ECompoundConstProperties _key, const std::string& _name, const std::wstring& _units, double _defaultValue)
{
	if (HasConstProperty(_key)) return;
	m_vConstProperties.emplace_back(_key, _name, _units, _defaultValue);
}

void CCompound::AddTPDepProperty(ECompoundTPProperties _key, const std::string& _name, const std::wstring& _units, double _defaultValue)
{
	if (HasTPProperty(_key)) return;
	m_vTPProperties.emplace_back(_key, _name, _units, CCorrelation{ ECorrelationTypes::CONSTANT, {_defaultValue} });
}

void CCompound::RemoveConstProperty(ECompoundConstProperties _key)
{
	for (size_t i = 0; i < m_vConstProperties.size(); ++i)
		if (m_vConstProperties[i].GetType() == _key)
			m_vConstProperties.erase(m_vConstProperties.begin() + i);
}

void CCompound::RemoveTPDepProperty(ECompoundTPProperties _key)
{
	for (size_t i = 0; i < m_vTPProperties.size(); ++i)
		if (m_vTPProperties[i].GetType() == _key)
			m_vTPProperties.erase(m_vTPProperties.begin() + i);
}

CConstProperty* CCompound::GetConstProperty(ECompoundConstProperties _nType)
{
	return const_cast<CConstProperty*>(static_cast<const CCompound&>(*this).GetConstProperty(_nType));
}

const CConstProperty* CCompound::GetConstProperty(ECompoundConstProperties _nType) const
{
	for (const auto& c : m_vConstProperties)
		if (c.GetType() == _nType)
			return &c;
	return nullptr;
}

const std::vector<CConstProperty>& CCompound::GetConstProperties() const
{
	return m_vConstProperties;
}

CTPDProperty* CCompound::GetTPProperty(ECompoundTPProperties _nType)
{
	return const_cast<CTPDProperty*>(static_cast<const CCompound&>(*this).GetTPProperty(_nType));
}

const CTPDProperty* CCompound::GetTPProperty(ECompoundTPProperties _nType) const
{
	for (const auto& tp : m_vTPProperties)
		if (tp.GetType() == _nType)
			return &tp;
	return nullptr;
}

CTPDProperty* CCompound::GetTPPropertyByIndex(size_t _index)
{
	return const_cast<CTPDProperty*>(static_cast<const CCompound&>(*this).GetTPPropertyByIndex(_index));
}

const CTPDProperty* CCompound::GetTPPropertyByIndex(size_t _index) const
{
	if (_index >= m_vTPProperties.size())
		return nullptr;
	return &m_vTPProperties[_index];
}

const std::vector<CTPDProperty>& CCompound::GetTPProperties() const
{
	return m_vTPProperties;
}

double CCompound::GetConstPropertyValue(ECompoundConstProperties _nType) const
{
	if (const CConstProperty *prop = GetConstProperty(_nType))
		return prop->GetValue();
	return 0;
}

double CCompound::GetTPPropertyValue(ECompoundTPProperties _nType, double _dT, double _dP) const
{
	if (const CTPDProperty *prop = GetTPProperty(_nType))
		return prop->GetValue(_dT, _dP);
	return 0;
}
