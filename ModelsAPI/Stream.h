/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BaseStream.h"

class CHoldup;

/**
* \brief Class describes the material flow.
*/
class CStream : public CBaseStream
{
	friend CHoldup;

public:
	/**
	 * \private
	 * \brief Basic constructor creating an empty material stream.
	 * \details Creates a material stream object with a unique key and a single overall property - mass.
	 * If unique key is empty, it is randomly generated.
	 * \param _key Unique key of the material stream.
	 */
	CStream(const std::string& _key = "");
	/**
	 * \private
	 * \brief Constructor configuring the whole structure.
	 * \details Creates a material stream object with a unique key and all structural settings.
	 * If unique key is empty, it is randomly generated.
	 * \param _key Unique key of the material stream.
	 * \param _materialsDB Pointer to materials database.
	 * \param _grid Multidimensional grid of distributed parameters of solids.
	 * \param _overall List of overall stream properties.
	 * \param _phases List of phases.
	 * \param _cache Cache settings.
	 * \param _tolerance Tolerance settings.
	 * \param _thermodynamics Thermodynamic settings.
	 */
	CStream(const std::string& _key, const CMaterialsDatabase* _materialsDB, const CMultidimensionalGrid& _grid,
		const std::vector<SOverallDescriptor>* _overall, const std::vector<SPhaseDescriptor>* _phases,
		const SCacheSettings* _cache, const SToleranceSettings* _tolerance, const SThermodynamicsSettings* _thermodynamics);
	/**
	 * \private
	 * \brief Copy constructor.
	 * \details Copies all structural parameters and settings from the source material stream, including unique key and data.
	 * \param _other Source material stream.
	 */
	CStream(const CBaseStream& _other);

	/**
	 * \brief Copies all data at the given time point from another material stream.
	 * \details All data after the target time point are removed from this material stream. Uses function CBaseStream::Copy(double, const CBaseStream&).
	 * \param _time Target time point to copy.
	 * \param _source Source material stream.
	 */
	void CopyFromStream(double _time, const CStream* _source);
	/**
	 * \brief Copies all data at the given time interval from another material stream.
	 * \details All data after the end time point are removed from this material stream. Uses function CBaseStream::Copy(double, double, const CBaseStream&).
	 * \param _timeBeg Begin of the time interval to copy.
	 * \param _timeEnd End of the time interval to copy.
	 * \param _source Source material stream.
	 */
	void CopyFromStream(double _timeBeg, double _timeEnd, const CStream* _source);
	/**
	 * \brief Copies all data from the given time point of another material stream to another time point of this material stream.
	 * \details All data after the time point are removed from this material stream. Uses function CBaseStream::Copy(double, const CBaseStream&, double).
	 * \param _timeDst Time point of the destination material stream to copy.
	 * \param _source Source material stream.
	 * \param _timeSrc Time point of the source material stream to copy.
	 */
	void CopyFromStream(double _timeDst, const CStream* _source, double _timeSrc);

	/**
	 * \brief Copies all data at the given time point from the holdup.
	 * \details All data after the time point are removed from this material stream.
	 * After data copying, sets new mass flow to the material stream.
	 * \param _time Target time point to copy.
	 * \param _source Source holdup.
	 * \param _massFlow Value of the overall mass flow.
	 */
	void CopyFromHoldup(double _time, const CHoldup* _source, double _massFlow);
	/**
	 * \brief Copies all data from the given time point of the holdup to another time point of this material stream.
	 * \details All data after the time point are removed from this material stream.
	 * After data copying, sets new mass flow to the material stream.
	 * \param _timeDst Time point of the destination material stream to copy.
	 * \param _source Source holdup.
	 * \param _timeSrc Time point of the source holdup to copy.
	 * \param _massFlow Value of the overall mass flow.
	 */
	void CopyFromHoldup(double _timeDst, const CHoldup* _source, double _timeSrc, double _massFlow);

	/**
	 * \brief Mixes the specified material stream with the current material stream at the given time point.
	 * \details Uses function CBaseStream::Add(double, const CBaseStream&).
	 * \param _time Target time point to copy.
	 * \param _source Source material stream.
	 */
	void AddStream(double _time, const CStream* _source);
	/**
	 * \brief Mixes the specified material stream with the current material stream for each time point from the given time interval.
	 * \details Uses function CBaseStream::Add(double, double, const CBaseStream&).
	 * \param _timeBeg Begin of the time interval.
	 * \param _timeEnd End of the time interval.
	 * \param _source Source material stream.
	 */
	void AddStream(double _timeBeg, double _timeEnd, const CStream* _source);

	/**
	 * \brief Returns the mass flow of the material stream in [kg/s] at the given time point.
	 * \details Uses CBaseStream::GetMass(double) const.
	 * \param _time Target time point.
	 * \return Mass flow.
	 */
	double GetMassFlow(double _time) const;
	/**
	 * \brief Returns the molar flow of the material stream in [mol/s] at the given time point.
	 * \details Uses function CBaseStream::GetMol(double) const.
	 * \param _time Target time point.
	 * \return Molar flow.
	 */
	double GetMolFlow(double _time) const;
	/**
	 * \brief Sets the mass flow of the material stream in [kg/s] at the given time point.
	 * \details Uses function CBaseStream::SetMass(double, double).
	 * \param _time Target time point.
	 * \param _value Value of the mass flow.
	 */
	void SetMassFlow(double _time, double _value);
	/**
	 * \brief Sets the molar flow of the material stream in [mol/s] at the given time point.
	 * \details Uses function CBaseStream::SetMol(double, double).
	 * \param _time Target time point.
	 * \param _value Value of the molar flow.
	 */
	void SetMolFlow(double _time, double _value);

	/**
	 * \brief Returns the mass flow of the compound in the specified phase at the given time point.
	 * \details Uses function CBaseStream::GetCompoundMass(double, const std::string&, EPhase) const.
	 * \param _time Target time point.
	 * \param _compoundKey Unique key of the compound.
	 * \param _phase Identifier of phase type.
	 * \return Mass flow of the compound.
	 */
	double GetCompoundMassFlow(double _time, const std::string& _compoundKey, EPhase _phase) const;
	/**
	 * \brief Returns the mass flow of the compound in the total mixture at the given time point.
	 * \details Uses function CBaseStream::GetCompoundMass(double, const std::string&) const.
	 * \param _time Target time point.
	 * \param _compoundKey Unique key of the compound.
	 * \return Mass flow of the compound.
	 */
	double GetCompoundMassFlow(double _time, const std::string& _compoundKey) const;
	/**
	 * \brief Returns the molar flow of the compound in the specified phase at the given time point.
	 * \details Uses function CBaseStream::GetCompoundMol(double, const std::string&, EPhase) const.
	 * \param _time Target time point.
	 * \param _compoundKey Unique key of the compound.
	 * \param _phase Identifier of phase type.
	 * \return Molar flow of the compound.
	 */
	double GetCompoundMolFlow(double _time, const std::string& _compoundKey, EPhase _phase) const;
	/**
	 * \brief Returns mass flows of all defined compounds at the given time point.
	 * \details Uses function CBaseStream::GetCompoundsMasses(double) const.
	 * \param _time Target time point.
	 * \return Mass flows of all defined compounds.
	 */
	std::vector<double> GetCompoundsMassFlows(double _time) const;
	/**
	 * \brief Returns mass flows of all defined compounds in the specified phase at the given time point.
	 * \details Uses function CBaseStream::GetCompoundsMasses(double, EPhase) const.
	 * \param _time Target time point.
	 * \param _phase Identifier of phase type.
	 * \return Mass flows of all defined compounds.
	 */
	std::vector<double> GetCompoundsMassFlows(double _time, EPhase _phase) const;

	/**
	 * \brief Returns mass flow of the specified phase at the given time point.
	 * \details Uses function CBaseStream::GetPhaseMass(double, EPhase) const.
	 * \param _time Target time point.
	 * \param _phase Identifier of phase type.
	 * \return Mass flow of the specified phase.
	 */
	double GetPhaseMassFlow(double _time, EPhase _phase) const;
	/**
	 * \brief Returns molar flow of the specified phase at the given time point.
	 * \details Uses function CBaseStream::GetPhaseMol(double, EPhase) const.
	 * \param _time Target time point.
	 * \param _phase Identifier of phase type.
	 * \return Molar flow of the specified phase.
	 */
	double GetPhaseMolFlow(double _time, EPhase _phase) const;
	/**
	 * \brief Sets mass flow of the specified phase at the given time point.
	 * \details Total mass flow of the stream is correspondingly adjusted, masses of other phases remain the same. Uses function CBaseStream::SetPhaseMass(double, EPhase, double).
	 * \param _time Target time point.
	 * \param _phase Identifier of phase type.
	 * \param _value Value of mass flow.
	 */
	void SetPhaseMassFlow(double _time, EPhase _phase, double _value);
	/**
	 * \brief Sets molar flow of the specified phase at the given time point.
	 * \details Total mass flow of the stream is correspondingly adjusted, masses of other phases remain the same. Uses function CBaseStream::SetPhaseMol(double, EPhase, double).
	 * \param _time Target time point.
	 * \param _phase Identifier of phase type.
	 * \param _value Molar flow of the specified phase.
	 */
	void SetPhaseMolFlow(double _time, EPhase _phase, double _value);

	// TODO: move it somewhere
	////////////////////////////////////////////////////////////////////////////////
	// Deprecated functions
	/**
	 * \private
	 */
	[[deprecated("WARNING! CopyFromStream(const CStream*, double) is deprecated. Use CopyFromStream(double, const CStream*) instead.")]]
	void CopyFromStream(const CStream* _source, double _time, bool _deleteDataAfter = true);
	/**
	 * \private
	 */
	[[deprecated("WARNING! CopyFromStream(const CStream*, double, double) is deprecated. Use CopyFromStream(double, double, const CStream*) instead.")]]
	void CopyFromStream(const CStream* _source, double _timeBeg, double _timeEnd);
	/**
	 * \private
	 */
	[[deprecated("WARNING! CopyFromHoldup(const CHoldup*, double, double, bool) is deprecated. Use CopyFromHoldup(double, const CHoldup*, double) instead.")]]
	void CopyFromHoldup(const CHoldup* _source, double _time, double _massFlow, bool _deleteDataAfter = true);
	/**
	 * \private
	 */
	[[deprecated("WARNING! AddStream(const CStream*, double) is deprecated. Use AddStream(double, const CStream*) instead.")]]
	void AddStream(const CStream* _source, double _time);
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetMassFlow(double, eValueBasises) is deprecated. Use GetMassFlow(double) or GetMolFlow(double) instead.")]]
	double GetMassFlow(double _time, unsigned _basis) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! SetMassFlow(double, _value, eValueBasises) is deprecated. Use SetMassFlow(double, double) or SetMolFlow(double, double) instead.")]]
	void SetMassFlow(double _time, double _value, unsigned _basis);
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetCompoundMassFlow(double, const std::string&, unsigned, eValueBasises) is deprecated. Use GetCompoundMassFlow(double, const std::string&, EPhase) or GetCompoundMolFlow(double, const std::string&, EPhase) instead.")]]
	double GetCompoundMassFlow(double _time, const std::string& _compound, unsigned _soa, unsigned _basis = BASIS_MASS) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetPhaseMassFlow(double, unsigned, eValueBasises) is deprecated. Use GetPhaseMassFlow(double, EPhase) or GetPhaseMolFlow(double, EPhase) instead.")]]
	double GetPhaseMassFlow(double _time, unsigned _soa, unsigned _basis = BASIS_MASS) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! SetPhaseMassFlow(double, unsigned, double, eValueBasises) is deprecated. Use SetPhaseMassFlow(double, EPhase, double) or SetPhaseMolFlow(double, EPhase, double) instead.")]]
	void SetPhaseMassFlow(double _time, unsigned _soa, double _value, unsigned _basis = BASIS_MASS);
};

using CMaterialStream = CStream;