/* Copyright (c) 2021, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

/*
 * All parsing types are described here.
 * It is required to define
 * friend std::istream& operator>>(std::istream&, T&)
 * friend std::ostream& operator<<(std::ostream&, const T&)
 * for each defined type.
 */

#pragma once
#include "NameConverters.h"
#include "DyssolStringConstants.h"
#include <iostream>

namespace ScriptInterface
{
	// Help structure to work with entries that can be defined either by their name or by their index.
	struct SNameOrIndex
	{
		std::string name{};	// Name of the entry. Either this or index must be set.
		size_t index{};		// Index of the entry. Either this or name must be set.
		friend std::istream& operator>>(std::istream& _s, SNameOrIndex& _obj);
		friend std::ostream& operator<<(std::ostream& _s, const SNameOrIndex& _obj);
	};

	// Help structure to work with entries that can be defined either by their name or by their numerical key (enumerator).
	struct SNamedEnum
	{
		std::string name{};	// Name of the entry. Either this or key must be set.
		int64_t key{ -1 };	// Key of the entry. Either this or name must be set.
		SNamedEnum() = default;
		// Creates entry from enumerator value, converting it to the name.
		template<typename E, typename = std::enable_if_t<std::is_enum_v<E>>> explicit SNamedEnum(E _key)
			: name{ Enum2Name<E>(_key) }, key{ static_cast<int64_t>(_key) } {}
		// Ensures that both name and key are filled, converting one to another if necessary. Returns a converted object itself.
		template<typename E, typename = std::enable_if_t<std::is_enum_v<E>>> SNamedEnum& FillAndCheck()
		{
			if (key == -1) key = static_cast<int64_t>(Name2Enum<E>(StringFunctions::ToUpperCase(name)));
			else		   name = Enum2Name<E>(static_cast<E>(key));
			if (static_cast<E>(key) == static_cast<E>(-1))
				std::cout << StrConst::DyssolC_WarningUnknown(name) << std::endl;
			return *this;
		}
		friend std::istream& operator>>(std::istream& _s, SNamedEnum& _obj);
		friend std::ostream& operator<<(std::ostream& _s, const SNamedEnum& _obj);
	};

	// Struct to parse script entries (SE) with unit parameters.
	struct SUnitParameterSE
	{
		SNameOrIndex unit{};	// Name or index of the unit container.
		SNameOrIndex param{};	// Name or index of the parameter.
		std::string values{};	// Value(s) of the unit parameter as a string for postponed parsing.
		friend std::istream& operator>>(std::istream& _s, SUnitParameterSE& _obj);
		friend std::ostream& operator<<(std::ostream& _s, const SUnitParameterSE& _obj);
	};

	// Struct to parse script entries (SE) with unit holdups' time-dependent parameters (overall, phases).
	struct SHoldupDependentSE
	{
		SNameOrIndex unit{};		// Name or index of the unit container.
		SNameOrIndex holdup{};		// Name or index of the holdup within the unit.
		std::vector<double> values;	// Value(s) of the time-dependent parameters for postponed parsing.
		friend std::istream& operator>>(std::istream& _s, SHoldupDependentSE& _obj);
		friend std::ostream& operator<<(std::ostream& _s, const SHoldupDependentSE& _obj);
	};

	// Struct to parse script entries (SE) with unit holdups' time-dependent compound fractions.
	struct SHoldupCompoundsSE
	{
		SNameOrIndex unit{};			// Name or index of the unit container.
		SNameOrIndex holdup{};			// Name or index of the holdup within the unit.
		SNamedEnum phase{};				// Key and/or name of the phase.
		std::vector<double> values{};	// Value(s) of the time-dependent compound fractions for postponed parsing.
		friend std::istream& operator>>(std::istream& _s, SHoldupCompoundsSE& _obj);
		friend std::ostream& operator<<(std::ostream& _s, const SHoldupCompoundsSE& _obj);
	};

	// Struct to parse script entries (SE) with unit holdups' time-dependent distributed parameters of solids.
	struct SHoldupDistributionSE
	{
		SNameOrIndex unit{};			// Name or index of the unit container.
		SNameOrIndex holdup{};			// Name or index of the holdup within the unit.
		SNamedEnum distrType{};			// Type of the distributed parameter (SIZE/PART_POROSITY/FORM_FACTOR/...).
		std::string compound{};			// Name or key of the compound, or MIXTURE keyword.
		SNamedEnum psdType{};			// Type of the PSD (MASS_FRACTION/NUMBER/Q0_DENSITY/Q0_CUMULATIVE/...).
		SNamedEnum psdMeans{};			// Type of the mean values for PSD (DIAMETER/VOLUME).
		SNamedEnum function{};			// Function to define distribution (MANUAL/NORMAL/RRSB/...).
		std::vector<double> values{};	// Value(s) of the time-dependent distributed parameters for postponed parsing.
		friend std::istream& operator>>(std::istream& _s, SHoldupDistributionSE& _obj);
		friend std::ostream& operator<<(std::ostream& _s, const SHoldupDistributionSE& _obj);
	};

	// Struct to parse script entries (SE) with distribution grids.
	struct SGridDimensionSE
	{
		SNameOrIndex unit{};					// Name or index of the unit container or GLOBAL keyword (index 0).
		SNamedEnum distrType{};					// Type of the distributed parameter (SIZE/PART_POROSITY/FORM_FACTOR/...).
		SNamedEnum entryType{};					// Type of grid entries (NUMERIC/SYMBOLIC).
		SNamedEnum function{};					// Function to define the grid (MANUAL/EQUIDISTANT/GEOMETRIC_INC/...).
		SNamedEnum psdMeans{};					// Type of the mean values for PSD (DIAMETER/VOLUME).
		size_t classes{};						// Number of classes.
		std::vector<double> valuesNum{};		// Values of grid boundaries of function parameters for numeric grid.
		std::vector<std::string> valuesSym{};	// Value(s) of classes names for symbolic grid.
		friend std::istream& operator>>(std::istream& _s, SGridDimensionSE& _obj);
		friend std::ostream& operator<<(std::ostream& _s, const SGridDimensionSE& _obj);
	};

	// Struct to parse script entries (SE) with phase descriptors.
	struct SPhasesSE
	{
		std::vector<std::string> names;	// Names of phases.
		std::vector<SNamedEnum> types;	// Types of phases.
		friend std::istream& operator>>(std::istream& _s, SPhasesSE& _obj);
		friend std::ostream& operator<<(std::ostream& _s, const SPhasesSE& _obj);
	};

	// Struct to parse script entries (SE) with streams.
	struct SStreamSE
	{
		std::string name{};		// Name of the stream.
		SNameOrIndex unitO{};	// Name or index of the unit container, from which the stream starts.
		SNameOrIndex portO{};	// Name or index of the model's port, from which the stream starts.
		SNameOrIndex unitI{};	// Name or index of the unit container, where the stream ends.
		SNameOrIndex portI{};	// Name or index of the model's port, where the stream ends.
		friend std::istream& operator>>(std::istream& _s, SStreamSE& _obj);
		friend std::ostream& operator<<(std::ostream& _s, const SStreamSE& _obj);
	};

	// Struct to parse script entries (SE) with streams export.
	struct SExportStreamSE
	{
		SNameOrIndex stream{};			// Name or index of the stream.
		std::vector<double> times{};	// Time points to export.
		friend std::istream& operator>>(std::istream& _s, SExportStreamSE& _obj);
		friend std::ostream& operator<<(std::ostream& _s, const SExportStreamSE& _obj);
	};

	// Struct to parse script entries (SE) with holdups export.
	struct SExportHoldupSE
	{
		SNameOrIndex unit{};			// Name or index of the unit container.
		SNameOrIndex holdup{};			// Name or index of the holdup within the unit.
		std::vector<double> times{};	// Time points to export.
		friend std::istream& operator>>(std::istream& _s, SExportHoldupSE& _obj);
		friend std::ostream& operator<<(std::ostream& _s, const SExportHoldupSE& _obj);
	};

	// Struct to parse script entries (SE) with state variables export.
	struct SExportStateVarSE
	{
		SNameOrIndex unit{};			// Name or index of the unit container.
		SNameOrIndex variable{};		// Name or index of the state variable within the unit.
		std::vector<double> times{};	// Time points to export.
		friend std::istream& operator>>(std::istream& _s, SExportStateVarSE& _obj);
		friend std::ostream& operator<<(std::ostream& _s, const SExportStateVarSE& _obj);
	};

	// Struct to parse script entries (SE) with state variables export.
	struct SExportPlotSE
	{
		SNameOrIndex unit{};			// Name or index of the unit container.
		SNameOrIndex plot{};			// Name or index of the plot within the unit.
		SNameOrIndex curve{};			// Name or index of the curve within the plot.
		friend std::istream& operator>>(std::istream& _s, SExportPlotSE& _obj);
		friend std::ostream& operator<<(std::ostream& _s, const SExportPlotSE& _obj);
	};
}
