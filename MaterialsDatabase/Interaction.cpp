/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Interaction.h"
#include <utility>

CInteraction::CInteraction(const MDBDescriptors::interDescr& _interDescrs, std::string _sKey1, std::string _sKey2) :
	m_sCompoundKey1(std::move(_sKey1)), m_sCompoundKey2(std::move(_sKey2))
{
	// add all interaction properties
	for (const auto& p : _interDescrs)
		m_vProperties.emplace_back(p.first, p.second.name, p.second.units, CCorrelation{ ECorrelationTypes::CONSTANT, std::vector<double>{p.second.defaultParameters} });
}

std::string CInteraction::GetKey1() const
{
	return m_sCompoundKey1;
}

std::string CInteraction::GetKey2() const
{
	return m_sCompoundKey2;
}

void CInteraction::SetKeys(const std::string& _sKey1, const std::string& _sKey2)
{
	m_sCompoundKey1 = _sKey1;
	m_sCompoundKey2 = _sKey2;
}

size_t CInteraction::PropertiesNumber() const
{
	return m_vProperties.size();
}

bool CInteraction::HasProperty(EInteractionProperties _key) const
{
	for (const auto& tp : m_vProperties)
		if (tp.GetType() == _key)
			return true;
	return false;
}

void CInteraction::AddProperty(EInteractionProperties _key, const std::string& _name, const std::wstring& _units, double _defaultValue)
{
	if (HasProperty(_key)) return;
	m_vProperties.emplace_back(_key, _name, _units, CCorrelation{ ECorrelationTypes::CONSTANT, {_defaultValue} });
}

void CInteraction::RemoveProperty(EInteractionProperties _key)
{
	for (size_t i = 0; i < m_vProperties.size(); ++i)
		if (m_vProperties[i].GetType() == _key)
			m_vProperties.erase(m_vProperties.begin() + i);
}

CTPDProperty* CInteraction::GetProperty(EInteractionProperties _nType)
{
	return const_cast<CTPDProperty*>(static_cast<const CInteraction&>(*this).GetProperty(_nType));
}

const CTPDProperty* CInteraction::GetProperty(EInteractionProperties _nType) const
{
	for (const auto& p : m_vProperties)
		if (p.GetType() == _nType)
			return &p;
	return nullptr;
}

CTPDProperty* CInteraction::GetPropertyByIndex(size_t _index)
{
	return const_cast<CTPDProperty*>(static_cast<const CInteraction&>(*this).GetPropertyByIndex(_index));
}

const CTPDProperty* CInteraction::GetPropertyByIndex(size_t _index) const
{
	if (_index < m_vProperties.size())
		return &m_vProperties[_index];
	return nullptr;
}

const std::vector<CTPDProperty>& CInteraction::GetProperties() const
{
	return m_vProperties;
}

double CInteraction::GetPropertyValue(EInteractionProperties _nType, double _dT, double _dP) const
{
	if (const CTPDProperty *prop = GetProperty(_nType))
		return prop->GetValue(_dT, _dP);
	return 0;
}

bool CInteraction::IsBetween(const std::string& _sKey1, const std::string& _sKey2) const
{
	return m_sCompoundKey1 == _sKey1 && m_sCompoundKey2 == _sKey2 || m_sCompoundKey1 == _sKey2 && m_sCompoundKey2 == _sKey1;
}
