/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2025, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define _USE_MATH_DEFINES
#include "Holdup.h"
#include "Stream.h"
#include "TimeDependentValue.h"
#include "Phase.h"
#include "ContainerFunctions.h"
#include <cmath>

CHoldup::CHoldup(const std::string& _key) :
	CBaseStream{ _key }
{
	m_overall[EOverall::OVERALL_MASS]->SetName("Mass");
	m_overall[EOverall::OVERALL_MASS]->SetUnits("kg");
}

CHoldup::CHoldup(const std::string& _key, const CMaterialsDatabase* _materialsDB, const CMultidimensionalGrid& _grid,
	const std::vector<SOverallDescriptor>* _overall, const std::vector<SPhaseDescriptor>* _phases,
	const SCacheSettings* _cache, const SToleranceSettings* _tolerance, const SThermodynamicsSettings* _thermodynamics) :
	CBaseStream{ _key, _materialsDB, _grid, _overall, _phases, _cache, _tolerance, _thermodynamics }
{
	m_overall[EOverall::OVERALL_MASS]->SetName("Mass");
	m_overall[EOverall::OVERALL_MASS]->SetUnits("kg");
}

CHoldup::CHoldup(const CBaseStream& _other) :
	CBaseStream{ _other }
{
	m_overall[EOverall::OVERALL_MASS]->SetName("Mass");
	m_overall[EOverall::OVERALL_MASS]->SetUnits("kg");
}

void CHoldup::CopyFromHoldup(double _time, const CHoldup* _source)
{
	Copy(_time,  *_source);
}

void CHoldup::CopyFromHoldup(double _timeBeg, double _timeEnd, const CHoldup* _source)
{
	Copy(_timeBeg, _timeEnd, *_source);
}

void CHoldup::CopyFromHoldup(double _timeDst, const CHoldup* _source, double _timeSrc)
{
	Copy(_timeDst, *_source, _timeSrc);
}

/* All time points after _timeEnd are removed, since after mixing they make no sense anymore.
   Nevertheless, _timeEnd should stay to enable successive adding of several streams to the holdup.
   If holdup has no time points after _timeBeg, adds the stream to the values in _timeBeg.
   If there are other time points after _timeBeg, adds the stream to the values in _timeEnd (possibly extrapolated from the last defined time point)*/
void CHoldup::AddStream(double _timeBeg, double _timeEnd, const CStream* _source)
{
	if (_timeBeg > _timeEnd) return;
	if (!HaveSameStructure(*this, *_source)) return;

	// remove discarded time points
	RemoveTimePointsAfter(_timeEnd);

	// special handling if there are no time points in the holdup
	if (m_timePoints.empty())
	{
		Copy(_timeBeg, *_source);
		SetMass(_timeBeg, 0.0);
	}

	// ensure the stream has only these two time points for proper interpolation
	const CStream* stream = _source;
	if (std::fabs(_source->GetPreviousTimePoint(_timeEnd) - _timeBeg) > m_epsilon) // there are some points in-between - create a copy with only two points
	{
		auto* temp = new CStream();
		temp->SetupStructure(_source);
		temp->SetCacheSettings(SCacheSettings{ false, 0, {} });
		temp->Copy(_timeBeg, *_source);
		temp->Copy(_timeEnd, *_source);
		stream = temp;
	}

	// calculate time point in the stream
	const double timeDelta = (_timeEnd - _timeBeg) / M_SQRT2;
	const bool timeGrowth = stream->GetMassFlow(_timeBeg) < stream->GetMassFlow(_timeEnd);
	const double timeSrc = timeGrowth ? _timeBeg + timeDelta : _timeEnd - timeDelta;

	// get masses
	const double massSrc = stream->GetMassFlow((_timeBeg + _timeEnd) / 2) * (_timeEnd - _timeBeg);
	const double massDst = GetLastTimePoint() <= _timeBeg ? GetMass(_timeBeg) : GetMass(_timeEnd);

	// calculate mixture
	const mix_type mix = CalculateMix(timeSrc, *stream, massSrc, _timeEnd, *this, massDst);

	// set obtained mixture to the temporary holdup
	SetMix(_timeEnd, mix);

	// remove time points between begin and end, since they are not consistent anymore
	RemoveTimePoints(_timeBeg, _timeEnd, false);

	// clean up temporary stream if necessary
	if (stream != _source)
		delete stream;
}

void CHoldup::AddHoldup(double _time, const CHoldup* _source)
{
	Add(_time, *_source);
}

void CHoldup::AddHoldup(double _timeBeg, double _timeEnd, const CHoldup* _source)
{
	Add(_timeBeg, _timeEnd, *_source);
}
