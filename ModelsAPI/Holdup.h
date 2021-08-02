/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "Stream.h"

class CStream;

class CHoldup : public CBaseStream
{
public:
	// Basic constructor.
	CHoldup(const std::string& _key = "");
	// Constructor configuring the whole structure.
	CHoldup(const std::string& _key, const CMaterialsDatabase* _materialsDB, const CDistributionsGrid* _grid,
		const std::vector<SOverallDescriptor>* _overall, const std::vector<SPhaseDescriptor>* _phases,
		const SCacheSettings* _cache, const SToleranceSettings* _tolerance, const SThermodynamicsSettings* _thermodynamics);
	// Copy constructor.
	CHoldup(const CBaseStream& _other);

	// Copies all data at the given time point from another holdup. All data after the time point are removed from this holdup.
	void CopyFromHoldup(double _time, const CHoldup* _source);
	// Copies all data at the given time interval from another holdup. All data after the end time point are removed from this holdup.
	void CopyFromHoldup(double _timeBeg, double _timeEnd, const CHoldup* _source);
	// Copies all data from the given time point of another holdup to another time point of this stream. All data after the time point are removed from this holdup.
	void CopyFromHoldup(double _timeDst, const CHoldup* _source, double _timeSrc);

	/* Mixes the content of the specified stream at the given time interval with the holdup. Before mixing, all data after the end time point are removed.
	 * Takes into account only two given time points, regardless of whether they were defined in the stream. All possible time points within the interval are discarded.*/
	void AddStream(double _timeBeg, double _timeEnd, const CStream* _source);

	// Mixes the specified holdup with the current holdup at the given time point.
	void AddHoldup(double _time, const CHoldup* _source);
	// Mixes the specified holdup with the current holdup for each time point from the given time interval.
	void AddHoldup(double _timeBeg, double _timeEnd, const CHoldup* _source);

	// TODO: move it somewhere
	////////////////////////////////////////////////////////////////////////////////
	/// Deprecated functions
	[[deprecated("WARNING! CopyFromHoldup(const CHoldup*, double) is deprecated. Use CopyFromHoldup(double, const CHoldup*) instead.")]]
	void CopyFromHoldup(const CHoldup* _source, double _time);
	[[deprecated("WARNING! AddHoldup(const CHoldup*, double) is deprecated. Use AddHoldup(double, const CHoldup*) instead.")]]
	void AddHoldup(const CHoldup* _source, double _time);
	[[deprecated("WARNING! AddStream(const CStream*, double, double) is deprecated. Use AddStream(double, const CStream*), AddStream(double, double, const CStream*) or AddStreamInterval(double, double, const CStream*) instead.")]]
	void AddStream(const CStream* _source, double _timeBeg, double _timeEnd);
	[[deprecated("WARNING! AddStream2(const CStream*, double, double) is deprecated. Use AddStream(double, double, const CStream*) instead.")]]
	void AddStream2(const CStream* _source, double _timeBeg, double _timeEnd);
	[[deprecated("WARNING! GetMass(double, eValueBasises) is deprecated. Use GetMass(double) or GetMol(double) instead.")]]
	double GetMass(double _time, unsigned _basis) const;
	[[deprecated("WARNING! SetMass(double, double, eValueBasises) is deprecated. Use SetMass(double, double) or SetMol(double, double) instead.")]]
	void SetMass(double _time, double _value, unsigned _basis);
	[[deprecated("WARNING! GetCompoundMass(double, const std::string&, EPhaseTypes, eValueBasises) is deprecated. Use GetCompoundMass(double, const std::string&, EPhase) or GetCompoundMol(double, const std::string&, EPhase) instead.")]]
	double GetCompoundMass(double _time, const std::string& _compoundKey, unsigned _soa, unsigned _basis = BASIS_MASS) const;
	[[deprecated("WARNING! GetPhaseMass(double, EPhaseTypes, eValueBasises) is deprecated. Use GetPhaseMass(double, EPhase) or GetPhaseMol(double, EPhase) instead.")]]
	double GetPhaseMass(double _time, unsigned _soa, unsigned _basis = BASIS_MASS) const;
	[[deprecated("WARNING! SetPhaseMass(double, EPhaseTypes, double, eValueBasises) is deprecated. Use SetPhaseMass(double, EPhase, double) or SetPhaseMol(double, EPhase, double) instead.")]]
	void SetPhaseMass(double _time, unsigned _soa, double _value, unsigned _basis = BASIS_MASS);

	// Is required, so that deprecated functions do not hide CBaseStream functions
	using CBaseStream::GetMass;
	using CBaseStream::SetMass;
	using CBaseStream::GetPhaseMass;
	using CBaseStream::SetPhaseMass;
	using CBaseStream::GetCompoundMass;
};
