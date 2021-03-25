/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolTypes.h"
#include "DyssolDefines.h"
#include "DefinesMDB.h"
#include "MixtureEnthalpyLookup.h"
#include <limits>
#include <memory>

class CH5Handler;
class CDenseMDMatrix;
class CPhase;
class CTimeDependentValue;
class CMaterialsDatabase;
class CDistributionsGrid;
class CMatrix2D;
class CTransformMatrix;

class CBaseStream;
class CStream;
class CHoldup;

/* Base class for material flow description.*/
class CBaseStream
{
	static const unsigned m_saveVersion{ 2 }; // Current version of the saving procedure.

	std::string m_name{ "Stream" };	// Name of the stream.
	std::string m_key;				// Unique key of the stream.

protected:
	// TODO: make in global and rename
	inline static const double m_epsilon{ 16 * std::numeric_limits<double>::epsilon() };

	const CMaterialsDatabase* m_materialsDB{ nullptr };	// Pointer to a database of materials.
	const CDistributionsGrid* m_grid{ nullptr };		// Pointer to a distribution grid.

	std::vector<double> m_timePoints;											// Time points on which the stream is defined.
	std::map<EOverall, std::unique_ptr<CTimeDependentValue>> m_overall;			// Defined overall properties.
	std::map<EPhase, std::unique_ptr<CPhase>> m_phases;							// Defined phases.
	std::vector<std::string> m_compounds;										// Keys of chemical compounds defined in this stream.
	mutable std::unique_ptr<CMixtureEnthalpyLookup> m_enthalpyCalculator;		// Lookup table to calculate temperature<->enthalpy.
	SCacheSettings m_cacheSettings;												// Settings for caching in the stream.
	SToleranceSettings m_toleranceSettings;										// Settings for tolerances in the stream.
	SThermodynamicsSettings m_thermodynamicsSettings;							// Settings for thermodynamics in the stream.

public:
	// TODO: remove empty constructor, always set pointers to MDB and grid.
	// Basic constructor.
	CBaseStream(const std::string& _key = "");
	// Constructor configuring the whole structure.
	CBaseStream(const std::string& _key, const CMaterialsDatabase* _materialsDB, const CDistributionsGrid* _grid,
		const std::vector<std::string>* _compounds, const std::vector<SOverallDescriptor>* _overall, const std::vector<SPhaseDescriptor>* _phases,
		const SCacheSettings* _cache, const SToleranceSettings* _tolerance, const SThermodynamicsSettings* _thermodynamics);
	// Copy constructor.
	CBaseStream(const CBaseStream& _other);
	virtual ~CBaseStream() = default;

	CBaseStream(CBaseStream&& _other) = delete;
	CBaseStream& operator=(const CBaseStream& _other) = delete;
	CBaseStream& operator=(CBaseStream&& _other) = delete;

	// Removes all existing data from the stream.
	void Clear();

	// Sets up the stream structure (MD dimensions, phases, materials, etc.) the same as an in the given stream. Removes all existing data.
	void SetupStructure(const CBaseStream* _other);

	// Checks whether both streams have the same structure (phases, dimensions, etc.).
	static bool HaveSameStructure(const CBaseStream& _stream1, const CBaseStream& _stream2);

	////////////////////////////////////////////////////////////////////////////////
	// Basic stream properties
	//

	// Returns name of the stream.
	std::string GetName() const;
	// Sets new name of the stream.
	void SetName(const std::string& _name);
	// Returns unique key of the stream.
	std::string GetKey() const;
	// Sets new unique key of the stream.
	void SetKey(const std::string& _key);

	////////////////////////////////////////////////////////////////////////////////
	// Time points
	//

	// Adds a new temp point _time if it doesn't already exist.
	void AddTimePoint(double _time);
	// Adds a new temp point _timeDst if it doesn't already exist and fills it with the data of existing time point _timeSrc.
	void CopyTimePoint(double _timeDst, double _timeSrc);
	// Removes the specified time point if it does already exist.
	void RemoveTimePoint(double _time);
	// Removes all existing time points in the specified interval.
	void RemoveTimePoints(double _timeBeg, double _timeEnd);
	// Removes all existing time points after the specified one, inclusive or exclusive _time.
	void RemoveTimePointsAfter(double _time, bool _inclusive = false);
	// Removes all existing time points.
	void RemoveAllTimePoints();
	// Removes time points within the specified interval [timeBeg; timeEnd) that are closer together than step.
	void ReduceTimePoints(double _timeBeg, double _timeEnd, double _step);
	// Returns all defined time points.
	std::vector<double> GetAllTimePoints() const;
	// Returns all defined time points in the specified time interval.
	std::vector<double> GetTimePoints(double _timeBeg, double _timeEnd) const;
	// Returns all defined time points in the specified closed time interval, boundaries are unconditionally included into result.
	std::vector<double> GetTimePointsClosed(double _timeBeg, double _timeEnd) const;
	// Returns the last (largest) defined time point.
	double GetLastTimePoint() const;
	// Returns the nearest time point before _time, or zero if such time can not be found.
	double GetPreviousTimePoint(double _time) const;

	////////////////////////////////////////////////////////////////////////////////
	// Overall parameters
	//

	// Adds new overall property to the stream, if it does not exist yet and returns a pointer to it or already existing property.
	CTimeDependentValue* AddOverallProperty(EOverall _property, const std::string& _name, const std::string& _units);
	// Removes an overall property from the stream, if it does exist.
	void RemoveOverallProperty(EOverall _property);
	// TODO: maybe remove
	// Returns a pointer to an overall property or nullptr if such property doesn't exist
	CTimeDependentValue* GetOverallProperty(EOverall _property);
	// TODO: maybe remove
	// Returns a pointer to an overall property or nullptr if such property doesn't exist
	const CTimeDependentValue* GetOverallProperty(EOverall _property) const;

	// Returns a value of the specified overall property at the given time point. Returns default value if such overall property has not been defined.
	double GetOverallProperty(double _time, EOverall _property) const;
	// Returns a value of the overall mass at the given time point.
	double GetMass(double _time) const;
	// Returns a value of the overall temperature at the given time point. Returns standard condition temperature if temperature overall property has not been defined.
	double GetTemperature(double _time) const;
	// Returns a value of the overall pressure at the given time point. Returns standard condition pressure if pressure overall property has not been defined.
	double GetPressure(double _time) const;

	// Sets a value of the specified overall property at the given time point if such property exists.
	void SetOverallProperty(double _time, EOverall _property, double _value);
	// Sets a value of the overall mass at the given time point.
	void SetMass(double _time, double _value);
	// Sets a value of the overall temperature at the given time point if such property exists.
	void SetTemperature(double _time, double _value);
	// Sets a value of the overall pressure at the given time point if such property exists.
	void SetPressure(double _time, double _value);

	// Returns a value of the overall amount of substance at the given time point.
	double GetMol(double _time) const;
	// Sets a value of the overall amount of substance at the given time point.
	void SetMol(double _time, double _value);

	////////////////////////////////////////////////////////////////////////////////
	// Compounds
	//

	// Adds a compound with the specified unique key to the stream.
	void AddCompound(const std::string& _compoundKey);
	// Removes a compound with the specified unique key from the stream.
	void RemoveCompound(const std::string& _compoundKey);
	// TODO: remove this.
	// Removes all defined compounds.
	void ClearCompounds();
	// Returns all defined materials.
	std::vector<std::string> GetAllCompounds() const;

	// Returns the mass fraction of the compound in the total mixture at the given time point.
	double GetCompoundFraction(double _time, const std::string& _compoundKey) const;
	// Returns the mass fraction of the compound in the specified phase at the given time point.
	double GetCompoundFraction(double _time, const std::string& _compoundKey, EPhase _phase) const;
	// Returns the mass of the compound in the total mixture at the given time point.
	double GetCompoundMass(double _time, const std::string& _compoundKey) const;
	// Returns the mass of the compound in the specified phase at the given time point.
	double GetCompoundMass(double _time, const std::string& _compoundKey, EPhase _phase) const;
	// Returns mass fraction of all defined compounds at the given time point.
	std::vector<double> GetCompoundsFractions(double _time) const;
	// Returns mass fraction of all defined compounds in the specified phase at the given time point.
	std::vector<double> GetCompoundsFractions(double _time, EPhase _phase) const;

	// Sets the mass fraction of the compound in the specified phase at the given time point.
	void SetCompoundFraction(double _time, const std::string& _compoundKey, EPhase _phase, double _value);
	// Sets mass fraction of all defined compounds in all defined phases at the given time point.
	void SetCompoundsFractions(double _time, const std::vector<double>& _value);
	// Sets mass fraction of all defined compounds in the specified phase at the given time point.
	void SetCompoundsFractions(double _time, EPhase _phase, const std::vector<double>& _value);

	// Returns the molar fraction of the compound in the specified phase at the given time point.
	double GetCompoundMolFraction(double _time, const std::string& _compoundKey, EPhase _phase) const;
	// Returns the amount of substance of the compound in the specified phase at the given time point.
	double GetCompoundMol(double _time, const std::string& _compoundKey, EPhase _phase) const;
	// Sets the molar fraction of the compound in the specified phase at the given time point.
	void SetCompoundMolFraction(double _time, const std::string& _compoundKey, EPhase _phase, double _value);

	////////////////////////////////////////////////////////////////////////////////
	// Phases
	//

	// Adds the specified phase to the stream, if it does not exist yet and returns a pointer to it or already existing phase.
	CPhase* AddPhase(EPhase _phase, const std::string& _name);
	// Removes the specified phase from the stream.
	void RemovePhase(EPhase _phase);
	// TODO: maybe remove
	// Returns a pointer to a phase or nullptr if such phase doesn't exist
	CPhase* GetPhase(EPhase _phase);
	// TODO: maybe remove
	// Returns a pointer to a phase or nullptr if such phase doesn't exist
	const CPhase* GetPhase(EPhase _phase) const;
	// TODO: remove this.
	// Removes all defined phases.
	void ClearPhases();

	// Returns the mass fraction of the specified phase at the given time point.
	double GetPhaseFraction(double _time, EPhase _phase) const;
	// Returns the mass of the specified phase at the given time point.
	double GetPhaseMass(double _time, EPhase _phase) const;
	// Returns the value of the overall property of the specified phase at the given time point.
	double GetPhaseProperty(double _time, EPhase _phase, EOverall _property) const;
	// Returns the value of the constant physical property (MOLAR_MASS) of the specified phase at the given time point.
	double GetPhaseProperty(double _time, EPhase _phase, ECompoundConstProperties _property) const;
	// Returns the value of the temperature/pressure-dependent physical property (DENSITY, ENTHALPY, etc) of the specified phase at the given time point.
	double GetPhaseProperty(double _time, EPhase _phase, ECompoundTPProperties _property) const;

	// Sets the mass fraction of the specified phase at the given time point.
	void SetPhaseFraction(double _time, EPhase _phase, double _value);
	// Sets the mass of the specified phase at the given time point. Total mass of the stream is correspondingly adjusted, masses of other phases remain the same.
	void SetPhaseMass(double _time, EPhase _phase, double _value);

	// Returns the mass fraction of the specified phase at the given time point.
	double GetPhaseMolFraction(double _time, EPhase _phase) const;
	// Returns the amount of substance of the specified phase at the given time point.
	double GetPhaseMol(double _time, EPhase _phase) const;
	// Sets the molar fraction of the specified phase at the given time point.
	void SetPhaseMolFraction(double _time, EPhase _phase, double _value);
	// Sets the amount of substance of the specified phase at the given time point. Total mass of the stream is correspondingly adjusted, masses of other phases remain the same.
	void SetPhaseMol(double _time, EPhase _phase, double _value);

	////////////////////////////////////////////////////////////////////////////////
	// Properties of the total mixture
	//

	// Returns the value of the property of the total mixture in the stream at the given time point.
	double GetMixtureProperty(double _time, EOverall _property) const;
	// Returns the value of the constant physical property (MOLAR_MASS) of the total mixture in the stream at the given time point.
	double GetMixtureProperty(double _time, ECompoundConstProperties _property) const;
	// Returns the value of the temperature/pressure-dependent physical property (DENSITY, ENTHALPY, etc) of the total mixture in the stream at the given time point.
	double GetMixtureProperty(double _time, ECompoundTPProperties _property) const;

	// Sets the value of the property of the total mixture in the stream at the given time point.
	void SetMixtureProperty(double _time, EOverall _property, double _value);

	////////////////////////////////////////////////////////////////////////////////
	// Material database
	//

	// Returns the value of the constant physical property (CRITICAL_TEMPERATURE, MOLAR_MASS, etc) of the specified compound.
	double GetCompoundProperty(const std::string& _compoundKey, ECompoundConstProperties _property) const;
	// Returns the value of the temperature/pressure-dependent physical property (DENSITY, ENTHALPY, etc) of the specified compound with the given temperature [K] and pressure [Pa].
	double GetCompoundProperty(const std::string& _compoundKey, ECompoundTPProperties _property, double _temperature, double _pressure) const;
	// Returns the value of the temperature/pressure-dependent physical property (DENSITY, ENTHALPY, etc) of the specified compound at temperature and pressure at the given time point.
	double GetCompoundProperty(double _time, const std::string& _compoundKey, ECompoundTPProperties _property) const;
	// Returns the value of the interaction physical property (INTERFACE_TENSION, etc) between the specified compounds with the given specified temperature [K] and pressure [Pa].
	double GetCompoundProperty(const std::string& _compoundKey1, const std::string& _compoundKey2, EInteractionProperties _property, double _temperature, double _pressure) const;
	// Returns the value of the interaction physical property (INTERFACE_TENSION, etc) between the specified compounds at temperature and pressure at the given time point.
	double GetCompoundProperty(double _time, const std::string& _compoundKey1, const std::string& _compoundKey2, EInteractionProperties _property) const;

	////////////////////////////////////////////////////////////////////////////////
	// Distributed properties of the solid phase
	//

	// Returns the mass fraction of a solid material at the specified multidimensional coordinates given for all defined dimensions.
	double GetFraction(double _time, const std::vector<size_t>& _coords) const;
	// Sets the mass fraction of a solid material at the specified multidimensional coordinates given for all defined dimensions.
	void SetFraction(double _time, const std::vector<size_t>& _coords, double _value);

	// Returns the one-dimensional distribution of a solid material over the specified parameter for the given time point.
	std::vector<double> GetDistribution(double _time, EDistrTypes _distribution) const;
	// Returns the two-dimensional distribution of a solid material over the specified parameters for the given time point.
	CMatrix2D GetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2) const;
	// Returns the multi-dimensional distribution of a solid material over the specified parameters for the given time point.
	CDenseMDMatrix GetDistribution(double _time, const std::vector<EDistrTypes>& _distributions) const;
	// Returns the one-dimensional distribution of a solid material over the specified parameter for the given compound and time point.
	std::vector<double> GetDistribution(double _time, EDistrTypes _distribution, const std::string& _compoundKey) const;
	// Returns the two-dimensional distribution of a solid material over the specified parameters for the given compound and time point.
	CMatrix2D GetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2, const std::string& _compoundKey) const;
	// Returns the multi-dimensional distribution of a solid material over the specified parameters for the given compound and time point.
	CDenseMDMatrix GetDistribution(double _time, const std::vector<EDistrTypes>& _distributions, const std::string& _compoundKey) const;

	// Sets the one-dimensional distribution of a solid material over the specified parameter for the given time point.
	void SetDistribution(double _time, EDistrTypes _distribution, const std::vector<double>& _value);
	// Sets the two-dimensional distribution of a solid material over the specified parameter for the given time point.
	void SetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2, const CMatrix2D& _value);
	// Sets the multi-dimensional distribution of a solid material over the specified parameter for the given time point.
	void SetDistribution(double _time, const CDenseMDMatrix& _value);
	// Sets the one-dimensional distribution of a solid material over the specified parameter for the given compound and time point.
	void SetDistribution(double _time, EDistrTypes _distribution, const std::string& _compoundKey, const std::vector<double>& _value);
	// Sets the two-dimensional distribution of a solid material over the specified parameters for the given compound and time point.
	void SetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2, const std::string& _compoundKey, const CMatrix2D& _value);
	// Sets the multi-dimensional distribution of a solid material over the specified parameters for the given compound and time point.
	void SetDistribution(double _time, const std::string& _compoundKey, const CDenseMDMatrix& _value);

	// Applies the movement matrix to transform the multidimensional distributed parameters of a solid material for the given time point.
	void ApplyTM(double _time, const CTransformMatrix& _matrix);
	// Applies the movement matrix to transform the multidimensional distributed parameters of a solid material for the given compound and time point.
	void ApplyTM(double _time, const std::string& _compoundKey, const CTransformMatrix& _matrix);

	// Normalizes data in the solids distribution matrix for the given time point.
	void Normalize(double _time);
	// Normalizes data in the solids distribution matrix for the given time interval.
	void Normalize(double _timeBeg, double _timeEnd);
	// Normalizes data in the solids distribution matrix for all time points.
	void Normalize();

	////////////////////////////////////////////////////////////////////////////////
	// Particle size distributions
	//

	// Returns the specified type of the PSD of a total mixture of all solid materials for the given time point.
	std::vector<double> GetPSD(double _time, EPSDTypes _type, EPSDGridType _grid = EPSDGridType::DIAMETER) const;
	// Returns the specified type of the PSD of a selected compound for the given time point.
	std::vector<double> GetPSD(double _time, EPSDTypes _type, const std::string& _compoundKey, EPSDGridType _grid = EPSDGridType::DIAMETER) const;
	// Returns the specified type of the PSD of a mixture of selected compounds for the given time point. If the list of compounds is empty, the whole mixture is considered.
	std::vector<double> GetPSD(double _time, EPSDTypes _type, const std::vector<std::string>& _compoundKeys, EPSDGridType _grid = EPSDGridType::DIAMETER) const;

	/* Sets the specified type of the PSD of a total mixture of all solid materials for the given time point.
	 * For number-related PSD, the distribution is normalized and the total particle mass remains unchanged.*/
	void SetPSD(double _time, EPSDTypes _type, const std::vector<double>& _value, EPSDGridType _grid = EPSDGridType::DIAMETER);
	/* Sets the specified type of the PSD of a selected compound for the given time point.
	 * For number-related PSD, the distribution is normalized and the total particle mass remains unchanged. If the compound key if empty, the whole mixture is considered.*/
	void SetPSD(double _time, EPSDTypes _type, const std::string& _compoundKey, const std::vector<double>& _value, EPSDGridType _grid = EPSDGridType::DIAMETER);

	////////////////////////////////////////////////////////////////////////////////
	// Interactions with other streams
	//

	// Copies all stream data at the given time point. All data after the time point are removed from the stream.
	void Copy(double _time, const CBaseStream& _source);
	// Copies all stream data at the given time interval. All data after the end time point are removed from the stream.
	void Copy(double _timeBeg, double _timeEnd, const CBaseStream& _source);
	// Copies all stream data to the given time point from another time point of source stream. All data after the time point are removed from the stream.
	void Copy(double _timeDst, const CBaseStream& _source, double _timeSrc);

	// Mixes the specified stream with the current stream at the given time point. Can be applied only for streams with the same structure (MD dimensions, phases, materials, etc.).
	void Add(double _time, const CBaseStream& _source);
	/* Mixes the specified stream with the current stream at the given time interval. Can be applied only for streams with the same structure (MD dimensions, phases, materials, etc.).
	 * The stream will contain the union of time points from both streams.*/
	void Add(double _timeBeg, double _timeEnd, const CBaseStream& _source);

	// Checks whether all values in the streams at the given time point are equal accurate to the specified tolerances.
	static bool AreEqual(double _time, const CBaseStream& _stream1, const CBaseStream& _stream2);

	////////////////////////////////////////////////////////////////////////////////
	// Thermodynamics
	//

	// Returns a pointer to enthalpy calculator.
	[[nodiscard]] CMixtureEnthalpyLookup* GetEnthalpyCalculator() const;

	// Calculates enthalpy of the stream mixture for the temperature at the given time point using a lookup table.
	[[nodiscard]] double CalculateEnthalpyFromTemperature(double _time) const;
	// Calculates temperature of the stream mixture for the enthalpy at the given time point using a lookup table.
	[[nodiscard]] double CalculateTemperatureFromEnthalpy(double _time) const;

	////////////////////////////////////////////////////////////////////////////////
	// Other
	//

	// TODO: remove, initialize MDB in constructor
	// Sets pointer to the used materials database.
	void SetMaterialsDatabase(const CMaterialsDatabase* _database);

	// TODO: remove, initialize grid in constructor
	// Sets pointer to the used distributions grid.
	void SetGrid(const CDistributionsGrid* _grid);
	// Updates grids of distributed parameters.
	void UpdateDistributionsGrid();

	// Sets new cache settings.
	void SetCacheSettings(const SCacheSettings& _settings);

	// Sets tolerance settings.
	void SetToleranceSettings(const SToleranceSettings& _settings);

	// Sets thermodynamics settings.
	void SetThermodynamicsSettings(const SThermodynamicsSettings& _settings);

	// Performs nearest-neighbor extrapolation of all stream data.
	void Extrapolate(double _timeExtra, double _time);
	// Performs linear extrapolation of all stream data.
	void Extrapolate(double _timeExtra, double _time1, double _time2);
	// Performs cubic spline extrapolation of all stream data.
	void Extrapolate(double _timeExtra, double _time1, double _time2, double _time3);

	// Saves data to file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _path);
	// Loads data from file.
	void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);
	// Loads data from file of an older version.
	void LoadFromFile_v1(const CH5Handler& _h5File, const std::string& _path);

protected:
	using mix_type = std::tuple<std::map<EOverall, double>, std::map<EPhase, double>, std::map<EPhase, CDenseMDMatrix>>;
	// Calculates the mixture of two streams. Does not perform any checks.
	static mix_type CalculateMix(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2);
	// Sets the result of mixing two streams into the specified stream at the given time point.
	static void SetMix(CBaseStream& _stream, double _time, const mix_type& _data);
	// Calculates the pressure of the mixture of two streams. Does not perform any checks.
	static double CalculateMixPressure(double _time1, const CBaseStream& _stream1, double _time2, const CBaseStream& _stream2);
	// Calculates the temperature of the mixture of two streams. Does not perform any checks.
	static double CalculateMixTemperature(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2);
	// Calculates the general overall property of the mixture of two streams. Does not perform any checks.
	static double CalculateMixOverall(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2, EOverall _property);
	// Calculates the phase fractions of the mixture of two streams for the given phase. Does not perform any checks.
	static double CalculateMixPhaseFractions(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2, EPhase _phase);
	// Calculates the multidimensional distributions of the mixture of two streams for the given phase. Does not perform any checks.
	static CDenseMDMatrix CalculateMixDistribution(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2, EPhase _phase);

private:
	// Inserts the new time into the list of time points, it it does not exist yet.
	void InsertTimePoint(double _time);
	// Checks whether the given time point exists.
	bool HasTime(double _time) const;

	// Checks whether the specified overall property is defined in the stream.
	bool HasOverallProperty(EOverall _property) const;

	// Checks whether the specified compound is defined in the stream.
	bool HasCompound(const std::string& _compoundKey) const;
	// Checks whether all the specified compounds are defined in the stream.
	bool HasCompounds(const std::vector<std::string>& _compoundKeys) const;
	// Returns index of the compound.
	size_t CompoundIndex(const std::string& _compoundKey) const;

	// Checks whether the specified phase is defined in the stream.
	bool HasPhase(EPhase _phase) const;

	/* Calculates the PSD of the stream in mass fractions for the selected compounds. If the list of components is empty, calculates the PSD for the entire mixture.
	 * Takes into account porosity, if specified. All checks of parameters, phases, grids availability, etc. must be executed by the calling code.*/
	std::vector<double> GetPSDMassFraction(double _time, const std::vector<std::string>& _compoundKeys) const;
	/* Calculates the number particle distribution of the stream for the selected compounds. If the list of components is empty, calculates the PSD for the entire mixture.
	 * Takes into account porosity, if specified. All checks of parameters, phases, grids availability, etc. must be executed by the calling code.*/
	std::vector<double> GetPSDNumber(double _time, const std::vector<std::string>& _compoundKeys, EPSDGridType _grid) const;

	// Clears enthalpy calculator.
	void ClearEnthalpyCalculator();

	// TODO: move it somewhere
	////////////////////////////////////////////////////////////////////////////////
	/// Deprecated functions
public:
	[[deprecated("WARNING! AddTimePoint(double, double) is deprecated. Use CopyTimePoint(double, double) instead.")]]
	void AddTimePoint(double _timeDst, double _timeSrc);
	[[deprecated("WARNING! GetStreamName() is deprecated. Use GetName() instead.")]]
	std::string GetStreamName() const;
	[[deprecated("WARNING! SetupStream(const CBaseStream*) is deprecated. Use SetupStructure(const CBaseStream*) instead.")]]
	void SetupStream(const CBaseStream* _stream);
	[[deprecated("WARNING! GetTimePointsForInterval(double, double, bool) is deprecated. Use GetTimePoints(double, double) or GetTimePointsClosed(double, double) instead.")]]
	std::vector<double> GetTimePointsForInterval(double _timeBeg, double _timeEnd, bool _inclusive) const;
	[[deprecated("WARNING! GetOverallProperty(double, unsigned) is deprecated. Use GetOverallProperty(double, EOverall), GetMixtureProperty(double, EOverall), GetMixtureProperty(double, ECompoundConstProperties) or GetMixtureProperty(double, ECompoundTPProperties) instead.")]]
	double GetOverallProperty(double _time, unsigned _property) const;
	[[deprecated("WARNING! GetMass_Base(double) is deprecated. Use GetMass(double) instead.")]]
	double GetMass_Base(double _time) const;
	[[deprecated("WARNING! SetMass_Base(double, double) is deprecated. Use SetMass(double, double) instead.")]]
	void SetMass_Base(double _time, double _value);
	[[deprecated("WARNING! GetCompoundsList() is deprecated. Use a unit-level version CBaseUnit::GetCompoundsList() instead.")]]
	std::vector<std::string> GetCompoundsList() const;
	[[deprecated("WARNING! GetCompoundsNames() is deprecated. Use a unit-level version CBaseUnit::GetAllCompoundsNames(), CBaseUnit::GetCompoundName(const std::string&) or CBaseUnit::GetCompoundName(size_t) instead.")]]
	std::vector<std::string> GetCompoundsNames() const;
	[[deprecated("WARNING! GetCompoundsNumber() is deprecated. Use a unit-level version CBaseUnit::GetCompoundsNumber() instead.")]]
	size_t GetCompoundsNumber() const;
	[[deprecated("WARNING! GetCompoundPhaseFraction(double, const std::string&, EPhaseTypes) is deprecated. Use GetCompoundFraction(double, const std::string&, EPhase) or GetCompoundsFractions(_time, EPhase) instead.")]]
	double GetCompoundPhaseFraction(double _time, const std::string& _compoundKey, unsigned _soa) const;
	[[deprecated("WARNING! GetCompoundPhaseFraction(double, unsigned, EPhaseTypes) is deprecated. Use GetCompoundFraction(double, const std::string&, EPhase) or GetCompoundsFractions(_time, EPhase) instead.")]]
	double GetCompoundPhaseFraction(double _time, unsigned _index, unsigned _soa) const;
	[[deprecated("WARNING! SetCompoundPhaseFraction(double, const std::string&, EPhaseTypes, double, eValueBasises) is deprecated. Use SetCompoundFraction(double, const std::string&, EPhase, double) or SetCompoundMolFraction(double, const std::string&, EPhase, double) instead.")]]
	void SetCompoundPhaseFraction(double _time, const std::string& _compoundKey, unsigned _soa, double _value, unsigned _basis = BASIS_MASS);
	[[deprecated("WARNING! GetPhaseMass_Base(double, EPhaseTypes) is deprecated. Use GetPhaseMass(double, EPhase) instead.")]]
	double GetPhaseMass_Base(double _time, unsigned _soa) const;
	[[deprecated("WARNING! SetPhaseMass_Base(double, EPhaseTypes, double) is deprecated. Use SetPhaseMass(double, EPhase, double) instead.")]]
	void SetPhaseMass_Base(double _time, unsigned _soa, double _value);
	[[deprecated("WARNING! GetPhaseSOA(unsigned) is deprecated. Use a unit-level version CBaseUnit::GetPhaseType(size_t) instead.")]]
	unsigned GetPhaseSOA(unsigned _index) const;
	[[deprecated("WARNING! GetPhaseIndex(EPhaseTypes) is deprecated. Access phases by their type.")]]
	unsigned GetPhaseIndex(unsigned _soa) const;
	[[deprecated("WARNING! GetPhasesNumber() is deprecated. Use a unit-level version CBaseUnit::GetPhasesNumber() instead.")]]
	size_t GetPhasesNumber() const;
	[[deprecated("WARNING! GetCompoundConstant(const std::string&, unsigned) is deprecated. Use GetCompoundProperty(const std::string&, ECompoundConstProperties) instead.")]]
	double GetCompoundConstant(const std::string& _compoundKey, unsigned _property) const;
	[[deprecated("WARNING! GetCompoundTPDProp(double, const std::string&, unsigned) is deprecated. Use GetCompoundProperty(double, const std::string&, ECompoundTPProperties) instead.")]]
	double GetCompoundTPDProp(double _time, const std::string& _compoundKey, unsigned _property) const;
	[[deprecated("WARNING! GetCompoundTPDProp(const std::string&, unsigned, double, double) is deprecated. Use GetCompoundProperty(const std::string&, ECompoundTPProperties, double, double) instead.")]]
	double GetCompoundTPDProp(const std::string& _compoundKey, unsigned _property, double _temperature, double _pressure) const;
	[[deprecated("WARNING! GetCompoundInteractionProp(double, const std::string&, const std::string&, unsigned) is deprecated. Use GetCompoundProperty(double, const std::string&, const std::string&, ECompoundTPProperties) instead.")]]
	double GetCompoundInteractionProp(double _time, const std::string& _compoundKey1, const std::string& _compoundKey2, unsigned _property) const;
	[[deprecated("WARNING! GetCompoundInteractionProp(const std::string&, const std::string&, unsigned, double, double) is deprecated. Use GetCompoundProperty(const std::string&, const std::string&, ECompoundTPProperties, double, double) instead.")]]
	double GetCompoundInteractionProp(const std::string& _compoundKey1, const std::string& _compoundKey2, unsigned _property, double _temperature, double _pressure) const;
	[[deprecated("WARNING! GetDistribution(double, EDistrTypes, std::vector<double>&) is deprecated. Use GetDistribution(double, EDistrTypes) instead.")]]
	bool GetDistribution(double _time, EDistrTypes _distribution, std::vector<double>& _result) const;
	[[deprecated("WARNING! CopyFromStream_Base(const CBaseStream&, double, bool) is deprecated. Use Copy(double, const CBaseStream&) instead.")]]
	void CopyFromStream_Base(const CBaseStream& _source, double _time, bool _deleteDataAfter = true);
	[[deprecated("WARNING! AddStream_Base(const CBaseStream&, double) is deprecated. Use Add(double, const CBaseStream&) instead.")]]
	void AddStream_Base(const CBaseStream& _source, double _time);
	[[deprecated("WARNING! GetSinglePhaseProp(double, unsigned, EPhaseTypes) is deprecated. Use GetPhaseFraction(double, EPhase), GetPhaseMass(double, EPhase), GetPhaseProperty(double, EPhase, EOverall), GetPhaseProperty(double, EPhase, ECompoundConstProperties) or GetPhaseProperty(double, EPhase, ECompoundTPProperties) instead.")]]
	double GetSinglePhaseProp(double _time, unsigned _property, unsigned _soa) const;
	[[deprecated("WARNING! GetPhaseTPDProp(double, unsigned, EPhaseTypes) is deprecated. Use GetPhaseProperty(double, EPhase, ECompoundTPProperties) instead.")]]
	double GetPhaseTPDProp(double _time, unsigned _property, unsigned _soa) const;
	[[deprecated("WARNING! SetSinglePhaseProp(double, unsigned, EPhaseTypes, double) is deprecated. Use SetPhaseFraction(double, EPhase, double) or SetPhaseMass(double, EPhase, double) instead.")]]
	void SetSinglePhaseProp(double _time, unsigned _property, unsigned _soa, double _value);
protected:
	static EPhase SOA2EPhase(unsigned _soa);
	static unsigned EPhase2SOA(EPhase _phase);
};
