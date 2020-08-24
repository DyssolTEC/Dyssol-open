/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "LookupTable.h"
#include "DyssolUtilities.h"

CLookupTable::CLookupTable() :
	m_bFlippedConsistent(false),
	m_pMaterialsDB(nullptr),
	m_dMin(DEFAULT_LOWER_LIMIT),
	m_dMax(DEFAULT_UPPER_LIMIT),
	m_dStep(DEFAULT_STEP),
	m_nDependenceType(EDependencyTypes::DEPENDENCE_UNKNOWN),
	m_nProperty(static_cast<ECompoundTPProperties>(0))
{
}

CLookupTable::CLookupTable(const CMaterialsDatabase* _pMaterialsDB, const std::vector<std::string>& _vCompoundKeys, ECompoundTPProperties _nProperty, EDependencyTypes _nDependenceType) :
	m_bFlippedConsistent(false),
	m_pMaterialsDB(_pMaterialsDB),
	m_dMin(DEFAULT_LOWER_LIMIT),
	m_dMax(DEFAULT_UPPER_LIMIT),
	m_dStep(DEFAULT_STEP),
	m_nDependenceType(_nDependenceType),
	m_nProperty(_nProperty),
	m_vCompoundKeys(_vCompoundKeys)
{
	UpdateTableLimits();
	InitializeCompoundTables();
}

void CLookupTable::Initialize(const CMaterialsDatabase* _pMaterialsDB, const std::vector<std::string>& _vCompoundKeys, ECompoundTPProperties _nProperty, EDependencyTypes _nDependenceType)
{
	m_pMaterialsDB = _pMaterialsDB;
	m_vCompoundKeys = _vCompoundKeys;
	m_nProperty = _nProperty;
	m_nDependenceType = _nDependenceType;
	UpdateTableLimits();
	InitializeCompoundTables();
}

void CLookupTable::Clear()
{
	m_table.clear();
	m_flippedTable.clear();
	m_bFlippedConsistent = false;
	m_pMaterialsDB = nullptr;
	m_dMin = DEFAULT_LOWER_LIMIT;
	m_dMax = DEFAULT_UPPER_LIMIT;
	m_dStep = DEFAULT_STEP;
	m_nDependenceType = EDependencyTypes::DEPENDENCE_UNKNOWN;
	m_nProperty = static_cast<ECompoundTPProperties>(0);
	m_vCompoundKeys.clear();
	m_vCompoundTables.clear();
}

bool CLookupTable::IsValid() const
{
	return !(m_nDependenceType == EDependencyTypes::DEPENDENCE_UNKNOWN || m_nProperty == 0 || m_vCompoundKeys.empty() || m_vCompoundKeys.size() != m_vCompoundTables.size());
}

void CLookupTable::SetCompoundFractions(const std::vector<double>& _vFractions)
{
	if (_vFractions.size() != m_vCompoundKeys.size()) return;

	// Normalize mass fractions vector
	const std::vector<double> vNormFractions = Normalize(_vFractions);

	// Create m_table from compound lookup tables
	// set the first...
	if (!m_vCompoundTables.empty())
		SetTable(m_vCompoundTables.front(), vNormFractions.front());
	// ...add the rest
	for (size_t i = 1; i < m_vCompoundTables.size(); ++i)
		Add(m_vCompoundTables[i], vNormFractions[i]);

	// Flipped table not more consistent
	m_bFlippedConsistent = false;
}

double CLookupTable::GetValue(double _dParam) const
{
	return m_table.GetValue(_dParam);
}

double CLookupTable::GetParam(double _dValue) const
{
	UpdateFlippedTable();
	return m_flippedTable.GetValue(_dValue);
}

void CLookupTable::SetTable(const CDependentValues& _table, double _dWeight /*= 1.*/)
{
	m_table = _table;
	MultiplyTable(_dWeight);
	m_bFlippedConsistent = false;
}

void CLookupTable::Add(double _dValue, double _dWeight /*= 1.*/)
{
	for (auto& v : m_table)
		v.second += _dWeight * _dValue;
	m_bFlippedConsistent = false;
}

void CLookupTable::Add(const CDependentValues& _table, double _dWeight /*= 1.*/)
{
	if (!m_table.empty())
		for (auto& v : m_table)
			v.second += _dWeight * _table.GetValue(v.first);
	else
		SetTable(_table, _dWeight);
	m_bFlippedConsistent = false;
}

void CLookupTable::Add(const CLookupTable& _lookupTable, double _dWeight /*= 1.*/)
{
	if (_lookupTable.m_vCompoundKeys != m_vCompoundKeys || _lookupTable.m_nProperty != m_nProperty || _lookupTable.m_nDependenceType != m_nDependenceType)
		return;

	Add(_lookupTable.m_table, _dWeight);
	m_bFlippedConsistent = false;
}

void CLookupTable::MultiplyTable(double _dWeight)
{
	for (auto& v : m_table)
		v.second *= _dWeight;
	m_bFlippedConsistent = false;
}

void CLookupTable::InitializeCompoundTables()
{
	m_vCompoundTables.clear();
	for (const auto& key : m_vCompoundKeys)
	{
		const CDependentValues cCompoundTable = GetParametersList(key);
		if (cCompoundTable.empty())	continue;
		m_vCompoundTables.push_back(cCompoundTable);
	}
}

void CLookupTable::UpdateTableLimits()
{
	if (!m_pMaterialsDB || m_nDependenceType == EDependencyTypes::DEPENDENCE_UNKNOWN) return;

	// Initialize temporary minimum and maximum
	double dMin = -1.;
	double dMax = -1.;

	// Find minimum and maximum of dependency for each compound
	for (const auto& key : m_vCompoundKeys)
	{
		// Initialize variable pair with minimum and maximum value
		SInterval interval{ -1, -1 };

		// Update variable pair
		if (m_nDependenceType == EDependencyTypes::DEPENDENCE_TEMP)
			interval = m_pMaterialsDB->GetTPPropertyTInterval(key, m_nProperty);
		else
			interval = m_pMaterialsDB->GetTPPropertyPInterval(key, m_nProperty);

		// Set temporary minimum
		if (interval.min != -1)	dMin = (dMin == -1) ? interval.min : std::min(dMin, interval.min);

		// Set temporary maximum
		if (interval.max != -1)	dMax = (dMax == -1) ? interval.max : std::max(dMax, interval.max);
	}

	// Set minimum and maximum
	if (dMin != -1)	m_dMin = dMin;
	if (dMax != -1)	m_dMax = dMax;

	// Check minimum value
	if (m_dMin < 1)	m_dMin = 1;

	// Check maximum value
	if (m_dMax < 1)
	{
		if (m_nDependenceType == EDependencyTypes::DEPENDENCE_TEMP)
			m_dMax = MAX_TEMPERATURE;
		else
			m_dMax = MAX_PRESSURE;
	}

	// Calculate interval size of table
	if (m_nDependenceType == EDependencyTypes::DEPENDENCE_TEMP)
		m_dStep = std::min(int((dMax - dMin) / 100), MAX_TEMPERATURE_STEPS);
	else
		m_dStep = std::min(int((dMax - dMin) / 100), MAX_PRESSURE_STEPS);
}

CDependentValues CLookupTable::GetParametersList(const std::string& _sCompoundKey) const
{
	if (!m_pMaterialsDB) return {};

	CDependentValues table;
	double dParam = m_dMin;
	do
	{
		// Check for dependence type of compound tables
		double dTemperature, dPressure;
		switch (m_nDependenceType)
		{
		case EDependencyTypes::DEPENDENCE_TEMP:
			dTemperature = dParam;
			dPressure = STANDARD_CONDITION_P;
			break;
		case EDependencyTypes::DEPENDENCE_PRES:
			dTemperature = STANDARD_CONDITION_T;
			dPressure = dParam;
			break;
		default:
			dTemperature = STANDARD_CONDITION_T;
			dPressure = STANDARD_CONDITION_P;
		}

		const double dVal = m_pMaterialsDB->GetTPPropertyValue(_sCompoundKey, m_nProperty, dTemperature, dPressure);
		table.SetValue(dParam, dVal);

		dParam += m_dStep;
	} while (dParam < m_dMax);

	return table;
}

void CLookupTable::UpdateFlippedTable() const
{
	if (m_bFlippedConsistent) return;
	m_flippedTable.clear();
	for (const auto& v : m_table)
		m_flippedTable.SetValue(v.second, v.first);
	m_bFlippedConsistent = true;
}
