/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "UnitParameters.h"
#include "DyssolStringConstants.h"
#include "ContainerFunctions.h"
#include "DyssolUtilities.h"
#include <set>
#include <utility>


template<typename T>
EUnitParameter DeduceTypeConst()
{
	if (std::is_same_v<T, double>)		return EUnitParameter::CONSTANT_DOUBLE;
	if (std::is_same_v<T, int64_t>)		return EUnitParameter::CONSTANT_INT64;
	if (std::is_same_v<T, uint64_t>)	return EUnitParameter::CONSTANT_UINT64;
	return EUnitParameter::UNKNOWN;
}

///< Deduces type of the unit parameter depending on the template argument.
template <typename T>
EUnitParameter DeduceTypeList()
{
	if (std::is_same_v<T, double>)		return EUnitParameter::LIST_DOUBLE;
	if (std::is_same_v<T, int64_t>)		return EUnitParameter::LIST_INT64;
	if (std::is_same_v<T, uint64_t>)	return EUnitParameter::LIST_UINT64;
	return EUnitParameter::UNKNOWN;
}



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
CConstUnitParameter<T>::CConstUnitParameter() :
	CBaseUnitParameter(DeduceTypeConst<T>())
{
}

template<typename T>
CConstUnitParameter<T>::CConstUnitParameter(std::string _name, std::string _units, std::string _description, T _min, T _max, T _value) :
	CBaseUnitParameter(DeduceTypeConst<T>(), std::move(_name), std::move(_units), std::move(_description)),
	m_value{ _value },
	m_min{ _min },
	m_max{ _max }
{
}

template<typename T>
void CConstUnitParameter<T>::Clear()
{
	m_value = {};
}

template<typename T>
bool CConstUnitParameter<T>::IsInBounds() const
{
	return m_value >= m_min && m_value <= m_max;
}

template <typename T>
std::ostream& CConstUnitParameter<T>::ValueToStream(std::ostream& _s)
{
	return _s << m_value;
}

template <typename T>
std::istream& CConstUnitParameter<T>::ValueFromStream(std::istream& _s)
{
	return _s >> m_value;
}

template<typename T>
void CConstUnitParameter<T>::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::BUnit_H5AttrSaveVersion, m_cnSaveVersion);

	// save data
	_h5File.WriteData(_path, StrConst::UParam_H5Values, m_value);
}

template<typename T>
void CConstUnitParameter<T>::LoadFromFile(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// load version of save procedure
	//const int version = _h5File.ReadAttribute(_path, StrConst::BUnit_H5AttrSaveVersion);

	// read data
	_h5File.ReadData(_path, StrConst::UParam_H5Values, m_value);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// CListUnitParameter

template <typename T>
CListUnitParameter<T>::CListUnitParameter()
	: CBaseUnitParameter(DeduceTypeList<T>())
{
}

template <typename T>
CListUnitParameter<T>::CListUnitParameter(std::string _name, std::string _units, std::string _description, T _min, T _max, std::vector<T> _values)
	: CBaseUnitParameter{ DeduceTypeList<T>(), std::move(_name), std::move(_units), std::move(_description) }
	, m_values{ std::move(_values) }
	, m_min{ _min }
	, m_max{ _max }
{
}

template <typename T>
std::ostream& CListUnitParameter<T>::ValueToStream(std::ostream& _s)
{
	for (const auto& v : m_values)
		_s << " " << v;
	return _s;
}

template <typename T>
std::istream& CListUnitParameter<T>::ValueFromStream(std::istream& _s)
{
	m_values.clear();
	while (!_s.eof())
		m_values.push_back(StringFunctions::GetValueFromStream<T>(_s));
	return _s;
}

template <typename T>
void CListUnitParameter<T>::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::H5AttrSaveVersion, m_saveVersion);

	// save data
	_h5File.WriteData(_path, StrConst::UParam_H5Values, m_values);
}

template <typename T>
void CListUnitParameter<T>::LoadFromFile(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// load version of save procedure
	//const int version = _h5File.ReadAttribute(_path, StrConst::H5AttrSaveVersion);

	m_values.clear();

	// read data
	_h5File.ReadData(_path, StrConst::UParam_H5Values, m_values);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/// CDependentUnitParameter
CDependentUnitParameter::CDependentUnitParameter() :
	CBaseUnitParameter(EUnitParameter::PARAM_DEPENDENT)
{
}

CDependentUnitParameter::CDependentUnitParameter(std::string _valueName, double _valueInit, std::string _valueUnits, std::string _paramName, double _paramInit, std::string _paramUnits, std::string _description, double _valueMin, double _valueMax, double _paramMin, double _paramMax) :
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

std::string CDependentUnitParameter::GetParamUnits() const
{
	return m_paramUnits;
}

void CDependentUnitParameter::SetParamUnits(const std::string& _paramUnits)
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

CTDUnitParameter::CTDUnitParameter(std::string _name, std::string _units, std::string _description, double _min, double _max, double _value) :
	CDependentUnitParameter(std::move(_name), _value, std::move(_units), "Time", 0.0, "s", std::move(_description),
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
	: CBaseUnitParameter(EUnitParameter::REACTION, std::move(_name), "", std::move(_description))
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
	m_parameters.emplace_back(new CConstUnitParameter<double>{ _name, _units, _description, _min, _max, _value });
}

void CUnitParametersManager::AddConstIntParameter(const std::string& _name, const std::string& _units, const std::string& _description, int64_t _min, int64_t _max, int64_t _value)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CConstUnitParameter<int64_t>{ _name, _units, _description, _min, _max, _value });
}

void CUnitParametersManager::AddConstUIntParameter(const std::string& _name, const std::string& _units, const std::string& _description, uint64_t _min, uint64_t _max, uint64_t _value)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CConstUnitParameter<uint64_t>{ _name, _units, _description, _min, _max, _value });
}

void CUnitParametersManager::AddDependentParameter(const std::string& _name, const std::string& _units, const std::string& _description, double _min, double _max, double _value, const std::string& _paramName, const std::string& _paramUnits, double _paramMin, double _paramMax, double _paramValue)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CDependentUnitParameter{ _name, _value, _units, _paramName, _paramValue, _paramUnits, _description, _min, _max, _paramMin, _paramMax });
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

void CUnitParametersManager::AddListRealParameter(const std::string& _name, const std::string& _units, const std::string& _description, double _min, double _max, const std::vector<double>& _values)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CListUnitParameter<double>{ _name, _units, _description, _min, _max, _values });
}

void CUnitParametersManager::AddListIntParameter(const std::string& _name, const std::string& _units, const std::string& _description, int64_t _min, int64_t _max, const std::vector<int64_t>& _values)
{
	if (IsNameExist(_name)) return;
	m_parameters.emplace_back(new CListUnitParameter<int64_t>{ _name, _units, _description, _min, _max, _values });
}

void CUnitParametersManager::AddListUIntParameter(const std::string& _name, const std::string& _units, const std::string& _description, uint64_t _min, uint64_t _max, const std::vector<uint64_t>& _values)
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

double CUnitParametersManager::GetListRealParameterValue(size_t _index, size_t _vauleIndex) const
{
	if (const auto* p = GetListRealParameter(_index))
		return p->GetValue(_vauleIndex);
	return {};
}

int64_t CUnitParametersManager::GetListIntParameterValue(size_t _index, size_t _vauleIndex) const
{
	if (const auto* p = GetListIntParameter(_index))
		return p->GetValue(_vauleIndex);
	return {};
}

uint64_t CUnitParametersManager::GetListUIntParameterValue(size_t _index, size_t _vauleIndex) const
{
	if (const auto* p = GetListUIntParameter(_index))
		return p->GetValue(_vauleIndex);
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
