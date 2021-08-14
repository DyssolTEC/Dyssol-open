/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include "DyssolHelperDefines.h"
#include "UnitParameters.h"
#include "ContainerFunctions.h"
#include "StringFunctions.h"
#include <filesystem>
#include <string>
#include <vector>
#include <variant>

namespace ScriptInterface
{
	// All possible script keys.
	enum class EScriptKeys
	{
		JOB,
		SOURCE_FILE,
		RESULT_FILE,
		MATERIALS_DATABASE,
		MODELS_PATH,
		SIMULATION_TIME,
		RELATIVE_TOLERANCE,
		ABSOLUTE_TOLERANCE,
		MINIMAL_FRACTION,
		SAVE_TIME_STEP_HINT,
		SAVE_FLAG_FOR_HOLDUPS,
		THERMO_TEMPERATURE_MIN,
		THERMO_TEMPERATURE_MAX,
		THERMO_TEMPERATURE_INTERVALS,
		INIT_TIME_WINDOW,
		MIN_TIME_WINDOW,
		MAX_TIME_WINDOW,
		MAX_ITERATIONS_NUMBER,
		WINDOW_CHANGE_RATE,
		ITERATIONS_UPPER_LIMIT,
		ITERATIONS_LOWER_LIMIT,
		ITERATIONS_UPPER_LIMIT_1ST,
		CONVERGENCE_METHOD,
		RELAXATION_PARAMETER,
		ACCELERATION_LIMIT,
		EXTRAPOLATION_METHOD,
		UNIT_PARAMETER,
		HOLDUPS_KEEP_EXISTING_VALUES,
		HOLDUP_OVERALL,
		HOLDUP_PHASES,
		HOLDUP_COMPOUNDS,
		HOLDUP_DISTRIBUTION,
	};

	// All possible types of script entries.
	enum class EEntryType
	{
		EMPTY,					// Key without value
		BOOL,					// bool
		INT,					// int64_t
		UINT,					// uint64_t
		DOUBLE,					// double
		STRING,					// std::string
		PATH,					// std::filesystem::path
		NAME_OR_KEY,			// SNameOrKey
		UNIT_PARAMETER,			// SUnitParameterSE
		HOLDUP_DEPENDENT,		// SHoldupDependentSE
		HOLDUP_COMPOUNDS,		// SHoldupCompoundSE
		HOLDUP_DISTRIBUTION,	// SHoldupDistributionSE
	};

	// Help structure to work with entries that can be defined either by their name or by their index.
	struct SNameOrIndex
	{
		std::string name{};	// Name of the entry. Either this or index must be set.
		size_t index{};		// Index of the entry. Either this or name must be set.
		friend std::istream& operator>>(std::istream& _s, SNameOrIndex& _obj) // Input stream operator.
		{
			const auto str = StringFunctions::GetValueFromStream<std::string>(_s);
			_obj.name  = StringFunctions::IsSimpleUInt(str) ? "" : str;
			_obj.index = StringFunctions::IsSimpleUInt(str) ? std::stoull(str) - 1 : -1;
			return _s;
		}
	};

	// Help structure to work with entries that can be defined either by their name or by their numerical key.
	struct SNameOrKey
	{
		std::string name{};	// Name of the entry. Either this or key must be set.
		uint64_t key{};		// Key of the entry. Either this or name must be set.
		// Checks if the struct contains parsed key.
		[[nodiscard]] bool HasKey() const { return key != static_cast<decltype(key)>(-1); }
		// Input stream operator.
		friend std::istream& operator>>(std::istream& _s, SNameOrKey& _obj)
		{
			const auto str = StringFunctions::GetValueFromStream<std::string>(_s);
			_obj.name = StringFunctions::IsSimpleUInt(str) ? "" : str;
			_obj.key  = StringFunctions::IsSimpleUInt(str) ? std::stoull(str) : -1;
			return _s;
		}
	};

	// Struct to parse script entries (SE) with unit parameters.
	struct SUnitParameterSE
	{
		SNameOrIndex unit{};	// Name or index of the unit container.
		SNameOrIndex param{};	// Name or index of the parameter.
		std::string values{};	// Value(s) of the unit parameter as a string for postponed parsing.
		// Input stream operator.
		friend std::istream& operator>>(std::istream& _s, SUnitParameterSE& _obj)
		{
			_obj.unit   = StringFunctions::GetValueFromStream<SNameOrIndex>(_s);
			_obj.param  = StringFunctions::GetValueFromStream<SNameOrIndex>(_s);
			_obj.values = StringFunctions::GetRestOfLine(_s);	// format depends on parameter type, but type resolution is only possible when the flowsheet is loaded, so postpone final parsing
			return _s;
		}
	};

	// Struct to parse script entries (SE) with unit holdups' time-dependent parameters (overall, phases).
	struct SHoldupDependentSE
	{
		SNameOrIndex unit{};		// Name or index of the unit container.
		SNameOrIndex holdup{};		// Name or index of the holdup within the unit.
		std::vector<double> values;	// Value(s) of the time-dependent parameters for postponed parsing.
		friend std::istream& operator>>(std::istream& _s, SHoldupDependentSE& _obj)
		{
			_obj.unit   = StringFunctions::GetValueFromStream<SNameOrIndex>(_s);
			_obj.holdup = StringFunctions::GetValueFromStream<SNameOrIndex>(_s);
			_obj.values = StringFunctions::GetValueFromStream<std::vector<double>>(_s);	// format depends on the flowsheet settings, so postpone final parsing until the flowsheet is loaded
			return _s;
		}
	};

	// Struct to parse script entries (SE) with unit holdups' time-dependent compound fractions.
	struct SHoldupCompoundsSE
	{
		SNameOrIndex unit{};			// Name or index of the unit container.
		SNameOrIndex holdup{};			// Name or index of the holdup within the unit.
		SNameOrKey phase{};				// Name or key of the phase.
		std::vector<double> values{};	// Value(s) of the time-dependent compound fractions for postponed parsing.
		friend std::istream& operator>>(std::istream& _s, SHoldupCompoundsSE& _obj)
		{
			_obj.unit   = StringFunctions::GetValueFromStream<SNameOrIndex>(_s);
			_obj.holdup = StringFunctions::GetValueFromStream<SNameOrIndex>(_s);
			_obj.phase  = StringFunctions::GetValueFromStream<SNameOrKey>(_s);
			_obj.values = StringFunctions::GetValueFromStream<std::vector<double>>(_s);	// format depends on the flowsheet settings, so postpone final parsing until the flowsheet is loaded
			return _s;
		}
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
		friend std::istream& operator>>(std::istream& _s, SHoldupDistributionSE& _obj)
		{
			_obj.unit      = StringFunctions::GetValueFromStream<SNameOrIndex>(_s);
			_obj.holdup    = StringFunctions::GetValueFromStream<SNameOrIndex>(_s);
			_obj.distrType = StringFunctions::GetValueFromStream<SNameOrKey>(_s);
			_obj.compound  = StringFunctions::GetValueFromStream<std::string>(_s);
			// special treatment for PSD
			if (!_obj.distrType.HasKey() && StringFunctions::ToUpperCase(_obj.distrType.name) == "SIZE" || _obj.distrType.HasKey() && _obj.distrType.key == DISTR_SIZE)
			{
				_obj.psdType  = StringFunctions::GetValueFromStream<SNameOrKey>(_s);
				_obj.psdMeans = StringFunctions::GetValueFromStream<SNameOrKey>(_s);
			}
			_obj.function = StringFunctions::GetValueFromStream<SNameOrKey>(_s);
			_obj.values   = StringFunctions::GetValueFromStream<std::vector<double>>(_s);
			return _s;
		}
	};

	// Descriptor for an entry of the script file.
	struct SScriptEntryDescriptor
	{
		EScriptKeys key{ EScriptKeys::JOB };    // Script key (SOURCE_FILE | RESULT_FILE | SIMULATION_TIME | UNIT_PARAMETER |...).
		std::string keyStr{};					// A string version of the script key.
		EEntryType type{ EEntryType::EMPTY };	// Type of the entry value (DOUBLE | UNSIGNED | STRING |...).
	};

	// Entry of the script file.
	struct SScriptEntry : SScriptEntryDescriptor
	{
		// Value of the entry of different types.
		std::variant<bool, int64_t, uint64_t, double, std::string, std::filesystem::path,
			SNameOrKey, SUnitParameterSE, SHoldupDependentSE, SHoldupCompoundsSE, SHoldupDistributionSE> value{};

		SScriptEntry() = default;
		SScriptEntry(const SScriptEntryDescriptor& _descr) : SScriptEntryDescriptor{ _descr } {}
	};

	// Number of symbols to discard: length of 'EScriptKeys::'.
	constexpr size_t DISCARD = 13;
	// Converts a fully resolved name EScriptKeys::SOURCE_FILE to string "SOURCE_FILE".
	#define ARG2STR(X) std::string(MACRO_TOSTRING(X)).substr(DISCARD, std::string(MACRO_TOSTRING(X)).length())
	// Creates an empty new argument.
	#define MAKE_ARG(ARG_NAME, ARG_TYPE) { ARG_NAME, std::string{ARG2STR(ARG_NAME)}, ARG_TYPE }

	/* Creates a vector of all possible script arguments and initializes them.
	 * Name of the key from EScriptKeys is converted to a string and used as a key for script file.
	 * To add a new key of existing type, extend EScriptKeys and add it with MAKE_ARG macro.
	 * To add a new key of a new type, additionally extend EEntryType and extend SScriptEntry::value with the real value type. */
	inline std::vector<SScriptEntryDescriptor> AllScriptArguments()
	{
		return {
			// paths
			MAKE_ARG(EScriptKeys::JOB                         , EEntryType::EMPTY),
			MAKE_ARG(EScriptKeys::SOURCE_FILE                 , EEntryType::PATH),
			MAKE_ARG(EScriptKeys::RESULT_FILE                 , EEntryType::PATH),
			MAKE_ARG(EScriptKeys::MATERIALS_DATABASE          , EEntryType::PATH),
			MAKE_ARG(EScriptKeys::MODELS_PATH                 , EEntryType::PATH),
			// flowsheet parameters
			MAKE_ARG(EScriptKeys::SIMULATION_TIME             , EEntryType::DOUBLE),
			MAKE_ARG(EScriptKeys::RELATIVE_TOLERANCE          , EEntryType::DOUBLE),
			MAKE_ARG(EScriptKeys::ABSOLUTE_TOLERANCE          , EEntryType::DOUBLE),
			MAKE_ARG(EScriptKeys::MINIMAL_FRACTION            , EEntryType::DOUBLE),
			MAKE_ARG(EScriptKeys::SAVE_TIME_STEP_HINT         , EEntryType::DOUBLE),
			MAKE_ARG(EScriptKeys::SAVE_FLAG_FOR_HOLDUPS       , EEntryType::BOOL),
			MAKE_ARG(EScriptKeys::THERMO_TEMPERATURE_MIN      , EEntryType::DOUBLE),
			MAKE_ARG(EScriptKeys::THERMO_TEMPERATURE_MAX      , EEntryType::DOUBLE),
			MAKE_ARG(EScriptKeys::THERMO_TEMPERATURE_INTERVALS, EEntryType::UINT),
			MAKE_ARG(EScriptKeys::INIT_TIME_WINDOW            , EEntryType::DOUBLE),
			MAKE_ARG(EScriptKeys::MIN_TIME_WINDOW             , EEntryType::DOUBLE),
			MAKE_ARG(EScriptKeys::MAX_TIME_WINDOW             , EEntryType::DOUBLE),
			MAKE_ARG(EScriptKeys::MAX_ITERATIONS_NUMBER       , EEntryType::UINT),
			MAKE_ARG(EScriptKeys::WINDOW_CHANGE_RATE          , EEntryType::DOUBLE),
			MAKE_ARG(EScriptKeys::ITERATIONS_UPPER_LIMIT      , EEntryType::UINT),
			MAKE_ARG(EScriptKeys::ITERATIONS_LOWER_LIMIT      , EEntryType::UINT),
			MAKE_ARG(EScriptKeys::ITERATIONS_UPPER_LIMIT_1ST  , EEntryType::UINT),
			MAKE_ARG(EScriptKeys::CONVERGENCE_METHOD          , EEntryType::NAME_OR_KEY),
			MAKE_ARG(EScriptKeys::RELAXATION_PARAMETER        , EEntryType::DOUBLE),
			MAKE_ARG(EScriptKeys::ACCELERATION_LIMIT          , EEntryType::DOUBLE),
			MAKE_ARG(EScriptKeys::EXTRAPOLATION_METHOD        , EEntryType::NAME_OR_KEY),
			// unit settings
			MAKE_ARG(EScriptKeys::UNIT_PARAMETER              , EEntryType::UNIT_PARAMETER),
			// holdup and input streams parameters
			MAKE_ARG(EScriptKeys::HOLDUPS_KEEP_EXISTING_VALUES, EEntryType::BOOL),
			MAKE_ARG(EScriptKeys::HOLDUP_OVERALL			  , EEntryType::HOLDUP_DEPENDENT),
			MAKE_ARG(EScriptKeys::HOLDUP_PHASES				  , EEntryType::HOLDUP_DEPENDENT),
			MAKE_ARG(EScriptKeys::HOLDUP_COMPOUNDS            , EEntryType::HOLDUP_COMPOUNDS),
			MAKE_ARG(EScriptKeys::HOLDUP_DISTRIBUTION         , EEntryType::HOLDUP_DISTRIBUTION),
		};
	}

	// Returns a vector of all possible script keys.
	inline std::vector<std::string> AllScriptKeys()
	{
		std::vector<std::string> res;
		for (const auto& a : AllScriptArguments())
			res.push_back(a.keyStr);
		return res;
	}

	// Returns a script entry descriptor for the given key. If a descriptor for such key was not defined, returns a default descriptor with empty key.
	inline SScriptEntryDescriptor Key2Descriptor(const std::string& _key)
	{
		const auto allArgs = AllScriptArguments();
		const size_t index = VectorFind(allArgs, [&](const SScriptEntryDescriptor& _d) { return _d.keyStr == _key; });
		if (index == static_cast<size_t>(-1)) return {};
		return allArgs[index];
	}

	// Converts a script key to its string representation.
	inline std::string StrKey(const EScriptKeys& _key)
	{
		const auto allArgs = AllScriptArguments();
		const size_t index = VectorFind(allArgs, [&](const SScriptEntryDescriptor& _d) { return _d.key == _key; });
		if (index == static_cast<size_t>(-1)) return {};
		return allArgs[index].keyStr;
	}
}