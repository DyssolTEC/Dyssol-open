/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "MixtureLookup.h"
#include "DyssolTypes.h"

class CMaterialsDatabase;

/**
 * Bidirectional lookup table to find the correspondence between temperature and enthalpy of a mixture of materials.
 */
class CMixtureEnthalpyLookup
{
	SInterval m_limits{ DEFAULT_ENTHALPY_MIN_T, DEFAULT_ENTHALPY_MAX_T };	///< Temperature limits.
	size_t m_intervals{ DEFAULT_ENTHALPY_INTERVALS };						///< Number of temperature intervals.

	CMixtureLookup m_mixtureLookup;				///< Lookup table for the mixture.
	const CMaterialsDatabase* m_materialsDB{};	///< Pointer to a materials database.
	std::vector<std::string> m_compounds;		///< Vector with keys of the chemical compounds which contains this lookup table

public:
	/**
	 * \brief Default constructor.
	 */
	CMixtureEnthalpyLookup() = default;
	/**
	 * \brief Constructs lookup table with the pointer to materials database and compounds and default values of limits and number of intervals.
	 * \param _materialsDB Pointer to materials database.
	 * \param _compounds List of compounds.
	 */
	CMixtureEnthalpyLookup(const CMaterialsDatabase* _materialsDB, std::vector<std::string> _compounds);
	/**
	 * \brief Constructs lookup table with the pointer to materials database, compounds, values of limits and number of intervals.
	 * \param _materialsDB Pointer to materials database.
	 * \param _compounds List of compounds.
	 * \param _limits Temperature limits.
	 * \param _intervalsNumber Number of temperature intervals.
	 */
	CMixtureEnthalpyLookup(const CMaterialsDatabase* _materialsDB, std::vector<std::string> _compounds, const SInterval& _limits, size_t _intervalsNumber);
	CMixtureEnthalpyLookup(const CMixtureEnthalpyLookup& _other) = default;

	/**
	 * \brief Sets temperature limits and number of intervals for the lookup table.
	 * \param _limits Temperature limits.
	 * \param _number Number of temperature intervals.
	 */
	void SetLimits(const SInterval& _limits, size_t _number);
	/**
	 * \brief Returns current temperature limits.
	 * \return Temperature limits.
	 */
	[[nodiscard]] SInterval GetLimits() const;
	/**
	 * \brief Returns current number of temperature intervals.
	 * \return Number of intervals.
	 */
	[[nodiscard]] size_t GetIntervalsNumber() const;

	/**
	 * \brief Sets pointer to materials database
	 * \param _materialsDB Pointer to materials database.
	 */
	void SetMaterialsDatabase(const CMaterialsDatabase* _materialsDB);
	/**
	 * \brief Sets new list of _compounds.
	 * \param _compounds List of compounds.
	 */
	void SetCompounds(const std::vector<std::string>& _compounds);
	/**
	 * \brief Sets new fractions of all compounds.
	 * \details The length of fractions must be equal to the number of previously defined compounds.
	 * \param _fractions Compounds fractions.
	 */
	void SetCompoundFractions(const std::vector<double>& _fractions);
	/**
	 * \brief Returns current fractions of compounds.
	 * \return Compounds fractions.
	 */
	[[nodiscard]] std::vector<double> GetCompoundFractions() const;
	/**
	 * \brief Returns the number of entries in the lookup table.
	 * \return Number of entries.
	 */
	[[nodiscard]] size_t Size() const;

	/**
	 * \brief Returns enthalpy for the given temperature.
	 * \param _temperature Temperature.
	 * \return Enthalpy.
	 */
	[[nodiscard]] double GetEnthalpy(double _temperature) const;
	/**
	 * \brief Returns temperature for the given _enthalpy.
	 * \param _enthalpy Enthalpy.
	 * \return Temperature.
	 */
	[[nodiscard]] double GetTemperature(double _enthalpy) const;

	/**
	 * \brief Sets new fractions of all compounds and returns enthalpy for the given temperature.
	 * \details The length of fractions must be equal to the number of previously defined compounds.
	 * \param _temperature Temperature.
	 * \param _fractions Compounds fractions.
	 * \return Enthalpy.
	 */
	[[nodiscard]] double GetEnthalpy(double _temperature, const std::vector<double>& _fractions);
	/**
	 * \brief Sets new _fractions of all compounds and returns temperature for the given enthalpy.
	 * \details The length of fractions must be equal to the number of previously defined compounds.
	 * \param _enthalpy Enthalpy.
	 * \param _fractions Compounds fractions.
	 * \return Temperature.
	 */
	[[nodiscard]] double GetTemperature(double _enthalpy, const std::vector<double>& _fractions);

	/**
	 * \brief Removes all information.
	 */
	void Clear();

	/**
	 * \brief Adds value to each right (dependent) entry of the mixture table.
	 * \param _value New value.
	 */
	void Add(double _value);
	/**
	 * \brief Adds a component with some weight to each right (dependent) entry of the mixture table.
	 * \param _component New component.
	 * \param _weight Weight of the component.
	 */
	void Add(const CDependentValues& _component, double _weight = 1.);
	/**
	 * \brief Adds another mixture table with some weight to each right (dependent) entry of the mixture table.
	 * \details Table must have the same number of compounds.
	 * \param _table Mixture table.
	 * \param _weight Weight of the component.
	 */
	void Add(const CMixtureEnthalpyLookup& _table, double _weight = 1.);
	/**
	 * \brief Multiplies each right (dependent) entry of the mixture table with a value.
	 * \param _value Value.
	 */
	void Multiply(double _value);

	/**
	 * \brief Adds a value to all entries of the lookup table.
	 * \param _d Value.
	 * \return Copy of the lookup table.
	 */
	CMixtureEnthalpyLookup operator+(double _d) const;
	/**
	 * \brief Multiplies all entries of the lookup table with a value.
	 * \param _d Value.
	 * \return Copy of the lookup table.
	 */
	CMixtureEnthalpyLookup operator*(double _d) const;
	/**
	 * \brief Adds another lookup table.
	 * \param _t Lookup table.
	 * \return Copy of the lookup table.
	 */
	CMixtureEnthalpyLookup operator+(const CMixtureEnthalpyLookup& _t) const;
	/**
	 * \brief Adds a value to all entries of the lookup table.
	 * \param _d Value.
	 * \return Reference to the lookup table.
	 */
	CMixtureEnthalpyLookup& operator+=(double _d);
	/**
	 * \brief Multiplies all entries of the lookup table with a value.
	 * \param _d Value.
	 * \return Reference to the lookup table.
	 */
	CMixtureEnthalpyLookup& operator*=(double _d);
	/**
	 * \brief Adds another lookup table.
	 * \param _t Lookup table.
	 * \return Reference to the lookup table.
	 */
	CMixtureEnthalpyLookup& operator+=(const CMixtureEnthalpyLookup& _t);

	/**
	 * \brief Compares two lookup tables.
	 * \param _t Lookup table.
	 * \return Whether lookup tables are the same.
	 */
	bool operator==(const CMixtureEnthalpyLookup& _t) const;

private:
	/**
	 * \brief Set enthalpies to the table according to the defined limits, compounds and their fractions.
	 */
	void UpdateCompoundsEnthalpies();
};

