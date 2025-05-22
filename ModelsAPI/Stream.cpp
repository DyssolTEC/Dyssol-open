/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2025, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Stream.h"
#include "Holdup.h"
#include "TimeDependentValue.h"
#include "Phase.h"

CStream::CStream(const std::string& _key) :
	CBaseStream{ _key }
{
	m_overall[EOverall::OVERALL_MASS]->SetName("Mass flow");
	m_overall[EOverall::OVERALL_MASS]->SetUnits("kg/s");
}

CStream::CStream(const std::string& _key, const CMaterialsDatabase* _materialsDB, const CMultidimensionalGrid& _grid,
	const std::vector<SOverallDescriptor>* _overall, const std::vector<SPhaseDescriptor>* _phases,
	const SCacheSettings* _cache, const SToleranceSettings* _tolerance, const SThermodynamicsSettings* _thermodynamics) :
	CBaseStream{ _key, _materialsDB, _grid, _overall, _phases, _cache, _tolerance, _thermodynamics }
{
	m_overall[EOverall::OVERALL_MASS]->SetName("Mass flow");
	m_overall[EOverall::OVERALL_MASS]->SetUnits("kg/s");
}

CStream::CStream(const CBaseStream& _other) :
	CBaseStream{ _other }
{
	m_overall[EOverall::OVERALL_MASS]->SetName("Mass flow");
	m_overall[EOverall::OVERALL_MASS]->SetUnits("kg/s");
}

void CStream::CopyFromStream(double _time, const CStream* _source)
{
	Copy(_time, *_source);
}

void CStream::CopyFromStream(double _timeBeg, double _timeEnd, const CStream* _source)
{
	Copy(_timeBeg, _timeEnd, *_source);
}

void CStream::CopyFromStream(double _timeDst, const CStream* _source, double _timeSrc)
{
	Copy(_timeDst, *_source, _timeSrc);
}

void CStream::CopyFromHoldup(double _time, const CHoldup* _source, double _massFlow)
{
	Copy(_time, *_source);
	SetMassFlow(_time, _massFlow);
}

void CStream::CopyFromHoldup(double _timeDst, const CHoldup* _source, double _timeSrc, double _massFlow)
{
	Copy(_timeDst, *_source, _timeSrc);
	SetMassFlow(_timeDst, _massFlow);
}

void CStream::AddStream(double _time, const CStream* _source)
{
	Add(_time, *_source);
}

void CStream::AddStream(double _timeBeg, double _timeEnd, const CStream* _source)
{
	Add(_timeBeg, _timeEnd, *_source);
}

double CStream::GetMassFlow(double _time) const
{
	return GetMass(_time);
}

double CStream::GetAccumulatedMass(double _timeBeg, double _timeEnd) const
{
	const std::vector<double> tp = GetTimePointsClosed(_timeBeg, _timeEnd);
	double res{ 0 };
	for (size_t i = 0; i < tp.size() - 1; ++i)
		res += (GetMassFlow(tp[i + 1]) + GetMassFlow(tp[i])) / 2. * (tp[i + 1] - tp[i]);
	return res;
}

double CStream::GetMolFlow(double _time) const
{
	return GetMol(_time);
}

void CStream::SetMassFlow(double _time, double _value)
{
	SetMass(_time, _value);
}

void CStream::SetMolFlow(double _time, double _value)
{
	SetMol(_time, _value);
}

double CStream::GetCompoundMassFlow(double _time, const std::string& _compoundKey, EPhase _phase) const
{
	return GetCompoundMass(_time, _compoundKey, _phase);
}

double CStream::GetCompoundMassFlow(double _time, const std::string& _compoundKey) const
{
	return GetCompoundMass(_time, _compoundKey);
}

double CStream::GetCompoundMolFlow(double _time, const std::string& _compoundKey, EPhase _phase) const
{
	return GetCompoundMol(_time, _compoundKey, _phase);
}

std::vector<double> CStream::GetCompoundsMassFlows(double _time) const
{
	return GetCompoundsMasses(_time);
}

std::vector<double> CStream::GetCompoundsMassFlows(double _time, EPhase _phase) const
{
	return GetCompoundsMasses(_time, _phase);
}

double CStream::GetPhaseMassFlow(double _time, EPhase _phase) const
{
	return GetPhaseMass(_time, _phase);
}

double CStream::GetPhaseMolFlow(double _time, EPhase _phase) const
{
	return GetPhaseMol(_time, _phase);
}

void CStream::SetPhaseMassFlow(double _time, EPhase _phase, double _value)
{
	SetPhaseMass(_time, _phase, _value);
}

void CStream::SetPhaseMolFlow(double _time, EPhase _phase, double _value)
{
	SetPhaseMol(_time, _phase, _value);
}
