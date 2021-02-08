/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BaseStream.h"

class CHoldup;

/* Class describes the material flow. */
class CStream : public CBaseStream
{
	friend CHoldup;

public:
	// Basic constructor.
	CStream(const std::string& _key = "");
	// Constructor configuring the whole structure.
	CStream(const std::string& _key, const CMaterialsDatabase* _materialsDB, const CDistributionsGrid* _grid,
		const std::vector<std::string>* _compounds, const std::vector<SOverallDescriptor>* _overall, const std::vector<SPhaseDescriptor>* _phases,
		const SCacheSettings* _cache, const SToleranceSettings* _tolerance, const SThermodynamicsSettings* _thermodynamics);
	// Copy constructor.
	CStream(const CBaseStream& _other);

	// Copies all data at the given time point from another stream. All data after the time point are removed from this stream.
	void CopyFromStream(double _time, const CStream* _source);
	// Copies all data at the given time interval from another stream. All data after the end time point are removed from this stream.
	void CopyFromStream(double _timeBeg, double _timeEnd, const CStream* _source);
	// Copies all data from the given time point of another stream to another time point of this stream. All data after the time point are removed from this stream.
	void CopyFromStream(double _timeDst, const CStream* _source, double _timeSrc);

	// Copies all data at the given time point from the holdup. All data after the time point are removed from this stream.
	void CopyFromHoldup(double _time, const CHoldup* _source, double _massFlow);
	// Copies all data from the given time point of the holdup to another time point of this stream. All data after the time point are removed from this stream.
	void CopyFromHoldup(double _timeDst, const CHoldup* _source, double _timeSrc, double _massFlow);

	// Mixes the specified stream with the current stream at the given time point.
	void AddStream(double _time, const CStream* _source);
	// Mixes the specified stream with the current stream for each time point from the given time interval.
	void AddStream(double _timeBeg, double _timeEnd, const CStream* _source);

	// Returns the mass flow of the stream at the given time point.
	double GetMassFlow(double _time) const;
	// Returns the molar flow of the stream at the given time point.
	double GetMolFlow(double _time) const;
	// Sets the mass flow of the stream at the given time point.
	void SetMassFlow(double _time, double _value);
	// Sets the molar flow of the stream at the given time point.
	void SetMolFlow(double _time, double _value);

	// Returns the mass flow of the compound in the specified phase at the given time point.
	double GetCompoundMassFlow(double _time, const std::string& _compoundKey, EPhase _phase) const;
	// Returns the mass flow of the compound in the total mixture at the given time point.
	double GetCompoundMassFlow(double _time, const std::string& _compoundKey) const;
	// Returns the molar flow of the compound in the specified phase at the given time point.
	double GetCompoundMolFlow(double _time, const std::string& _compoundKey, EPhase _phase) const;

	// Returns the mass flow of the specified phase at the given time point.
	double GetPhaseMassFlow(double _time, EPhase _phase) const;
	// Returns the molar flow of the specified phase at the given time point.
	double GetPhaseMolFlow(double _time, EPhase _phase) const;
	// Sets the mass flow of the specified phase at the given time point. Total mass flow of the stream is correspondingly adjusted, masses of other phases remain the same.
	void SetPhaseMassFlow(double _time, EPhase _phase, double _value);
	// Sets the molar flow of the specified phase at the given time point. Total mass flow of the stream is correspondingly adjusted, masses of other phases remain the same.
	void SetPhaseMolFlow(double _time, EPhase _phase, double _value);

	// TODO: move it somewhere
	////////////////////////////////////////////////////////////////////////////////
	/// Deprecated functions
	[[deprecated("WARNING! CopyFromStream(const CStream*, double) is deprecated. Use CopyFromStream(double, const CStream*) instead.")]]
	void CopyFromStream(const CStream* _source, double _time);
	[[deprecated("WARNING! CopyFromStream(const CStream*, double, double) is deprecated. Use CopyFromStream(double, double, const CStream*) instead.")]]
	void CopyFromStream(const CStream* _source, double _timeBeg, double _timeEnd);
	[[deprecated("WARNING! CopyFromHoldup(const CHoldup*, double, double, bool) is deprecated. Use CopyFromHoldup(double, double, const CHoldup*) instead.")]]
	void CopyFromHoldup(const CHoldup* _source, double _time, double _massFlow, bool _deleteDataAfter = true);
	[[deprecated("WARNING! AddStream(const CStream*, double) is deprecated. Use AddStream(double, const CStream*) instead.")]]
	void AddStream(const CStream* _source, double _time);
	[[deprecated("WARNING! GetMassFlow(double, eValueBasises) is deprecated. Use GetMassFlow(double) or GetMolFlow(double) instead.")]]
	double GetMassFlow(double _time, unsigned _basis) const;
	[[deprecated("WARNING! SetMassFlow(double, _value, eValueBasises) is deprecated. Use SetMassFlow(double, double) or SetMolFlow(double, double) instead.")]]
	void SetMassFlow(double _time, double _value, unsigned _basis);
	[[deprecated("WARNING! GetCompoundMassFlow(double, const std::string&, unsigned, eValueBasises) is deprecated. Use GetCompoundMassFlow(double, const std::string&, EPhase) or GetCompoundMolFlow(double, const std::string&, EPhase) instead.")]]
	double GetCompoundMassFlow(double _time, const std::string& _compound, unsigned _soa, unsigned _basis = BASIS_MASS) const;
	[[deprecated("WARNING! GetPhaseMassFlow(double, unsigned, eValueBasises) is deprecated. Use GetPhaseMassFlow(double, EPhase) or GetPhaseMolFlow(double, EPhase) instead.")]]
	double GetPhaseMassFlow(double _time, unsigned _soa, unsigned _basis = BASIS_MASS) const;
	[[deprecated("WARNING! SetPhaseMassFlow(double, unsigned, double, eValueBasises) is deprecated. Use SetPhaseMassFlow(double, EPhase, double) or SetPhaseMolFlow(double, EPhase, double) instead.")]]
	void SetPhaseMassFlow(double _time, unsigned _soa, double _value, unsigned _basis = BASIS_MASS);
};

using CMaterialStream = CStream;