/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "MixtureLookup.h"
#include "DyssolTypes.h"

class CMaterialsDatabase;

/* Bidirectional lookup table to find the correspondence between temperature and enthalpy of a mixture of materials. */
class CMixtureEnthalpyLookup
{
	SInterval m_limits{ DEFAULT_ENTHALPY_MIN_T, DEFAULT_ENTHALPY_MAX_T };	// Temperature limits.
	size_t m_intervals{ DEFAULT_ENTHALPY_INTERVALS };						// Number of temperature intervals.

	CMixtureLookup m_mixtureLookup;				// Lookup table for the mixture.
	const CMaterialsDatabase* m_materialsDB{};	// Pointer to a materials database.
	std::vector<std::string> m_compounds;		// Vector with keys of the chemical compounds which contains this lookup table

public:
	CMixtureEnthalpyLookup() = default;
	CMixtureEnthalpyLookup(const CMaterialsDatabase* _materialsDB, std::vector<std::string> _compounds);
	CMixtureEnthalpyLookup(const CMaterialsDatabase* _materialsDB, std::vector<std::string> _compounds, const SInterval& _limits, size_t _intervalsNumber);

	// Sets temperature limits and number of intervals for the lookup table.
	void SetLimits(const SInterval& _limits, size_t _number);
	// Returns current temperature limits.
	[[nodiscard]] SInterval GetLimits() const;
	// Returns current number of temperature intervals.
	[[nodiscard]] size_t GetIntervalsNumber() const;

	// Sets pointer to materials database
	void SetMaterialsDatabase(const CMaterialsDatabase* _materialsDB);
	// Sets new list of _compounds.
	void SetCompounds(const std::vector<std::string>& _compounds);
	// Sets new _fractions of all compounds. The length of _fractions must be equal to the number of previously defined compounds.
	void SetCompoundFractions(const std::vector<double>& _fractions);
	// Returns current fractions of compounds.
	[[nodiscard]] std::vector<double> GetCompoundFractions() const;

	// Returns enthalpy for the given _temperature.
	[[nodiscard]] double GetEnthalpy(double _temperature) const;
	// Returns temperature for the given _enthalpy.
	[[nodiscard]] double GetTemperature(double _enthalpy) const;

	// Sets new _fractions of all compounds and returns enthalpy for the given _temperature. The length of _fractions must be equal to the number of previously defined compounds.
	[[nodiscard]] double GetEnthalpy(double _temperature, const std::vector<double>& _fractions);
	// Sets new _fractions of all compounds and returns temperature for the given _enthalpy. The length of _fractions must be equal to the number of previously defined compounds.
	[[nodiscard]] double GetTemperature(double _enthalpy, const std::vector<double>& _fractions);

	// Removes all information.
	void Clear();

	// Adds _value to each _right (dependent) entry of the mixture table.
	void Add(double _value);
	// Adds a _component with some _weight to each _right (dependent) entry of the mixture table.
	void Add(const CDependentValues& _component, double _weight = 1.);
	// Adds another mixture _table with some _weight to each _right (dependent) entry of the mixture table. _table must have the same number of compounds.
	void Add(const CMixtureEnthalpyLookup& _table, double _weight = 1.);
	// Multiplies each _right (dependent) entry of the mixture table with a _value.
	void Multiply(double _value);

	CMixtureEnthalpyLookup operator+(double _d) const;
	CMixtureEnthalpyLookup operator*(double _d) const;
	CMixtureEnthalpyLookup operator+(const CMixtureEnthalpyLookup& _t) const;
	CMixtureEnthalpyLookup& operator+=(double _d);
	CMixtureEnthalpyLookup& operator*=(double _d);
	CMixtureEnthalpyLookup& operator+=(const CMixtureEnthalpyLookup& _t);

private:
	// Set enthalpies to the table according to the defined limits, compounds and their fractions.
	void UpdateCompoundsEnthalpies();
};

