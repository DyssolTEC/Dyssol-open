/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "LookupTables.h"

class CDenseMDMatrix;
class CPhase;
class CTimeDependentValue;
class CMaterialsDatabase;
class CDistributionsGrid;
class CMatrix2D;
class CTransformMatrix;

/** Base not instantiated class for material stream description.*/
class CStream2
{
	static const unsigned m_saveVersion{ 2 }; // Current version of the saving procedure.

	// TODO: make it accessible or global
	double m_eps{ 1e-20 };

	std::string m_name{ "Stream" };	// Name of the stream.
	std::string m_key;				// Unique key of the stream.

	std::vector<double> m_timePoints;									// Time points on which the stream is defined.
	std::map<EOverall, std::unique_ptr<CTimeDependentValue>> m_overall;	// Defined overall properties.
	std::map<EPhase, std::unique_ptr<CPhase>> m_phases;					// Defined phases.
	std::vector<std::string> m_compounds;								// Keys of chemical compounds defined in this stream.
	CLookupTables m_lookupTables;										// Lookup tables to calculate TP-dependent properties.
	SCacheSettings m_cacheSettings;										// Settings for caching in the stream.

	// TODO: make reference
	const CMaterialsDatabase* m_materialsDB{ nullptr };	// Pointer to a database of materials.
	// TODO: make reference
	const CDistributionsGrid* m_grid{ nullptr };		// Pointer to a distribution grid.

public:
	////////////////////////////////////////////////////////////////////////////////
	// Basic stream properties
	//

	std::string GetName() const;			// Returns name of the stream.
	void SetName(const std::string& _name);	// Sets new name of the stream.
	std::string GetKey() const;				// Returns unique key of the stream.
	void SetKey(const std::string& _key);	// Sets new unique key of the stream.

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
	// Returns all defined time points.
	std::vector<double> GetAllTimePoints() const;
	// Returns all defined time points in the specified time interval.
	std::vector<double> GetTimePointsInInterval(double _timeBeg, double _timeEnd) const;
	// Returns the last (largest) defined time point.
	double GetLastTimePoint() const;
	// Returns the nearest time point before _time, or zero if such time can not be found.
	double GetPreviousTimePoint(double _time) const;

	////////////////////////////////////////////////////////////////////////////////
	// Overall parameters
	//

	// Adds new overall property to the stream, if it does not exist yet.
	void AddOverallProperty(EOverall _property);
	// Removes an overall property from the stream, if it does exist.
	void RemoveOverallProperty(EOverall _property);
	// Returns a pointer to an overall property or nullptr if such property doesn't exist
	CTimeDependentValue* GetOverallProperty(EOverall _property);

	// Returns a value of the specified overall property at the given time point. Returns default value if such overall property has not been defined.
	double GetOverallProperty(double _time, EOverall _property) const;
	// Returns a value of the overall mass at the given time point. Returns 0 if mass overall property has not been defined.
	double GetMass(double _time) const;
	// Returns a value of the overall temperature at the given time point. Returns standard condition temperature if temperature overall property has not been defined.
	double GetTemperature(double _time) const;
	// Returns a value of the overall pressure at the given time point. Returns standard condition pressure if pressure overall property has not been defined.
	double GetPressure(double _time) const;

	// Sets a value of the specified overall property at the given time point if such property exists.
	void SetOverallProperty(double _time, EOverall _property, double _value);
	// Sets a value of the overall mass at the given time point if such property exists.
	void SetMass(double _time, double _value);
	// Sets a value of the overall temperature at the given time point if such property exists.
	void SetTemperature(double _time, double _value);
	// Sets a value of the overall pressure at the given time point if such property exists.
	void SetPressure(double _time, double _value);

	////////////////////////////////////////////////////////////////////////////////
	// Compounds
	//

	// Adds compound with the specified unique key to the stream.
	void AddCompound(const std::string& _compoundKey);
	// Removes compound with the specified unique key from the stream.
	void RemoveCompound(const std::string& _compoundKey);

	// Returns the mass fraction of the compound in the total mixture at the given time point.
	double GetCompoundFraction(double _time, const std::string& _compoundKey) const;
	// Returns the mass fraction of the compound in the specified phase at the given time point.
	double GetCompoundFraction(double _time, const std::string& _compoundKey, EPhase _phase) const;
	// Returns the mass fraction of the compound in the specified phase at the given time point in moll basis.
	double GetCompoundFractionMoll(double _time, const std::string& _compoundKey, EPhase _phase) const;
	// Returns the mass of the compound in the specified phase at the given time point.
	double GetCompoundMass(double _time, const std::string& _compoundKey, EPhase _phase) const;

	// Sets the mass fraction of the compound in the specified phase at the given time point.
	void SetCompoundFraction(double _time, const std::string& _compoundKey, EPhase _phase, double _value);

	////////////////////////////////////////////////////////////////////////////////
	// Phases
	//

	// Adds the specified phase to the stream.
	void AddPhase(EPhase _phase);
	// Removes the specified phase from the stream.
	void RemovePhase(EPhase _phase);

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

	// TODO: rename, make the same
	// Returns the value of the constant physical property (CRITICAL_TEMPERATURE, MOLAR_MASS, etc) of the specified compound.
	double GetCompoundConstProperty(const std::string& _compoundKey, ECompoundConstProperties _property) const;
	// Returns the value of the temperature/pressure-dependent physical property (DENSITY, ENTHALPY, etc) of the specified compound with the given temperature [K] and pressure [Pa].
	double GetCompoundTPProperty(const std::string& _compoundKey, ECompoundTPProperties _property, double _temperature, double _pressure) const;
	// Returns the value of the temperature/pressure-dependent physical property (DENSITY, ENTHALPY, etc) of the specified compound at temperature and pressure at the given time point.
	double GetCompoundTPProperty(double _time, const std::string& _compoundKey, ECompoundTPProperties _property) const;
	// Returns the value of the interaction physical property (INTERFACE_TENSION, etc) between the specified compounds with the given specified temperature [K] and pressure [Pa].
	double GetCompoundInteractionProperty(const std::string& _compoundKey1, const std::string& _compoundKey2, EInteractionProperties _property, double _temperature, double _pressure) const;
	// Returns the value of the interaction physical property (INTERFACE_TENSION, etc) between the specified compounds at temperature and pressure at the given time point.
	double GetCompoundInteractionProperty(double _time, const std::string& _compoundKey1, const std::string& _compoundKey2, EInteractionProperties _property) const;

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




	CLookupTables* GetLookupTables();

private:
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
};
