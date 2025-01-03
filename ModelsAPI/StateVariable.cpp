/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2024, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "StateVariable.h"

#include "ContainerFunctions.h"
#include "DyssolStringConstants.h"
#include "DyssolUtilities.h"
#include "H5Handler.h"

////////////////////////////////////////////////////////////////////////////////
// CStateVariable
//

CStateVariable::CStateVariable(std::string _name, double _initValue) :
	m_name{ std::move(_name) },
	m_value{ _initValue },
	m_valueStored{ 0.0 }
{
}

void CStateVariable::Clear()
{
	m_value = 0.0;
	m_valueStored = 0.0;
	m_history.clear();
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
	return m_history.size() > 1;
}

std::vector<STDValue> CStateVariable::GetHistory() const
{
	return m_history;
}

double CStateVariable::GetHistoryValue(double _time) const
{
	if (!HasHistory()) return {};
	return Interpolate(m_history, _time);
}

void CStateVariable::SetHistory(const std::vector<STDValue>& _history)
{
	m_history = _history;
}

void CStateVariable::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::H5AttrSaveVersion, m_saveVersion);

	_h5File.WriteData(_path, StrConst::SVar_H5Name,    m_name);
	_h5File.WriteData(_path, StrConst::SVar_H5Value,   m_value);
	_h5File.WriteData(_path, StrConst::SVar_H5History, m_history);
}

void CStateVariable::LoadFromFile(CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	//const int version = _h5File.ReadAttribute(_path, StrConst::H5AttrSaveVersion);

	_h5File.ReadData(_path, StrConst::SVar_H5Name,    m_name);
	_h5File.ReadData(_path, StrConst::SVar_H5Value,   m_value);
	_h5File.ReadData(_path, StrConst::SVar_H5History, m_history);
}

void CStateVariable::AddToHistory(double _time, double _value)
{
	if (_time < 0) return;
	if (m_history.empty() || m_history.back().time < _time)		// time is larger as all already stored
		m_history.emplace_back(_time , _value);					// add it to the end
	else if (std::abs(m_history.back().time - _time) <= m_eps)	// this time is the last stored
		m_history.back() = { _time, _value };					// replace it
	else														// there are larger time points
	{
		// clear larger time points
		m_history.erase(std::lower_bound(m_history.begin(), m_history.end(), STDValue{ _time, 0.0 }), m_history.end());
		// add value
		m_history.emplace_back(_time, _value);
	}
}

////////////////////////////////////////////////////////////////////////////////
// CStateVariablesManager
//

CStateVariablesManager::CStateVariablesManager(const CStateVariablesManager& _other)
	: m_stateVariables{ DeepCopy(_other.m_stateVariables) }
{
}

CStateVariablesManager::CStateVariablesManager(CStateVariablesManager&& _other) noexcept
{
	swap(*this, _other);
}

CStateVariablesManager& CStateVariablesManager::operator=(CStateVariablesManager _other)
{
	swap(*this, _other);
	return *this;
}

CStateVariablesManager& CStateVariablesManager::operator=(CStateVariablesManager&& _other) noexcept
{
	CStateVariablesManager tmp{ std::move(_other) };
	swap(*this, tmp);
	return *this;
}

void swap(CStateVariablesManager& _first, CStateVariablesManager& _second) noexcept
{
	using std::swap;
	swap(_first.m_stateVariables, _second.m_stateVariables);
}

CStateVariable* CStateVariablesManager::AddStateVariable(const std::string& _name, double _initValue)
{
	if (GetStateVariable(_name)) return nullptr;
	m_stateVariables.emplace_back(new CStateVariable{ _name, _initValue });
	return m_stateVariables.back().get();
}

const CStateVariable* CStateVariablesManager::GetStateVariable(size_t _index) const
{
	if (_index >= m_stateVariables.size()) return {};
	return m_stateVariables[_index].get();
}

CStateVariable* CStateVariablesManager::GetStateVariable(size_t _index)
{
	return const_cast<CStateVariable*>(const_cast<const CStateVariablesManager&>(*this).GetStateVariable(_index));
}

const CStateVariable* CStateVariablesManager::GetStateVariable(const std::string& _name) const
{
	for (const auto& v : m_stateVariables)
		if (v->GetName() == _name)
			return v.get();
	return nullptr;
}

CStateVariable* CStateVariablesManager::GetStateVariable(const std::string& _name)
{
	return const_cast<CStateVariable*>(const_cast<const CStateVariablesManager&>(*this).GetStateVariable(_name));
}

std::vector<const CStateVariable*> CStateVariablesManager::GetAllStateVariables() const
{
	std::vector<const CStateVariable*> res;
	for (const auto& v : m_stateVariables)
		res.push_back(v.get());
	return res;
}

std::vector<CStateVariable*> CStateVariablesManager::GetAllStateVariables()
{
	std::vector<CStateVariable*> res;
	for (auto& v : m_stateVariables)
		res.push_back(v.get());
	return res;
}

std::vector<const CStateVariable*> CStateVariablesManager::GetAllStateVariablesWithHistory() const
{
	std::vector<const CStateVariable*> res;
	for (const auto& v : m_stateVariables)
		if (v->HasHistory())
			res.push_back(v.get());
	return res;
}

std::vector<CStateVariable*> CStateVariablesManager::GetAllStateVariablesWithHistory()
{
	std::vector<CStateVariable*> res;
	for (const auto& v : m_stateVariables)
		if (v->HasHistory())
			res.push_back(v.get());
	return res;
}

size_t CStateVariablesManager::GetStateVariablesNumber() const
{
	return m_stateVariables.size();
}

void CStateVariablesManager::ClearData()
{
	for (auto& v : m_stateVariables)
		v->Clear();
}

void CStateVariablesManager::Clear()
{
	m_stateVariables.clear();
}

void CStateVariablesManager::SaveState()
{
	for (auto& var : m_stateVariables)
		var->SaveState();
}

void CStateVariablesManager::LoadState()
{
	for (auto& var : m_stateVariables)
		var->LoadState();
}

void CStateVariablesManager::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::H5AttrSaveVersion, m_saveVersion);

	_h5File.WriteAttribute(_path, StrConst::SVMngr_H5AttrStateVarsNum, static_cast<int>(m_stateVariables.size()));
	for (size_t i = 0; i < m_stateVariables.size(); ++i)
	{
		const std::string variablePath = _h5File.CreateGroup(_path, StrConst::SVMngr_H5GroupStateVarName + std::to_string(i));
		m_stateVariables[i]->SaveToFile(_h5File, variablePath);
	}
}

void CStateVariablesManager::LoadFromFile(CH5Handler& _h5File, const std::string& _path)
{
	Clear();

	if (!_h5File.IsValid()) return;

	// current version of save procedure
	//const int version = _h5File.ReadAttribute(_path, StrConst::H5AttrSaveVersion);

	const size_t nVariables = _h5File.ReadAttribute(_path, StrConst::SVMngr_H5AttrStateVarsNum);
	for (size_t i = 0; i < nVariables; ++i)
	{
		const std::string variablePath = _path + "/" + StrConst::SVMngr_H5GroupStateVarName + std::to_string(i);
		AddStateVariable("", {})->LoadFromFile(_h5File, variablePath);
	}
}

void CStateVariablesManager::LoadFromFile_v0(const CH5Handler& _h5File, const std::string& _path)
{
	Clear();

	if (!_h5File.IsValid()) return;

	const size_t nVariables = _h5File.ReadAttribute(_path, StrConst::SVMngr_H5AttrStateVarsNum);
	if (nVariables == static_cast<size_t>(-1)) return;
	for (size_t i = 0; i < nVariables; ++i)
	{
		std::string variablePath = _path + "/" + StrConst::BUnit_H5GroupStateVars + "/" + StrConst::SVMngr_H5GroupStateVarName + std::to_string(i);
		auto* variable = AddStateVariable("", {});
		std::string name;
		_h5File.ReadData(variablePath, StrConst::SVar_H5Name, name);
		variable->SetName(name);
		double value;
		_h5File.ReadData(variablePath, StrConst::SVar_H5Value, value);
		variable->SetValue(value);
		bool hasHistory;
		_h5File.ReadData(variablePath, StrConst::BUnit_H5StateVarIsSaved, hasHistory);
		if (hasHistory)
		{
			std::vector<double> times, values;
			_h5File.ReadData(variablePath, StrConst::BUnit_H5StateVarTimes, times);
			_h5File.ReadData(variablePath, StrConst::BUnit_H5StateVarValues, values);
			if (!times.empty() && times.size() == values.size())
			{
				std::vector<STDValue> data(times.size());
				for (size_t i = 0; i < data.size(); ++i)
					data[i] = { times[i], values[i] };
				variable->SetHistory(data);
			}
		}
	}
}
