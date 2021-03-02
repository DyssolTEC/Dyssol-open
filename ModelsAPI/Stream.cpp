/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

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

CStream::CStream(const std::string& _key, const CMaterialsDatabase* _materialsDB, const CDistributionsGrid* _grid,
	const std::vector<std::string>* _compounds, const std::vector<SOverallDescriptor>* _overall, const std::vector<SPhaseDescriptor>* _phases,
	const SCacheSettings* _cache, const SToleranceSettings* _tolerance, const SThermodynamicsSettings* _thermodynamics) :
	CBaseStream{ _key, _materialsDB, _grid, _compounds, _overall, _phases, _cache, _tolerance, _thermodynamics }
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

// TODO: move it somewhere
////////////////////////////////////////////////////////////////////////////////
/// Deprecated functions

void CStream::CopyFromStream(const CStream* _source, double _time, [[maybe_unused]] bool _deleteDataAfter)
{
	CopyFromStream(_time, _source);
}

void CStream::CopyFromStream(const CStream* _source, double _timeBeg, double _timeEnd)
{
	CopyFromStream(_timeBeg, _timeEnd, _source);
}

void CStream::CopyFromHoldup(const CHoldup* _source, double _time, double _massFlow, [[maybe_unused]] bool _deleteDataAfter)
{
	CopyFromHoldup(_time, _source, _massFlow);
}

void CStream::AddStream(const CStream* _source, double _time)
{
	AddStream(_time, _source);
}

double CStream::GetMassFlow(double _time, unsigned _basis) const
{
	if (_basis == 0)
		return GetMassFlow(_time);
	else
		return GetMolFlow(_time);
}

void CStream::SetMassFlow(double _time, double _value, unsigned _basis)
{
	if (_basis == 0)
		SetMassFlow(_time, _value);
	else
		SetMolFlow(_time, _value);
}

double CStream::GetCompoundMassFlow(double _time, const std::string& _compound, unsigned _soa, unsigned _basis) const
{
	if (_basis == 0)
		return GetCompoundMassFlow(_time, _compound, SOA2EPhase(_soa));
	else
		return GetCompoundMolFlow(_time, _compound, SOA2EPhase(_soa));
}

double CStream::GetPhaseMassFlow(double _time, unsigned _soa, unsigned _basis) const
{
	if (_basis == 0)
		return GetPhaseMassFlow(_time, SOA2EPhase(_soa));
	else
		return GetPhaseMolFlow(_time, SOA2EPhase(_soa));
}

void CStream::SetPhaseMassFlow(double _time, unsigned _soa, double _value, unsigned _basis)
{
	if (_basis == 0)
		SetPhaseMassFlow(_time, SOA2EPhase(_soa), _value);
	else
		SetPhaseMolFlow(_time, SOA2EPhase(_soa), _value);
}
