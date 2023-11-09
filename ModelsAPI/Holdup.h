/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "Stream.h"

class CStream;

/**
* \brief Class describes the holdup material.
*/
class CHoldup : public CBaseStream
{
public:
	/**
	 * \private
	 * \brief Basic constructor creating an empty holdup.
	 * \details Creates a holdup object with a unique key and a single overall property - mass.
	 * If unique key is empty, it is randomly generated.
	 * \param _key Unique key of the holdup.
	 */
	CHoldup(const std::string& _key = "");
	/**
	 * \private
	 * \brief Constructor configuring the whole structure.
	 * \details Creates a holdup object with a unique key and all structural settings.
	 * If unique key is empty, it is randomly generated.
	 * \param _key Unique key of the holdup.
	 * \param _materialsDB Pointer to materials database.
	 * \param _grid Multidimensional grid of distributed parameters of solids.
	 * \param _overall List of overall stream properties.
	 * \param _phases List of phases.
	 * \param _cache Cache settings.
	 * \param _tolerance Tolerance settings.
	 * \param _thermodynamics Thermodynamic settings.
	 */
	CHoldup(const std::string& _key, const CMaterialsDatabase* _materialsDB, const CMultidimensionalGrid& _grid,
		const std::vector<SOverallDescriptor>* _overall, const std::vector<SPhaseDescriptor>* _phases,
		const SCacheSettings* _cache, const SToleranceSettings* _tolerance, const SThermodynamicsSettings* _thermodynamics);
	/**
	 * \private
	 * \brief Copy constructor.
	 * \details Copies all structural parameters and settings from the source holdup, including unique key and data.
	 * \param _other Source holdup.
	 */
	CHoldup(const CBaseStream& _other);

	/**
	 * \brief Copies all data at the given time point from another holdup.
	 * \details All data after the time point are removed from this holdup. Uses function CBaseStream::Copy(double, const CBaseStream&).
	 * \param _time Time point to copy.
	 * \param _source Source holdup.
	 */
	void CopyFromHoldup(double _time, const CHoldup* _source);
	/**
	 * \brief Copies all data at the given time interval from another holdup.
	 * \details All data after the end time point are removed from this holdup. Uses function CBaseStream::Copy(double, double, const CBaseStream&).
	 * \param _timeBeg Begin of the time interval to copy.
	 * \param _timeEnd End of the time interval to copy.
	 * \param _source Source holdup.
	 */
	void CopyFromHoldup(double _timeBeg, double _timeEnd, const CHoldup* _source);
	/**
	 * \brief Copies all data from the given time point of another holdup to another time point of this holdup.
	 * \details All data after the time point are removed from this holdup. Uses function CBaseStream::Copy(double, const CBaseStream&, double).
	 * \param _timeDst Time point of the destination holdup to copy.
	 * \param _source Source holdup.
	 * \param _timeSrc Time point of the source holdup to copy.
	 */
	void CopyFromHoldup(double _timeDst, const CHoldup* _source, double _timeSrc);

	/**
	 * \brief Mixes the content of the specified material stream at the given time interval with the holdup.
	 * \details Before mixing, all data after the end time point are removed.
	 * Takes into account only two given time points, regardless of whether they were defined in the material stream.
	 * All possible time points within the interval are discarded.
	 * \param _timeBeg Begin of the time interval.
	 * \param _timeEnd End of the time interval.
	 * \param _source Source material stream.
	 */
	void AddStream(double _timeBeg, double _timeEnd, const CStream* _source);

	/**
	 * \brief Mixes the specified holdup with the current holdup at the given time point.
	 * \details Uses function CBaseStream::Add(double, const CBaseStream&).
	 * \param _time Target time point.
	 * \param _source Source holdup.
	 */
	void AddHoldup(double _time, const CHoldup* _source);
	/**
	 * \brief Mixes the specified holdup with the current holdup for each time point from the given time interval.
	 * \details Uses function CBaseStream::Add(double, double, const CBaseStream&).
	 * \param _timeBeg Begin of the time interval.
	 * \param _timeEnd End of the time interval.
	 * \param _source Source holdup.
	 */
	void AddHoldup(double _timeBeg, double _timeEnd, const CHoldup* _source);

	// TODO: move it somewhere
	////////////////////////////////////////////////////////////////////////////////
	// Deprecated functions
	/**
	 * \private
	 */
	[[deprecated("WARNING! CopyFromHoldup(const CHoldup*, double) is deprecated. Use CopyFromHoldup(double, const CHoldup*) instead.")]]
	void CopyFromHoldup(const CHoldup* _source, double _time);
	/**
	 * \private
	 */
	[[deprecated("WARNING! AddHoldup(const CHoldup*, double) is deprecated. Use AddHoldup(double, const CHoldup*) instead.")]]
	void AddHoldup(const CHoldup* _source, double _time);
	/**
	 * \private
	 */
	[[deprecated("WARNING! AddStream(const CStream*, double, double) is deprecated. Use AddStream(double, const CStream*), AddStream(double, double, const CStream*) or AddStreamInterval(double, double, const CStream*) instead.")]]
	void AddStream(const CStream* _source, double _timeBeg, double _timeEnd);
	/**
	 * \private
	 */
	[[deprecated("WARNING! AddStream2(const CStream*, double, double) is deprecated. Use AddStream(double, double, const CStream*) instead.")]]
	void AddStream2(const CStream* _source, double _timeBeg, double _timeEnd);
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetMass(double, eValueBasises) is deprecated. Use GetMass(double) or GetMol(double) instead.")]]
	double GetMass(double _time, unsigned _basis) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! SetMass(double, double, eValueBasises) is deprecated. Use SetMass(double, double) or SetMol(double, double) instead.")]]
	void SetMass(double _time, double _value, unsigned _basis);
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetCompoundMass(double, const std::string&, EPhaseTypes, eValueBasises) is deprecated. Use GetCompoundMass(double, const std::string&, EPhase) or GetCompoundMol(double, const std::string&, EPhase) instead.")]]
	double GetCompoundMass(double _time, const std::string& _compoundKey, unsigned _soa, unsigned _basis = BASIS_MASS) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetPhaseMass(double, EPhaseTypes, eValueBasises) is deprecated. Use GetPhaseMass(double, EPhase) or GetPhaseMol(double, EPhase) instead.")]]
	double GetPhaseMass(double _time, unsigned _soa, unsigned _basis = BASIS_MASS) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! SetPhaseMass(double, EPhaseTypes, double, eValueBasises) is deprecated. Use SetPhaseMass(double, EPhase, double) or SetPhaseMol(double, EPhase, double) instead.")]]
	void SetPhaseMass(double _time, unsigned _soa, double _value, unsigned _basis = BASIS_MASS);

	// Is required, so that deprecated functions do not hide CBaseStream functions
	using CBaseStream::GetMass;
	using CBaseStream::SetMass;
	using CBaseStream::GetPhaseMass;
	using CBaseStream::SetPhaseMass;
	using CBaseStream::GetCompoundMass;
};
