/* Copyright (c) 2021, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ChemicalReaction.h"
#include "MaterialsDatabase.h"
#include "H5Handler.h"
#include "ContainerFunctions.h"
#include "DyssolStringConstants.h"
#include "DyssolUtilities.h"
#include <memory>

CChemicalReaction::CChemicalReaction(const CChemicalReaction& _other)
	: m_name{ _other.m_name }
	, m_enthalpy{ _other.m_enthalpy }
	, m_iBase{_other.m_iBase}
{
	m_substances.clear();
	m_substances.reserve(_other.m_substances.size());
	for (const auto& s : _other.m_substances)
		m_substances.emplace_back(new SChemicalSubstance{ *s });
}

CChemicalReaction& CChemicalReaction::operator=(const CChemicalReaction& _other)
{
	CChemicalReaction temp{ _other };
	Swap(temp);
	return *this;
}

void CChemicalReaction::Swap(CChemicalReaction& _other) noexcept
{
	std::swap(m_name      , _other.m_name);
	std::swap(m_enthalpy  , _other.m_enthalpy);
	std::swap(m_iBase     , _other.m_iBase);
	std::swap(m_substances, _other.m_substances);
}

bool CChemicalReaction::operator==(const CChemicalReaction& _other) const
{
	if (m_name != _other.m_name) return false;
	if (m_enthalpy != _other.m_enthalpy) return false;
	if (m_iBase != _other.m_iBase) return false;
	if (m_substances.size() != _other.m_substances.size()) return false;
	for (size_t i = 0; i < m_substances.size(); ++i)
		if (*m_substances[i] != *_other.m_substances[i])
			return false;
	return true;
}

void CChemicalReaction::SetName(const std::string& _name)
{
	m_name = _name;
}

std::string CChemicalReaction::GetName() const
{
	return m_name;
}

CChemicalReaction::SChemicalSubstance* CChemicalReaction::AddSubstance()
{
	return m_substances.emplace_back().get();
}

void CChemicalReaction::AddSubstance(const SChemicalSubstance& _substance)
{
	m_substances.emplace_back(new SChemicalSubstance{ _substance });
}

void CChemicalReaction::RemoveSubstance(size_t _index)
{
	if (_index >= m_substances.size()) return;
	m_substances.erase(m_substances.begin() + _index);
}

std::vector<const CChemicalReaction::SChemicalSubstance*> CChemicalReaction::GetSubstances() const
{
	auto res = ReservedVector<const SChemicalSubstance*>(m_substances.size());
	for (const auto& s : m_substances)
		res.emplace_back(s.get());
	return res;
}

std::vector<CChemicalReaction::SChemicalSubstance*> CChemicalReaction::GetSubstances()
{
	auto res = ReservedVector<SChemicalSubstance*>(m_substances.size());
	for (auto& s : m_substances)
		res.emplace_back(s.get());
	return res;
}

size_t CChemicalReaction::GetSubstancesNumber() const
{
	return m_substances.size();
}

std::vector<const CChemicalReaction::SChemicalSubstance*> CChemicalReaction::GetSubstances(EPhase _phase) const
{
	auto res = ReservedVector<const SChemicalSubstance*>(m_substances.size());
	for (const auto& s : m_substances)
		if (s->phase == _phase)
			res.emplace_back(s.get());
	return res;
}

std::vector<CChemicalReaction::SChemicalSubstance*> CChemicalReaction::GetSubstances(EPhase _phase)
{
	auto res = ReservedVector<SChemicalSubstance*>(m_substances.size());
	for (auto& s : m_substances)
		if (s->phase == _phase)
			res.emplace_back(s.get());
	return res;
}

std::vector<const CChemicalReaction::SChemicalSubstance*> CChemicalReaction::GetSubstances(ESubstance _type) const
{
	auto res = ReservedVector<const SChemicalSubstance*>(m_substances.size());
	for (const auto& s : m_substances)
		if (_type == s->GetType())
			res.emplace_back(s.get());
	return res;
}

std::vector<CChemicalReaction::SChemicalSubstance*> CChemicalReaction::GetSubstances(ESubstance _type)
{
	auto res = ReservedVector<SChemicalSubstance*>(m_substances.size());
	for (auto& s : m_substances)
		if (_type == s->GetType())
			res.emplace_back(s.get());
	return res;
}

std::vector<const CChemicalReaction::SChemicalSubstance*> CChemicalReaction::GetSubstances(EPhase _phase, ESubstance _type) const
{
	auto res = ReservedVector<const SChemicalSubstance*>(m_substances.size());
	for (const auto& s : m_substances)
		if (s->phase == _phase && _type == s->GetType())
			res.emplace_back(s.get());
	return res;
}

std::vector<CChemicalReaction::SChemicalSubstance*> CChemicalReaction::GetSubstances(EPhase _phase, ESubstance _type)
{
	auto res = ReservedVector<SChemicalSubstance*>(m_substances.size());
	for (auto& s : m_substances)
		if (s->phase == _phase && _type == s->GetType())
			res.emplace_back(s.get());
	return res;
}

void CChemicalReaction::SetBaseSubstance(size_t _index)
{
	if (_index >= m_substances.size()) return;
	m_iBase = _index;
}

const CChemicalReaction::SChemicalSubstance* CChemicalReaction::GetBaseSubstance() const
{
	if (m_iBase >= m_substances.size()) return nullptr;
	return m_substances[m_iBase].get();
}

CChemicalReaction::SChemicalSubstance* CChemicalReaction::GetBaseSubstance()
{
	if (m_iBase >= m_substances.size()) return nullptr;
	return m_substances[m_iBase].get();
}

size_t CChemicalReaction::GetBaseSubstanceIndex() const
{
	return m_iBase;
}

void CChemicalReaction::SetEnthalpy(double _enthalpy)
{
	m_enthalpy = _enthalpy;
}

double CChemicalReaction::GetEnthalpy() const
{
	return m_enthalpy;
}

void CChemicalReaction::Initialize(const CMaterialsDatabase& _materials)
{
	m_enthalpy = 0.0;
	for (const auto& s : m_substances)
		m_enthalpy += s->nu * _materials.GetConstPropertyValue(s->key, STANDARD_FORMATION_ENTHALPY);
	for (auto& s : m_substances)
		s->MM = _materials.GetConstPropertyValue(s->key, MOLAR_MASS);
}

void CChemicalReaction::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::H5AttrSaveVersion, m_saveVersion);

	_h5File.WriteData(_path, StrConst::Reac_H5Name, m_name);
	_h5File.WriteData(_path, StrConst::Reac_H5Base, m_iBase);
	_h5File.WriteAttribute(_path, StrConst::Reac_H5SubstancesNumber, static_cast<int>(m_substances.size()));
	for (size_t i = 0; i < m_substances.size(); ++i)
	{
		const std::string group = _h5File.CreateGroup(_path, StrConst::Reac_H5Substance + std::to_string(i));
		_h5File.WriteData(group, StrConst::Reac_H5SubstanceKey  , m_substances[i]->key);
		_h5File.WriteData(group, StrConst::Reac_H5SubstanceNu   , m_substances[i]->nu);
		_h5File.WriteData(group, StrConst::Reac_H5SubstanceOrder, m_substances[i]->order);
		_h5File.WriteData(group, StrConst::Reac_H5SubstancePhase, E2I(m_substances[i]->phase));
	}
}

void CChemicalReaction::LoadFromFile(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// load version of save procedure
	//const int version = _h5File.ReadAttribute(_path, StrConst::H5AttrSaveVersion);

	_h5File.ReadData(_path, StrConst::Reac_H5Name, m_name);
	_h5File.ReadData(_path, StrConst::Reac_H5Base, m_iBase);
	const size_t number = _h5File.ReadAttribute(_path, StrConst::Reac_H5SubstancesNumber);
	m_substances.reserve(number);
	for (size_t i = 0; i < number; ++i)
	{
		m_substances.emplace_back(new SChemicalSubstance);
		const std::string group = _path + "/" + StrConst::Reac_H5Substance + std::to_string(i);
		_h5File.ReadData(group, StrConst::Reac_H5SubstanceKey  , m_substances.back()->key);
		_h5File.ReadData(group, StrConst::Reac_H5SubstanceNu   , m_substances.back()->nu);
		_h5File.ReadData(group, StrConst::Reac_H5SubstanceOrder, m_substances.back()->order);
		uint32_t temp;
		_h5File.ReadData(group, StrConst::Reac_H5SubstancePhase, temp);
		m_substances.back()->phase = static_cast<EPhase>(temp);
	}
}
