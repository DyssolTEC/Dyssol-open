/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

/*
 * All parsed types are described here.
 * It is required to define
 * friend std::istream& operator>>(std::istream&, T&)
 * for each defined type.
 */

#pragma once
#include <istream>
#include <vector>

namespace ScriptInterface
{
	// Help structure to work with entries that can be defined either by their name or by their index.
	struct SNameOrIndex
	{
		std::string name{};	// Name of the entry. Either this or index must be set.
		size_t index{};		// Index of the entry. Either this or name must be set.
		friend std::istream& operator>>(std::istream& _s, SNameOrIndex& _obj);
	};

	// Help structure to work with entries that can be defined either by their name or by their numerical key.
	struct SNameOrKey
	{
		std::string name{};	// Name of the entry. Either this or key must be set.
		int64_t key{ -1 };	// Key of the entry. Either this or name must be set.
		// Checks if the struct contains parsed key.
		[[nodiscard]] bool HasKey() const { return key != -1; }
		friend std::istream& operator>>(std::istream& _s, SNameOrKey& _obj);
	};

	// Struct to parse script entries (SE) with unit parameters.
	struct SUnitParameterSE
	{
		SNameOrIndex unit{};	// Name or index of the unit container.
		SNameOrIndex param{};	// Name or index of the parameter.
		std::string values{};	// Value(s) of the unit parameter as a string for postponed parsing.
		friend std::istream& operator>>(std::istream& _s, SUnitParameterSE& _obj);
	};

	// Struct to parse script entries (SE) with unit holdups' time-dependent parameters (overall, phases).
	struct SHoldupDependentSE
	{
		SNameOrIndex unit{};		// Name or index of the unit container.
		SNameOrIndex holdup{};		// Name or index of the holdup within the unit.
		std::vector<double> values;	// Value(s) of the time-dependent parameters for postponed parsing.
		friend std::istream& operator>>(std::istream& _s, SHoldupDependentSE& _obj);
	};

	// Struct to parse script entries (SE) with unit holdups' time-dependent compound fractions.
	struct SHoldupCompoundsSE
	{
		SNameOrIndex unit{};			// Name or index of the unit container.
		SNameOrIndex holdup{};			// Name or index of the holdup within the unit.
		SNameOrKey phase{};				// Key and/or name of the phase.
		std::vector<double> values{};	// Value(s) of the time-dependent compound fractions for postponed parsing.
		friend std::istream& operator>>(std::istream& _s, SHoldupCompoundsSE& _obj);
	};

	// Struct to parse script entries (SE) with unit holdups' time-dependent distributed parameters of solids.
	struct SHoldupDistributionSE
	{
		SNameOrIndex unit{};			// Name or index of the unit container.
		SNameOrIndex holdup{};			// Name or index of the holdup within the unit.
		SNameOrKey distrType{};			// Type of the distributed parameter (SIZE/PART_POROSITY/FORM_FACTOR/...).
		std::string compound{};			// Name or key of the compound, or MIXTURE keyword.
		SNameOrKey psdType{};			// Type of the PSD (MASS_FRACTION/NUMBER/Q0_DENSITY/Q0_CUMULATIVE/...).
		SNameOrKey psdMeans{};			// Type of the mean values for PSD (DIAMETER/VOLUME).
		SNameOrKey function{};			// Function to define distribution (MANUAL/NORMAL/RRSB/...).
		std::vector<double> values{};	// Value(s) of the time-dependent distributed parameters for postponed parsing.
		friend std::istream& operator>>(std::istream& _s, SHoldupDistributionSE& _obj);
	};

	// Struct to parse script entries (SE) with distribution grids.
	struct SGridDimensionSE
	{
		SNameOrIndex unit{};					// Name or index of the unit container or GLOBAL keyword (index 0).
		SNameOrKey distrType{};					// Type of the distributed parameter (SIZE/PART_POROSITY/FORM_FACTOR/...).
		SNameOrKey entryType{};					// Type of grid entries (NUMERIC/SYMBOLIC).
		SNameOrKey function{};					// Function to define the grid (MANUAL/EQUIDISTANT/GEOMETRIC_INC/...).
		SNameOrKey psdMeans{};					// Type of the mean values for PSD (DIAMETER/VOLUME).
		size_t classes{};						// Number of classes.
		std::vector<double> valuesNum{};		// Values of grid boundaries of function parameters for numeric grid.
		std::vector<std::string> valuesSym{};	// Value(s) of classes names for symbolic grid.
		friend std::istream& operator>>(std::istream& _s, SGridDimensionSE& _obj);
	};
}