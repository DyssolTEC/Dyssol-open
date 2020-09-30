/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "UnitParameters.h"
#include "DyssolStringConstants.h"
#include "DyssolTypes.h"
#include "ContainerFunctions.h"
#include <set>
#include <utility>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CBaseUnitParameter

CBaseUnitParameter::CBaseUnitParameter() :
	m_type(EUnitParameter::UNKNOWN)
{
}

CBaseUnitParameter::CBaseUnitParameter(EUnitParameter _type) :
	m_type(_type)
{
}

CBaseUnitParameter::CBaseUnitParameter(EUnitParameter _type, std::string _name, std::string _units, std::string _description) :
	m_type(_type),
	m_name(std::move(_name)),
	m_units(std::move(_units)),
	m_description(std::move(_description))
{
}

EUnitParameter CBaseUnitParameter::GetType() const
{
	return m_type;
}

std::string CBaseUnitParameter::GetName() const
{
	return m_name;
}

std::string CBaseUnitParameter::GetUnits() const
{
	return m_units;
}

std::string CBaseUnitParameter::GetDescription() const
{
	return m_description;
}

void CBaseUnitParameter::SetType(EUnitParameter _type)
{
	m_type = _type;
}

void CBaseUnitParameter::SetName(const std::string& _name)
{
	m_name = _name;
}

void CBaseUnitParameter::SetUnits(const std::string& _units)
{
	m_units = _units;
}

void CBaseUnitParameter::SetDescription(const std::string& _description)
{
	m_description = _description;
}

bool CBaseUnitParameter::IsInBounds() const
{
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// CConstUnitParameter

template<typename T>
CBaseConstUnitParameter<T>::CBaseConstUnitParameter() :
	CBaseUnitParameter(DeduceType())
{
}

template<typename T>
CBaseConstUnitParameter<T>::CBaseConstUnitParameter(std::string _name, std::string _units, std::string _description, T _min, T _max, T _value) :
	CBaseUnitParameter(DeduceType(), std::move(_name), std::move(_units), std::move(_description)),
	m_value{ _value },
	m_min{ _min },
	m_max{ _max }
{
}

template<typename T>
void CBaseConstUnitParameter<T>::Clear()
{
	m_value = {};
}

template<typename T>
bool CBaseConstUnitParameter<T>::IsInBounds() const
{
	return m_value >= m_min && m_value <= m_max;
}

template<typename T>
void CBaseConstUnitParameter<T>::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::BUnit_H5AttrSaveVersion, m_cnSaveVersion);

	// save data
	_h5File.WriteData(_path, StrConst::UParam_H5Values, m_value);
}

template<typename T>
void CBaseConstUnitParameter<T>::LoadFromFile(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// load version of save procedure
	//const int version = _h5File.ReadAttribute(_path, StrConst::BUnit_H5AttrSaveVersion);

	// read data
	_h5File.ReadData(_path, StrConst::UParam_H5Values, m_value);
}

template <typename T>
EUnitParameter CBaseConstUnitParameter<T>::DeduceType() const
{
	if (std::is_same_v<T, double>)
		return EUnitParameter::CONSTANT_DOUBLE;
	if (std::is_same_v<T, int64_t>)
		return EUnitParameter::CONSTANT_INT64;
	if (std::is_same_v<T, uint64_t>)
		return EUnitParameter::CONSTANT_UINT64;
	return EUnitParameter::CONSTANT;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// CTDUnitParameter

CTDUnitParameter::CTDUnitParameter() :
	CBaseUnitParameter(EUnitParameter::TIME_DEPENDENT)
{
}

CTDUnitParameter::CTDUnitParameter(std::string _name, std::string _units, std::string _description, double _min, double _max, double _value) :
	CBaseUnitParameter(EUnitParameter::TIME_DEPENDENT, std::move(_name), std::move(_units), std::move(_description)),
	m_min{ _min },
	m_max{ _max }
{
	m_values.SetValue(0, _value);
}

void CTDUnitParameter::Clear()
{
	m_values.clear();
}

double CTDUnitParameter::GetMin() const
{
	return m_min;
}

double CTDUnitParameter::GetMax() const
{
	return m_max;
}

void CTDUnitParameter::SetMin(double _min)
{
	m_min = _min;
}

void CTDUnitParameter::SetMax(double _max)
{
	m_max = _max;
}

double CTDUnitParameter::GetValue(double _time) const
{
	return m_values.GetValue(_time);
}

void CTDUnitParameter::SetValue(double _time, double _value)
{
	m_values.SetValue(_time, _value);
}

void CTDUnitParameter::RemoveValue(double _time)
{
	m_values.RemoveValue(_time);
}

std::vector<double> CTDUnitParameter::GetTimes() const
{
	return m_values.GetParamsList();
}

std::vector<double> CTDUnitParameter::GetValues() const
{
	return m_values.GetValuesList();
}

const CDependentValues& CTDUnitParameter::GetTDData() const
{
	return m_values;
}

size_t CTDUnitParameter::Size() const
{
	return m_values.size();
}

bool CTDUnitParameter::IsEmpty() const
{
	return m_values.empty();
}

bool CTDUnitParameter::IsInBounds() const
{
	for (const auto& [time, value] : m_values)
		if (value < m_min || value > m_max)
			return false;
	return true;

}

void CTDUnitParameter::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::BUnit_H5AttrSaveVersion, m_cnSaveVersion);

	// save data
	_h5File.WriteData(_path, StrConst::UParam_H5Times, GetTimes());
	_h5File.WriteData(_path, StrConst::UParam_H5Values, GetValues());
}

void CTDUnitParameter::LoadFromFile(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// load version of save procedure
	//const int version = _h5File.ReadAttribute(_path, StrConst::BUnit_H5AttrSaveVersion);

	m_values.clear();

	// read data
	std::vector<double> times, values;
	_h5File.ReadData(_path, StrConst::UParam_H5Times, times);
	_h5File.ReadData(_path, StrConst::UParam_H5Values, values);
	for (size_t i = 0; i < times.size(); ++i)
		SetValue(times[i], values[i]);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// CStringUnitParameter

const unsigned CStringUnitParameter::m_cnSaveVersion = 1;

CStringUnitParameter::CStringUnitParameter() :
	CBaseUnitParameter(EUnitParameter::STRING)
{
}

CStringUnitParameter::CStringUnitParameter(std::string _name, std::string _description, std::string _value) :
	CBaseUnitParameter(EUnitParameter::STRING, std::move(_name), "", std::move(_description)),
	m_value(std::move(_value))
{
}

void CStringUnitParameter::Clear()
{
	m_value.clear();
}

std::string CStringUnitParameter::GetValue() const
{
	return m_value;
}

void CStringUnitParameter::SetValue(const std::string& _value)
{
	m_value = _value;
}

void CStringUnitParameter::SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const
{
	if (!_h5Saver.IsValid()) return;

	// current version of save procedure
	_h5Saver.WriteAttribute(_path, StrConst::BUnit_H5AttrSaveVersion, m_cnSaveVersion);

	// save data
	_h5Saver.WriteData(_path, StrConst::UParam_H5StrValue, m_value);
}

void CStringUnitParameter::LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path)
{
	if (!_h5Loader.IsValid()) return;

	// load version of save procedure
	//const int version = _h5Loader.ReadAttribute(_path, StrConst::BUnit_H5AttrSaveVersion);

	// read data
	_h5Loader.ReadData(_path, StrConst::UParam_H5StrValue, m_value);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// CCheckBoxUnitParameter

const unsigned CCheckBoxUnitParameter::m_cnSaveVersion = 1;

CCheckBoxUnitParameter::CCheckBoxUnitParameter() :
	CBaseUnitParameter(EUnitParameter::CHECKBOX),
	m_checked(false)
{
}

CCheckBoxUnitParameter::CCheckBoxUnitParameter(std::string _name, std::string _description, bool _checked) :
	CBaseUnitParameter(EUnitParameter::CHECKBOX, std::move(_name), "", std::move(_description)),
	m_checked(_checked)
{
}

void CCheckBoxUnitParameter::Clear()
{
	m_checked = false;
}

bool CCheckBoxUnitParameter::IsChecked() const
{
	return m_checked;
}

void CCheckBoxUnitParameter::SetChecked(bool _checked)
{
	m_checked = _checked;
}

void CCheckBoxUnitParameter::SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const
{
	if (!_h5Saver.IsValid()) return;

	// current version of save procedure
	_h5Saver.WriteAttribute(_path, StrConst::BUnit_H5AttrSaveVersion, m_cnSaveVersion);

	// save data
	_h5Saver.WriteData(_path, StrConst::UParam_H5Values, m_checked);
}

void CCheckBoxUnitParameter::LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path)
{
	if (!_h5Loader.IsValid()) return;

	// load version of save procedure
	//const int version = _h5Loader.ReadAttribute(_path, StrConst::BUnit_H5AttrSaveVersion);

	// read data
	_h5Loader.ReadData(_path, StrConst::UParam_H5Values, m_checked);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// CSolverUnitParameter

const unsigned CSolverUnitParameter::m_cnSaveVersion = 1;

CSolverUnitParameter::CSolverUnitParameter() :
	CBaseUnitParameter(EUnitParameter::SOLVER),
	m_solverType(ESolverTypes::SOLVER_NONE)
{
}

CSolverUnitParameter::CSolverUnitParameter(std::string _name, std::string _description, ESolverTypes _type) :
	CBaseUnitParameter(EUnitParameter::SOLVER, std::move(_name), "", std::move(_description)),
	m_solverType(_type)
{
}

void CSolverUnitParameter::Clear()
{
	m_key.clear();
	m_solverType = ESolverTypes::SOLVER_NONE;
}

std::string CSolverUnitParameter::GetKey() const
{
	return m_key;
}

ESolverTypes CSolverUnitParameter::GetSolverType() const
{
	return m_solverType;
}

CBaseSolver* CSolverUnitParameter::GetSolver() const
{
	return m_solver;
}

void CSolverUnitParameter::SetKey(const std::string& _key)
{
	m_key = _key;
}

void CSolverUnitParameter::SetSolverType(ESolverTypes _type)
{
	m_solverType = _type;
}

void CSolverUnitParameter::SetSolver(CBaseSolver* _solver)
{
	m_solver = _solver;
}

void CSolverUnitParameter::SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const
{
	if (!_h5Saver.IsValid()) return;

	// current version of save procedure
	_h5Saver.WriteAttribute(_path, StrConst::BUnit_H5AttrSaveVersion, m_cnSaveVersion);

	// save data
	_h5Saver.WriteData(_path, StrConst::UParam_H5StrValue, m_key);
}

void CSolverUnitParameter::LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path)
{
	if (!_h5Loader.IsValid()) return;

	// load version of save procedure
	//const int version = _h5Loader.ReadAttribute(_path, StrConst::BUnit_H5AttrSaveVersion);

	// read data
	_h5Loader.ReadData(_path, StrConst::UParam_H5StrValue, m_key);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// CComboUnitParameter

const unsigned CComboUnitParameter::m_cnSaveVersion = 1;

CComboUnitParameter::CComboUnitParameter() :
	CBaseUnitParameter(EUnitParameter::COMBO)
{
}

CComboUnitParameter::CComboUnitParameter(std::string _name, std::string _description, size_t _itemDefault, const std::vector<size_t>& _items, const std::vector<std::string>& _itemsNames) :
	CBaseUnitParameter(EUnitParameter::COMBO, std::move(_name), "", std::move(_description))
{
	if (_items.size() != _itemsNames.size()) return;
	for (size_t i = 0; i < _itemsNames.size(); ++i)
		m_items[_items[i]] = _itemsNames[i];
	if (VectorContains(_items, _itemDefault))
		m_selected = _itemDefault;
	else if (!_items.empty())
		m_selected = _items.front();
}

void CComboUnitParameter::Clear()
{
	m_selected = -1;
}

size_t CComboUnitParameter::GetValue() const
{
	return m_selected;
}

void CComboUnitParameter::SetValue(size_t _item)
{
	if (HasItem(_item))
		m_selected = _item;
}

std::vector<size_t> CComboUnitParameter::GetItems() const
{
	std::vector<size_t> res;
	for (const auto& pair : m_items)
		res.push_back(pair.first);
	return res;
}

std::vector<std::string> CComboUnitParameter::GetNames() const
{
	std::vector<std::string> res;
	for (const auto& pair : m_items)
		res.push_back(pair.second);
	return res;
}

size_t CComboUnitParameter::GetItemByName(const std::string& _name) const
{
	for (const auto& p : m_items)
		if (p.second == _name)
			return p.first;
	return -1;
}

bool CComboUnitParameter::HasItem(size_t _item) const
{
	return MapContainsKey(m_items, _item);
}

bool CComboUnitParameter::HasName(const std::string& _name) const
{
	return MapContainsValue(m_items, _name);
}

bool CComboUnitParameter::IsInBounds() const
{
	return MapContainsKey(m_items, m_selected);
}

void CComboUnitParameter::SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const
{
	if (!_h5Saver.IsValid()) return;

	// current version of save procedure
	_h5Saver.WriteAttribute(_path, StrConst::BUnit_H5AttrSaveVersion, m_cnSaveVersion);

	// save data
	_h5Saver.WriteData(_path, StrConst::UParam_H5Values, static_cast<uint32_t>(m_selected));
}

void CComboUnitParameter::LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path)
{
	if (!_h5Loader.IsValid()) return;

	// load version of save procedure
	//const int version = _h5Loader.ReadAttribute(_path, StrConst::BUnit_H5AttrSaveVersion);

	// read data
	uint32_t temp;
	_h5Loader.ReadData(_path, StrConst::UParam_H5Values, temp);
	m_selected = temp;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// CCompoundUnitParameter

const unsigned CCompoundUnitParameter::m_cnSaveVersion = 1;

CCompoundUnitParameter::CCompoundUnitParameter() :
	CBaseUnitParameter(EUnitParameter::COMPOUND)
{
}

CCompoundUnitParameter::CCompoundUnitParameter(std::string _name, std::string _description) :
	CBaseUnitParameter(EUnitParameter::COMPOUND, std::move(_name), "", std::move(_description))
{
}

void CCompoundUnitParameter::Clear()
{
	m_key.clear();
}

std::string CCompoundUnitParameter::GetCompound() const
{
	return m_key;
}

void CCompoundUnitParameter::SetCompound(const std::string& _key)
{
	m_key = _key;
}

void CCompoundUnitParameter::SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const
{
	if (!_h5Saver.IsValid()) return;

	// current version of save procedure
	_h5Saver.WriteAttribute(_path, StrConst::BUnit_H5AttrSaveVersion, m_cnSaveVersion);

	// save data
	_h5Saver.WriteData(_path, StrConst::UParam_H5Values, m_key);
}

void CCompoundUnitParameter::LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path)
{
	if (!_h5Loader.IsValid()) return;

	// load version of save procedure
	//const int version = _h5Loader.ReadAttribute(_path, StrConst::BUnit_H5AttrSaveVersion);

	// read data
	_h5Loader.ReadData(_path, StrConst::UParam_H5Values, m_key);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// CUnitParametersManager

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

void CUnitParametersManager::AddConstRealParameter(const std::string& _name, const std::string& _units, const std::string& _description, double _min, double _max, double _value)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CBaseConstUnitParameter<double>{ _name, _units, _description, _min, _max, _value });
}

void CUnitParametersManager::AddConstIntParameter(const std::string& _name, const std::string& _units, const std::string& _description, int64_t _min, int64_t _max, int64_t _value)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CBaseConstUnitParameter<int64_t>{ _name, _units, _description, _min, _max, _value });
}

void CUnitParametersManager::AddConstUIntParameter(const std::string& _name, const std::string& _units, const std::string& _description, uint64_t _min, uint64_t _max, uint64_t _value)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CBaseConstUnitParameter<uint64_t>{ _name, _units, _description, _min, _max, _value });
}

void CUnitParametersManager::AddTDParameter(const std::string& _name, const std::string& _units, const std::string& _description, double _min, double _max, double _value)
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

std::vector<CBaseUnitParameter*> CUnitParametersManager::GetParameters() const
{
	std::vector<CBaseUnitParameter*> res;
	for (const auto& p : m_parameters)
		res.push_back(p.get());
	return res;
}

const CBaseUnitParameter* CUnitParametersManager::GetParameter(size_t _index) const
{
	if(_index >= m_parameters.size()) return nullptr;
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

std::vector<const CSolverUnitParameter*> CUnitParametersManager::GetAllSolverParameters() const
{
	std::vector<const CSolverUnitParameter*> res;
	for (const auto& p : m_parameters)
		if (p->GetType() == EUnitParameter::SOLVER)
			res.push_back(dynamic_cast<const CSolverUnitParameter*>(p.get()));
	return res;
}

std::vector<double> CUnitParametersManager::GetAllTimePoints(double _tBeg, double _tEnd) const
{
	std::set<double> res;
	for (const auto& p : m_parameters)
		if (p->GetType() == EUnitParameter::TIME_DEPENDENT)
			for (const auto t : dynamic_cast<const CTDUnitParameter*>(p.get())->GetTimes())
				if (t >= _tBeg && t <= _tEnd)
					res.insert(t);
	return std::vector<double>{res.begin(), res.end()};
}

void CUnitParametersManager::AddParametersToGroup(size_t _block, size_t _group, const std::vector<size_t>& _parameters)
{
	const auto* groupParam = dynamic_cast<const CComboUnitParameter*>(GetComboParameter(_block));
	if (!groupParam) return;										// _block does not exist
	if (!groupParam->HasItem(_group)) return;						// _group does not exist
	for (const auto& i : _parameters) if (!GetParameter(i)) return;	// some of parameters do not exist
	for (const auto& i : _parameters)								// add all existing parameters to group
		AddToGroup(i, _block, _group);
}

void CUnitParametersManager::AddParametersToGroup(const std::string& _block, const std::string& _group, const std::vector<std::string>& _parameters)
{
	if (const auto* groupParam = dynamic_cast<const CComboUnitParameter*>(GetComboParameter(_block)))
		AddParametersToGroup(Name2Index(_block), groupParam->GetItemByName(_group), Name2Index(_parameters));
}

bool CUnitParametersManager::IsParameterActive(size_t _index) const
{
	if (!MapContainsKey(m_groups, _index)) return true;												// does not belong to any group
	if (m_groups.at(_index).empty()) return true;													// does not belong to any group
	for (size_t i = 0; i < m_parameters.size(); ++i)												// for all group parameters
		if (const auto& group = dynamic_cast<const CComboUnitParameter*>(m_parameters[i].get()))	// if group parameter
			if (MapContainsKey(m_groups.at(_index), i) &&											// this parameter has selected group
				VectorContains(m_groups.at(_index).at(i), group->GetValue()) &&						// this parameter is in selected group
				IsParameterActive(i))																// the whole parameter block is active
				return true;
	return false;
}

bool CUnitParametersManager::IsParameterActive(const CBaseUnitParameter& _parameter) const
{
	return IsParameterActive(Name2Index(_parameter.GetName()));
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
		case EUnitParameter::CONSTANT:			dynamic_cast<CBaseConstUnitParameter<double>*>  (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::CONSTANT_DOUBLE:   dynamic_cast<CBaseConstUnitParameter<double>*>  (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::CONSTANT_INT64:    dynamic_cast<CBaseConstUnitParameter<int64_t>*> (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::CONSTANT_UINT64:   dynamic_cast<CBaseConstUnitParameter<uint64_t>*>(m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::TIME_DEPENDENT:	dynamic_cast<CTDUnitParameter*>                 (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::STRING:			dynamic_cast<CStringUnitParameter*>             (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::CHECKBOX:			dynamic_cast<CCheckBoxUnitParameter*>           (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::SOLVER:			dynamic_cast<CSolverUnitParameter*>             (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::COMBO:				dynamic_cast<CComboUnitParameter*>              (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::GROUP:				dynamic_cast<CComboUnitParameter*>              (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
		case EUnitParameter::COMPOUND:			dynamic_cast<CCompoundUnitParameter*>           (m_parameters[i].get())->SaveToFile(_h5Saver, groupPath);	break;
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
			case EUnitParameter::CONSTANT:			dynamic_cast<CBaseConstUnitParameter<double>*>  (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::CONSTANT_DOUBLE:	dynamic_cast<CBaseConstUnitParameter<double>*>  (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::CONSTANT_INT64: 	dynamic_cast<CBaseConstUnitParameter<int64_t>*> (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::CONSTANT_UINT64:	dynamic_cast<CBaseConstUnitParameter<uint64_t>*>(m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::TIME_DEPENDENT:	dynamic_cast<CTDUnitParameter*>                 (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::STRING:			dynamic_cast<CStringUnitParameter*>             (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::CHECKBOX:			dynamic_cast<CCheckBoxUnitParameter*>           (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::SOLVER:			dynamic_cast<CSolverUnitParameter*>             (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::COMBO:				dynamic_cast<CComboUnitParameter*>              (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::GROUP:				dynamic_cast<CComboUnitParameter*>              (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
			case EUnitParameter::COMPOUND:			dynamic_cast<CCompoundUnitParameter*>           (m_parameters[i].get())->LoadFromFile(_h5Loader, paramPath); break;
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
