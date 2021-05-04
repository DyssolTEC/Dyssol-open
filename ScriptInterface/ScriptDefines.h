/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include "DyssolHelperDefines.h"
#include "UnitParameters.h"
#include "ContainerFunctions.h"
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
	};

	// All possible types of script entries.
	enum class EEntryType
	{
		EMPTY,		// Key without value
		BOOL,		// bool
		INT,		// int64_t
		UINT,		// uint64_t
		DOUBLE,		// double
		STRING,		// std::string
		PATH,		// std::filesystem::path
		UNIT_PARAM,	// SUnitParameterScriptEntry
	};

	// Help struct to parse unit parameters.
	struct SUnitParameterScriptEntry
	{
		std::string unitName{};		// Name of the unit container. Either this or index will be read.
		size_t unitIndex{};			// Index of the unit container. Either this or name will be read.
		std::string paramName{};	// Name of the parameter. Either this or index will be read.
		size_t paramIndex{};		// Index of the parameter. Either this or name will be read.
		std::string values{};		// Value(s) of the unit parameter as a string.
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
		std::variant<bool, int64_t, uint64_t, double, std::string, std::filesystem::path, SUnitParameterScriptEntry> value{};

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
			MAKE_ARG(EScriptKeys::CONVERGENCE_METHOD          , EEntryType::UINT),
			MAKE_ARG(EScriptKeys::RELAXATION_PARAMETER        , EEntryType::DOUBLE),
			MAKE_ARG(EScriptKeys::ACCELERATION_LIMIT          , EEntryType::DOUBLE),
			MAKE_ARG(EScriptKeys::EXTRAPOLATION_METHOD        , EEntryType::UINT),
			// unit settings
			MAKE_ARG(EScriptKeys::UNIT_PARAMETER              , EEntryType::UNIT_PARAM),
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
	inline SScriptEntryDescriptor ScriptKey2Descriptor(const std::string& _key)
	{
		const auto allArgs = AllScriptArguments();
		const size_t index = VectorFind(allArgs, [&](const SScriptEntryDescriptor& _d) { return _d.keyStr == _key; });
		if (index == static_cast<size_t>(-1)) return {};
		return allArgs[index];
	}
}
