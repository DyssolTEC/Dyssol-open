/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2024, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "UnitParametersManager.h"

#include "ContainerFunctions.h"

CUnitParametersManager::CUnitParametersManager(const CUnitParametersManager& _other)
	: m_groups{ _other.m_groups }
{
	m_parameters.reserve(_other.m_parameters.size());
	for (const auto& p : _other.m_parameters)
		m_parameters.emplace_back(p->clone());
}

CUnitParametersManager::CUnitParametersManager(CUnitParametersManager&& _other) noexcept
{
	swap(*this, _other);
}

CUnitParametersManager& CUnitParametersManager::operator=(CUnitParametersManager _other)
{
	swap(*this, _other);
	return *this;
}

CUnitParametersManager& CUnitParametersManager::operator=(CUnitParametersManager&& _other) noexcept
{
	CUnitParametersManager tmp{ std::move(_other) };
	swap(*this, tmp);
	return *this;
}

void swap(CUnitParametersManager& _first, CUnitParametersManager& _second) noexcept
{
	using std::swap;
	swap(_first.m_parameters, _second.m_parameters);
	swap(_first.m_groups    , _second.m_groups);
}

size_t CUnitParametersManager::ParametersNumber() const
{
	return m_parameters.size();
}

bool CUnitParametersManager::IsNameExist(const std::string& _name) const
{
	for (const auto& p : m_parameters)
		if (p->GetName() == _name)
			return true;
	return false;
}

void CUnitParametersManager::AddConstRealParameter(const std::string& _name, const std::wstring& _units, const std::string& _description, double _min, double _max, double _value)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CConstUnitParameter<double>{ _name, _units, _description, _min, _max, _value });
}

void CUnitParametersManager::AddConstIntParameter(const std::string& _name, const std::wstring& _units, const std::string& _description, int64_t _min, int64_t _max, int64_t _value)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CConstUnitParameter<int64_t>{ _name, _units, _description, _min, _max, _value });
}

void CUnitParametersManager::AddConstUIntParameter(const std::string& _name, const std::wstring& _units, const std::string& _description, uint64_t _min, uint64_t _max, uint64_t _value)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CConstUnitParameter<uint64_t>{ _name, _units, _description, _min, _max, _value });
}

void CUnitParametersManager::AddDependentParameter(const std::string& _name, const std::wstring& _units, const std::string& _description, double _min, double _max, double _value, const std::string& _paramName, const std::wstring& _paramUnits, double _paramMin, double _paramMax, double _paramValue)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CDependentUnitParameter{ _name, _value, _units, _paramName, _paramValue, _paramUnits, _description, _min, _max, _paramMin, _paramMax });
}

void CUnitParametersManager::AddTDParameter(const std::string& _name, const std::wstring& _units, const std::string& _description, double _min, double _max, double _value)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CTDUnitParameter{ _name, _units, _description, _min, _max, _value });
}

void CUnitParametersManager::AddStringParameter(const std::string& _name, const std::string& _description, const std::string& _value)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CStringUnitParameter{ _name, _description, _value });
}

void CUnitParametersManager::AddCheckBoxParameter(const std::string& _name, const std::string& _description, bool _value)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CCheckBoxUnitParameter{ _name, _description, _value });
}

void CUnitParametersManager::AddSolverParameter(const std::string& _name, const std::string& _description, ESolverTypes _type)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CSolverUnitParameter{ _name, _description, _type });
}

void CUnitParametersManager::AddComboParameter(const std::string& _name, const std::string& _description, size_t _itemDefault, const std::vector<size_t>& _items, const std::vector<std::string>& _itemsNames)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CComboUnitParameter{ _name, _description, _itemDefault, _items, _itemsNames });
}

void CUnitParametersManager::AddCompoundParameter(const std::string& _name, const std::string& _description)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CCompoundUnitParameter{ _name, _description });
}

void CUnitParametersManager::AddMDBCompoundParameter(const std::string& _name, const std::string& _description)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CMDBCompoundUnitParameter{ _name, _description });
}

void CUnitParametersManager::AddReactionParameter(const std::string& _name, const std::string& _description)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CReactionUnitParameter{ _name, _description });
}

void CUnitParametersManager::AddListRealParameter(const std::string& _name, const std::wstring& _units, const std::string& _description, double _min, double _max, const std::vector<double>& _values)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CListUnitParameter<double>{ _name, _units, _description, _min, _max, _values });
}

void CUnitParametersManager::AddListIntParameter(const std::string& _name, const std::wstring& _units, const std::string& _description, int64_t _min, int64_t _max, const std::vector<int64_t>& _values)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CListUnitParameter<int64_t>{ _name, _units, _description, _min, _max, _values });
}

void CUnitParametersManager::AddListUIntParameter(const std::string& _name, const std::wstring& _units, const std::string& _description, uint64_t _min, uint64_t _max, const std::vector<uint64_t>& _values)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CListUnitParameter<uint64_t>{ _name, _units, _description, _min, _max, _values });
}

std::vector<CBaseUnitParameter*> CUnitParametersManager::GetParameters() const
{
	std::vector<CBaseUnitParameter*> res;
	for (const auto& p : m_parameters)
		res.push_back(p.get());
	return res;
}

std::vector<CBaseUnitParameter*> CUnitParametersManager::GetActiveParameters() const
{
	auto res = ReservedVector<CBaseUnitParameter*>(m_parameters.size());
	for (const auto& p : m_parameters)
		if (IsParameterActive(*p))
			res.push_back(p.get());
	return res;
}

const CBaseUnitParameter* CUnitParametersManager::GetParameter(size_t _index) const
{
	if (_index >= m_parameters.size()) return nullptr;
	return m_parameters[_index].get();
}

CBaseUnitParameter* CUnitParametersManager::GetParameter(size_t _index)
{
	return const_cast<CBaseUnitParameter*>(static_cast<const CUnitParametersManager&>(*this).GetParameter(_index));
}

const CBaseUnitParameter* CUnitParametersManager::GetParameter(const std::string& _name) const
{
	return GetParameter(Name2Index(_name));
}

CBaseUnitParameter* CUnitParametersManager::GetParameter(const std::string& _name)
{
	return const_cast<CBaseUnitParameter*>(static_cast<const CUnitParametersManager&>(*this).GetParameter(_name));
}

const CConstRealUnitParameter* CUnitParametersManager::GetConstRealParameter(size_t _index) const
{
	return dynamic_cast<const CConstRealUnitParameter*>(GetParameter(_index));
}

CConstRealUnitParameter* CUnitParametersManager::GetConstRealParameter(size_t _index)
{
	return const_cast<CConstRealUnitParameter*>(static_cast<const CUnitParametersManager&>(*this).GetConstRealParameter(_index));
}

const CConstIntUnitParameter* CUnitParametersManager::GetConstIntParameter(size_t _index) const
{
	return dynamic_cast<const CConstIntUnitParameter*>(GetParameter(_index));
}

CConstIntUnitParameter* CUnitParametersManager::GetConstIntParameter(size_t _index)
{
	return const_cast<CConstIntUnitParameter*>(static_cast<const CUnitParametersManager&>(*this).GetConstIntParameter(_index));
}

const CConstUIntUnitParameter* CUnitParametersManager::GetConstUIntParameter(size_t _index) const
{
	return dynamic_cast<const CConstUIntUnitParameter*>(GetParameter(_index));
}

CConstUIntUnitParameter* CUnitParametersManager::GetConstUIntParameter(size_t _index)
{
	return const_cast<CConstUIntUnitParameter*>(static_cast<const CUnitParametersManager&>(*this).GetConstUIntParameter(_index));
}

const CDependentUnitParameter* CUnitParametersManager::GetDependentParameter(size_t _index) const
{
	return dynamic_cast<const CDependentUnitParameter*>(GetParameter(_index));
}

CDependentUnitParameter* CUnitParametersManager::GetDependentParameter(size_t _index)
{
	return const_cast<CDependentUnitParameter*>(static_cast<const CUnitParametersManager&>(*this).GetDependentParameter(_index));
}

const CTDUnitParameter* CUnitParametersManager::GetTDParameter(size_t _index) const
{
	return dynamic_cast<const CTDUnitParameter*>(GetParameter(_index));
}

CTDUnitParameter* CUnitParametersManager::GetTDParameter(size_t _index)
{
	return const_cast<CTDUnitParameter*>(static_cast<const CUnitParametersManager&>(*this).GetTDParameter(_index));
}


const CStringUnitParameter* CUnitParametersManager::GetStringParameter(size_t _index) const
{
	return dynamic_cast<const CStringUnitParameter*>(GetParameter(_index));
}

CStringUnitParameter* CUnitParametersManager::GetStringParameter(size_t _index)
{
	return const_cast<CStringUnitParameter*>(static_cast<const CUnitParametersManager&>(*this).GetStringParameter(_index));
}

const CCheckBoxUnitParameter* CUnitParametersManager::GetCheckboxParameter(size_t _index) const
{
	return dynamic_cast<const CCheckBoxUnitParameter*>(GetParameter(_index));
}

CCheckBoxUnitParameter* CUnitParametersManager::GetCheckboxParameter(size_t _index)
{
	return const_cast<CCheckBoxUnitParameter*>(static_cast<const CUnitParametersManager&>(*this).GetCheckboxParameter(_index));
}

const CSolverUnitParameter* CUnitParametersManager::GetSolverParameter(size_t _index) const
{
	return dynamic_cast<const CSolverUnitParameter*>(GetParameter(_index));
}

CSolverUnitParameter* CUnitParametersManager::GetSolverParameter(size_t _index)
{
	return const_cast<CSolverUnitParameter*>(static_cast<const CUnitParametersManager&>(*this).GetSolverParameter(_index));
}

const CComboUnitParameter* CUnitParametersManager::GetComboParameter(size_t _index) const
{
	return dynamic_cast<const CComboUnitParameter*>(GetParameter(_index));
}

CComboUnitParameter* CUnitParametersManager::GetComboParameter(size_t _index)
{
	return const_cast<CComboUnitParameter*>(static_cast<const CUnitParametersManager&>(*this).GetComboParameter(_index));
}

const CCompoundUnitParameter* CUnitParametersManager::GetCompoundParameter(size_t _index) const
{
	return dynamic_cast<const CCompoundUnitParameter*>(GetParameter(_index));
}

CCompoundUnitParameter* CUnitParametersManager::GetCompoundParameter(size_t _index)
{
	return const_cast<CCompoundUnitParameter*>(static_cast<const CUnitParametersManager&>(*this).GetCompoundParameter(_index));
}

const CMDBCompoundUnitParameter* CUnitParametersManager::GetMDBCompoundParameter(size_t _index) const
{
	return dynamic_cast<const CMDBCompoundUnitParameter*>(GetParameter(_index));
}

CMDBCompoundUnitParameter* CUnitParametersManager::GetMDBCompoundParameter(size_t _index)
{
	return const_cast<CMDBCompoundUnitParameter*>(static_cast<const CUnitParametersManager&>(*this).GetMDBCompoundParameter(_index));
}

const CReactionUnitParameter* CUnitParametersManager::GetReactionParameter(size_t _index) const
{
	return dynamic_cast<const CReactionUnitParameter*>(GetParameter(_index));
}

CReactionUnitParameter* CUnitParametersManager::GetReactionParameter(size_t _index)
{
	return const_cast<CReactionUnitParameter*>(static_cast<const CUnitParametersManager&>(*this).GetReactionParameter(_index));
}

const CListRealUnitParameter* CUnitParametersManager::GetListRealParameter(size_t _index) const
{
	return dynamic_cast<const CListRealUnitParameter*>(GetParameter(_index));
}

CListRealUnitParameter* CUnitParametersManager::GetListRealParameter(size_t _index)
{
	return const_cast<CListRealUnitParameter*>(static_cast<const CUnitParametersManager&>(*this).GetListRealParameter(_index));
}

const CListIntUnitParameter* CUnitParametersManager::GetListIntParameter(size_t _index) const
{
	return dynamic_cast<const CListIntUnitParameter*>(GetParameter(_index));
}

CListIntUnitParameter* CUnitParametersManager::GetListIntParameter(size_t _index)
{
	return const_cast<CListIntUnitParameter*>(static_cast<const CUnitParametersManager&>(*this).GetListIntParameter(_index));
}

const CListUIntUnitParameter* CUnitParametersManager::GetListUIntParameter(size_t _index) const
{
	return dynamic_cast<const CListUIntUnitParameter*>(GetParameter(_index));
}

CListUIntUnitParameter* CUnitParametersManager::GetListUIntParameter(size_t _index)
{
	return const_cast<CListUIntUnitParameter*>(static_cast<const CUnitParametersManager&>(*this).GetListUIntParameter(_index));
}

const CConstRealUnitParameter* CUnitParametersManager::GetConstRealParameter(const std::string& _name) const
{
	return GetConstRealParameter(Name2Index(_name));
}

CConstRealUnitParameter* CUnitParametersManager::GetConstRealParameter(const std::string& _name)
{
	return GetConstRealParameter(Name2Index(_name));
}

const CConstIntUnitParameter* CUnitParametersManager::GetConstIntParameter(const std::string& _name) const
{
	return GetConstIntParameter(Name2Index(_name));
}

CConstIntUnitParameter* CUnitParametersManager::GetConstIntParameter(const std::string& _name)
{
	return GetConstIntParameter(Name2Index(_name));
}

const CConstUIntUnitParameter* CUnitParametersManager::GetConstUIntParameter(const std::string& _name) const
{
	return GetConstUIntParameter(Name2Index(_name));
}

CConstUIntUnitParameter* CUnitParametersManager::GetConstUIntParameter(const std::string& _name)
{
	return GetConstUIntParameter(Name2Index(_name));
}

const CDependentUnitParameter* CUnitParametersManager::GetDependentParameter(const std::string& _name) const
{
	return GetDependentParameter(Name2Index(_name));
}

CDependentUnitParameter* CUnitParametersManager::GetDependentParameter(const std::string& _name)
{
	return GetDependentParameter(Name2Index(_name));
}

const CTDUnitParameter* CUnitParametersManager::GetTDParameter(const std::string& _name) const
{
	return GetTDParameter(Name2Index(_name));
}

CTDUnitParameter* CUnitParametersManager::GetTDParameter(const std::string& _name)
{
	return GetTDParameter(Name2Index(_name));
}

const CStringUnitParameter* CUnitParametersManager::GetStringParameter(const std::string& _name) const
{
	return GetStringParameter(Name2Index(_name));
}

CStringUnitParameter* CUnitParametersManager::GetStringParameter(const std::string& _name)
{
	return GetStringParameter(Name2Index(_name));
}

const CCheckBoxUnitParameter* CUnitParametersManager::GetCheckboxParameter(const std::string& _name) const
{
	return GetCheckboxParameter(Name2Index(_name));
}

CCheckBoxUnitParameter* CUnitParametersManager::GetCheckboxParameter(const std::string& _name)
{
	return GetCheckboxParameter(Name2Index(_name));
}

const CSolverUnitParameter* CUnitParametersManager::GetSolverParameter(const std::string& _name) const
{
	return GetSolverParameter(Name2Index(_name));
}

CSolverUnitParameter* CUnitParametersManager::GetSolverParameter(const std::string& _name)
{
	return GetSolverParameter(Name2Index(_name));
}

const CComboUnitParameter* CUnitParametersManager::GetComboParameter(const std::string& _name) const
{
	return GetComboParameter(Name2Index(_name));
}

CComboUnitParameter* CUnitParametersManager::GetComboParameter(const std::string& _name)
{
	return GetComboParameter(Name2Index(_name));
}

const CCompoundUnitParameter* CUnitParametersManager::GetCompoundParameter(const std::string& _name) const
{
	return GetCompoundParameter(Name2Index(_name));
}

CCompoundUnitParameter* CUnitParametersManager::GetCompoundParameter(const std::string& _name)
{
	return GetCompoundParameter(Name2Index(_name));
}

const CMDBCompoundUnitParameter* CUnitParametersManager::GetMDBCompoundParameter(const std::string& _name) const
{
	return GetMDBCompoundParameter(Name2Index(_name));
}

CMDBCompoundUnitParameter* CUnitParametersManager::GetMDBCompoundParameter(const std::string& _name)
{
	return GetMDBCompoundParameter(Name2Index(_name));
}

const CReactionUnitParameter* CUnitParametersManager::GetReactionParameter(const std::string& _name) const
{
	return GetReactionParameter(Name2Index(_name));
}

CReactionUnitParameter* CUnitParametersManager::GetReactionParameter(const std::string& _name)
{
	return GetReactionParameter(Name2Index(_name));
}

const CListRealUnitParameter* CUnitParametersManager::GetListRealParameter(const std::string& _name) const
{
	return GetListRealParameter(Name2Index(_name));
}

CListRealUnitParameter* CUnitParametersManager::GetListRealParameter(const std::string& _name)
{
	return GetListRealParameter(Name2Index(_name));
}

const CListIntUnitParameter* CUnitParametersManager::GetListIntParameter(const std::string& _name) const
{
	return GetListIntParameter(Name2Index(_name));
}

CListIntUnitParameter* CUnitParametersManager::GetListIntParameter(const std::string& _name)
{
	return GetListIntParameter(Name2Index(_name));
}

const CListUIntUnitParameter* CUnitParametersManager::GetListUIntParameter(const std::string& _name) const
{
	return GetListUIntParameter(Name2Index(_name));
}

CListUIntUnitParameter* CUnitParametersManager::GetListUIntParameter(const std::string& _name)
{
	return GetListUIntParameter(Name2Index(_name));
}

double CUnitParametersManager::GetConstRealParameterValue(size_t _index) const
{
	if (const auto* p = GetConstRealParameter(_index))
		return p->GetValue();
	return {};
}

int64_t CUnitParametersManager::GetConstIntParameterValue(size_t _index) const
{
	if (const auto* p = GetConstIntParameter(_index))
		return p->GetValue();
	return {};
}

uint64_t CUnitParametersManager::GetConstUIntParameterValue(size_t _index) const
{
	if (const auto* p = GetConstUIntParameter(_index))
		return p->GetValue();
	return {};
}

double CUnitParametersManager::GetDependentParameterValue(size_t _index, double _param) const
{
	if (const auto* p = GetDependentParameter(_index))
		return p->GetValue(_param);
	return {};
}

double CUnitParametersManager::GetTDParameterValue(size_t _index, double _time) const
{
	if (const auto* p = GetTDParameter(_index))
		return p->GetValue(_time);
	return {};
}

std::string CUnitParametersManager::GetStringParameterValue(size_t _index) const
{
	if (const auto* p = GetStringParameter(_index))
		return p->GetValue();
	return {};
}

bool CUnitParametersManager::GetCheckboxParameterValue(size_t _index) const
{
	if (const auto* p = GetCheckboxParameter(_index))
		return p->IsChecked();
	return false;
}

std::string CUnitParametersManager::GetSolverParameterValue(size_t _index) const
{
	if (const auto* p = GetSolverParameter(_index))
		return p->GetKey();
	return {};
}

size_t CUnitParametersManager::GetComboParameterValue(size_t _index) const
{
	if (const auto* p = GetComboParameter(_index))
		return p->GetValue();
	return -1;
}

std::string CUnitParametersManager::GetCompoundParameterValue(size_t _index) const
{
	if (const auto* p = GetCompoundParameter(_index))
		return p->GetCompound();
	return {};
}

std::string CUnitParametersManager::GetMDBCompoundParameterValue(size_t _index) const
{
	if (const auto* p = GetCompoundParameter(_index))
		return p->GetCompound();
	return {};
}

std::vector<CChemicalReaction> CUnitParametersManager::GetReactionParameterValue(size_t _index) const
{
	if (const auto* p = GetReactionParameter(_index))
		return p->GetReactions();
	return {};
}

double CUnitParametersManager::GetListRealParameterValue(size_t _index, size_t _valueIndex) const
{
	if (const auto* p = GetListRealParameter(_index))
		return p->GetValue(_valueIndex);
	return {};
}

int64_t CUnitParametersManager::GetListIntParameterValue(size_t _index, size_t _valueIndex) const
{
	if (const auto* p = GetListIntParameter(_index))
		return p->GetValue(_valueIndex);
	return {};
}

uint64_t CUnitParametersManager::GetListUIntParameterValue(size_t _index, size_t _valueIndex) const
{
	if (const auto* p = GetListUIntParameter(_index))
		return p->GetValue(_valueIndex);
	return {};
}

double CUnitParametersManager::GetConstRealParameterValue(const std::string& _name) const
{
	return GetConstRealParameterValue(Name2Index(_name));
}

int64_t CUnitParametersManager::GetConstIntParameterValue(const std::string& _name) const
{
	return GetConstIntParameterValue(Name2Index(_name));
}

uint64_t CUnitParametersManager::GetConstUIntParameterValue(const std::string& _name) const
{
	return GetConstUIntParameterValue(Name2Index(_name));
}

double CUnitParametersManager::GetDependentParameterValue(const std::string& _name, double _param) const
{
	return GetDependentParameterValue(Name2Index(_name), _param);
}

double CUnitParametersManager::GetTDParameterValue(const std::string& _name, double _time) const
{
	return GetTDParameterValue(Name2Index(_name), _time);
}

std::string CUnitParametersManager::GetStringParameterValue(const std::string& _name) const
{
	return GetStringParameterValue(Name2Index(_name));
}

bool CUnitParametersManager::GetCheckboxParameterValue(const std::string& _name) const
{
	return GetCheckboxParameterValue(Name2Index(_name));
}

std::string CUnitParametersManager::GetSolverParameterValue(const std::string& _name) const
{
	return GetSolverParameterValue(Name2Index(_name));
}

size_t CUnitParametersManager::GetComboParameterValue(const std::string& _name) const
{
	return GetComboParameterValue(Name2Index(_name));
}

std::string CUnitParametersManager::GetCompoundParameterValue(const std::string& _name) const
{
	return GetCompoundParameterValue(Name2Index(_name));
}

std::string CUnitParametersManager::GetMDBCompoundParameterValue(const std::string& _name) const
{
	return GetMDBCompoundParameterValue(Name2Index(_name));
}

std::vector<CChemicalReaction> CUnitParametersManager::GetReactionParameterValue(const std::string& _name) const
{
	return GetReactionParameterValue(Name2Index(_name));
}

double CUnitParametersManager::GetListRealParameterValue(const std::string& _name, size_t _index) const
{
	return GetListRealParameterValue(Name2Index(_name), _index);
}

int64_t CUnitParametersManager::GetListIntParameterValue(const std::string& _name, size_t _index) const
{
	return GetListIntParameterValue(Name2Index(_name), _index);
}

uint64_t CUnitParametersManager::GetListUIntParameterValue(const std::string& _name, size_t _index) const
{
	return GetListUIntParameterValue(Name2Index(_name), _index);
}

std::vector<const CReactionUnitParameter*> CUnitParametersManager::GetAllReactionParameters() const
{
	std::vector<const CReactionUnitParameter*> res;
	for (const auto& p : m_parameters)
		if (p->GetType() == EUnitParameter::REACTION)
			res.push_back(dynamic_cast<const CReactionUnitParameter*>(p.get()));
	return res;
}

std::vector<CReactionUnitParameter*> CUnitParametersManager::GetAllReactionParameters()
{
	std::vector<CReactionUnitParameter*> res;
	for (auto& p : m_parameters)
		if (p->GetType() == EUnitParameter::REACTION)
			res.push_back(dynamic_cast<CReactionUnitParameter*>(p.get()));
	return res;
}

std::vector<const CCompoundUnitParameter*> CUnitParametersManager::GetAllCompoundParameters() const
{
	std::vector<const CCompoundUnitParameter*> res;
	for (const auto& p : m_parameters)
		if (p->GetType() == EUnitParameter::COMPOUND || p->GetType() == EUnitParameter::MDB_COMPOUND)
			res.push_back(dynamic_cast<const CCompoundUnitParameter*>(p.get()));
	return res;
}

std::vector<CCompoundUnitParameter*> CUnitParametersManager::GetAllCompoundParameters()
{
	std::vector<CCompoundUnitParameter*> res;
	for (const auto& p : m_parameters)
		if (p->GetType() == EUnitParameter::COMPOUND || p->GetType() == EUnitParameter::MDB_COMPOUND)
			res.push_back(dynamic_cast<CCompoundUnitParameter*>(p.get()));
	return res;
}

std::vector<const CSolverUnitParameter*> CUnitParametersManager::GetAllSolverParameters() const
{
	std::vector<const CSolverUnitParameter*> res;
	for (const auto& p : m_parameters)
		if (p->GetType() == EUnitParameter::SOLVER)
			res.push_back(dynamic_cast<const CSolverUnitParameter*>(p.get()));
	return res;
}

std::vector<CSolverUnitParameter*> CUnitParametersManager::GetAllSolverParameters()
{
	std::vector<CSolverUnitParameter*> res;
	for (auto& p : m_parameters)
		if (p->GetType() == EUnitParameter::SOLVER)
			res.push_back(dynamic_cast<CSolverUnitParameter*>(p.get()));
	return res;
}

std::vector<double> CUnitParametersManager::GetAllTimePoints() const
{
	std::vector<double> res;
	for (const auto& p : m_parameters)
		if (p->GetType() == EUnitParameter::TIME_DEPENDENT)
			res = VectorsUnionSorted(res, dynamic_cast<const CTDUnitParameter*>(p.get())->GetParams());
	return res;
}

std::vector<double> CUnitParametersManager::GetAllTimePoints(double _tBeg, double _tEnd) const
{
	std::set<double> res;
	for (const auto& p : m_parameters)
		if (p->GetType() == EUnitParameter::TIME_DEPENDENT)
			for (const auto t : dynamic_cast<const CTDUnitParameter*>(p.get())->GetParams())
				if (t >= _tBeg && t <= _tEnd)
					res.insert(t);
	return std::vector<double>{res.begin(), res.end()};
}

void CUnitParametersManager::AddParametersToGroup(size_t _block, size_t _group, const std::vector<size_t>& _parameters)
{
	const auto param = GetParameter(_group);
	if (!param) return;											    // _block does not exist
	if (param->GetType() == EUnitParameter::COMBO)
	{
		const auto* comboParam = dynamic_cast<CComboUnitParameter*>(param);
		if (!comboParam) return;									// _block does not exist
		if (!comboParam->HasItem(_group)) return;					// _group does not exist
	}
	for (const auto& i : _parameters) if (!GetParameter(i)) return;	// some of the parameters do not exist
	for (const auto& i : _parameters)								// add all existing parameters to group
		AddToGroup(i, _block, _group);
}

void CUnitParametersManager::AddParametersToGroup(const std::string& _block, const std::string& _group, const std::vector<std::string>& _parameters)
{
	if (const auto* groupParam = GetComboParameter(_block))
		AddParametersToGroup(Name2Index(_block), groupParam->GetItemByName(_group), Name2Index(_parameters));
}

void CUnitParametersManager::AddParametersToGroup(const std::string& _block, bool _group, const std::vector<std::string>& _parameters)
{
	if (GetCheckboxParameter(_block))
		AddParametersToGroup(Name2Index(_block), _group, Name2Index(_parameters));
}

bool CUnitParametersManager::IsParameterActive(size_t _index) const
{
	if (!MapContainsKey(m_groups, _index)) return true;												// does not belong to any group
	if (m_groups.at(_index).empty()) return true;													// does not belong to any group
	for (size_t i = 0; i < m_parameters.size(); ++i)												// for all group parameters
		if (const auto& combo = dynamic_cast<const CComboUnitParameter*>(m_parameters[i].get()))	// if combo parameter
		{
			if (MapContainsKey(m_groups.at(_index), i) &&											// this parameter has selected group
				VectorContains(m_groups.at(_index).at(i), combo->GetValue()) &&					// this parameter is in selected group
				IsParameterActive(i))															// the whole parameter block is active
				return true;
		}
		else if (const auto& checkbox = dynamic_cast<const CCheckBoxUnitParameter*>(m_parameters[i].get()))	// if checkbox parameter
		{
			if (MapContainsKey(m_groups.at(_index), i) &&													// this parameter has selected group
				VectorContains(m_groups.at(_index).at(i), static_cast<size_t>(checkbox->GetValue())) &&	// this parameter is in selected group
				IsParameterActive(i))																	// the whole parameter block is active
				return true;
		}
	return false;
}

bool CUnitParametersManager::IsParameterActive(const CBaseUnitParameter& _parameter) const
{
	return IsParameterActive(Name2Index(_parameter.GetName()));
}

bool CUnitParametersManager::IsGroupSelector(const CBaseUnitParameter& _parameter) const
{
	return std::any_of(m_groups.begin(), m_groups.end(), [&](const auto& _group)
	{
		return MapContainsKey(_group.second, Name2Index(_parameter.GetName()));
	});
}

void CUnitParametersManager::ClearGroups()
{
	m_groups.clear();
}

void CUnitParametersManager::SaveToFile(CH5Handler& _h5Saver, const std::string& _path)
{
	if (!_h5Saver.IsValid()) return;

	// current version of save procedure
	_h5Saver.WriteAttribute(_path, StrConst::BUnit_H5AttrSaveVersion, m_cnSaveVersion);

	// save unit parameters
	for (size_t i = 0; i < m_parameters.size(); ++i)
	{
		const std::string groupPath = _h5Saver.CreateGroup(_path, StrConst::UParam_H5GroupParamName + std::to_string(i));
		/* HACK: If call normally as a virtual function, HDF5 context changes somehow, and the content of H5I_id_type_list_g
		becomes wrong. When HDF5 tries to access it, function fails to find an ID of an opened file there.
		May be, it somehow depends on the previous call to LoadLibrary() to get unit from DLL. */
		switch (m_parameters[i]->GetType())
		{
		case EUnitParameter::CONSTANT:			dynamic_cast<CConstUnitParameter<double>*>  (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::CONSTANT_DOUBLE:   dynamic_cast<CConstUnitParameter<double>*>  (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::CONSTANT_INT64:    dynamic_cast<CConstUnitParameter<int64_t>*> (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::CONSTANT_UINT64:   dynamic_cast<CConstUnitParameter<uint64_t>*>(m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::PARAM_DEPENDENT:	dynamic_cast<CDependentUnitParameter*>      (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::TIME_DEPENDENT:	dynamic_cast<CTDUnitParameter*>             (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::STRING:			dynamic_cast<CStringUnitParameter*>         (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::CHECKBOX:			dynamic_cast<CCheckBoxUnitParameter*>       (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::SOLVER:			dynamic_cast<CSolverUnitParameter*>         (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::COMBO:				dynamic_cast<CComboUnitParameter*>          (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::GROUP:				dynamic_cast<CComboUnitParameter*>          (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::COMPOUND:			dynamic_cast<CCompoundUnitParameter*>       (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::MDB_COMPOUND:		dynamic_cast<CMDBCompoundUnitParameter*>    (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::REACTION:			dynamic_cast<CReactionUnitParameter*>       (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::LIST_DOUBLE:		dynamic_cast<CListRealUnitParameter*>       (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::LIST_INT64:		dynamic_cast<CListIntUnitParameter*>        (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::LIST_UINT64:		dynamic_cast<CListUIntUnitParameter*>       (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::UNKNOWN: break;
		}
	}

	// save unit parameters names
	std::vector<std::string> names;
	for (const auto& p : m_parameters)
		names.push_back(p->GetName());
	_h5Saver.WriteData(_path, StrConst::UParam_H5Names, names);
}

void CUnitParametersManager::LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path)
{
	if (!_h5Loader.IsValid()) return;

	// load version of save procedure
	//const int version = _h5Loader.ReadAttribute(_path, StrConst::BUnit_H5AttrSaveVersion);

	// read data
	std::vector<std::string> names;
	_h5Loader.ReadData(_path, StrConst::UParam_H5Names, names);
	for (size_t i = 0; i < m_parameters.size(); ++i)
	{
		std::string paramPath;
		for (size_t j = 0; j < names.size(); ++j) // try to find by name
			if (m_parameters[i]->GetName() == names[j])
			{
				paramPath = _path + "/" + StrConst::UParam_H5GroupParamName + std::to_string(j);
				break;
			}
		if (paramPath.empty() && i < names.size()) // get by index if not found by name
			paramPath = _path + "/" + StrConst::UParam_H5GroupParamName + std::to_string(i);
		if (!paramPath.empty())
		{
			/* HACK: If call normally as a virtual function, HDF5 context changes somehow, and the content of H5I_id_type_list_g
			became wrong. When HDF5 tries to access it, function fails to find an ID of opened file there.
			May be somehow depend on the previous call to LoadLibrary() to to get unit from DLL. */
			switch (m_parameters[i]->GetType())
			{
			case EUnitParameter::CONSTANT:			dynamic_cast<CConstUnitParameter<double>*>  (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::CONSTANT_DOUBLE:	dynamic_cast<CConstUnitParameter<double>*>  (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::CONSTANT_INT64: 	dynamic_cast<CConstUnitParameter<int64_t>*> (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::CONSTANT_UINT64:	dynamic_cast<CConstUnitParameter<uint64_t>*>(m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::PARAM_DEPENDENT:	dynamic_cast<CDependentUnitParameter*>      (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::TIME_DEPENDENT:	dynamic_cast<CTDUnitParameter*>             (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::STRING:			dynamic_cast<CStringUnitParameter*>         (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::CHECKBOX:			dynamic_cast<CCheckBoxUnitParameter*>       (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::SOLVER:			dynamic_cast<CSolverUnitParameter*>         (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::COMBO:				dynamic_cast<CComboUnitParameter*>          (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::GROUP:				dynamic_cast<CComboUnitParameter*>          (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::COMPOUND:			dynamic_cast<CCompoundUnitParameter*>       (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::MDB_COMPOUND:		dynamic_cast<CMDBCompoundUnitParameter*>    (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::REACTION:			dynamic_cast<CReactionUnitParameter*>       (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::LIST_DOUBLE:		dynamic_cast<CListUnitParameter<double>*>   (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::LIST_INT64: 		dynamic_cast<CListUnitParameter<int64_t>*>  (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::LIST_UINT64:		dynamic_cast<CListUnitParameter<uint64_t>*> (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::UNKNOWN: break;
			}
		}
	}
}

void CUnitParametersManager::AddToGroup(size_t _parameter, size_t _block, size_t _group)
{
	m_groups[_parameter][_block].push_back(_group);
}

size_t CUnitParametersManager::Name2Index(const std::string& _name) const
{
	for (size_t i = 0; i < m_parameters.size(); ++i)
		if (m_parameters[i]->GetName() == _name)
			return i;
	return -1;
}

std::vector<size_t> CUnitParametersManager::Name2Index(const std::vector<std::string>& _names) const
{
	std::vector<size_t> res;
	for (const auto& name : _names)
		res.push_back(Name2Index(name));
	return res;
}
