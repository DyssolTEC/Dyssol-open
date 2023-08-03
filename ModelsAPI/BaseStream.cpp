/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Stream.h"
#include "TimeDependentValue.h"
#include "Phase.h"
#include "MaterialsDatabase.h"
#include "ContainerFunctions.h"
#include "MultidimensionalGrid.h"
#include "DyssolStringConstants.h"
#include "DyssolUtilities.h"

// TODO: remove all reinterpret_cast and static_cast for MDMatrix.

CBaseStream::CBaseStream(const std::string& _key) :
	m_key{ _key.empty() ? StringFunctions::GenerateRandomKey() : _key }
{
	m_overall.insert({ EOverall::OVERALL_MASS, std::make_unique<CTimeDependentValue>() });
}

CBaseStream::CBaseStream(const std::string& _key, const CMaterialsDatabase* _materialsDB, const CMultidimensionalGrid& _grid,
	const std::vector<SOverallDescriptor>* _overall, const std::vector<SPhaseDescriptor>* _phases,
	const SCacheSettings* _cache, const SToleranceSettings* _tolerance, const SThermodynamicsSettings* _thermodynamics) :
	m_key{ _key.empty() ? StringFunctions::GenerateRandomKey() : _key },
	m_materialsDB{ _materialsDB },
	m_grid{ _grid }
{
	// order is important!
	SetCacheSettings(*_cache);
	SetThermodynamicsSettings(*_thermodynamics);
	for (const auto& overall : *_overall)
		AddOverallProperty(overall.type, overall.name, overall.units);
	for (const auto& phase : *_phases)
		AddPhase(phase.state, phase.name);
	SetToleranceSettings(*_tolerance);
}

CBaseStream::CBaseStream(const CBaseStream& _other) :
	m_name{ _other.m_name },
	m_key{ _other.m_key },
	m_materialsDB{ _other.m_materialsDB },
	m_timePoints{ _other.m_timePoints },
	m_grid{ _other.m_grid },
	m_cacheSettings{ _other.m_cacheSettings },
	m_toleranceSettings{ _other.m_toleranceSettings },
	m_thermodynamicsSettings{ _other.m_thermodynamicsSettings }
{
	for (const auto& [type, param] : _other.m_overall)
		m_overall.insert({ type, std::make_unique<CTimeDependentValue>(*param) });
	for (const auto& [state, phase] : _other.m_phases)
		m_phases.insert({ state, std::make_unique<CPhase>(*phase) });
}

CBaseStream::CBaseStream(CBaseStream&& _other) :
	m_name{ std::move(_other.m_name) },
	m_key{ std::move(_other.m_key) },
	m_materialsDB{ _other.m_materialsDB },
	m_timePoints{ std::move(_other.m_timePoints) },
	m_grid{ std::move(_other.m_grid) },
	m_cacheSettings{ std::move(_other.m_cacheSettings) },
	m_toleranceSettings{ std::move(_other.m_toleranceSettings) },
	m_thermodynamicsSettings{ std::move(_other.m_thermodynamicsSettings) }
{
	for (auto& [type, param] : _other.m_overall)
		m_overall.insert({ type, std::move(param) });
	for (auto& [state, phase] : _other.m_phases)
		m_phases.insert({ state, std::move(phase) });
}

void CBaseStream::Clear()
{
	m_timePoints.clear();
	m_overall.clear();
	m_phases.clear();
	ClearEnthalpyCalculator();
}

void CBaseStream::SetupStructure(const CBaseStream* _other)
{
	// TODO: check that thermodynamic and tolerance settings are correctly set here (if they need to be set at all)
	Clear();
	SetMaterialsDatabasePtr(_other->m_materialsDB);
	SetCacheSettings(_other->m_cacheSettings);
	m_grid = _other->m_grid;
	for (const auto& [type, old] : _other->m_overall)
		AddOverallProperty(type, old->GetName(), old->GetUnits());
	for (const auto& [type, old] : _other->m_phases)
	{
		auto* phase = AddPhase(type, old->GetName());
		phase->MDDistr()->SetMinimalFraction(old->MDDistr()->GetMinimalFraction());
	}
}

bool CBaseStream::HaveSameOverall(const CBaseStream& _stream1, const CBaseStream& _stream2)
{
	return MapKeys(_stream1.m_overall) == MapKeys(_stream2.m_overall);
}

bool CBaseStream::HaveSamePhases(const CBaseStream& _stream1, const CBaseStream& _stream2)
{
	return MapKeys(_stream1.m_phases) == MapKeys(_stream2.m_phases);
}

bool CBaseStream::HaveSameGrids(const CBaseStream& _stream1, const CBaseStream& _stream2)
{
	return _stream1.m_grid == _stream2.m_grid;
}

bool CBaseStream::HaveSameOverallAndPhases(const CBaseStream& _stream1, const CBaseStream& _stream2)
{
	return HaveSameOverall(_stream1, _stream2) && HaveSamePhases(_stream1, _stream2);
}

bool CBaseStream::HaveSameStructure(const CBaseStream& _stream1, const CBaseStream& _stream2)
{
	return HaveSameOverall(_stream1, _stream2) && HaveSamePhases(_stream1, _stream2) && HaveSameGrids(_stream1, _stream2);
}

std::string CBaseStream::GetName() const
{
	return m_name;
}

void CBaseStream::SetName(const std::string& _name)
{
	m_name = _name;
}

std::string CBaseStream::GetKey() const
{
	return m_key;
}

void CBaseStream::SetKey(const std::string& _key)
{
	m_key = _key;
}

void CBaseStream::AddTimePoint(double _time)
{
	if (HasTime(_time)) return;
	CopyTimePoint(_time, GetPreviousTimePoint(_time));
}

void CBaseStream::CopyTimePoint(double _timeDst, double _timeSrc)
{
	if (_timeDst < 0) return;

	// insert time point
	InsertTimePoint(_timeDst);

	// copy data in overall parameters
	for (auto& [type, param] : m_overall)
		param->CopyTimePoint(_timeDst, _timeSrc);

	// copy data in phases
	for (auto& [state, phase] : m_phases)
		phase->CopyTimePoint(_timeDst, _timeSrc);
}

void CBaseStream::RemoveTimePoint(double _time)
{
	RemoveTimePoints(_time, _time);
}

void CBaseStream::RemoveTimePoints(double _timeBeg, double _timeEnd, bool _inclusive/* = true*/)
{
	if (m_timePoints.empty()) return;
	if (_timeBeg > _timeEnd) return;

	// remove time points
	const auto& beg = _inclusive ? std::lower_bound(m_timePoints.begin(), m_timePoints.end(), _timeBeg)
								 : std::upper_bound(m_timePoints.begin(), m_timePoints.end(), _timeBeg);
	const auto& end = _inclusive ? std::upper_bound(m_timePoints.begin(), m_timePoints.end(), _timeEnd)
								 : std::lower_bound(m_timePoints.begin(), m_timePoints.end(), _timeEnd);
	if (end == m_timePoints.begin() || beg == m_timePoints.end() || beg == end) return;
	m_timePoints.erase(beg, end);

	// remove data in overall parameters
	for (auto& [type, param] : m_overall)
		param->RemoveTimePoints(_timeBeg, _timeEnd, _inclusive);

	// remove data in phases
	for (auto& [state, phase] : m_phases)
		phase->RemoveTimePoints(_timeBeg, _timeEnd, _inclusive);
}

void CBaseStream::RemoveTimePointsAfter(double _time, bool _inclusive/* = false*/)
{
	const auto& beg = _inclusive ? std::lower_bound(m_timePoints.begin(), m_timePoints.end(), _time)
								 : std::upper_bound(m_timePoints.begin(), m_timePoints.end(), _time);
	if (beg == m_timePoints.end()) return;
	RemoveTimePoints(*beg, GetLastTimePoint());
}

void CBaseStream::RemoveAllTimePoints()
{
	// remove time points
	m_timePoints.clear();
	// remove data in overall parameters
	for (auto& [type, param] : m_overall)
		param->RemoveAllTimePoints();
	// remove data in phases
	for (auto& [state, phase] : m_phases)
		phase->RemoveAllTimePoints();
}

void CBaseStream::ReduceTimePoints(double _timeBeg, double _timeEnd, double _step)
{
	std::vector<double> timePoints = GetTimePoints(_timeBeg, _timeEnd);
	if (timePoints.size() <= 3) return;
	timePoints.pop_back();

	size_t iTime1 = 0;
	size_t iTime2 = 1;
	while (iTime1 < timePoints.size() && iTime2 < timePoints.size())
	{
		if (std::fabs(timePoints[iTime1] - timePoints[iTime2]) < _step)
		{
			RemoveTimePoint(timePoints[iTime2]);
			VectorDelete(timePoints, iTime2);
		}
		else
		{
			iTime1++;
			iTime2++;
		}
	}
}

std::vector<double> CBaseStream::GetAllTimePoints() const
{
	return m_timePoints;
}

std::vector<double> CBaseStream::GetTimePoints(double _timeBeg, double _timeEnd) const
{
	if (m_timePoints.empty()) return {};
	if (_timeBeg > _timeEnd) return {};

	const auto end = std::upper_bound(m_timePoints.begin(), m_timePoints.end(), _timeEnd);
	if (end == m_timePoints.begin()) return {};
	const auto beg = std::lower_bound(m_timePoints.begin(), m_timePoints.end(), _timeBeg);

	return { beg, end };
}

std::vector<double> CBaseStream::GetTimePointsClosed(double _timeBeg, double _timeEnd) const
{
	return CloseInterval(GetTimePoints(_timeBeg, _timeEnd), _timeBeg, _timeEnd);
}

double CBaseStream::GetLastTimePoint() const
{
	if (m_timePoints.empty()) return {};

	return m_timePoints.back();
}

double CBaseStream::GetPreviousTimePoint(double _time) const
{
	if (m_timePoints.empty()) return {};

	auto pos = std::lower_bound(m_timePoints.begin(), m_timePoints.end(), _time);
	if (pos == m_timePoints.begin()) return {};
	return *--pos;
}

CTimeDependentValue* CBaseStream::AddOverallProperty(EOverall _property, const std::string& _name, const std::string& _units)
{
	// TODO: add cache settings to constructor here when caching is implemented for CTimeDependentValue
	auto [it, flag] = m_overall.insert({ _property, std::make_unique<CTimeDependentValue>(_name, _units) });

	if (flag) // a new property added
	{
		// add time points
		// TODO: maybe remove this
		for (const auto& t : m_timePoints)
			it->second->AddTimePoint(t);
	}

	return it->second.get();
}

void CBaseStream::RemoveOverallProperty(EOverall _property)
{
	m_overall.erase(_property);
}

CTimeDependentValue* CBaseStream::GetOverallProperty(EOverall _property)
{
	if (!HasOverallProperty(_property)) return nullptr;

	return m_overall[_property].get();
}

const CTimeDependentValue* CBaseStream::GetOverallProperty(EOverall _property) const
{
	if (!HasOverallProperty(_property)) return nullptr;

	return m_overall.at(_property).get();
}

double CBaseStream::GetOverallProperty(double _time, EOverall _property) const
{
	if (!HasOverallProperty(_property))
	{
		if (_property == EOverall::OVERALL_TEMPERATURE)	return STANDARD_CONDITION_T;
		if (_property == EOverall::OVERALL_PRESSURE)	return STANDARD_CONDITION_P;
		return {};
	}

	return m_overall.at(_property)->GetValue(_time);
}

double CBaseStream::GetMass(double _time) const
{
	return GetOverallProperty(_time, EOverall::OVERALL_MASS);
}

double CBaseStream::GetMol(double _time) const
{
	double res = 0.0;
	for (const auto& [state, phase] : m_phases)
		res += GetPhaseMol(_time, state);
	return res;
}

void CBaseStream::SetMol(double _time, double _value)
{
	SetMass(_time, GetMass(_time) * _value / GetMol(_time));
}

double CBaseStream::GetTemperature(double _time) const
{
	return GetOverallProperty(_time, EOverall::OVERALL_TEMPERATURE);
}

double CBaseStream::GetPressure(double _time) const
{
	return GetOverallProperty(_time, EOverall::OVERALL_PRESSURE);
}

void CBaseStream::SetOverallProperty(double _time, EOverall _property, double _value)
{
	if (!HasOverallProperty(_property)) return;

	AddTimePoint(_time);
	m_overall[_property]->SetValue(_time, _value);
}

void CBaseStream::SetMass(double _time, double _value)
{
	SetOverallProperty(_time, EOverall::OVERALL_MASS, _value);
}

void CBaseStream::SetTemperature(double _time, double _value)
{
	SetOverallProperty(_time, EOverall::OVERALL_TEMPERATURE, _value);
}

void CBaseStream::SetPressure(double _time, double _value)
{
	SetOverallProperty(_time, EOverall::OVERALL_PRESSURE, _value);
}

void CBaseStream::AddCompound(const std::string& _compoundKey)
{
	if (HasCompound(_compoundKey)) return;

	// update phases
	for (auto& [state, phase] : m_phases)
		phase->AddCompound(_compoundKey);

	// add to the grid
	m_grid.GetGridDimensionSymbolic(DISTR_COMPOUNDS)->AddClass(_compoundKey);

	// invalidate enthalpy calculator
	ClearEnthalpyCalculator();
}

void CBaseStream::RemoveCompound(const std::string& _compoundKey)
{
	if (!HasCompound(_compoundKey)) return;

	// update phases
	for (auto& [state, phase] : m_phases)
		phase->RemoveCompound(_compoundKey);

	// remove from the grid
	m_grid.GetGridDimensionSymbolic(DISTR_COMPOUNDS)->RemoveClass(_compoundKey);

	// invalidate enthalpy calculator
	ClearEnthalpyCalculator();
}

std::vector<std::string> CBaseStream::GetAllCompounds() const
{
	return m_grid.GetSymbolicGrid(DISTR_COMPOUNDS);
}

double CBaseStream::GetCompoundFraction(double _time, const std::string& _compoundKey) const
{
	if (!HasCompound(_compoundKey)) return {};

	const size_t iCompound = CompoundIndex(_compoundKey);
	double res = 0.0;
	for (const auto& [state, phase] : m_phases)
		res += phase->GetFraction(_time) * phase->GetCompoundFraction(_time, iCompound);
	return res;
}

double CBaseStream::GetCompoundFraction(double _time, const std::string& _compoundKey, EPhase _phase) const
{
	if (!HasCompound(_compoundKey) || !HasPhase(_phase)) return {};

	return m_phases.at(_phase)->GetCompoundFraction(_time, CompoundIndex(_compoundKey));
}

double CBaseStream::GetCompoundMass(double _time, const std::string& _compoundKey) const
{
	if (!HasCompound(_compoundKey)) return {};

	return GetCompoundFraction(_time, _compoundKey) * GetMass(_time);
}

double CBaseStream::GetCompoundMass(double _time, const std::string& _compoundKey, EPhase _phase) const
{
	if (!HasCompound(_compoundKey) || !HasPhase(_phase)) return {};

	return m_phases.at(_phase)->GetFraction(_time)										// mass fraction of phase
		* m_phases.at(_phase)->GetCompoundFraction(_time, CompoundIndex(_compoundKey))	// mass fraction of compound in phase
		* GetMass(_time);																// whole mass
}

std::vector<double> CBaseStream::GetCompoundsFractions(double _time) const
{
	const auto& compounds = GetAllCompounds();
	std::vector<double> res(compounds.size());
	for (size_t i = 0; i < compounds.size(); ++i)
		res[i] = GetCompoundFraction(_time, compounds[i]);
	return res;
}

std::vector<double> CBaseStream::GetCompoundsFractions(double _time, EPhase _phase) const
{
	if (!HasPhase(_phase)) return {};

	return m_phases.at(_phase)->GetCompoundsDistribution(_time);
}

std::vector<double> CBaseStream::GetCompoundsMasses(double _time) const
{
	return MultVector(GetCompoundsFractions(_time), GetMass(_time));
}

std::vector<double> CBaseStream::GetCompoundsMasses(double _time, EPhase _phase) const
{
	return MultVector(GetCompoundsFractions(_time, _phase), GetPhaseMass(_time, _phase));
}

void CBaseStream::SetCompoundFraction(double _time, const std::string& _compoundKey, EPhase _phase, double _value)
{
	if (!HasCompound(_compoundKey) || !HasPhase(_phase)) return;

	AddTimePoint(_time);
	m_phases[_phase]->SetCompoundFraction(_time, CompoundIndex(_compoundKey), _value);
}

void CBaseStream::SetCompoundsFractions(double _time, const std::vector<double>& _value)
{
	AddTimePoint(_time);
	for (const auto& [state, phase] : m_phases)
		phase->SetCompoundsDistribution(_time, _value);
}

void CBaseStream::SetCompoundsFractions(double _time, EPhase _phase, const std::vector<double>& _value)
{
	if (!HasPhase(_phase)) return;

	AddTimePoint(_time);
	m_phases[_phase]->SetCompoundsDistribution(_time, _value);
}

double CBaseStream::GetCompoundMolFraction(double _time, const std::string& _compoundKey, EPhase _phase) const
{
	if (!HasCompound(_compoundKey) || !HasPhase(_phase)) return {};

	return GetCompoundFraction(_time, _compoundKey, _phase) *
		GetPhaseProperty(_time, _phase, ECompoundConstProperties::MOLAR_MASS) /
		GetCompoundProperty(_compoundKey, ECompoundConstProperties::MOLAR_MASS);
}

double CBaseStream::GetCompoundMol(double _time, const std::string& _compoundKey, EPhase _phase) const
{
	return GetCompoundMass(_time, _compoundKey, _phase) / GetCompoundProperty(_compoundKey, MOLAR_MASS);
}

void CBaseStream::SetCompoundMolFraction(double _time, const std::string& _compoundKey, EPhase _phase, double _value)
{
	const double m = GetPhaseMass(_time, _phase);
	const double M = GetCompoundProperty(_compoundKey, MOLAR_MASS);
	const double n = GetPhaseMol(_time, _phase);
	SetCompoundFraction(_time, _compoundKey, _phase, m != 0.0 ? _value * M * n / m : 0.0);
}

CPhase* CBaseStream::AddPhase(EPhase _phase, const std::string& _name)
{
	// add phase
	auto [it, flag] = m_phases.insert({ _phase, std::make_unique<CPhase>(_phase, _name, m_grid, m_cacheSettings) });

	if (flag) // a new phase added
	{
		// add time points
		// TODO: maybe remove this
		for (const auto& t : m_timePoints)
			it->second->AddTimePoint(t);
	}

	return it->second.get();
}

void CBaseStream::RemovePhase(EPhase _phase)
{
	if (!HasPhase(_phase)) return;

	m_phases.erase(_phase);
}

CPhase* CBaseStream::GetPhase(EPhase _phase)
{
	if (!HasPhase(_phase)) return {};

	return m_phases[_phase].get();
}

const CPhase* CBaseStream::GetPhase(EPhase _phase) const
{
	if (!HasPhase(_phase)) return {};

	return m_phases.at(_phase).get();
}

void CBaseStream::ClearPhases()
{
	for (const auto& p : MapKeys(m_phases))
		RemovePhase(p);
}

double CBaseStream::GetPhaseFraction(double _time, EPhase _phase) const
{
	if (!HasPhase(_phase)) return {};

	return m_phases.at(_phase)->GetFraction(_time);
}

double CBaseStream::GetPhaseMass(double _time, EPhase _phase) const
{
	if (!HasPhase(_phase)) return {};

	return m_phases.at(_phase)->GetFraction(_time) * GetMass(_time);
}

double CBaseStream::GetPhaseProperty(double _time, EPhase _phase, EOverall _property) const
{
	if (!HasPhase(_phase)) return {};

	if (_property == EOverall::OVERALL_MASS)
		return GetPhaseMass(_time, _phase);
	return GetOverallProperty(_time, _property);
}

double CBaseStream::GetPhaseProperty(double _time, EPhase _phase, ECompoundConstProperties _property) const
{
	if (!HasPhase(_phase)) return {};

	// TODO: implement other properties if needed
	if (_property == MOLAR_MASS)
	{
		double res = 0.0;
		for (const auto& c : GetAllCompounds())
		{
			const double molarMass = GetCompoundProperty(c, MOLAR_MASS);
			if (molarMass != 0.0)
				res += GetCompoundFraction(_time, c, _phase) / molarMass;
		}
		if (res != 0.0)
			return 1.0 / res;
	}
	else
	{
		double res{ 0.0 };
		for (const auto& c : GetAllCompounds())
			res += GetCompoundFraction(_time, c, _phase) * GetCompoundProperty(c, _property);
		return res;
	}

	return {};
}

double CBaseStream::GetPhaseProperty(double _time, EPhase _phase, ECompoundTPProperties _property) const
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
		for (const auto& c : GetAllCompounds())
			pressures.push_back(m_materialsDB->GetTPPropertyValue(c, _property, T, P));
		return VectorMin(pressures);
	}
	case VISCOSITY:
		switch (_phase)
		{
		case EPhase::LIQUID:
			for (const auto& c : GetAllCompounds())
			{
				const double visco = m_materialsDB->GetTPPropertyValue(c, _property, T, P);
				if (visco > 0.0)
					res += GetCompoundFraction(_time, c, _phase) * std::log(visco);
			}
			if (res != 0.0)
				return std::exp(res);
			break;
		case EPhase::VAPOR:
		{
			double numerator = 0.0, denominator = 0.0;
			for (const auto& c : GetAllCompounds())
			{
				const double visco = m_materialsDB->GetTPPropertyValue(c, _property, T, P);
				const double mollMass = GetCompoundProperty(c, MOLAR_MASS);
				const double mollFrac = GetCompoundMolFraction(_time, c, _phase);
				numerator += mollFrac * visco * std::sqrt(mollMass);
				denominator += mollFrac * std::sqrt(mollMass);
			}
			if (denominator != 0.0)
				return numerator / denominator;
			break;
		}
		case EPhase::SOLID:
			for (const auto& c : GetAllCompounds())
				res += GetCompoundFraction(_time, c, _phase) * m_materialsDB->GetTPPropertyValue(c, _property, T, P);
			return res;
		case EPhase::UNDEFINED: return {};
		}
		break;
	case THERMAL_CONDUCTIVITY:
		switch (_phase)
		{
		case EPhase::LIQUID:
			for (const auto& c : GetAllCompounds())
				if (const auto conduct = m_materialsDB->GetTPPropertyValue(c, _property, T, P); conduct != 0.0)
					res += GetCompoundMolFraction(_time, c, _phase) / std::pow(conduct, 2.0);
			if (res != 0.0)
				return 1.0 / std::sqrt(res);
			break;
		case EPhase::VAPOR:
			for (const auto& c1 : GetAllCompounds())
			{
				const double conduct1 = m_materialsDB->GetTPPropertyValue(c1, _property, T, P);
				const double mollMass1 = GetCompoundProperty(c1, MOLAR_MASS);
				const double numerator = GetCompoundMolFraction(_time, c1, _phase) * conduct1;
				double denominator = 0.0;
				for (const auto& c2 : GetAllCompounds())
				{
					const double conduct2 = m_materialsDB->GetTPPropertyValue(c2, _property, T, P);
					const double mollMass2 = GetCompoundProperty(c2, MOLAR_MASS);
					if (mollMass1 != 0.0 && mollMass2 != 0.0 && conduct2 != 0.0)
						denominator += GetCompoundMolFraction(_time, c2, _phase) * std::pow(1 + std::sqrt(conduct1 / conduct2) * std::pow(mollMass2 / mollMass1, 1. / 4.), 2) / std::sqrt(8 * (1 + mollMass1 / mollMass2));
				}
				if (denominator != 0.0)
					res += numerator / denominator;
			}
			return res;
		case EPhase::SOLID:
			for (const auto& c : GetAllCompounds())
				res += GetCompoundFraction(_time, c, _phase) * m_materialsDB->GetTPPropertyValue(c, _property, T, P);
			return res;
		case EPhase::UNDEFINED: return {};
		}
		break;
	case DENSITY:
		if (_phase == EPhase::SOLID && m_grid.HasDimension(DISTR_PART_POROSITY))
		{
			const auto& compounds = GetAllCompounds();
			CMatrix2D distr = m_phases.at(_phase)->MDDistr()->GetDistribution(_time, DISTR_COMPOUNDS, DISTR_PART_POROSITY);
			const size_t nCompounds = compounds.size();
			const size_t nPorosities = m_grid.GetGridDimension(DISTR_PART_POROSITY)->ClassesNumber();
			const std::vector<double> porosities = m_grid.GetGridDimensionNumeric(DISTR_PART_POROSITY)->GetClassesMeans();
			for (size_t iCompound = 0; iCompound < nCompounds; ++iCompound)
			{
				const double density = GetCompoundProperty(_time, compounds[iCompound], DENSITY);
				for (size_t iPoros = 0; iPoros < nPorosities; ++iPoros)
					res += density * (1 - porosities[iPoros]) * distr[iCompound][iPoros];
			}
			return res;
		}
		else
		{
			for (const auto& c : GetAllCompounds())
			{
				const double componentDensity = m_materialsDB->GetTPPropertyValue(c, _property, T, P);
				if (componentDensity != 0.0)
					res += GetCompoundFraction(_time, c, _phase) / componentDensity;
			}
			if (res != 0.0)
				return 1.0 / res;
		}
		break;
	case HEAT_CAPACITY_CP:
	case ENTHALPY:
	case PERMITTIVITY:
	case EQUILIBRIUM_MOISTURE_CONTENT:
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
		for (const auto& c : GetAllCompounds())
			res += GetCompoundFraction(_time, c, _phase) * m_materialsDB->GetTPPropertyValue(c, _property, T, P);
		return res;
	case TP_PROP_NO_PROERTY: break;
	}

	return {};
}

void CBaseStream::SetPhaseFraction(double _time, EPhase _phase, double _value)
{
	if (!HasPhase(_phase)) return;

	AddTimePoint(_time);
	m_phases[_phase]->SetFraction(_time, _value);
}

void CBaseStream::SetPhaseMass(double _time, EPhase _phase, double _value)
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

double CBaseStream::GetPhaseMolFraction(double _time, EPhase _phase) const
{
	if (!HasPhase(_phase)) return {};

	return GetPhaseMol(_time, _phase) / GetMol(_time);
}

double CBaseStream::GetPhaseMol(double _time, EPhase _phase) const
{
	if (!HasPhase(_phase)) return {};

	double res = 0.0;
	for (const auto& c : GetAllCompounds())
		res += GetCompoundMol(_time, c, _phase);

	return res;
}

void CBaseStream::SetPhaseMolFraction(double _time, EPhase _phase, double _value)
{
	SetPhaseFraction(_time, _phase, _value * GetPhaseFraction(_time, _phase) / GetPhaseMolFraction(_time, _phase));
}

void CBaseStream::SetPhaseMol(double _time, EPhase _phase, double _value)
{
	SetPhaseMass(_time, _phase, _value * GetPhaseMass(_time, _phase) / GetPhaseMol(_time, _phase));
}

double CBaseStream::GetMixtureProperty(double _time, EOverall _property) const
{
	return GetOverallProperty(_time, _property);
}

double CBaseStream::GetMixtureProperty(double _time, ECompoundConstProperties _property) const
{
	double res = 0.0;
	for (const auto& [state, phase] : m_phases)
		res += GetPhaseProperty(_time, state, _property) * phase->GetFraction(_time);
	return res;
}

double CBaseStream::GetMixtureProperty(double _time, ECompoundTPProperties _property) const
{
	double res = 0.0;
	for (const auto& [state, phase] : m_phases)
		res += GetPhaseProperty(_time, state, _property) * phase->GetFraction(_time);
	return res;
}

void CBaseStream::SetMixtureProperty(double _time, EOverall _property, double _value)
{
	SetOverallProperty(_time, _property, _value);
}

double CBaseStream::GetCompoundProperty(const std::string& _compoundKey, ECompoundConstProperties _property) const
{
	if (!m_materialsDB) return {};

	return m_materialsDB->GetConstPropertyValue(_compoundKey, _property);
}

double CBaseStream::GetCompoundProperty(const std::string& _compoundKey, ECompoundTPProperties _property, double _temperature, double _pressure) const
{
	if (!m_materialsDB) return {};

	return m_materialsDB->GetTPPropertyValue(_compoundKey, _property, _temperature, _pressure);
}

double CBaseStream::GetCompoundProperty(double _time, const std::string& _compoundKey, ECompoundTPProperties _property) const
{
	return GetCompoundProperty(_compoundKey, _property, GetTemperature(_time), GetPressure(_time));
}

double CBaseStream::GetCompoundProperty(const std::string& _compoundKey1, const std::string& _compoundKey2, EInteractionProperties _property, double _temperature, double _pressure) const
{
	if (!m_materialsDB) return {};

	return m_materialsDB->GetInteractionPropertyValue(_compoundKey1, _compoundKey2, _property, _temperature, _pressure);
}

double CBaseStream::GetCompoundProperty(double _time, const std::string& _compoundKey1, const std::string& _compoundKey2, EInteractionProperties _property) const
{
	return GetCompoundProperty(_compoundKey1, _compoundKey2, _property, GetTemperature(_time), GetPressure(_time));
}

double CBaseStream::GetFraction(double _time, const std::vector<size_t>& _coords) const
{
	if (!HasPhase(EPhase::SOLID)) return {};

	return m_phases.at(EPhase::SOLID)->MDDistr()->GetValue(_time, vector_cast<unsigned>(_coords));
}

void CBaseStream::SetFraction(double _time, const std::vector<size_t>& _coords, double _value)
{
	if (!HasPhase(EPhase::SOLID)) return;

	// TODO: remove when it is not needed by MD matrix
	AddTimePoint(_time);
	m_phases.at(EPhase::SOLID)->MDDistr()->SetValue(_time, vector_cast<unsigned>(_coords), _value);
}

std::vector<double> CBaseStream::GetDistribution(double _time, EDistrTypes _distribution) const
{
	if (!HasPhase(EPhase::SOLID)) return {};

	return m_phases.at(EPhase::SOLID)->MDDistr()->GetDistribution(_time, _distribution);
}

CMatrix2D CBaseStream::GetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2) const
{
	if (!HasPhase(EPhase::SOLID)) return {};

	return m_phases.at(EPhase::SOLID)->MDDistr()->GetDistribution(_time, _distribution1, _distribution2);
}

CDenseMDMatrix CBaseStream::GetDistribution(double _time, const std::vector<EDistrTypes>& _distributions) const
{
	if (!HasPhase(EPhase::SOLID)) return {};

	return m_phases.at(EPhase::SOLID)->MDDistr()->GetDistribution(_time, vector_cast<unsigned>(_distributions));
}

std::vector<double> CBaseStream::GetDistribution(double _time, EDistrTypes _distribution, const std::string& _compoundKey) const
{
	if (!HasPhase(EPhase::SOLID)) return {};
	if (!HasCompound(_compoundKey)) return {};
	if (_distribution == DISTR_COMPOUNDS) return {};

	return Normalized(m_phases.at(EPhase::SOLID)->MDDistr()->GetVectorValue(_time, DISTR_COMPOUNDS, static_cast<unsigned>(CompoundIndex(_compoundKey)), _distribution));
}

CMatrix2D CBaseStream::GetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2, const std::string& _compoundKey) const
{
	if (!HasPhase(EPhase::SOLID)) return {};
	if (!HasCompound(_compoundKey)) return {};
	if (_distribution1 == DISTR_COMPOUNDS || _distribution2 == DISTR_COMPOUNDS) return {};

	// prepare coordinates
	const size_t size1 = m_grid.GetGridDimension(_distribution1)->ClassesNumber();
	const size_t size2 = m_grid.GetGridDimension(_distribution2)->ClassesNumber();
	const std::vector<EDistrTypes> dims{ DISTR_COMPOUNDS, _distribution1, _distribution2 };
	std::vector<size_t> coords{ CompoundIndex(_compoundKey), 0 };
	CMatrix2D res{ size1, size2 };

	// gather data
	for (size_t i = 0; i < size1; ++i)
	{
		res.SetRow(i, m_phases.at(EPhase::SOLID)->MDDistr()->GetVectorValue(_time, vector_cast<unsigned>(dims), vector_cast<unsigned>(coords)));
		coords.back()++;
	}
	res.Normalize();

	return res;
}

CDenseMDMatrix CBaseStream::GetDistribution(double _time, const std::vector<EDistrTypes>& _distributions, const std::string& _compoundKey) const
{
	if (!HasPhase(EPhase::SOLID)) return {};
	if (!HasCompound(_compoundKey)) return {};
	if (VectorContains(_distributions, DISTR_COMPOUNDS)) return {};

	// prepare common parameters
	std::vector<size_t> sizes(_distributions.size());
	for (size_t i = 0; i < _distributions.size(); ++i)
		sizes[i] = m_grid.GetGridDimension(_distributions[i])->ClassesNumber();
	CDenseMDMatrix res(vector_cast<unsigned>(_distributions), vector_cast<unsigned>(sizes));

	// prepare parameters for reading
	std::vector<EDistrTypes> getDims = _distributions;
	getDims.insert(getDims.begin(), DISTR_COMPOUNDS);
	std::vector<size_t> getCoords(_distributions.size());
	getCoords.front() = CompoundIndex(_compoundKey);
	std::vector<size_t> getSizes = sizes;
	getSizes.insert(getSizes.begin(), m_grid.GetGridDimension(DISTR_COMPOUNDS)->ClassesNumber());
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
		std::vector<double> vec = m_phases.at(EPhase::SOLID)->MDDistr()->GetVectorValue(_time, vector_cast<unsigned>(getDims), vector_cast<unsigned>(getCoords));
		res.SetVectorValue(vector_cast<unsigned>(setDims), vector_cast<unsigned>(setCoords), vec);
		IncrementCoords(getCoords, getSizes);
		notFinished = IncrementCoords(setCoords, setSizes);
	} while (notFinished);
	res.Normalize();

	return res;
}

void CBaseStream::SetDistribution(double _time, EDistrTypes _distribution, const std::vector<double>& _value)
{
	if (!HasPhase(EPhase::SOLID)) return;

	AddTimePoint(_time);
	m_phases[EPhase::SOLID]->MDDistr()->SetDistribution(_time, _distribution, _value);
}

void CBaseStream::SetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2, const CMatrix2D& _value)
{
	if (!HasPhase(EPhase::SOLID)) return;

	// TODO: remove when it is not needed by MD matrix
	AddTimePoint(_time);
	m_phases[EPhase::SOLID]->MDDistr()->SetDistribution(_time, _distribution1, _distribution2, _value);
}

void CBaseStream::SetDistribution(double _time, const CDenseMDMatrix& _value)
{
	if (!HasPhase(EPhase::SOLID)) return;

	// TODO: remove when it is not needed by MD matrix
	AddTimePoint(_time);
	m_phases[EPhase::SOLID]->MDDistr()->SetDistribution(_time, _value);
}

void CBaseStream::SetDistribution(double _time, EDistrTypes _distribution, const std::string& _compoundKey, const std::vector<double>& _value)
{
	if (!HasPhase(EPhase::SOLID)) return;
	if (!HasCompound(_compoundKey)) return;
	if (_distribution == DISTR_COMPOUNDS) return;

	// TODO: remove when it is not needed by MD matrix
	AddTimePoint(_time);
	m_phases[EPhase::SOLID]->MDDistr()->SetVectorValue(_time, DISTR_COMPOUNDS, static_cast<unsigned>(CompoundIndex(_compoundKey)), _distribution, _value, true);
	m_phases[EPhase::SOLID]->MDDistr()->NormalizeMatrix(_time);
}

void CBaseStream::SetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2, const std::string& _compoundKey, const CMatrix2D& _value)
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
		inSizes[i] = m_grid.GetGridDimension(inDims[i])->ClassesNumber();

	// prepare parameters for reading from input 2D-distribution
	const size_t size1 = m_grid.GetGridDimension(_distribution1)->ClassesNumber();

	// prepare parameters for writing to output MD-distribution
	std::vector<EDistrTypes> setDims = inDims;
	setDims.insert(setDims.begin(), DISTR_COMPOUNDS);
	std::vector<size_t> setCoords(inDims.size());
	setCoords.front() = CompoundIndex(_compoundKey);
	std::vector<size_t> setSizes = inSizes;
	setSizes.insert(setSizes.begin(), m_grid.GetGridDimension(DISTR_COMPOUNDS)->ClassesNumber());

	// get old distribution with compounds
	std::vector<EDistrTypes> fullDims = inDims;
	fullDims.insert(fullDims.begin(), DISTR_COMPOUNDS);
	CDenseMDMatrix matrixMD = m_phases[EPhase::SOLID]->MDDistr()->GetDistribution(_time, vector_cast<unsigned>(fullDims));

	// set new values from 2D-distribution to MD-distribution
	for (size_t i = 0; i < size1; ++i)
	{
		matrixMD.SetVectorValue(vector_cast<unsigned>(setDims), vector_cast<unsigned>(setCoords), _value.GetRow(i));
		IncrementCoords(setCoords, setSizes);
	}

	// set output MD-distribution into main distribution
	matrixMD.Normalize();
	m_phases[EPhase::SOLID]->MDDistr()->SetDistribution(_time, matrixMD);
}

void CBaseStream::SetDistribution(double _time, const std::string& _compoundKey, const CDenseMDMatrix& _value)
{
	if (!HasPhase(EPhase::SOLID)) return;
	if (!HasCompound(_compoundKey)) return;
	// TODO: remove cast
	if (VectorContains(_value.GetDimensions(), E2I(DISTR_COMPOUNDS))) return;

	// TODO: remove when it is not needed by MD matrix
	AddTimePoint(_time);

	// prepare common parameters
	// TODO: remove cast
	std::vector<EDistrTypes> inDims = vector_cast<EDistrTypes>(_value.GetDimensions());
	std::vector<size_t> inSizes(inDims.size());
	for (size_t i = 0; i < inDims.size(); ++i)
		inSizes[i] = m_grid.GetGridDimension(inDims[i])->ClassesNumber();

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
	setSizes.insert(setSizes.begin(), m_grid.GetGridDimension(DISTR_COMPOUNDS)->ClassesNumber());

	// get old distribution with compounds
	std::vector<EDistrTypes> fullDims = inDims;
	fullDims.insert(fullDims.begin(), DISTR_COMPOUNDS);
	CDenseMDMatrix matrixMD = m_phases[EPhase::SOLID]->MDDistr()->GetDistribution(_time, vector_cast<unsigned>(fullDims));

	// set new values from input MD-distribution to output MD-distribution
	bool notFinished;
	do
	{
		matrixMD.SetVectorValue(vector_cast<unsigned>(setDims), vector_cast<unsigned>(setCoords), _value.GetVectorValue(vector_cast<unsigned>(getDims), vector_cast<unsigned>(getCoords)));
		IncrementCoords(setCoords, setSizes);
		notFinished = IncrementCoords(getCoords, getSizes);
	} while (notFinished);

	// set output MD-distribution into main distribution
	matrixMD.Normalize();
	m_phases[EPhase::SOLID]->MDDistr()->SetDistribution(_time, matrixMD);
}

void CBaseStream::ApplyTM(double _time, const CTransformMatrix& _matrix)
{
	if (!HasPhase(EPhase::SOLID)) return;

	m_phases[EPhase::SOLID]->MDDistr()->Transform(_time, _matrix);
	m_phases[EPhase::SOLID]->MDDistr()->NormalizeMatrix(_time);
}

void CBaseStream::ApplyTM(double _time, const std::string& _compoundKey, const CTransformMatrix& _matrix)
{
	if (!HasPhase(EPhase::SOLID)) return;
	if (!HasCompound(_compoundKey)) return;
	if (_matrix.GetDimensionsNumber() == 0) return;
	// TODO: remove cast
	if (VectorContains(_matrix.GetDimensions(), E2I(DISTR_COMPOUNDS))) return;

	// prepare all parameters
	const size_t iCompound = CompoundIndex(_compoundKey);
	// TODO: remove cast
	std::vector<EDistrTypes> inDims = vector_cast<EDistrTypes>(_matrix.GetDimensions());
	std::vector<EDistrTypes> newDims = inDims;
	newDims.insert(newDims.begin(), DISTR_COMPOUNDS);
	// TODO: remove cast
	std::vector<size_t> oldSizes = vector_cast<size_t>(_matrix.GetClasses());
	std::vector<size_t> oldSizesFull = oldSizes;
	oldSizesFull.insert(oldSizesFull.end(), oldSizes.begin(), oldSizes.end() - 1);
	std::vector<size_t> newSizes = oldSizes;
	newSizes.insert(newSizes.begin(), m_grid.GetGridDimension(DISTR_COMPOUNDS)->ClassesNumber());
	std::vector<size_t> oldCoordsFull(oldSizes.size() * 2 - 1);
	std::vector<size_t> newCoords(newSizes.size());
	newCoords.front() = iCompound;

	// create new transformation matrix
	CTransformMatrix newTM(vector_cast<unsigned>(newDims), vector_cast<unsigned>(newSizes));

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
		std::copy(oldCoordsFull.begin() + static_cast<std::vector<size_t>::difference_type>(oldSrcSize), oldCoordsFull.end(), oldDstCoords.begin());
		std::copy_n(oldCoordsFull.begin(), oldSrcSize, newSrcCoords.begin() + 1);
		std::copy(oldCoordsFull.begin() + static_cast<std::vector<size_t>::difference_type>(oldSrcSize), oldCoordsFull.end(), newDstCoords.begin() + 1);
		std::vector<double> vec = _matrix.GetVectorValue(vector_cast<unsigned>(oldSrcCoords), vector_cast<unsigned>(oldDstCoords));
		newTM.SetVectorValue(vector_cast<unsigned>(newSrcCoords), vector_cast<unsigned>(newDstCoords), vec);
		notFinished = IncrementCoords(oldCoordsFull, oldSizesFull);
	} while (notFinished);

	// set values for other compounds
	std::fill(newCoords.begin(), newCoords.end(), 0);
	notFinished = true;
	do
	{
		if (newCoords.front() != iCompound)
			newTM.SetValue(vector_cast<unsigned>(newCoords), vector_cast<unsigned>(newCoords), 1);
		notFinished = IncrementCoords(newCoords, newSizes);
	} while (notFinished);

	// transform
	m_phases[EPhase::SOLID]->MDDistr()->Transform(_time, newTM);
	m_phases[EPhase::SOLID]->MDDistr()->NormalizeMatrix(_time);
}

void CBaseStream::Normalize(double _time)
{
	if (!HasPhase(EPhase::SOLID)) return;

	return m_phases[EPhase::SOLID]->MDDistr()->NormalizeMatrix(_time);
}

void CBaseStream::Normalize(double _timeBeg, double _timeEnd)
{
	if (!HasPhase(EPhase::SOLID)) return;

	return m_phases[EPhase::SOLID]->MDDistr()->NormalizeMatrix(_timeBeg, _timeEnd);
}

void CBaseStream::Normalize()
{
	if (!HasPhase(EPhase::SOLID)) return;

	return m_phases[EPhase::SOLID]->MDDistr()->NormalizeMatrix();
}

std::vector<double> CBaseStream::GetPSD(double _time, EPSDTypes _type, EPSDGridType _grid) const
{
	return GetPSD(_time, _type, std::vector<std::string>{}, _grid);
}

std::vector<double> CBaseStream::GetPSD(double _time, EPSDTypes _type, const std::string& _compoundKey, EPSDGridType _grid) const
{
	return GetPSD(_time, _type, std::vector<std::string>(1, _compoundKey), _grid);
}

std::vector<double> CBaseStream::GetPSD(double _time, EPSDTypes _type, const std::vector<std::string>& _compoundKeys, EPSDGridType _grid) const
{
	if (!HasPhase(EPhase::SOLID)) return {};
	if (!m_grid.HasDimension(DISTR_SIZE)) return {};
	if (!HasCompounds(_compoundKeys)) return {};

	switch (_type)
	{
	case PSD_MassFrac:	return GetPSDMassFraction(_time, _compoundKeys);
	case PSD_Number:	return GetPSDNumber(_time, _compoundKeys, _grid);
	case PSD_q3:		return ConvertMassFractionsToq3(m_grid.GetPSDGrid(_grid), GetPSDMassFraction(_time, _compoundKeys));
	case PSD_Q3:		return ConvertMassFractionsToQ3(GetPSDMassFraction(_time, _compoundKeys));
	case PSD_q0:		return ConvertNumbersToq0(m_grid.GetPSDGrid(_grid), GetPSDNumber(_time, _compoundKeys, _grid));
	case PSD_Q0:		return ConvertNumbersToQ0(m_grid.GetPSDGrid(_grid), GetPSDNumber(_time, _compoundKeys, _grid));
	case PSD_q2:		return ConvertNumbersToq2(m_grid.GetPSDGrid(_grid), GetPSDNumber(_time, _compoundKeys, _grid));
	case PSD_Q2:		return ConvertNumbersToQ2(m_grid.GetPSDGrid(_grid), GetPSDNumber(_time, _compoundKeys, _grid));
	}

	return {};
}

void CBaseStream::SetPSD(double _time, EPSDTypes _type, const std::vector<double>& _value, EPSDGridType _grid)
{
	SetPSD(_time, _type, "", _value, _grid);
}

void CBaseStream::SetPSD(double _time, EPSDTypes _type, const std::string& _compoundKey, const std::vector<double>& _value, EPSDGridType _grid)
{
	if (!HasPhase(EPhase::SOLID)) return;
	if (!m_grid.HasDimension(DISTR_SIZE)) return;

	AddTimePoint(_time);

	// construct distribution
	std::vector<double> distr;
	switch (_type)
	{
	case PSD_MassFrac:	distr = _value;																break;
	case PSD_Number:	distr = ConvertNumbersToMassFractions(m_grid.GetPSDGrid(_grid), _value);	break;
	case PSD_q3:		distr = Convertq3ToMassFractions(m_grid.GetPSDGrid(_grid), _value);			break;
	case PSD_Q3:		distr = ConvertQ3ToMassFractions(_value);									break;
	case PSD_q0:		distr = Convertq0ToMassFractions(m_grid.GetPSDGrid(_grid), _value);			break;
	case PSD_Q0:		distr = ConvertQ0ToMassFractions(m_grid.GetPSDGrid(_grid), _value);			break;
	case PSD_q2:		distr = Convertq2ToMassFractions(m_grid.GetPSDGrid(_grid), _value);			break;
	case PSD_Q2:		distr = ConvertQ2ToMassFractions(m_grid.GetPSDGrid(_grid), _value);			break;
	}
	::Normalize(distr);

	// set distribution
	if (_compoundKey.empty())			// for the total mixture
		m_phases[EPhase::SOLID]->MDDistr()->SetDistribution(_time, DISTR_SIZE, distr);
	else if (HasCompound(_compoundKey))	// for specific compound
		m_phases[EPhase::SOLID]->MDDistr()->SetVectorValue(_time, DISTR_COMPOUNDS, static_cast<unsigned>(CompoundIndex(_compoundKey)), DISTR_SIZE, distr, true);
	m_phases[EPhase::SOLID]->MDDistr()->NormalizeMatrix(_time);
}

void CBaseStream::Copy(double _time, const CBaseStream& _source)
{
	if (!HaveSameOverallAndPhases(*this, _source)) return;

	// remove redundant time points
	RemoveTimePointsAfter(_time);

	// insert time point
	InsertTimePoint(_time);

	// copy data in overall parameters
	for (auto& [type, param] : m_overall)
		param->CopyFrom(_time, *_source.m_overall.at(type));

	// copy data in phases
	for (auto& [key, param] : m_phases)
		param->CopyFrom(_time, *_source.m_phases.at(key));
}

void CBaseStream::Copy(double _timeBeg, double _timeEnd, const CBaseStream& _source)
{
	if (!HaveSameOverallAndPhases(*this, _source)) return;

	// remove redundant time points
	RemoveTimePointsAfter(_timeBeg, true);

	// insert time points
	for (double t : _source.GetTimePoints(_timeBeg, _timeEnd))
		InsertTimePoint(t);

	// copy data in overall parameters
	for (auto& [type, param] : m_overall)
		param->CopyFrom(_timeBeg, _timeEnd, *_source.m_overall.at(type));

	// copy data in phases
	for (auto& [key, param] : m_phases)
		param->CopyFrom(_timeBeg, _timeEnd, *_source.m_phases.at(key));
}

void CBaseStream::Copy(double _timeDst, const CBaseStream& _source, double _timeSrc)
{
	if (!HaveSameOverallAndPhases(*this, _source)) return;

	// remove redundant time points
	RemoveTimePointsAfter(_timeDst);

	// insert time point
	InsertTimePoint(_timeDst);

	// copy data in overall parameters
	for (auto& [type, param] : m_overall)
		param->CopyFrom(_timeDst, *_source.m_overall.at(type), _timeSrc);

	// copy data in phases
	for (auto& [key, param] : m_phases)
		param->CopyFrom(_timeDst, *_source.m_phases.at(key), _timeSrc);
}

void CBaseStream::Add(double _time, const CBaseStream& _source)
{
	Add(_time, _time, _source);
}

void CBaseStream::Add(double _timeBeg, double _timeEnd, const CBaseStream& _source)
{
	if (!HaveSameStructure(*this, _source)) return;

	// gather all time points
	std::vector<double> timePoints = VectorsUnionSorted(_source.GetTimePoints(_timeBeg, _timeEnd), GetTimePoints(_timeBeg, _timeEnd));

	// prepare temporary vector for all data for each time point
	std::vector<mix_type> mix(timePoints.size());

	// calculate mixture for each time point
	for (size_t i = 0; i < timePoints.size(); ++i)
	{
		// get masses
		const double massSrc = _source.GetMass(timePoints[i]);
		const double massDst = GetMass(timePoints[i]);

		// calculate mixture
		mix[i] = CalculateMix(timePoints[i], _source, massSrc, timePoints[i], *this, massDst);
	}

	// set mixture data for each time point
	for (size_t i = 0; i < timePoints.size(); ++i)
		SetMix(timePoints[i], mix[i]);
}

bool CBaseStream::AreEqual(double _time, const CBaseStream& _stream1, const CBaseStream& _stream2)
{
	const auto& Same = [&](double _v1, double _v2)
	{
		return std::fabs(_v1 - _v2) < std::fabs(_v1) * _stream1.m_toleranceSettings.toleranceRel + _stream1.m_toleranceSettings.toleranceAbs;
	};

	if (!HaveSameStructure(_stream1, _stream2)) return false;

	// overall parameters
	for (const auto& [key, param] : _stream1.m_overall)
	{
		if (!Same(param->GetValue(_time), _stream2.m_overall.at(key)->GetValue(_time)))
		{
			return false;
		}
	}

	// phases
	for (const auto& [key, param] : _stream1.m_phases)
	{
		if (!Same(param->GetFraction(_time), _stream2.m_phases.at(key)->GetFraction(_time)))
		{
			return false;
		}

		const auto distr1 = param->MDDistr()->GetDistribution(_time);
		const auto distr2 = _stream2.m_phases.at(key)->MDDistr()->GetDistribution(_time);
		const double* arr1 = distr1.GetDataPtr();
		const double* arr2 = distr2.GetDataPtr();
		for (size_t i = 0; i < distr1.GetDataLength(); ++i)
		{
			if (!Same(arr1[i], arr2[i]))
			{
				return false;
			}
		}
	}

	return true;
}

bool CBaseStream::AreEqual(double _time1, double _time2, const CBaseStream& _stream)
{
	const auto& Same = [&](double _v1, double _v2)
	{
		return std::fabs(_v1 - _v2) < std::fabs(_v1) * _stream.m_toleranceSettings.toleranceRel + _stream.m_toleranceSettings.toleranceAbs;
	};

	// overall parameters
	for (const auto& [key, param] : _stream.m_overall)
	{
		if (!Same(param->GetValue(_time1), param->GetValue(_time2)))
		{
			return false;
		}
	}

	// phases
	for (const auto& [key, param] : _stream.m_phases)
	{
		if (!Same(param->GetFraction(_time1), param->GetFraction(_time2)))
		{
			return false;
		}

		const auto distr1 = param->MDDistr()->GetDistribution(_time1);
		const auto distr2 = param->MDDistr()->GetDistribution(_time2);
		const double* arr1 = distr1.GetDataPtr();
		const double* arr2 = distr2.GetDataPtr();
		for (size_t i = 0; i < distr1.GetDataLength(); ++i)
		{
			if (!Same(arr1[i], arr2[i]))
			{
				return false;
			}
		}
	}

	return true;
}

CMixtureEnthalpyLookup* CBaseStream::GetEnthalpyCalculator() const
{
	// lazy initialization
	if (!m_enthalpyCalculator)
		m_enthalpyCalculator = std::make_unique<CMixtureEnthalpyLookup>(m_materialsDB, GetAllCompounds(), m_thermodynamicsSettings.limits, m_thermodynamicsSettings.intervals);
	return m_enthalpyCalculator.get();
}

double CBaseStream::CalculateEnthalpyFromTemperature(double _time) const
{
	auto* table = GetEnthalpyCalculator();
	table->SetCompoundFractions(GetCompoundsFractions(_time));
	return table->GetEnthalpy(GetTemperature(_time));
}

double CBaseStream::CalculateTemperatureFromEnthalpy(double _time) const
{
	auto* table = GetEnthalpyCalculator();
	table->SetCompoundFractions(GetCompoundsFractions(_time));
	return table->GetTemperature(GetMixtureProperty(_time, ENTHALPY));
}

const CMultidimensionalGrid& CBaseStream::GetGrid() const
{
	return m_grid;
}

void CBaseStream::SetMaterialsDatabasePtr(const CMaterialsDatabase* _database)
{
	m_materialsDB = _database;
	ClearEnthalpyCalculator();
}

void CBaseStream::SetGrid(const CMultidimensionalGrid& _grid)
{
	if (m_grid == _grid) return;
	// save new grid
	m_grid = _grid;
	// update phases
	for (auto& [state, phase] : m_phases)
		phase->SetGrid(_grid);
}

void CBaseStream::SetCacheSettings(const SCacheSettings& _settings)
{
	m_cacheSettings = _settings;

	// overall parameters
	for (auto& [type, param] : m_overall)
		param->SetCacheSettings(_settings);

	// phases
	for (auto& [state, phase] : m_phases)
		phase->SetCacheSettings(_settings);
}

void CBaseStream::SetToleranceSettings(const SToleranceSettings& _settings)
{
	m_toleranceSettings = _settings;
	for (auto& [state, phase] : m_phases)
		phase->MDDistr()->SetMinimalFraction(_settings.minFraction);
}

void CBaseStream::SetThermodynamicsSettings(const SThermodynamicsSettings& _settings)
{
	m_thermodynamicsSettings = _settings;
	ClearEnthalpyCalculator();
}

void CBaseStream::Extrapolate(double _timeExtra, double _time)
{
	if (_time >= _timeExtra) return;

	// manage time points
	RemoveTimePointsAfter(_time, false);
	InsertTimePoint(_timeExtra);

	// extrapolate overall parameters
	for (auto& [type, param] : m_overall)
		param->Extrapolate(_timeExtra, _time);

	// extrapolate phases
	for (auto& [state, phase] : m_phases)
		phase->Extrapolate(_timeExtra, _time);
}

void CBaseStream::Extrapolate(double _timeExtra, double _time1, double _time2)
{
	if (_time1 >= _time2 || _time2 >= _timeExtra) return;

	// manage time points
	RemoveTimePointsAfter(_time2, false);
	InsertTimePoint(_timeExtra);

	// extrapolate overall parameters
	for (auto& [type, param] : m_overall)
		param->Extrapolate(_timeExtra, _time1, _time2);

	// extrapolate phases
	for (auto& [state, phase] : m_phases)
		phase->Extrapolate(_timeExtra, _time1, _time2);
}

void CBaseStream::Extrapolate(double _timeExtra, double _time1, double _time2, double _time3)
{
	if (_time1 >= _time2 || _time2 >= _time3 || _time3 >= _timeExtra) return;

	// manage time points
	RemoveTimePointsAfter(_time3, false);
	InsertTimePoint(_timeExtra);

	// extrapolate overall parameters
	for (auto& [type, param] : m_overall)
		param->Extrapolate(_timeExtra, _time1, _time2, _time3);

	// extrapolate phases
	for (auto& [state, phase] : m_phases)
		phase->Extrapolate(_timeExtra, _time1, _time2, _time3);
}

void CBaseStream::SaveToFile(CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::H5AttrSaveVersion, m_saveVersion);

	// stream name
	_h5File.WriteData(_path, StrConst::Stream_H5StreamName, m_name);

	// stream key
	_h5File.WriteData(_path, StrConst::Stream_H5StreamKey, m_key);

	// time points
	_h5File.WriteData(_path, StrConst::Stream_H5TimePoints, m_timePoints);

	// grid
	m_grid.SaveToFile(_h5File, _h5File.CreateGroup(_path, StrConst::H5GroupDistrGrid));

	// overall properties
	_h5File.WriteData(_path, StrConst::Stream_H5OverallKeys, E2I(MapKeys(m_overall)));
	const std::string groupOveralls = _h5File.CreateGroup(_path, StrConst::Stream_H5GroupOveralls);
	size_t iOverall = 0;
	for (auto& [type, param] : m_overall)
	{
		const std::string overallPath = _h5File.CreateGroup(groupOveralls, StrConst::Stream_H5GroupOverallName + std::to_string(iOverall++));
		param->SaveToFile(_h5File, overallPath);
	}

	// phases
	_h5File.WriteData(_path, StrConst::Stream_H5PhaseKeys, E2I(MapKeys(m_phases)));
	const std::string groupPhases = _h5File.CreateGroup(_path, StrConst::Stream_H5GroupPhases);
	size_t iPhase = 0;
	for (auto& [state, phase] : m_phases)
	{
		const std::string phasePath = _h5File.CreateGroup(groupPhases, StrConst::Stream_H5GroupPhaseName + std::to_string(iPhase++));
		phase->SaveToFile(_h5File, phasePath);
	}
}

void CBaseStream::LoadFromFile(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	const int version = _h5File.ReadAttribute(_path, StrConst::H5AttrSaveVersion);

	// compatibility with older versions
	if (version < 2)
	{
		LoadFromFile_v1(_h5File, _path);
		return;
	}

	// clear current state
	m_timePoints.clear();
	m_overall.clear();
	m_phases.clear();
	ClearEnthalpyCalculator();

	// basic data
	_h5File.ReadData(_path, StrConst::Stream_H5StreamName, m_name);
	_h5File.ReadData(_path, StrConst::Stream_H5StreamKey,  m_key);
	_h5File.ReadData(_path, StrConst::Stream_H5TimePoints, m_timePoints);

	// grid
	if (version >= 3)
		m_grid.LoadFromFile(_h5File, _path + "/" + StrConst::H5GroupDistrGrid);

	// overall properties
	std::vector<unsigned> keys;
	_h5File.ReadData(_path, StrConst::Stream_H5OverallKeys, keys);
	std::vector<EOverall> overallKeys = vector_cast<EOverall>(keys);
	size_t iOverall = 0;
	for (auto key : overallKeys)
	{
		std::string overallPath = _path + "/" + StrConst::Stream_H5GroupOveralls + "/" + StrConst::Stream_H5GroupOverallName + std::to_string(iOverall++);
		AddOverallProperty(key, "", "")->LoadFromFile(_h5File, overallPath);
	}

	// phases
	_h5File.ReadData(_path, StrConst::Stream_H5PhaseKeys, keys);
	std::vector<EPhase> phaseKeys = vector_cast<EPhase>(keys);
	size_t iPhase = 0;
	for (auto key : phaseKeys)
	{
		std::string phasePath = _path + "/" + StrConst::Stream_H5GroupPhases + "/" + StrConst::Stream_H5GroupPhaseName + std::to_string(iPhase++);
		AddPhase(key, "")->LoadFromFile(_h5File, phasePath);
	}
}

void CBaseStream::LoadFromFile_v1(const CH5Handler& _h5File, const std::string& _path)
{
	const auto& Transpose = [](const std::vector<std::vector<double>>& _vec)
	{
		if (_vec.empty()) return std::vector<std::vector<double>>{};
		std::vector<std::vector<double>> res(_vec.front().size(), std::vector<double>(_vec.size()));
		for (size_t i = 0; i < _vec.size(); ++i)
			for (size_t j = 0; j < _vec[i].size(); ++j)
				res[j][i] = _vec[i][j];
		return res;
	};

	// mass is always present in the stream, but cab be either mass or mass flow, so store it before clear
	const std::string massName = m_overall[EOverall::OVERALL_MASS]->GetName();
	const std::string massUnit = m_overall[EOverall::OVERALL_MASS]->GetUnits();

	// clear current state
	m_timePoints.clear();
	m_overall.clear();
	m_phases.clear();
	ClearEnthalpyCalculator();

	// prepare some values
	const std::string distrPathBase = _path + "/" + StrConst::Stream_H5Group2DDistrs + "/" + StrConst::Stream_H5Group2DDistrName;
	std::vector<double> times;
	std::vector<std::vector<double>> values;

	// basic data
	_h5File.ReadData(_path, StrConst::Stream_H5StreamName, m_name);
	_h5File.ReadData(_path, StrConst::Stream_H5StreamKey,  m_key);
	_h5File.ReadData(_path, StrConst::Stream_H5TimePoints, m_timePoints);

	// overall properties
	_h5File.ReadData(distrPathBase + std::to_string(1), StrConst::Distr2D_H5TimePoints, times);
	_h5File.ReadData(distrPathBase + std::to_string(1), StrConst::Distr2D_H5Data, values);
	if (values.empty())
	{
		AddOverallProperty(EOverall::OVERALL_MASS,        massName,      massUnit)->SetRawData({ {}, {} });
		AddOverallProperty(EOverall::OVERALL_TEMPERATURE, "Temperature", "K"     )->SetRawData({ {}, {} });
		AddOverallProperty(EOverall::OVERALL_PRESSURE,    "Pressure",    "Pa"    )->SetRawData({ {}, {} });
	}
	else
	{
		if (values.size() == 1 && values.size() != times.size())
			values.resize(times.size(), values.front());
		if (values.front().size() != 3) return;
		const auto data = Transpose(values);
		AddOverallProperty(EOverall::OVERALL_MASS,        massName,      massUnit)->SetRawData({ times, data[0] });
		AddOverallProperty(EOverall::OVERALL_TEMPERATURE, "Temperature", "K"     )->SetRawData({ times, data[1] });
		AddOverallProperty(EOverall::OVERALL_PRESSURE,    "Pressure",    "Pa"    )->SetRawData({ times, data[2] });
	}

	// phases
	const int nPhases = _h5File.ReadAttribute(_path, StrConst::Stream_H5AttrPhasesNum);
	_h5File.ReadData(distrPathBase + std::to_string(0), StrConst::Distr2D_H5TimePoints, times);
	_h5File.ReadData(distrPathBase + std::to_string(0), StrConst::Distr2D_H5Data, values);
	if (values.size() == 1 && values.size() != times.size())
		values.resize(times.size(), values.front());
	const auto data = Transpose(values);
	if (nPhases < 0) return;
	for (size_t iPhase = 0; iPhase < static_cast<size_t>(nPhases); ++iPhase)
	{
		std::string phasePath = _path + "/" + StrConst::Stream_H5GroupPhases + "/" + StrConst::Stream_H5GroupPhaseName + std::to_string(iPhase);
		std::string name;
		uint32_t soa;
		_h5File.ReadData(phasePath, StrConst::Stream_H5PhaseName, name);
		_h5File.ReadData(phasePath, StrConst::Stream_H5PhaseSOA,  soa);
		CPhase* phase;
		switch (soa)
		{
		case 0:	 phase = AddPhase(EPhase::SOLID, name);     break;
		case 1:  phase = AddPhase(EPhase::LIQUID, name);    break;
		case 2:  phase = AddPhase(EPhase::VAPOR, name);     break;
		default: phase = AddPhase(EPhase::UNDEFINED, name); break;
		}
		phase->MDDistr()->LoadFromFile(_h5File, phasePath);
		if (!data.empty())
			phase->Fractions()->SetRawData({times, data[iPhase]});
	}
}

CBaseStream::mix_type CBaseStream::CalculateMix(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2)
{
	// result data
	std::map<EOverall, double> mixOverall;
	std::map<EPhase, double> mixPhaseFrac;
	std::map<EPhase, CDenseMDMatrix> mixDistr;

	// calculate overall properties
	for (const auto& [type, param] : _stream1.m_overall)
	{
		switch (type)
		{
		case EOverall::OVERALL_MASS:
			mixOverall[type] = _mass1 + _mass2;
			break;
		case EOverall::OVERALL_TEMPERATURE:
			mixOverall[type] = CalculateMixTemperature(_time1, _stream1, _mass1, _time2, _stream2, _mass2);
			break;
		case EOverall::OVERALL_PRESSURE:
			mixOverall[type] = CalculateMixPressure(_time1, _stream1, _time2, _stream2);
			break;
		case EOverall::OVERALL_USER_DEFINED_01:
		case EOverall::OVERALL_USER_DEFINED_02:
		case EOverall::OVERALL_USER_DEFINED_03:
		case EOverall::OVERALL_USER_DEFINED_04:
		case EOverall::OVERALL_USER_DEFINED_05:
		case EOverall::OVERALL_USER_DEFINED_06:
		case EOverall::OVERALL_USER_DEFINED_07:
		case EOverall::OVERALL_USER_DEFINED_08:
		case EOverall::OVERALL_USER_DEFINED_09:
		case EOverall::OVERALL_USER_DEFINED_10:
			mixOverall[type] = CalculateMixOverall(_time1, _stream1, _mass1, _time2, _stream2, _mass2, type);
			break;
		}
	}

	// calculate phases
	for (const auto& [key, param] : _stream1.m_phases)
	{
		mixPhaseFrac[key] = CalculateMixPhaseFractions(_time1, _stream1, _mass1, _time2, _stream2, _mass2, key);
		mixDistr[key] = CalculateMixDistribution(_time1, _stream1, _mass1, _time2, _stream2, _mass2, key);
	}

	// normalize fractions
	double sum = 0.0;
	for (const auto& [key, param] : _stream1.m_phases)
		sum += mixPhaseFrac[key];
	if (sum != 0.0 && sum != 1.0)
		for (const auto& [key, param] : _stream1.m_phases)
			mixPhaseFrac[key] /= sum;

	return { mixOverall, mixPhaseFrac, mixDistr };
}

void CBaseStream::SetMix(double _time, const mix_type& _data)
{
	// add time point
	AddTimePoint(_time);

	// overall properties
	for (auto& [type, param] : m_overall)
		param->SetValue(_time, std::get<0>(_data).at(type));

	// phases
	for (auto& [key, param] : m_phases)
	{
		param->SetFraction(_time, std::get<1>(_data).at(key));
		param->MDDistr()->SetDistribution(_time, std::get<2>(_data).at(key));
	}
}

double CBaseStream::CalculateMixPressure(double _time1, const CBaseStream& _stream1, double _time2, const CBaseStream& _stream2)
{
	const double pressure1 = _stream1.GetPressure(_time1);
	const double pressure2 = _stream2.GetPressure(_time2);
	if (pressure1 == 0.0 || pressure2 == 0.0)
		return std::max(pressure2, pressure1);	// return nonzero pressure, if any
	return std::min(pressure2, pressure1);		// return minimum pressure
}

double CBaseStream::CalculateMixTemperature(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2)
{
	// TODO: check that T is in limits of lookup
	// get and check current temperatures
	const auto temperature1 = _stream1.GetTemperature(_time1);
	const auto temperature2 = _stream2.GetTemperature(_time2);
	if (temperature1 == temperature2)
		return temperature1;
	// calculate total mass
	const double massMix = _mass1 + _mass2;
	// if no material at all, return some arbitrary temperature
	if (massMix == 0.0)
		return 0.0;
	// get and check lookup tables for enthalpies
	const CMixtureEnthalpyLookup& lookup1 = *_stream1.GetEnthalpyCalculator();
	const CMixtureEnthalpyLookup& lookup2 = *_stream2.GetEnthalpyCalculator();
	if (lookup1.Size() == 1 && lookup2.Size() == 1 && lookup1 == lookup2)
		return (temperature1 * _mass1 + temperature2 * _mass2) / massMix;
	// get enthalpies
	const double enthalpy1 = _stream1.CalculateEnthalpyFromTemperature(_time1);
	const double enthalpy2 = _stream2.CalculateEnthalpyFromTemperature(_time2);
	// calculate (specific) total enthalpy
	const double enthalpyMix = (enthalpy1 * _mass1 + enthalpy2 * _mass2) / massMix;
	// combine both enthalpy tables for mixture enthalpy table
	const CMixtureEnthalpyLookup lookupMix = lookup1 * (_mass1 / massMix) + lookup2 * (_mass2 / massMix);
	// read out new temperature
	return lookupMix.GetTemperature(enthalpyMix);
}

double CBaseStream::CalculateMixOverall(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2, EOverall _property)
{
	if (_mass1 + _mass2 == 0.0) return {};

	const double value1 = _stream1.m_overall.at(_property)->GetValue(_time1);
	const double value2 = _stream2.m_overall.at(_property)->GetValue(_time2);
	return (_mass1 * value1 + _mass2 * value2) / (_mass1 + _mass2);
}

double CBaseStream::CalculateMixPhaseFractions(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2, EPhase _phase)
{
	if (_mass1 + _mass2 == 0.0) return {};

	const double fraction1 = _stream1.m_phases.at(_phase)->GetFraction(_time1);
	const double fraction2 = _stream2.m_phases.at(_phase)->GetFraction(_time2);
	return (_mass1 * fraction1 + _mass2 * fraction2) / (_mass1 + _mass2);
}

CDenseMDMatrix CBaseStream::CalculateMixDistribution(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2, EPhase _phase)
{
	const auto& phase1 = _stream1.m_phases.at(_phase);
	const auto& phase2 = _stream2.m_phases.at(_phase);
	const double phaseFrac1 = phase1->GetFraction(_time1);
	const double phaseFrac2 = phase2->GetFraction(_time2);
	CDenseMDMatrix distr1 = phase1->MDDistr()->GetDistribution(_time1);
	CDenseMDMatrix distr2 = phase2->MDDistr()->GetDistribution(_time2);
	// TODO: more elegant/effective solution
	CDenseMDMatrix mix;
	if (phaseFrac1 != 0.0 || phaseFrac2 != 0.0) // normal calculation
		mix = distr1 * _mass1 * phaseFrac1 + distr2 * _mass2 * phaseFrac2;
	else // preserve compounds distributions, if phase fractions are zero
		mix = distr1 * _mass1 + distr2 * _mass2;

	mix.Normalize();
	return mix;
}

void CBaseStream::InsertTimePoint(double _time)
{
	const auto pos = std::lower_bound(m_timePoints.begin(), m_timePoints.end(), _time);
	if (pos == m_timePoints.end())					// all existing times are smaller
		m_timePoints.emplace_back(_time);
	else if (std::fabs(*pos - _time) <= m_epsilon)	// this time already exists
		*pos = _time;
	else											// insert to the right position
		m_timePoints.insert(pos, _time);
}

bool CBaseStream::HasTime(double _time) const
{
	if (m_timePoints.empty()) return false;
	const auto pos = std::lower_bound(m_timePoints.begin(), m_timePoints.end(), _time);
	if (pos == m_timePoints.end()) return false;
	return std::fabs(*pos - _time) <= m_epsilon;
}

bool CBaseStream::HasOverallProperty(EOverall _property) const
{
	return MapContainsKey(m_overall, _property);
}

bool CBaseStream::HasCompound(const std::string& _compoundKey) const
{
	return VectorContains(GetAllCompounds(), _compoundKey);
}

bool CBaseStream::HasCompounds(const std::vector<std::string>& _compoundKeys) const
{
	for (const auto& c : _compoundKeys)
		if (!HasCompound(c))
			return false;
	return true;
}

size_t CBaseStream::CompoundIndex(const std::string& _compoundKey) const
{
	const auto& compounds = GetAllCompounds();
	for (size_t i = 0; i < compounds.size(); ++i)
		if (compounds[i] == _compoundKey)
			return i;
	return static_cast<size_t>(-1);
}

bool CBaseStream::HasPhase(EPhase _phase) const
{
	return MapContainsKey(m_phases, _phase);
}

std::vector<double> CBaseStream::GetPSDMassFraction(double _time, const std::vector<std::string>& _compoundKeys) const
{
	// for all available compounds
	if (_compoundKeys.empty() || _compoundKeys.size() == GetAllCompounds().size())
		return m_phases.at(EPhase::SOLID)->MDDistr()->GetDistribution(_time, DISTR_SIZE);

	// only for selected compounds
	std::vector<double> distr(m_grid.GetGridDimension(DISTR_SIZE)->ClassesNumber(), 0.0);
	for (const auto& key : _compoundKeys)
	{
		std::vector<double> vec = m_phases.at(EPhase::SOLID)->MDDistr()->GetVectorValue(_time, DISTR_COMPOUNDS, static_cast<unsigned>(CompoundIndex(key)), DISTR_SIZE);
		AddVectors(distr, vec, distr);
	}
	::Normalize(distr);
	return distr;
}

std::vector<double> CBaseStream::GetPSDNumber(double _time, const std::vector<std::string>& _compoundKeys, EPSDGridType _grid) const
{
	const auto& compounds = GetAllCompounds();
	std::vector<std::string> activeCompounds = _compoundKeys.empty() || _compoundKeys.size() == compounds.size() ? compounds : _compoundKeys;
	const bool hasPorosity = m_grid.HasDimension(DISTR_PART_POROSITY);
	std::vector<double> volumes = _grid == EPSDGridType::VOLUME ? m_grid.GetPSDMeans(EPSDGridType::VOLUME) : DiameterToVolume(m_grid.GetPSDMeans(EPSDGridType::DIAMETER));
	const double totalMass = GetPhaseMass(_time, EPhase::SOLID);
	const size_t nSizeClasses = m_grid.GetGridDimension(DISTR_SIZE)->ClassesNumber();

	// single compound with no porosity, only one compound defined in the stream
	if (!hasPorosity && activeCompounds.size() == 1 && compounds.size() == 1)
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
		std::vector<double> res = Normalized(m_phases.at(EPhase::SOLID)->MDDistr()->GetVectorValue(_time, DISTR_COMPOUNDS, static_cast<unsigned>(CompoundIndex(activeCompounds.front())), DISTR_SIZE));
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
		if (activeCompounds.size() != compounds.size())
		{
			// TODO: remove cast
			std::vector<size_t> classes = vector_cast<size_t>(distr.GetClasses());
			classes[0] = _compoundKeys.size(); // reduce number of compounds
			CDenseMDMatrix tempDistr{ distr.GetDimensions(), vector_cast<unsigned>(classes) };
			for (size_t iCompound = 0; iCompound < activeCompounds.size(); ++iCompound)
				for (size_t iSize = 0; iSize < nSizeClasses; ++iSize)
				{
					std::vector<double> vec = distr.GetVectorValue(DISTR_COMPOUNDS, static_cast<unsigned>(CompoundIndex(activeCompounds[iCompound])), DISTR_SIZE, static_cast<unsigned>(iSize), DISTR_PART_POROSITY);
					tempDistr.SetVectorValue(DISTR_COMPOUNDS, static_cast<unsigned>(iCompound), DISTR_SIZE, static_cast<unsigned>(iSize), vec);
				}
			tempDistr.Normalize();
			distr = tempDistr;
		}

		const size_t nPorosityClasses = m_grid.GetGridDimension(DISTR_PART_POROSITY)->ClassesNumber();
		const std::vector<double> porosities = m_grid.GetGridDimensionNumeric(DISTR_PART_POROSITY)->GetClassesMeans();

		// calculate distribution
		std::vector<double> res(nSizeClasses);
		for (size_t iCompound = 0; iCompound < activeCompounds.size(); ++iCompound)
		{
			std::vector<double> vec(nSizeClasses);
			const double density = GetCompoundProperty(_time, activeCompounds[iCompound], DENSITY);
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
		if (activeCompounds.size() != compounds.size())
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
			const double density = GetCompoundProperty(_time, activeCompounds[iComp], DENSITY);
			if (density == 0.0) return std::vector<double>(nSizeClasses, 0.0);
			for (size_t iSize = 0; iSize < nSizeClasses; ++iSize)
				if (volumes[iSize] != 0.0)
					res[iSize] += distr[iComp][iSize] / density / volumes[iSize];
		}
		return res;
	}
}

void CBaseStream::ClearEnthalpyCalculator()
{
	m_enthalpyCalculator.reset(nullptr);
}

////////////////////////////////////////////////////////////////////////////////
/// Deprecated functions

void CBaseStream::AddTimePoint(double _timeDst, double _timeSrc)
{
	CopyTimePoint(_timeDst, _timeSrc);
}

std::string CBaseStream::GetStreamName() const
{
	return GetName();
}

void CBaseStream::SetupStream(const CBaseStream* _stream)
{
	SetupStructure(_stream);
}

std::vector<double> CBaseStream::GetTimePointsForInterval(double _timeBeg, double _timeEnd, bool _inclusive) const
{
	return _inclusive ? GetTimePointsClosed(_timeBeg, _timeEnd) : GetTimePoints(_timeBeg, _timeEnd);
}

double CBaseStream::GetOverallProperty(double _time, unsigned _property) const
{
	switch (_property)
	{
	case FLOW:
	case TOTAL_FLOW:		return GetMass(_time);
	case TEMPERATURE:		return GetTemperature(_time);
	case PRESSURE:			return GetPressure(_time);
	default:				break;
	}
	if (_property >= 100 && _property < 200)
		return GetMixtureProperty(_time, static_cast<ECompoundConstProperties>(_property));
	if (_property >= 200 && _property < 300)
		return GetMixtureProperty(_time, static_cast<ECompoundTPProperties>(_property));
	return 0.0;
}

double CBaseStream::GetMass_Base(double _time) const
{
	return GetMass(_time);
}

void CBaseStream::SetMass_Base(double _time, double _value)
{
	SetMass(_time, _value);
}

std::vector<std::string> CBaseStream::GetCompoundsList() const
{
	return GetAllCompounds();
}

std::vector<std::string> CBaseStream::GetCompoundsNames() const
{
	const auto& compounds = GetAllCompounds();
	std::vector<std::string> res;
	res.reserve(compounds.size());
	for (const auto& key : compounds)
		if (const auto* compound = m_materialsDB->GetCompound(key))
			res.push_back(compound->GetName());
	return res;
}

size_t CBaseStream::GetCompoundsNumber() const
{
	return m_grid.GetGridDimension(DISTR_COMPOUNDS)->ClassesNumber();
}

double CBaseStream::GetCompoundPhaseFraction(double _time, const std::string& _compoundKey, unsigned _soa) const
{
	return GetCompoundFraction(_time, _compoundKey, SOA2EPhase(_soa));
}

double CBaseStream::GetCompoundPhaseFraction(double _time, unsigned _index, unsigned _soa) const
{
	const auto& compounds = GetAllCompounds();
	if (_index >= compounds.size()) return {};
	return GetCompoundFraction(_time, compounds[_index], SOA2EPhase(_soa));
}

void CBaseStream::SetCompoundPhaseFraction(double _time, const std::string& _compoundKey, unsigned _soa, double _value, unsigned _basis)
{
	if (_basis == 0)
		SetCompoundFraction(_time, _compoundKey, SOA2EPhase(_soa), _value);
	else
		SetCompoundMolFraction(_time, _compoundKey, SOA2EPhase(_soa), _value);
}

double CBaseStream::GetPhaseMass_Base(double _time, unsigned _soa) const
{
	return GetPhaseMass(_time, SOA2EPhase(_soa));
}

void CBaseStream::SetPhaseMass_Base(double _time, unsigned _soa, double _value)
{
	SetPhaseMass(_time, SOA2EPhase(_soa), _value);
}

unsigned CBaseStream::GetPhaseSOA(unsigned _index) const
{
	if (_index >= m_phases.size()) return {};
	auto it = m_phases.begin();
	std::advance(it, _index);
	return EPhase2SOA(it->first);
}

unsigned CBaseStream::GetPhaseIndex(unsigned _soa) const
{
	return static_cast<unsigned>(std::distance(m_phases.begin(), m_phases.find(SOA2EPhase(_soa))));
}

size_t CBaseStream::GetPhasesNumber() const
{
	return m_phases.size();
}

double CBaseStream::GetCompoundConstant(const std::string& _compoundKey, unsigned _property) const
{
	return GetCompoundProperty(_compoundKey, static_cast<ECompoundConstProperties>(_property));
}

double CBaseStream::GetCompoundTPDProp(double _time, const std::string& _compoundKey, unsigned _property) const
{
	return GetCompoundProperty(_time, _compoundKey, static_cast<ECompoundTPProperties>(_property));
}

double CBaseStream::GetCompoundTPDProp(const std::string& _compoundKey, unsigned _property, double _temperature, double _pressure) const
{
	return GetCompoundProperty(_compoundKey, static_cast<ECompoundTPProperties>(_property), _temperature, _pressure);
}

double CBaseStream::GetCompoundInteractionProp(double _time, const std::string& _compoundKey1, const std::string& _compoundKey2, unsigned _property) const
{
	return GetCompoundProperty(_time, _compoundKey1, _compoundKey2, static_cast<EInteractionProperties>(_property));
}

double CBaseStream::GetCompoundInteractionProp(const std::string& _compoundKey1, const std::string& _compoundKey2, unsigned _property, double _temperature, double _pressure) const
{
	return GetCompoundProperty(_compoundKey1, _compoundKey2, static_cast<EInteractionProperties>(_property), _temperature, _pressure);
}

bool CBaseStream::GetDistribution(double _time, EDistrTypes _distribution, std::vector<double>& _result) const
{
	_result = GetDistribution(_time, _distribution);
	return true;
}

void CBaseStream::CopyFromStream_Base(const CBaseStream& _source, double _time, bool _deleteDataAfter)
{
	Copy(_time, _source);
	if (_deleteDataAfter)
		RemoveTimePointsAfter(_time);
}

void CBaseStream::AddStream_Base(const CBaseStream& _source, double _time)
{
	Add(_time, _source);
}

double CBaseStream::GetSinglePhaseProp(double _time, unsigned _property, unsigned _soa) const
{
	if (_property == PHASE_FRACTION || _property == FRACTION)
		return GetPhaseFraction(_time, SOA2EPhase(_soa));
	if (_property == FLOW)
		return GetPhaseMass(_time, SOA2EPhase(_soa));
	if (_property == TEMPERATURE)
		return GetPhaseProperty(_time, SOA2EPhase(_soa), EOverall::OVERALL_TEMPERATURE);
	if (_property == PRESSURE)
		return GetPhaseProperty(_time, SOA2EPhase(_soa), EOverall::OVERALL_PRESSURE);
	if (100 <= _property && _property <= 199)
		return GetPhaseProperty(_time, SOA2EPhase(_soa), static_cast<ECompoundConstProperties>(_property));
	if (200 <= _property && _property <= 299)
		return GetPhaseProperty(_time, SOA2EPhase(_soa), static_cast<ECompoundTPProperties>(_property));
	return {};
}

double CBaseStream::GetPhaseTPDProp(double _time, unsigned _property, unsigned _soa) const
{
	return GetPhaseProperty(_time, SOA2EPhase(_soa), static_cast<ECompoundTPProperties>(_property));
}

void CBaseStream::SetSinglePhaseProp(double _time, unsigned _property, unsigned _soa, double _value)
{
	if (_property == PHASE_FRACTION || _property == FRACTION)
		SetPhaseFraction(_time, SOA2EPhase(_soa), _value);
	else if (_property == FLOW)
		SetPhaseMass(_time, SOA2EPhase(_soa), _value);
}

EPhase CBaseStream::SOA2EPhase(unsigned _soa)
{
	switch (_soa)
	{
	case 0:		return EPhase::SOLID;
	case 1:		return EPhase::LIQUID;
	case 2:		return EPhase::VAPOR;
	default:	return EPhase::UNDEFINED;
	}
}

unsigned CBaseStream::EPhase2SOA(EPhase _phase)
{
	switch (_phase)
	{
	case EPhase::SOLID:		return 0;
	case EPhase::LIQUID:	return 1;
	case EPhase::VAPOR:		return 2;
	case EPhase::UNDEFINED:	return 4;
	}
	return 4;
}