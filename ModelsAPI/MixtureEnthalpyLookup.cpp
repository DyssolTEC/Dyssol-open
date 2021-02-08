/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "MixtureEnthalpyLookup.h"
#include "MaterialsDatabase.h"
#include "DyssolUtilities.h"
#include <utility>

CMixtureEnthalpyLookup::CMixtureEnthalpyLookup(const CMaterialsDatabase* _materialsDB, std::vector<std::string> _compounds)
	: CMixtureEnthalpyLookup{ _materialsDB, std::move(_compounds), { DEFAULT_ENTHALPY_MIN_T, DEFAULT_ENTHALPY_MAX_T }, DEFAULT_ENTHALPY_INTERVALS }
{
}

CMixtureEnthalpyLookup::CMixtureEnthalpyLookup(const CMaterialsDatabase* _materialsDB, std::vector<std::string> _compounds, const SInterval& _limits, size_t _intervalsNumber)
	: m_limits{ _limits }
	, m_intervals{ _intervalsNumber }
	, m_materialsDB{ _materialsDB }
	, m_compounds{ std::move(_compounds) }
{
	UpdateCompoundsEnthalpies();
}

void CMixtureEnthalpyLookup::SetLimits(const SInterval& _limits, size_t _number)
{
	m_limits = _limits;
	m_intervals = _number;
	UpdateCompoundsEnthalpies();
}

SInterval CMixtureEnthalpyLookup::GetLimits() const
{
	return m_limits;
}

size_t CMixtureEnthalpyLookup::GetIntervalsNumber() const
{
	return m_intervals;
}

void CMixtureEnthalpyLookup::SetCompounds(const std::vector<std::string>& _compounds)
{
	m_compounds = _compounds;
	UpdateCompoundsEnthalpies();
}

void CMixtureEnthalpyLookup::SetCompoundFractions(const std::vector<double>& _fractions)
{
	if (_fractions.size() != m_compounds.size()) return;
	m_mixtureLookup.SetWeights(Normalize(_fractions));
}

std::vector<double> CMixtureEnthalpyLookup::GetCompoundFractions() const
{
	return m_mixtureLookup.GetWeights();
}

double CMixtureEnthalpyLookup::GetEnthalpy(double _temperature) const
{
	return m_mixtureLookup.GetRight(_temperature);
}

double CMixtureEnthalpyLookup::GetTemperature(double _enthalpy) const
{
	return m_mixtureLookup.GetLeft(_enthalpy);
}

void CMixtureEnthalpyLookup::Clear()
{
	m_mixtureLookup.Clear();
	m_compounds.clear();
}

void CMixtureEnthalpyLookup::Add(double _value)
{
	m_mixtureLookup.Add(_value);
}

void CMixtureEnthalpyLookup::Add(const CDependentValues& _component, double _weight)
{
	m_mixtureLookup.Add(_component, _weight);
}

void CMixtureEnthalpyLookup::Add(const CMixtureEnthalpyLookup& _table, double _weight)
{
	m_mixtureLookup.Add(_table.m_mixtureLookup, _weight);
}

void CMixtureEnthalpyLookup::Multiply(double _value)
{
	m_mixtureLookup.Multiply(_value);
}

CMixtureEnthalpyLookup CMixtureEnthalpyLookup::operator+(double _d) const
{
	CMixtureEnthalpyLookup res(*this);
	res.Add(_d);
	return res;
}

CMixtureEnthalpyLookup CMixtureEnthalpyLookup::operator*(double _d) const
{
	CMixtureEnthalpyLookup res(*this);
	res.Multiply(_d);
	return res;
}

CMixtureEnthalpyLookup CMixtureEnthalpyLookup::operator+(const CMixtureEnthalpyLookup& _t) const
{
	CMixtureEnthalpyLookup res(*this);
	res.Add(_t);
	return res;
}

CMixtureEnthalpyLookup& CMixtureEnthalpyLookup::operator+=(double _d)
{
	Add(_d);
	return *this;
}

CMixtureEnthalpyLookup& CMixtureEnthalpyLookup::operator*=(double _d)
{
	Multiply(_d);
	return *this;
}

CMixtureEnthalpyLookup& CMixtureEnthalpyLookup::operator+=(const CMixtureEnthalpyLookup& _t)
{
	Add(_t);
	return *this;
}

void CMixtureEnthalpyLookup::UpdateCompoundsEnthalpies()
{
	if (!m_materialsDB) return;

	// temperature step
	const double deltaT = (m_limits.max - m_limits.min) / static_cast<double>(m_intervals);
	// keep old weights
	auto weights = m_mixtureLookup.GetWeights();
	// if weights are wrong or not defined, set same weights
	if (weights.size() != m_compounds.size())
		weights.assign(m_compounds.size(), 1.0 / static_cast<double>(m_compounds.size()));
	// clear all previous data
	m_mixtureLookup.Clear();
	// fill compound tables
	for (size_t iCmp = 0; iCmp < m_compounds.size(); ++iCmp)
	{
		CDependentValues table;
		for (size_t iInt = 0; iInt <= m_intervals; ++iInt)
		{
			const double T = m_limits.min + deltaT * static_cast<double>(iInt);
			const double enthalpy = m_materialsDB->GetTPPropertyValue(m_compounds[iCmp], ENTHALPY, T, STANDARD_CONDITION_P);
			table.SetValue(T, enthalpy);
		}
		// check if it contains a constant, because of the requirements of CMixtureLookup on elements uniqueness
		if (table.IsConst())
		{
			// leave only one value
			const auto enthalpy = table.GetValueAt(0);
			table.Clear();
			table.SetValue((m_limits.max + m_limits.min) / 2.0, enthalpy);
		}
		// add compound table with its weight
		m_mixtureLookup.AddComponent(table, weights[iCmp]);
	}
}
