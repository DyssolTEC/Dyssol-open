/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "UnitParameters.h"
#include "ContainerFunctions.h"
#include "DyssolUtilities.h"

#include <set>
#include <utility>

template<>
EUnitParameter DeduceTypeConst<double>() { return EUnitParameter::CONSTANT_DOUBLE; }

template<>
EUnitParameter DeduceTypeConst<int64_t>() { return EUnitParameter::CONSTANT_INT64; }

template<>
EUnitParameter DeduceTypeConst<uint64_t>() { return EUnitParameter::CONSTANT_UINT64; }

///

template<>
EUnitParameter DeduceTypeList<double>() { return EUnitParameter::LIST_DOUBLE; }

template<>
EUnitParameter DeduceTypeList<int64_t>() { return EUnitParameter::LIST_INT64; }
template<>
EUnitParameter DeduceTypeList<uint64_t>() { return EUnitParameter::LIST_UINT64; }


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

CBaseUnitParameter::CBaseUnitParameter(EUnitParameter _type, std::string _name, std::wstring _units, std::string _description) :
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

std::wstring CBaseUnitParameter::GetUnits() const
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

void CBaseUnitParameter::SetUnits(const std::wstring& _units)
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
/// CDependentUnitParameter
CDependentUnitParameter::CDependentUnitParameter() :
	CBaseUnitParameter(EUnitParameter::PARAM_DEPENDENT)
{
}

CDependentUnitParameter::CDependentUnitParameter(std::string _valueName, double _valueInit, std::wstring _valueUnits, std::string _paramName, double _paramInit, std::wstring _paramUnits, std::string _description, double _valueMin, double _valueMax, double _paramMin, double _paramMax) :
	CBaseUnitParameter(EUnitParameter::PARAM_DEPENDENT, std::move(_valueName), std::move(_valueUnits), std::move(_description)),
	m_valueMin{ _valueMin },
	m_valueMax{ _valueMax },
	m_paramMin{ _paramMin },
	m_paramMax{ _paramMax },
	m_paramName{ std::move(_paramName) },
	m_paramUnits{ std::move(_paramUnits) }
{
	m_data.SetValue(_paramInit, _valueInit);
}

void CDependentUnitParameter::Clear()
{
	m_data.Clear();
}

std::string CDependentUnitParameter::GetParamName() const
{
	return m_paramName;
}

void CDependentUnitParameter::SetParamName(const std::string& _paramName)
{
	m_paramName = _paramName;
}

std::wstring CDependentUnitParameter::GetParamUnits() const
{
	return m_paramUnits;
}

void CDependentUnitParameter::SetParamUnits(const std::wstring& _paramUnits)
{
	m_paramUnits = _paramUnits;
}

double CDependentUnitParameter::GetValueMin() const
{
	return m_valueMin;
}

double CDependentUnitParameter::GetValueMax() const
{
	return m_valueMax;
}

SInterval CDependentUnitParameter::GetValueLimits() const
{
	return { m_valueMin, m_valueMax };
}

void CDependentUnitParameter::SetValueMin(double _valueMin)
{
	m_valueMin = _valueMin;
}

void CDependentUnitParameter::SetValueMax(double _valueMax)
{
	m_valueMax = _valueMax;
}

double CDependentUnitParameter::GetParamMin() const
{
	return m_paramMin;
}

double CDependentUnitParameter::GetParamMax() const
{
	return m_paramMax;
}

SInterval CDependentUnitParameter::GetParamLimits() const
{
	return { m_paramMin, m_paramMax };
}

void CDependentUnitParameter::SetParamMin(double _paramMin)
{
	m_paramMin = _paramMin;
}

void CDependentUnitParameter::SetParamMax(double _paramMax)
{
	m_paramMax = _paramMax;
}

double CDependentUnitParameter::GetValue(double _param) const
{
	return m_data.GetValue(_param);
}

void CDependentUnitParameter::SetValue(double _param, double _value)
{
	m_data.SetValue(_param, _value);
}

void CDependentUnitParameter::RemoveValue(double _param)
{
	m_data.RemoveValue(_param);
}

std::vector<double> CDependentUnitParameter::GetParams() const
{
	return m_data.GetParamsList();
}

std::vector<double> CDependentUnitParameter::GetValues() const
{
	return m_data.GetValuesList();
}

void CDependentUnitParameter::SetValues(const std::vector<double>& _params, const std::vector<double>& _values)
{
	m_data.Clear();
	m_data.SetValues(_params, _values);
}

std::vector<std::pair<double, double>> CDependentUnitParameter::GetParamValuePairs() const
{
	auto res = ReservedVector<std::pair<double, double>>(m_data.Size());
	for (size_t i = 0; i < m_data.Size(); ++i)
		res.push_back(m_data.GetPairAt(i));
	return res;
}

const CDependentValues& CDependentUnitParameter::GetDependentData() const
{
	return m_data;
}

size_t CDependentUnitParameter::Size() const
{
	return m_data.Size();
}

bool CDependentUnitParameter::IsEmpty() const
{
	return m_data.IsEmpty();
}

bool CDependentUnitParameter::IsInBounds() const
{
	for (const auto& value : m_data.GetValuesList())
		if (value < m_valueMin || value > m_valueMax)
			return false;
	for (const auto& param : m_data.GetParamsList())
		if (param < m_paramMin || param > m_paramMax)
			return false;
	return true;
}

bool CDependentUnitParameter::HasParam(double _param) const
{
	return m_data.HasParam(_param);
}

std::ostream& CDependentUnitParameter::ValueToStream(std::ostream& _s)
{
	return _s << m_data;
}

std::istream& CDependentUnitParameter::ValueFromStream(std::istream& _s)
{
	return _s >> m_data;
}

void CDependentUnitParameter::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::BUnit_H5AttrSaveVersion, m_cnSaveVersion);

	// save data
	_h5File.WriteData(_path, StrConst::UParam_H5Params, GetParams());
	_h5File.WriteData(_path, StrConst::UParam_H5Values, GetValues());
}

void CDependentUnitParameter::LoadFromFile(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// load version of save procedure
	//const int version = _h5File.ReadAttribute(_path, StrConst::BUnit_H5AttrSaveVersion);

	m_data.Clear();

	// read data
	std::vector<double> params, values;
	_h5File.ReadData(_path, StrConst::UParam_H5Params, params);
	_h5File.ReadData(_path, StrConst::UParam_H5Values, values);
	for (size_t i = 0; i < params.size(); ++i)
		SetValue(params[i], values[i]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CTDUnitParameter

CTDUnitParameter::CTDUnitParameter() :
	CDependentUnitParameter()
{
	SetType(EUnitParameter::TIME_DEPENDENT);
}

CTDUnitParameter::CTDUnitParameter(std::string _name, std::wstring _units, std::string _description, double _min, double _max, double _value) :
	CDependentUnitParameter(std::move(_name), _value, std::move(_units), "Time", 0.0, L"s", std::move(_description),
		_min, _max, 0.0, std::numeric_limits<double>::max())
{
	SetType(EUnitParameter::TIME_DEPENDENT);
}

std::vector<double> CTDUnitParameter::GetTimes() const
{
	return GetParams();
}

void CTDUnitParameter::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	CDependentUnitParameter::SaveToFile(_h5File, _path);
}

void CTDUnitParameter::LoadFromFile(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// load version of save procedure
	const int version = _h5File.ReadAttribute(_path, StrConst::BUnit_H5AttrSaveVersion);

	if (version <= 1)
	{
		Clear();

		// read data
		std::vector<double> times, values;
		_h5File.ReadData(_path, StrConst::UParam_H5Times, times);
		_h5File.ReadData(_path, StrConst::UParam_H5Values, values);
		for (size_t i = 0; i < times.size(); ++i)
			SetValue(times[i], values[i]);
	}
	else
		CDependentUnitParameter::LoadFromFile(_h5File, _path);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// CStringUnitParameter

const unsigned CStringUnitParameter::m_cnSaveVersion = 1;

CStringUnitParameter::CStringUnitParameter() :
	CBaseUnitParameter(EUnitParameter::STRING)
{
}

CStringUnitParameter::CStringUnitParameter(std::string _name, std::string _description, std::string _value) :
	CBaseUnitParameter(EUnitParameter::STRING, std::move(_name), L"", std::move(_description)),
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

std::ostream& CStringUnitParameter::ValueToStream(std::ostream& _s)
{
	return _s << m_value;
}

std::istream& CStringUnitParameter::ValueFromStream(std::istream& _s)
{
	m_value = StringFunctions::GetValueFromStream<std::string>(_s);
	return _s;
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
	CBaseUnitParameter(EUnitParameter::CHECKBOX, std::move(_name), L"", std::move(_description)),
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

std::ostream& CCheckBoxUnitParameter::ValueToStream(std::ostream& _s)
{
	return _s << m_checked;
}

std::istream& CCheckBoxUnitParameter::ValueFromStream(std::istream& _s)
{
	m_checked = StringFunctions::GetValueFromStream<bool>(_s);
	return _s;
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
	CBaseUnitParameter(EUnitParameter::SOLVER, std::move(_name), L"", std::move(_description)),
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

std::ostream& CSolverUnitParameter::ValueToStream(std::ostream& _s)
{
	return _s << m_key;
}

std::istream& CSolverUnitParameter::ValueFromStream(std::istream& _s)
{
	m_key = StringFunctions::GetValueFromStream<std::string>(_s);
	return _s;
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
	CBaseUnitParameter(EUnitParameter::COMBO, std::move(_name), L"", std::move(_description))
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

std::string CComboUnitParameter::GetNameByItem(size_t _item) const
{
	for (const auto& p : m_items)
		if (p.first == _item)
			return p.second;
	return {};
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

std::ostream& CComboUnitParameter::ValueToStream(std::ostream& _s)
{
	return _s << m_selected;
}

std::istream& CComboUnitParameter::ValueFromStream(std::istream& _s)
{
	return _s >> m_selected;
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
	CBaseUnitParameter(EUnitParameter::COMPOUND, std::move(_name), L"", std::move(_description))
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

std::ostream& CCompoundUnitParameter::ValueToStream(std::ostream& _s)
{
	return _s << m_key;
}

std::istream& CCompoundUnitParameter::ValueFromStream(std::istream& _s)
{
	m_key = StringFunctions::GetValueFromStream<std::string>(_s);
	return _s;
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
/// CCompoundMDBUnitParameter

const unsigned CMDBCompoundUnitParameter::m_cnSaveVersion = 1;

CMDBCompoundUnitParameter::CMDBCompoundUnitParameter() :
	CCompoundUnitParameter()
{
	SetType(EUnitParameter::MDB_COMPOUND);
}

CMDBCompoundUnitParameter::CMDBCompoundUnitParameter(std::string _name, std::string _description) :
	CCompoundUnitParameter(std::move(_name), std::move(_description))
{
	SetType(EUnitParameter::MDB_COMPOUND);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CReactionUnitParameter

const unsigned CReactionUnitParameter::m_cnSaveVersion = 1;

CReactionUnitParameter::CReactionUnitParameter()
	: CBaseUnitParameter(EUnitParameter::REACTION)
{
}

CReactionUnitParameter::CReactionUnitParameter(std::string _name, std::string _description)
	: CBaseUnitParameter(EUnitParameter::REACTION, std::move(_name), L"", std::move(_description))
{
}

void CReactionUnitParameter::Clear()
{
	m_reactions.clear();
}

std::vector<CChemicalReaction> CReactionUnitParameter::GetReactions() const
{
	return m_reactions;
}

std::vector<CChemicalReaction*> CReactionUnitParameter::GetReactionsPtr()
{
	std::vector<CChemicalReaction*> res;
	for (auto& r : m_reactions)
		res.push_back(&r);
	return res;
}

const CChemicalReaction* CReactionUnitParameter::GetReaction(size_t _index) const
{
	if (_index >= m_reactions.size()) return nullptr;
	return &m_reactions[_index];
}

CChemicalReaction* CReactionUnitParameter::GetReaction(size_t _index)
{
	if (_index >= m_reactions.size()) return nullptr;
	return &m_reactions[_index];
}

size_t CReactionUnitParameter::GetReactionsNumber() const
{
	return m_reactions.size();
}

void CReactionUnitParameter::AddReaction()
{
	m_reactions.emplace_back();
}

void CReactionUnitParameter::AddReaction(const CChemicalReaction& _reaction)
{
	m_reactions.push_back(_reaction);
}

void CReactionUnitParameter::SetReactions(const std::vector<CChemicalReaction>& _reactions)
{
	m_reactions = _reactions;
}

void CReactionUnitParameter::RemoveReaction(size_t _index)
{
	if (_index >= m_reactions.size()) return;
	VectorDelete(m_reactions, _index);
}

std::ostream& CReactionUnitParameter::ValueToStream(std::ostream& _s)
{
	for (const auto& r : m_reactions)
	{
		_s << " " << r.GetSubstancesNumber() << " " << r.GetBaseSubstanceIndex() + 1;
		for (const auto& s : r.GetSubstances())
			_s << " " << s->key << " " << s->nu << " " << s->order << " " << E2I(s->phase);
	}
	return _s;
}

std::istream& CReactionUnitParameter::ValueFromStream(std::istream& _s)
{
	m_reactions.clear();
	while (!_s.eof())
	{
		auto& r = m_reactions.emplace_back();
		const auto numberSubstances = StringFunctions::GetValueFromStream<size_t>(_s);
		const auto baseSubstance    = StringFunctions::GetValueFromStream<size_t>(_s) - 1;
		for (size_t j = 0; j < numberSubstances; ++j)
		{
			auto* s  = r.AddSubstance();
			s->key   = StringFunctions::GetValueFromStream<std::string>(_s);
			s->nu    = StringFunctions::GetValueFromStream<double>(_s);
			s->order = StringFunctions::GetValueFromStream<double>(_s);
			s->phase = StringFunctions::GetEnumFromStream<EPhase>(_s);
		}
		r.SetBaseSubstance(baseSubstance);
	}
	return _s;
}

void CReactionUnitParameter::SaveToFile(CH5Handler& _h5Saver, const std::string& _path) const
{
	if (!_h5Saver.IsValid()) return;

	// current version of save procedure
	_h5Saver.WriteAttribute(_path, StrConst::BUnit_H5AttrSaveVersion, m_cnSaveVersion);

	// save data
	_h5Saver.WriteAttribute(_path, StrConst::UParam_H5AttrReactionsNum, static_cast<int>(m_reactions.size()));
	for (size_t i = 0; i < m_reactions.size(); ++i)
		m_reactions[i].SaveToFile(_h5Saver, _h5Saver.CreateGroup(_path, StrConst::UParam_H5Reaction + std::to_string(i)));
}

void CReactionUnitParameter::LoadFromFile(const CH5Handler& _h5Loader, const std::string& _path)
{
	if (!_h5Loader.IsValid()) return;

	// load version of save procedure
	//const int version = _h5Loader.ReadAttribute(_path, StrConst::BUnit_H5AttrSaveVersion);

	// read data
	m_reactions.resize(static_cast<size_t>(_h5Loader.ReadAttribute(_path, StrConst::UParam_H5AttrReactionsNum)));
	for (size_t i = 0; i < m_reactions.size(); ++i)
		m_reactions[i].LoadFromFile(_h5Loader, _path + "/" + StrConst::UParam_H5Reaction + std::to_string(i));
}
