/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define _USE_MATH_DEFINES
#include "Holdup.h"
#include "Stream.h"
#include "TimeDependentValue.h"
#include "Phase.h"
#include "ContainerFunctions.h"

CHoldup::CHoldup(const std::string& _key) :
	CBaseStream{ _key }
{
	m_overall[EOverall::OVERALL_MASS]->SetName("Mass");
	m_overall[EOverall::OVERALL_MASS]->SetUnits("kg");
}

CHoldup::CHoldup(const std::string& _key, const CMaterialsDatabase* _materialsDB, const CDistributionsGrid* _grid,
	const std::vector<std::string>* _compounds, const std::vector<SOverallDescriptor>* _overall, const std::vector<SPhaseDescriptor>* _phases,
	const SCacheSettings* _cache, const SToleranceSettings* _tolerance) :
	CBaseStream{ _key, _materialsDB, _grid, _compounds, _overall, _phases, _cache, _tolerance }
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

void CHoldup::AddStream(double _timeEnd, const CStream* _source)
{
	AddStream(_source->GetPreviousTimePoint(_timeEnd), _timeEnd, _source);
}

void CHoldup::AddStream(double _timeBeg, double _timeEnd, const CStream* _source)
{
	if (_timeBeg > _timeEnd) return;
	if (!HaveSameStructure(*this, *_source)) return;

	// remove discarded time points
	RemoveTimePointsAfter(_timeBeg);

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
		temp->SetupStructure(*_source);
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
	const double massDst = GetMass(_timeBeg);

	// calculate mixture
	const mix_type mix = CalculateMix(timeSrc, *stream, massSrc, _timeBeg, *this, massDst);

	// set mixture data
	SetMix(*this, _timeEnd, mix);

	// clean up temporary stream if necessary
	if (stream != _source)
		delete stream;
}

void CHoldup::AddStreamInterval(double _timeBeg, double _timeEnd, const CStream* _source)
{
	if (_timeBeg > _timeEnd) return;
	if (!HaveSameStructure(*this, *_source)) return;

	// remove discarded time points
	RemoveTimePointsAfter(_timeBeg);

	// special handling if there are no time points in the holdup
	if (m_timePoints.empty())
	{
		Copy(_timeBeg, *_source);
		SetMass(_timeBeg, 0.0);
	}

	// get all time points
	std::vector<double> timePoints = _source->GetTimePointsClosed(_timeBeg, _timeEnd);

	// prepare temporary vector for all data for each time point
	std::vector<mix_type> mix(timePoints.size() - 1);

	// calculate mixture for each time point
	for (size_t i = 0; i < timePoints.size() - 1; ++i)
	{
		// calculate time point in the stream
		const double timeDelta = (timePoints[i + 1] - timePoints[i]) / M_SQRT2;
		const bool timeGrowth = _source->GetMassFlow(timePoints[i]) < _source->GetMassFlow(timePoints[i + 1]);
		const double timeSrc = timeGrowth ? timePoints[i] + timeDelta : timePoints[i + 1] - timeDelta;

		// get masses
		const double massSrc = _source->GetMassFlow((timePoints[i] + timePoints[i + 1]) / 2) * (timePoints[i + 1] - timePoints[i]);
		const double massDst = GetMass(timePoints[i]);

		mix[i] = CalculateMix(timeSrc, *_source, massSrc, timePoints[i], *this, massDst);
	}

	// set mixture data for each time point
	for (size_t i = 0; i < timePoints.size() - 1; ++i)
		SetMix(*this, timePoints[i + 1], mix[i]);
}

void CHoldup::AddHoldup(double _time, const CHoldup* _source)
{
	Add(_time, *_source);
}

void CHoldup::AddHoldup(double _timeBeg, double _timeEnd, const CHoldup* _source)
{
	Add(_timeBeg, _timeEnd, *_source);
}

void CHoldup::AddStream(const CStream* _source, double _timeBeg, double _timeEnd)
{
	AddStream(_timeBeg, _timeEnd, _source);
}

void CHoldup::AddStream2(const CStream* _source, double _timeBeg, double _timeEnd)
{
	AddStream(_timeBeg, _timeEnd, _source);
}
