/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Stream2.h"
#include "TimeDependentValue.h"
#include "Phase.h"
#include "ContainerFunctions.h"
#include "DistributionsGrid.h"
#include "DyssolUtilities.h"

// TODO: remove all reinterpret_cast and static_cast for MDMatrix

std::string CStream2::GetName() const
{
	return m_name;
}

void CStream2::SetName(const std::string& _name)
{
	m_name = _name;
}

std::string CStream2::GetKey() const
{
	return m_key;
}

void CStream2::SetKey(const std::string& _key)
{
	m_key = _key;
}

void CStream2::AddTimePoint(double _time)
{
	if (HasTime(_time)) return;
	CopyTimePoint(_time, GetPreviousTimePoint(_time));
}

void CStream2::CopyTimePoint(double _timeDst, double _timeSrc)
{
	if (_timeDst < 0) return;

	// insert time point
	const auto pos = std::lower_bound(m_timePoints.begin(), m_timePoints.end(), _timeDst);
	if (pos == m_timePoints.end())					// all existing times are smaller
		m_timePoints.emplace_back(_timeDst);
	else if (std::abs(*pos - _timeDst) <= m_eps)	// this time already exists
		*pos = _timeDst;
	else											// insert to the right position
		m_timePoints.insert(pos, _timeDst);

	// copy data in overall parameters
	for (auto& [state, param] : m_overall)
		param->CopyTimePoint(_timeDst, _timeSrc);

	// copy data in phases
	for (auto& [state, phase] : m_phases)
		phase->CopyTimePoint(_timeDst, _timeSrc);
}

void CStream2::RemoveTimePoint(double _time)
{
	RemoveTimePoints(_time, _time);
}

void CStream2::RemoveTimePoints(double _timeBeg, double _timeEnd)
{
	if (m_timePoints.empty()) return;
	if (_timeBeg > _timeEnd) return;

	// remove time points
	const auto beg = std::lower_bound(m_timePoints.begin(), m_timePoints.end(), _timeBeg);
	auto end = std::upper_bound(m_timePoints.begin(), m_timePoints.end(), _timeEnd);
	if (end == m_timePoints.begin()) return;
	m_timePoints.erase(beg, --end);

	// remove data in overall parameters
	for (auto& [state, param] : m_overall)
		param->RemoveTimePoints(_timeBeg, _timeEnd);

	// remove data in phases
	for (auto& [state, phase] : m_phases)
		phase->RemoveTimePoints(_timeBeg, _timeEnd);
}

void CStream2::RemoveTimePointsAfter(double _time, bool _inclusive)
{
	const auto beg = _inclusive ? std::lower_bound(m_timePoints.begin(), m_timePoints.end(), _time) : std::upper_bound(m_timePoints.begin(), m_timePoints.end(), _time);
	if (beg == m_timePoints.end()) return;
	RemoveTimePoints(*beg, GetLastTimePoint());
}

std::vector<double> CStream2::GetAllTimePoints() const
{
	return m_timePoints;
}

std::vector<double> CStream2::GetTimePointsInInterval(double _timeBeg, double _timeEnd) const
{
	if (m_timePoints.empty()) return {};
	if (_timeBeg > _timeEnd) return {};

	auto end = std::upper_bound(m_timePoints.begin(), m_timePoints.end(), _timeEnd);
	if (end == m_timePoints.begin()) return {};
	const auto beg = std::lower_bound(m_timePoints.begin(), m_timePoints.end(), _timeBeg);
	return { beg, --end };
}

double CStream2::GetLastTimePoint() const
{
	if (m_timePoints.empty()) return {};

	return m_timePoints.back();
}

double CStream2::GetPreviousTimePoint(double _time) const
{
	if (m_timePoints.empty()) return {};

	auto pos = std::lower_bound(m_timePoints.begin(), m_timePoints.end(), _time);
	if (pos == m_timePoints.begin()) return {};
	return *--pos;
}

void CStream2::AddOverallProperty(EOverall _property)
{
	m_overall.insert({ _property, std::make_unique<CTimeDependentValue>() });
}

void CStream2::RemoveOverallProperty(EOverall _property)
{
	m_overall.erase(_property);
}

CTimeDependentValue* CStream2::GetOverallProperty(EOverall _property)
{
	if (!HasOverallProperty(_property)) return nullptr;

	return m_overall[_property].get();
}

double CStream2::GetOverallProperty(double _time, EOverall _property) const
{
	if (!HasOverallProperty(_property))
	{
		if (_property == EOverall::OVERALL_TEMPERATURE)	return STANDARD_CONDITION_T;
		if (_property == EOverall::OVERALL_PRESSURE)		return STANDARD_CONDITION_P;
		return {};
	}

	return m_overall.at(_property)->GetValue(_time);
}

double CStream2::GetMass(double _time) const
{
	return GetOverallProperty(_time, EOverall::OVERALL_MASS);
}

double CStream2::GetTemperature(double _time) const
{
	return GetOverallProperty(_time, EOverall::OVERALL_TEMPERATURE);
}

double CStream2::GetPressure(double _time) const
{
	return GetOverallProperty(_time, EOverall::OVERALL_PRESSURE);
}

void CStream2::SetOverallProperty(double _time, EOverall _property, double _value)
{
	if (!HasOverallProperty(_property)) return;

	m_overall[_property]->SetValue(_time, _value);
}

void CStream2::SetMass(double _time, double _value)
{
	SetOverallProperty(_time, EOverall::OVERALL_MASS, _value);
}

void CStream2::SetTemperature(double _time, double _value)
{
	SetOverallProperty(_time, EOverall::OVERALL_TEMPERATURE, _value);
}

void CStream2::SetPressure(double _time, double _value)
{
	SetOverallProperty(_time, EOverall::OVERALL_PRESSURE, _value);
}

void CStream2::AddCompound(const std::string& _compoundKey)
{
	if (HasCompound(_compoundKey)) return;

	m_compounds.push_back(_compoundKey);
	for (auto& [state, phase] : m_phases)
		phase->AddCompound(_compoundKey);

	// TODO: Clear T-Lookup tables here
}

void CStream2::RemoveCompound(const std::string& _compoundKey)
{
	if (!HasCompound(_compoundKey)) return;

	VectorDelete(m_compounds, _compoundKey);
	for (auto& [state, phase] : m_phases)
		phase->RemoveCompound(_compoundKey);

	// TODO: Clear T-Lookup tables here
}

double CStream2::GetCompoundFraction(double _time, const std::string& _compoundKey) const
{
	if (!HasCompound(_compoundKey)) return {};

	const size_t iCompound = CompoundIndex(_compoundKey);
	double res = 0.0;
	for (const auto& [state, phase] : m_phases)
		res += phase->GetFraction(_time) * phase->GetCompoundFraction(_time, iCompound);
	return res;
}

double CStream2::GetCompoundFraction(double _time, const std::string& _compoundKey, EPhase _phase) const
{
	if (!HasCompound(_compoundKey) || !HasPhase(_phase)) return {};

	return m_phases.at(_phase)->GetCompoundFraction(_time, CompoundIndex(_compoundKey));
}

double CStream2::GetCompoundFractionMoll(double _time, const std::string& _compoundKey, EPhase _phase) const
{
	if (!HasCompound(_compoundKey) || !HasPhase(_phase)) return {};

	const double molarMass = GetCompoundConstProperty(_compoundKey, MOLAR_MASS);
	if (molarMass == 0.0) return {};
	return m_phases.at(_phase)->GetCompoundFraction(_time, CompoundIndex(_compoundKey)) / molarMass * GetPhaseProperty(_time, _phase, MOLAR_MASS);
}

double CStream2::GetCompoundMass(double _time, const std::string& _compoundKey, EPhase _phase) const
{
	if (!HasCompound(_compoundKey) || !HasPhase(_phase)) return {};

	return m_phases.at(_phase)->GetFraction(_time)										// mass fraction of phase
		* m_phases.at(_phase)->GetCompoundFraction(_time, CompoundIndex(_compoundKey))	// mass fraction of compound in phase
		* GetMass(_time);																// whole mass
}

void CStream2::SetCompoundFraction(double _time, const std::string& _compoundKey, EPhase _phase, double _value)
{
	if (!HasCompound(_compoundKey) || !HasPhase(_phase)) return;

	m_phases[_phase]->SetCompoundFraction(_time, CompoundIndex(_compoundKey), _value);
}

void CStream2::AddPhase(EPhase _phase)
{
	if (HasPhase(_phase)) return;

	// add phase
	m_phases.insert({ _phase, std::make_unique<CPhase>(_phase, *m_grid, m_compounds, m_cacheSettings) });

	// add time points
	// TODO: maybe remove this
	for (const auto& t : m_timePoints)
		m_phases[_phase]->AddTimePoint(t);
}

void CStream2::RemovePhase(EPhase _phase)
{
	if (!HasPhase(_phase)) return;

	m_phases.erase(_phase);
}

double CStream2::GetPhaseFraction(double _time, EPhase _phase) const
{
	if (!HasPhase(_phase)) return {};

	return m_phases.at(_phase)->GetFraction(_time);
}

double CStream2::GetPhaseMass(double _time, EPhase _phase) const
{
	if (!HasPhase(_phase)) return {};

	return GetMass(_time) * m_phases.at(_phase)->GetFraction(_time);
}

double CStream2::GetPhaseProperty(double _time, EPhase _phase, EOverall _property) const
{
	if (!HasPhase(_phase)) return {};

	if (_property == EOverall::OVERALL_MASS)
		return GetPhaseMass(_time, _phase);
	return GetOverallProperty(_time, _property);
}

double CStream2::GetPhaseProperty(double _time, EPhase _phase, ECompoundConstProperties _property) const
{
	if (!HasPhase(_phase)) return {};

	// TODO: implement other properties if needed
	if (_property == MOLAR_MASS)
	{
		double res = 0.0;
		for (const auto& c : m_compounds)
		{
			const double molarMass = GetCompoundConstProperty(c, MOLAR_MASS);
			if (molarMass != 0.0)
				res += GetCompoundFraction(_time, c, _phase) / molarMass;
		}
		if (res != 0.0)
			return 1.0 / res;
	}

	return {};
}

double CStream2::GetPhaseProperty(double _time, EPhase _phase, ECompoundTPProperties _property) const
{
	if (!HasPhase(_phase)) return {};

	double res = 0.0;
	const double T = GetPhaseProperty(_time, _phase, EOverall::OVERALL_TEMPERATURE);
	const double P = GetPhaseProperty(_time, _phase, EOverall::OVERALL_PRESSURE);

	switch (_property)
	{
	case VAPOR_PRESSURE:
	{
		std::vector<double> pressures;
		for (const auto& c : m_compounds)
			pressures.push_back(m_materialsDB->GetTPPropertyValue(c, _property, T, P));
		return VectorMin(pressures);
	}
	case VISCOSITY:
		switch (_phase)
		{
		case EPhase::LIQUID:
			for (const auto& c : m_compounds)
			{
				const double visco = m_materialsDB->GetTPPropertyValue(c, _property, T, P);
				if (visco != 0.0)
					res += GetCompoundFraction(_time, c, _phase) * std::log(visco);
			}
			if (res != 0.0)
				return std::exp(res);
			break;
		case EPhase::VAPOR:
		{
			double numerator = 0.0, denominator = 0.0;
			for (const auto& c : m_compounds)
			{
				const double visco = m_materialsDB->GetTPPropertyValue(c, _property, T, P);
				const double mollMass = GetCompoundConstProperty(c, MOLAR_MASS);
				const double mollFrac = GetCompoundFractionMoll(_time, c, _phase);
				numerator += mollFrac * visco * std::sqrt(mollMass);
				denominator += mollFrac * std::sqrt(mollMass);
			}
			if (denominator != 0.0)
				return numerator / denominator;
			break;
		}
		case EPhase::SOLID:
			for (const auto& c : m_compounds)
				res += GetCompoundFraction(_time, c, _phase) * m_materialsDB->GetTPPropertyValue(c, _property, T, P);
			return res;
		case EPhase::UNDEFINED: return {};
		}
		break;
	case THERMAL_CONDUCTIVITY:
		switch (_phase)
		{
		case EPhase::LIQUID:
			for (const auto& c : m_compounds)
				res += GetCompoundFractionMoll(_time, c, _phase) / std::pow(m_materialsDB->GetTPPropertyValue(c, _property, T, P), 2.0);
			if (res != 0.0)
				return 1.0 / std::sqrt(res);
			break;
		case EPhase::VAPOR:
			for (const auto& c1 : m_compounds)
			{
				const double conduct1 = m_materialsDB->GetTPPropertyValue(c1, _property, T, P);
				const double mollMass1 = GetCompoundConstProperty(c1, MOLAR_MASS);
				const double numerator = GetCompoundFractionMoll(_time, c1, _phase) * conduct1;
				double denominator = 0.0;
				for (const auto& c2 : m_compounds)
				{
					const double conduct2 = m_materialsDB->GetTPPropertyValue(c2, _property, T, P);
					const double mollMass2 = GetCompoundConstProperty(c2, MOLAR_MASS);
					const double f = std::pow((1 + std::sqrt(conduct1 / conduct2) * std::pow(mollMass2 / mollMass1, 1.0 / 4.0)), 2) / std::sqrt(8 * (1 + mollMass1 / mollMass2));
					denominator += GetCompoundFractionMoll(_time, c2, _phase) * f;
				}
				if (denominator != 0.0)
					res += numerator / denominator;
			}
			return res;
		case EPhase::SOLID:
			for (const auto& c : m_compounds)
				res += GetCompoundFraction(_time, c, _phase) * m_materialsDB->GetTPPropertyValue(c, _property, T, P);
			return res;
		case EPhase::UNDEFINED: return {};
		}
		break;
	case DENSITY:
		if (_phase == EPhase::SOLID && m_grid->IsDistrTypePresent(DISTR_PART_POROSITY))
		{
			CMatrix2D distr = m_phases.at(_phase)->MDDistr()->GetDistribution(_time, DISTR_COMPOUNDS, DISTR_PART_POROSITY);
			const size_t nCompounds = m_compounds.size();
			const size_t nPorosities = m_grid->GetClassesByDistr(DISTR_PART_POROSITY);
			const std::vector<double> vPorosities = m_grid->GetClassMeansByDistr(DISTR_PART_POROSITY);
			for (size_t iCompound = 0; iCompound < nCompounds; ++iCompound)
			{
				const double density = GetCompoundTPProperty(_time, m_compounds[iCompound], DENSITY);
				for (size_t iPoros = 0; iPoros < nPorosities; ++iPoros)
					res += density * (1 - vPorosities[iPoros]) * distr[iCompound][iPoros];
			}
			return res;
		}
		else
		{
			for (const auto& c : m_compounds)
			{
				const double componentDensity = m_materialsDB->GetTPPropertyValue(c, _property, T, P);
				if (componentDensity != 0.0)
					res += GetCompoundFraction(_time, c, _phase) / componentDensity;
			}
			if (res != 0.0)
				return 1.0 / res;
		}
		break;
	case ENTHALPY:
	case PERMITTIVITY:
	case TP_PROP_USER_DEFINED_01:
	case TP_PROP_USER_DEFINED_02:
	case TP_PROP_USER_DEFINED_03:
	case TP_PROP_USER_DEFINED_04:
	case TP_PROP_USER_DEFINED_05:
	case TP_PROP_USER_DEFINED_06:
	case TP_PROP_USER_DEFINED_07:
	case TP_PROP_USER_DEFINED_08:
	case TP_PROP_USER_DEFINED_09:
	case TP_PROP_USER_DEFINED_10:
	case TP_PROP_USER_DEFINED_11:
	case TP_PROP_USER_DEFINED_12:
	case TP_PROP_USER_DEFINED_13:
	case TP_PROP_USER_DEFINED_14:
	case TP_PROP_USER_DEFINED_15:
	case TP_PROP_USER_DEFINED_16:
	case TP_PROP_USER_DEFINED_17:
	case TP_PROP_USER_DEFINED_18:
	case TP_PROP_USER_DEFINED_19:
	case TP_PROP_USER_DEFINED_20:
		for (const auto& c : m_compounds)
			res += GetCompoundFraction(_time, c, _phase) * m_materialsDB->GetTPPropertyValue(c, _property, T, P);
		return res;
	case TP_PROP_NO_PROERTY: break;
	}

	return {};
}

void CStream2::SetPhaseFraction(double _time, EPhase _phase, double _value)
{
	if (!HasPhase(_phase)) return;

	m_phases[_phase]->SetFraction(_time, _value);
}

void CStream2::SetPhaseMass(double _time, EPhase _phase, double _value)
{
	if (!HasPhase(_phase)) return;

	// get current masses
	double totalMass = GetMass(_time);
	std::map<EPhase, double> phaseMasses;
	for (const auto& [state, phase] : m_phases)
		phaseMasses[state] = GetPhaseMass(_time, state);

	// calculate adjustment for the total mass
	totalMass += _value - phaseMasses[_phase];

	// set new phase fractions according to the changes phase masses
	phaseMasses[_phase] = _value;
	for (const auto& [state, phase] : m_phases)
		if (totalMass != 0.0)
			m_phases[state]->SetFraction(_time, phaseMasses[state] / totalMass);
		else
			m_phases[state]->SetFraction(_time, 0.0);

	// set new total mass
	SetMass(_time, totalMass);
}

double CStream2::GetMixtureProperty(double _time, EOverall _property) const
{
	return GetOverallProperty(_time, _property);
}

double CStream2::GetMixtureProperty(double _time, ECompoundConstProperties _property) const
{
	double res = 0.0;
	for (const auto& [state, phase] : m_phases)
		res += GetPhaseProperty(_time, state, _property) * phase->GetFraction(_time);
	return res;
}

double CStream2::GetMixtureProperty(double _time, ECompoundTPProperties _property) const
{
	double res = 0.0;
	for (const auto& [state, phase] : m_phases)
		res += GetPhaseProperty(_time, state, _property) * phase->GetFraction(_time);
	return res;
}

void CStream2::SetMixtureProperty(double _time, EOverall _property, double _value)
{
	SetOverallProperty(_time, _property, _value);
}

double CStream2::GetCompoundConstProperty(const std::string& _compoundKey, ECompoundConstProperties _property) const
{
	if (!m_materialsDB) return {};

	return m_materialsDB->GetConstPropertyValue(_compoundKey, _property);
}

double CStream2::GetCompoundTPProperty(const std::string& _compoundKey, ECompoundTPProperties _property, double _temperature, double _pressure) const
{
	if (!m_materialsDB) return {};

	return m_materialsDB->GetTPPropertyValue(_compoundKey, _property, _temperature, _pressure);
}

double CStream2::GetCompoundTPProperty(double _time, const std::string& _compoundKey, ECompoundTPProperties _property) const
{
	return GetCompoundTPProperty(_compoundKey, _property, GetTemperature(_time), GetPressure(_time));
}

double CStream2::GetCompoundInteractionProperty(const std::string& _compoundKey1, const std::string& _compoundKey2, EInteractionProperties _property, double _temperature, double _pressure) const
{
	if (!m_materialsDB) return {};

	return m_materialsDB->GetInteractionPropertyValue(_compoundKey1, _compoundKey2, _property, _temperature, _pressure);
}

double CStream2::GetCompoundInteractionProperty(double _time, const std::string& _compoundKey1, const std::string& _compoundKey2, EInteractionProperties _property) const
{
	return GetCompoundInteractionProperty(_compoundKey1, _compoundKey2, _property, GetTemperature(_time), GetPressure(_time));
}

double CStream2::GetFraction(double _time, const std::vector<size_t>& _coords) const
{
	if (!HasPhase(EPhase::SOLID)) return {};

	return m_phases.at(EPhase::SOLID)->MDDistr()->GetValue(_time, reinterpret_cast<const std::vector<unsigned>&>(_coords));
}

void CStream2::SetFraction(double _time, const std::vector<size_t>& _coords, double _value)
{
	if (!HasPhase(EPhase::SOLID)) return;

	// TODO: remove when it is not needed by MD matrix
	AddTimePoint(_time);
	m_phases.at(EPhase::SOLID)->MDDistr()->SetValue(_time, reinterpret_cast<const std::vector<unsigned>&>(_coords), _value);
}

std::vector<double> CStream2::GetDistribution(double _time, EDistrTypes _distribution) const
{
	if (!HasPhase(EPhase::SOLID)) return {};

	return m_phases.at(EPhase::SOLID)->MDDistr()->GetDistribution(_time, _distribution);
}

CMatrix2D CStream2::GetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2) const
{
	if (!HasPhase(EPhase::SOLID)) return {};

	return m_phases.at(EPhase::SOLID)->MDDistr()->GetDistribution(_time, _distribution1, _distribution2);
}

CDenseMDMatrix CStream2::GetDistribution(double _time, const std::vector<EDistrTypes>& _distributions) const
{
	if (!HasPhase(EPhase::SOLID)) return {};

	return m_phases.at(EPhase::SOLID)->MDDistr()->GetDistribution(_time, reinterpret_cast<const std::vector<unsigned>&>(_distributions));
}

std::vector<double> CStream2::GetDistribution(double _time, EDistrTypes _distribution, const std::string& _compoundKey) const
{
	if (!HasPhase(EPhase::SOLID)) return {};
	if (!HasCompound(_compoundKey)) return {};
	if (_distribution == DISTR_COMPOUNDS) return {};

	return ::Normalize(m_phases.at(EPhase::SOLID)->MDDistr()->GetVectorValue(_time, DISTR_COMPOUNDS, static_cast<unsigned>(CompoundIndex(_compoundKey)), _distribution));
}

CMatrix2D CStream2::GetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2, const std::string& _compoundKey) const
{
	if (!HasPhase(EPhase::SOLID)) return {};
	if (!HasCompound(_compoundKey)) return {};
	if (_distribution1 == DISTR_COMPOUNDS || _distribution2 == DISTR_COMPOUNDS) return {};

	// prepare coordinates
	const size_t size1 = m_grid->GetClassesByDistr(_distribution1);
	const size_t size2 = m_grid->GetClassesByDistr(_distribution2);
	const std::vector<EDistrTypes> dims{ DISTR_COMPOUNDS, _distribution1, _distribution2 };
	std::vector<size_t> coords{ CompoundIndex(_compoundKey), 0 };
	CMatrix2D res{ size1, size2 };

	// gather data
	for (size_t i = 0; i < size1; ++i)
	{
		res.SetRow(i, m_phases.at(EPhase::SOLID)->MDDistr()->GetVectorValue(_time, reinterpret_cast<const std::vector<unsigned>&>(dims), reinterpret_cast<const std::vector<unsigned>&>(coords)));
		coords.back()++;
	}
	res.Normalize();

	return res;
}

CDenseMDMatrix CStream2::GetDistribution(double _time, const std::vector<EDistrTypes>& _distributions, const std::string& _compoundKey) const
{
	if (!HasPhase(EPhase::SOLID)) return {};
	if (!HasCompound(_compoundKey)) return {};
	if (VectorContains(_distributions, DISTR_COMPOUNDS)) return {};

	// prepare common parameters
	std::vector<size_t> sizes(_distributions.size());
	for (size_t i = 0; i < _distributions.size(); ++i)
		sizes[i] = m_grid->GetClassesByDistr(_distributions[i]);
	CDenseMDMatrix res(reinterpret_cast<const std::vector<unsigned>&>(_distributions), reinterpret_cast<const std::vector<unsigned>&>(sizes));

	// prepare parameters for reading
	std::vector<EDistrTypes> getDims = _distributions;
	getDims.insert(getDims.begin(), DISTR_COMPOUNDS);
	std::vector<size_t> getCoords(_distributions.size());
	getCoords.front() = CompoundIndex(_compoundKey);
	std::vector<size_t> getSizes = sizes;
	getSizes.insert(getSizes.begin(), m_grid->GetClassesByDistr(DISTR_COMPOUNDS));
	getSizes.pop_back();

	// prepare parameters for writing
	const std::vector<EDistrTypes>& setDims = _distributions;
	std::vector<size_t> setCoords(_distributions.size() - 1);
	std::vector<size_t> setSizes = sizes;
	setSizes.pop_back();

	// gather data
	bool notFinished;
	do
	{
		std::vector<double> vec = m_phases.at(EPhase::SOLID)->MDDistr()->GetVectorValue(_time, reinterpret_cast<const std::vector<unsigned>&>(getDims), reinterpret_cast<const std::vector<unsigned>&>(getCoords));
		res.SetVectorValue(reinterpret_cast<const std::vector<unsigned>&>(setDims), reinterpret_cast<const std::vector<unsigned>&>(setCoords), vec);
		IncrementCoords(getCoords, getSizes);
		notFinished = IncrementCoords(setCoords, setSizes);
	} while (notFinished);
	res.Normalize();

	return res;
}

void CStream2::SetDistribution(double _time, EDistrTypes _distribution, const std::vector<double>& _value)
{
	if (!HasPhase(EPhase::SOLID)) return;

	m_phases[EPhase::SOLID]->MDDistr()->SetDistribution(_time, _distribution, _value);
}

void CStream2::SetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2, const CMatrix2D& _value)
{
	if (!HasPhase(EPhase::SOLID)) return;

	// TODO: remove when it is not needed by MD matrix
	AddTimePoint(_time);

	m_phases[EPhase::SOLID]->MDDistr()->SetDistribution(_time, _distribution1, _distribution2, _value);
}

void CStream2::SetDistribution(double _time, const CDenseMDMatrix& _value)
{
	if (!HasPhase(EPhase::SOLID)) return;

	// TODO: remove when it is not needed by MD matrix
	AddTimePoint(_time);

	m_phases[EPhase::SOLID]->MDDistr()->SetDistribution(_time, _value);
}

void CStream2::SetDistribution(double _time, EDistrTypes _distribution, const std::string& _compoundKey, const std::vector<double>& _value)
{
	if (!HasPhase(EPhase::SOLID)) return;
	if (!HasCompound(_compoundKey)) return;
	if (_distribution == DISTR_COMPOUNDS) return;

	// TODO: remove when it is not needed by MD matrix
	AddTimePoint(_time);

	m_phases[EPhase::SOLID]->MDDistr()->SetVectorValue(_time, DISTR_COMPOUNDS, static_cast<unsigned>(CompoundIndex(_compoundKey)), _distribution, _value, true);
	m_phases[EPhase::SOLID]->MDDistr()->NormalizeMatrix(_time);
}

void CStream2::SetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2, const std::string& _compoundKey, const CMatrix2D& _value)
{
	if (!HasPhase(EPhase::SOLID)) return;
	if (!HasCompound(_compoundKey)) return;
	if (_distribution1 == DISTR_COMPOUNDS || _distribution2 == DISTR_COMPOUNDS) return;

	// TODO: remove when it is not needed by MD matrix
	AddTimePoint(_time);

	// prepare common parameters
	std::vector<EDistrTypes> inDims{ _distribution1, _distribution2 };
	std::vector<size_t> inSizes(inDims.size());
	for (size_t i = 0; i < inDims.size(); ++i)
		inSizes[i] = m_grid->GetClassesByDistr(inDims[i]);

	// prepare parameters for reading from input 2D-distribution
	const size_t size1 = m_grid->GetClassesByDistr(_distribution1);

	// prepare parameters for writing to output MD-distribution
	std::vector<EDistrTypes> setDims = inDims;
	setDims.insert(setDims.begin(), DISTR_COMPOUNDS);
	std::vector<size_t> setCoords(inDims.size());
	setCoords.front() = CompoundIndex(_compoundKey);
	std::vector<size_t> setSizes = inSizes;
	setSizes.insert(setSizes.begin(), m_grid->GetClassesByDistr(DISTR_COMPOUNDS));

	// get old distribution with compounds
	std::vector<EDistrTypes> fullDims = inDims;
	fullDims.insert(fullDims.begin(), DISTR_COMPOUNDS);
	CDenseMDMatrix matrixMD = m_phases[EPhase::SOLID]->MDDistr()->GetDistribution(_time, reinterpret_cast<const std::vector<unsigned>&>(fullDims));

	// set new values from 2D-distribution to MD-distribution
	for (size_t i = 0; i < size1; ++i)
	{
		matrixMD.SetVectorValue(reinterpret_cast<const std::vector<unsigned>&>(setDims), reinterpret_cast<const std::vector<unsigned>&>(setCoords), _value.GetRow(i));
		IncrementCoords(setCoords, setSizes);
	}

	// set output MD-distribution into main distribution
	matrixMD.Normalize();
	m_phases[EPhase::SOLID]->MDDistr()->SetDistribution(_time, matrixMD);
}

void CStream2::SetDistribution(double _time, const std::string& _compoundKey, const CDenseMDMatrix& _value)
{
	if (!HasPhase(EPhase::SOLID)) return;
	if (!HasCompound(_compoundKey)) return;
	// TODO: remove cast
	if (VectorContains(_value.GetDimensions(), E2I(DISTR_COMPOUNDS))) return;

	// TODO: remove when it is not needed by MD matrix
	AddTimePoint(_time);

	// prepare common parameters
	// TODO: remove cast
	std::vector<EDistrTypes> inDims = VI2E<EDistrTypes>(_value.GetDimensions());
	std::vector<size_t> inSizes(inDims.size());
	for (size_t i = 0; i < inDims.size(); ++i)
		inSizes[i] = m_grid->GetClassesByDistr(inDims[i]);

	// prepare parameters for reading from input MD-distribution
	const std::vector<EDistrTypes> getDims = inDims;
	std::vector<size_t> getCoords(inDims.size() - 1);
	const std::vector<size_t> getSizes = inSizes;

	// prepare parameters for writing to output MD-distribution
	std::vector<EDistrTypes> setDims = inDims;
	setDims.insert(setDims.begin(), DISTR_COMPOUNDS);
	std::vector<size_t> setCoords(inDims.size());
	setCoords.front() = CompoundIndex(_compoundKey);
	std::vector<size_t> setSizes = inSizes;
	setSizes.insert(setSizes.begin(), m_grid->GetClassesByDistr(DISTR_COMPOUNDS));

	// get old distribution with compounds
	std::vector<EDistrTypes> fullDims = inDims;
	fullDims.insert(fullDims.begin(), DISTR_COMPOUNDS);
	CDenseMDMatrix matrixMD = m_phases[EPhase::SOLID]->MDDistr()->GetDistribution(_time, reinterpret_cast<const std::vector<unsigned>&>(fullDims));

	// set new values from input MD-distribution to output MD-distribution
	bool notFinished;
	do
	{
		matrixMD.SetVectorValue(reinterpret_cast<const std::vector<unsigned>&>(setDims), reinterpret_cast<const std::vector<unsigned>&>(setCoords), _value.GetVectorValue(reinterpret_cast<const std::vector<unsigned>&>(getDims), reinterpret_cast<const std::vector<unsigned>&>(getCoords)));
		IncrementCoords(setCoords, setSizes);
		notFinished = IncrementCoords(getCoords, getSizes);
	} while (notFinished);

	// set output MD-distribution into main distribution
	matrixMD.Normalize();
	m_phases[EPhase::SOLID]->MDDistr()->SetDistribution(_time, matrixMD);
}

void CStream2::ApplyTM(double _time, const CTransformMatrix& _matrix)
{
	if (!HasPhase(EPhase::SOLID)) return;

	m_phases[EPhase::SOLID]->MDDistr()->Transform(_time, _matrix);
	m_phases[EPhase::SOLID]->MDDistr()->NormalizeMatrix(_time);
}

void CStream2::ApplyTM(double _time, const std::string& _compoundKey, const CTransformMatrix& _matrix)
{
	if (!HasPhase(EPhase::SOLID)) return;
	if (!HasCompound(_compoundKey)) return;
	if (_matrix.GetDimensionsNumber() == 0) return;
	// TODO: remove cast
	if (VectorContains(_matrix.GetDimensions(), E2I(DISTR_COMPOUNDS))) return;

	// prepare all parameters
	const size_t iCompound = CompoundIndex(_compoundKey);
	// TODO: remove cast
	std::vector<EDistrTypes> inDims = VI2E<EDistrTypes>(_matrix.GetDimensions());
	std::vector<EDistrTypes> newDims = inDims;
	newDims.insert(newDims.begin(), DISTR_COMPOUNDS);
	// TODO: remove cast
	std::vector<size_t> oldSizes = VI2E<size_t>(_matrix.GetClasses());
	std::vector<size_t> oldSizesFull = oldSizes;
	oldSizesFull.insert(oldSizesFull.end(), oldSizes.begin(), oldSizes.end() - 1);
	std::vector<size_t> newSizes = oldSizes;
	newSizes.insert(newSizes.begin(), m_grid->GetClassesByDistr(DISTR_COMPOUNDS));
	std::vector<size_t> oldCoordsFull(oldSizes.size() * 2 - 1);
	std::vector<size_t> newCoords(newSizes.size());
	newCoords.front() = iCompound;

	// create new transformation matrix
	CTransformMatrix newTM(reinterpret_cast<const std::vector<unsigned>&>(newDims), reinterpret_cast<const std::vector<unsigned>&>(newSizes));

	// copy values from old to new TM
	bool notFinished;
	size_t oldSrcSize = newDims.size() - 1;
	size_t oldDstSize = newDims.size() - 2;
	size_t newSrcSize = newDims.size();
	size_t newDstSize = newDims.size() - 1;
	std::vector<size_t> oldSrcCoords(oldSrcSize);
	std::vector<size_t> oldDstCoords(oldDstSize);
	std::vector<size_t> newSrcCoords(newSrcSize);
	std::vector<size_t> newDstCoords(newDstSize);
	newSrcCoords.front() = iCompound;
	newDstCoords.front() = iCompound;
	do
	{
		std::copy_n(oldCoordsFull.begin(), oldSrcSize, oldSrcCoords.begin());
		std::copy(oldCoordsFull.begin() + oldSrcSize, oldCoordsFull.end(), oldDstCoords.begin());
		std::copy_n(oldCoordsFull.begin(), oldSrcSize, newSrcCoords.begin() + 1);
		std::copy(oldCoordsFull.begin() + oldSrcSize, oldCoordsFull.end(), newDstCoords.begin() + 1);
		std::vector<double> vec = _matrix.GetVectorValue(reinterpret_cast<const std::vector<unsigned>&>(oldSrcCoords), reinterpret_cast<const std::vector<unsigned>&>(oldDstCoords));
		newTM.SetVectorValue(reinterpret_cast<const std::vector<unsigned>&>(newSrcCoords), reinterpret_cast<const std::vector<unsigned>&>(newDstCoords), vec);
		notFinished = IncrementCoords(oldCoordsFull, oldSizesFull);
	} while (notFinished);

	// set values for other compounds
	std::fill(newCoords.begin(), newCoords.end(), 0);
	notFinished = true;
	do
	{
		if (newCoords.front() != iCompound)
			newTM.SetValue(reinterpret_cast<const std::vector<unsigned>&>(newCoords), reinterpret_cast<const std::vector<unsigned>&>(newCoords), 1);
		notFinished = IncrementCoords(newCoords, newSizes);
	} while (notFinished);

	// transform
	m_phases[EPhase::SOLID]->MDDistr()->Transform(_time, newTM);
	m_phases[EPhase::SOLID]->MDDistr()->NormalizeMatrix(_time);
}

void CStream2::Normalize(double _time)
{
	if (!HasPhase(EPhase::SOLID)) return;

	return m_phases[EPhase::SOLID]->MDDistr()->NormalizeMatrix(_time);
}

void CStream2::Normalize(double _timeBeg, double _timeEnd)
{
	if (!HasPhase(EPhase::SOLID)) return;

	return m_phases[EPhase::SOLID]->MDDistr()->NormalizeMatrix(_timeBeg, _timeEnd);
}

void CStream2::Normalize()
{
	if (!HasPhase(EPhase::SOLID)) return;

	return m_phases[EPhase::SOLID]->MDDistr()->NormalizeMatrix();
}

std::vector<double> CStream2::GetPSD(double _time, EPSDTypes _type, EPSDGridType _grid) const
{
	return GetPSD(_time, _type, std::vector<std::string>{}, _grid);
}

std::vector<double> CStream2::GetPSD(double _time, EPSDTypes _type, const std::string& _compoundKey, EPSDGridType _grid) const
{
	return GetPSD(_time, _type, std::vector<std::string>(1, _compoundKey), _grid);
}

std::vector<double> CStream2::GetPSD(double _time, EPSDTypes _type, const std::vector<std::string>& _compoundKeys, EPSDGridType _grid) const
{
	if (!HasPhase(EPhase::SOLID)) return {};
	if (!m_grid->IsDistrTypePresent(DISTR_SIZE)) return {};
	if (!HasCompounds(_compoundKeys)) return {};

	switch (_type)
	{
	case PSD_MassFrac:	return GetPSDMassFraction(_time, _compoundKeys);
	case PSD_Number:	return GetPSDNumber(_time, _compoundKeys, _grid);
	case PSD_q3:		return ConvertMassFractionsToq3(m_grid->GetPSDGrid(_grid), GetPSDMassFraction(_time, _compoundKeys));
	case PSD_Q3:		return ConvertMassFractionsToQ3(GetPSDMassFraction(_time, _compoundKeys));
	case PSD_q0:		return ConvertNumbersToq0(m_grid->GetPSDGrid(_grid), GetPSDNumber(_time, _compoundKeys, _grid));
	case PSD_Q0:		return ConvertNumbersToQ0(m_grid->GetPSDGrid(_grid), GetPSDNumber(_time, _compoundKeys, _grid));
	case PSD_q2:		return ConvertNumbersToq2(m_grid->GetPSDGrid(_grid), GetPSDNumber(_time, _compoundKeys, _grid));
	case PSD_Q2:		return ConvertNumbersToQ2(m_grid->GetPSDGrid(_grid), GetPSDNumber(_time, _compoundKeys, _grid));
	}

	return {};
}

void CStream2::SetPSD(double _time, EPSDTypes _type, const std::vector<double>& _value, EPSDGridType _grid)
{
	SetPSD(_time, _type, "", _value, _grid);
}

void CStream2::SetPSD(double _time, EPSDTypes _type, const std::string& _compoundKey, const std::vector<double>& _value, EPSDGridType _grid)
{
	if (!HasPhase(EPhase::SOLID)) return;
	if (!m_grid->IsDistrTypePresent(DISTR_SIZE)) return;

	// construct distribution
	std::vector<double> distr;
	switch (_type)
	{
	case PSD_MassFrac:	distr = _value;																break;
	case PSD_Number:	distr = ConvertNumbersToMassFractions(m_grid->GetPSDGrid(_grid), _value);	break;
	case PSD_q3:		distr = Convertq3ToMassFractions(m_grid->GetPSDGrid(_grid), _value);		break;
	case PSD_Q3:		distr = ConvertQ3ToMassFractions(_value);									break;
	case PSD_q0:		distr = Convertq0ToMassFractions(m_grid->GetPSDGrid(_grid), _value);		break;
	case PSD_Q0:		distr = ConvertQ0ToMassFractions(m_grid->GetPSDGrid(_grid), _value);		break;
	case PSD_q2:		distr = Convertq2ToMassFractions(m_grid->GetPSDGrid(_grid), _value);	    break;
	case PSD_Q2:		distr = ConvertQ2ToMassFractions(m_grid->GetPSDGrid(_grid), _value);		break;
	}
	::Normalize(distr);

	// set distribution
	if (_compoundKey.empty())			// for the total mixture
		m_phases[EPhase::SOLID]->MDDistr()->SetDistribution(_time, DISTR_SIZE, distr);
	else if (HasCompound(_compoundKey))	// for specific compound
		m_phases[EPhase::SOLID]->MDDistr()->SetVectorValue(_time, DISTR_COMPOUNDS, static_cast<unsigned>(CompoundIndex(_compoundKey)), DISTR_SIZE, distr, true);
	m_phases[EPhase::SOLID]->MDDistr()->NormalizeMatrix(_time);
}

bool CStream2::HasTime(double _time) const
{
	const auto pos = std::lower_bound(m_timePoints.begin(), m_timePoints.end(), _time);
	return std::abs(*pos - _time) <= m_eps;
}

bool CStream2::HasOverallProperty(EOverall _property) const
{
	return MapContainsKey(m_overall, _property);
}

bool CStream2::HasCompound(const std::string& _compoundKey) const
{
	return VectorContains(m_compounds, _compoundKey);
}

bool CStream2::HasCompounds(const std::vector<std::string>& _compoundKeys) const
{
	for (const auto& c : _compoundKeys)
		if (!HasCompound(c))
			return false;
	return true;
}

size_t CStream2::CompoundIndex(const std::string& _compoundKey) const
{
	for (size_t i = 0; i < m_compounds.size(); ++i)
		if (m_compounds[i] == _compoundKey)
			return i;
	return static_cast<size_t>(-1);
}

bool CStream2::HasPhase(EPhase _phase) const
{
	return MapContainsKey(m_phases, _phase);
}

std::vector<double> CStream2::GetPSDMassFraction(double _time, const std::vector<std::string>& _compoundKeys) const
{
	// for all available compounds
	if (_compoundKeys.empty() || _compoundKeys.size() == m_compounds.size())
		return m_phases.at(EPhase::SOLID)->MDDistr()->GetDistribution(_time, DISTR_SIZE);

	// only for selected compounds
	std::vector<double> distr(m_grid->GetClassesByDistr(DISTR_SIZE), 0.0);
	for (const auto& key : _compoundKeys)
	{
		std::vector<double> vec = m_phases.at(EPhase::SOLID)->MDDistr()->GetVectorValue(_time, DISTR_COMPOUNDS, static_cast<unsigned>(CompoundIndex(key)), DISTR_SIZE);
		AddVectors(distr, vec, distr);
	}
	::Normalize(distr);
	return distr;
}

std::vector<double> CStream2::GetPSDNumber(double _time, const std::vector<std::string>& _compoundKeys, EPSDGridType _grid) const
{
	std::vector<std::string> activeCompounds = _compoundKeys.empty() || _compoundKeys.size() == m_compounds.size() ? m_compounds : _compoundKeys;
	const bool hasPorosity = m_grid->IsDistrTypePresent(DISTR_PART_POROSITY);
	std::vector<double> volumes = _grid == EPSDGridType::VOLUME ? m_grid->GetPSDMeans(EPSDGridType::VOLUME) : DiameterToVolume(m_grid->GetPSDMeans(EPSDGridType::DIAMETER));
	const double totalMass = GetPhaseMass(_time, EPhase::SOLID);
	const size_t nSizeClasses = m_grid->GetClassesByDistr(DISTR_SIZE);

	// single compound with no porosity, only one compound defined in the stream
	if (!hasPorosity && activeCompounds.size() == 1 && m_compounds.size() == 1)
	{
		std::vector<double> res = m_phases.at(EPhase::SOLID)->MDDistr()->GetDistribution(_time, DISTR_SIZE);
		const double density = GetPhaseProperty(_time, EPhase::SOLID, DENSITY);
		if (density == 0.0) return std::vector<double>(nSizeClasses, 0.0);
		for (size_t i = 0; i < res.size(); ++i)
			if (volumes[i] != 0.0)
				res[i] *= totalMass / density / volumes[i];
		return res;
	}

	// single compound with no porosity, several compounds defined in the stream
	if (!hasPorosity && activeCompounds.size() == 1)
	{
		std::vector<double> res = ::Normalize(m_phases.at(EPhase::SOLID)->MDDistr()->GetVectorValue(_time, DISTR_COMPOUNDS, static_cast<unsigned>(CompoundIndex(activeCompounds.front())), DISTR_SIZE));
		const double density = GetPhaseProperty(_time, EPhase::SOLID, DENSITY);
		if (density == 0.0) return std::vector<double>(nSizeClasses, 0.0);
		for (size_t i = 0; i < res.size(); ++i)
			if (volumes[i] != 0.0)
				res[i] *= totalMass / density / volumes[i];
		return res;
	}

	// porosity is defined
	if (hasPorosity)
	{
		CDenseMDMatrix distr = m_phases.at(EPhase::SOLID)->MDDistr()->GetDistribution(_time, DISTR_COMPOUNDS, DISTR_SIZE, DISTR_PART_POROSITY);

		// filter by compounds
		if (activeCompounds.size() != m_compounds.size())
		{
			// TODO: remove cast
			std::vector<size_t> classes = VI2E<size_t>(distr.GetClasses());
			classes[0] = _compoundKeys.size(); // reduce number of compounds
			CDenseMDMatrix tempDistr{ distr.GetDimensions(), reinterpret_cast<const std::vector<unsigned>&>(classes) };
			for (size_t iCompound = 0; iCompound < activeCompounds.size(); ++iCompound)
				for (size_t iSize = 0; iSize < nSizeClasses; ++iSize)
				{
					std::vector<double> vec = distr.GetVectorValue(DISTR_COMPOUNDS, static_cast<unsigned>(CompoundIndex(activeCompounds[iCompound])), DISTR_SIZE, static_cast<unsigned>(iSize), DISTR_PART_POROSITY);
					tempDistr.SetVectorValue(DISTR_COMPOUNDS, static_cast<unsigned>(iCompound), DISTR_SIZE, static_cast<unsigned>(iSize), vec);
				}
			tempDistr.Normalize();
			distr = tempDistr;
		}

		const size_t nPorosityClasses = m_grid->GetClassesByDistr(DISTR_PART_POROSITY);
		const std::vector<double> porosities = m_grid->GetClassMeansByDistr(DISTR_PART_POROSITY);

		// calculate distribution
		std::vector<double> res(nSizeClasses);
		for (size_t iCompound = 0; iCompound < activeCompounds.size(); ++iCompound)
		{
			std::vector<double> vec(nSizeClasses);
			const double density = GetCompoundTPProperty(_time, activeCompounds[iCompound], DENSITY);
			if (density == 0.0) return std::vector<double>(nSizeClasses, 0.0);
			for (size_t iSize = 0; iSize < nSizeClasses; ++iSize)
			{
				for (size_t iPorosity = 0; iPorosity < nPorosityClasses; ++iPorosity)
				{
					const double fraction = distr.GetValue(DISTR_COMPOUNDS, static_cast<unsigned>(iCompound), DISTR_SIZE, static_cast<unsigned>(iSize), DISTR_PART_POROSITY, static_cast<unsigned>(iPorosity));
					vec[iSize] += fraction * totalMass / density * (1 - porosities[iPorosity]);
				}
				if (volumes[iSize] != 0.0)
					vec[iSize] /= volumes[iSize];
			}
			AddVectors(res, vec, res);
		}
		return res;
	}

	// several compounds without porosity
	{
		CMatrix2D distr = m_phases.at(EPhase::SOLID)->MDDistr()->GetDistribution(_time, DISTR_COMPOUNDS, DISTR_SIZE);

		// filter by compounds
		if (activeCompounds.size() != m_compounds.size())
		{
			CMatrix2D tempDistr{ distr.Rows(), distr.Cols() - 1 };
			for (size_t i = 0; i < activeCompounds.size(); ++i)
				tempDistr.SetRow(i, distr.GetRow(CompoundIndex(activeCompounds[i])));
			tempDistr.Normalize();
			distr = tempDistr;
		}
		distr *= totalMass;

		// calculate distribution
		std::vector<double> res(nSizeClasses);
		for (size_t iComp = 0; iComp < activeCompounds.size(); ++iComp)
		{
			const double density = GetCompoundTPProperty(_time, activeCompounds[iComp], DENSITY);
			if (density == 0.0) return std::vector<double>(nSizeClasses, 0.0);
			for (size_t iSize = 0; iSize < nSizeClasses; ++iSize)
				if (volumes[iSize] != 0.0)
					res[iSize] += distr[iComp][iSize] / density / volumes[iSize];
		}
		return res;
	}
}
