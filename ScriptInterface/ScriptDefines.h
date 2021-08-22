/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

/*
 * Main defines of the script interface.
 * All script keywords as they appear in the script are described in EScriptKeys.
 * EEntryType describes all possible types each line of the script can represent.
 * Each script line is read into SScriptEntry - it can hold any type defined in EEntryType.
 * How to read each type from EEntryType is given in ParseScriptEntry().
 * To make this function work, each type must define operator>>(std::istream&, T&), as it is given in ScriptTypes.h
 * The correspondence between EScriptKeys and EEntryType is set in AllScriptArguments().
 * To add a new type:
 *   1. Define the new type in ScriptTypes.h.
 *   2. Implement operator>>(std::istream&, T&) for this type in ScriptTypes.cpp.
 *   3. Add the type to std::variant of the SScriptEntry struct.
 *   4. Add the name of this type to EEntryType.
 *   5. Define the parsing rule for this name in ParseScriptEntry().
 * To add a new script key of existing type:
 *   1. Add it to EScriptKeys.
 *   2. Set the correspondence in AllScriptArguments().
 *   3. Use in ScriptRunner.cpp, e.g. as job.GetValue<double>(EScriptKeys::SIMULATION_TIME).
 */

#pragma once
#include "DyssolHelperDefines.h"
#include "ContainerFunctions.h"
#include "StringFunctions.h"
#include "ScriptTypes.h"
#include <filesystem>
#include <variant>

namespace ScriptInterface
{
	// All possible script keys, as they appear in script files.
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
		GRIDS_KEEP_EXISTING_VALUES,
		DISTRIBUTION_GRID,
	};

	// All possible types of script entries.
	enum class EEntryType
	{
		EMPTY              , // Key without value
		BOOL               , // bool
		INT                , // int64_t
		UINT               , // uint64_t
		DOUBLE             , // double
		STRING             , // std::string
		PATH               , // std::filesystem::path
		NAME_OR_KEY        , // SNameOrKey
		UNIT_PARAMETER     , // SUnitParameterSE
		HOLDUP_DEPENDENT   , // SHoldupDependentSE
		HOLDUP_COMPOUNDS   , // SHoldupCompoundSE
		HOLDUP_DISTRIBUTION, // SHoldupDistributionSE
		GRID_DIMENSION     , // SGridDimensionSE
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
			SNameOrKey, SUnitParameterSE, SHoldupDependentSE, SHoldupCompoundsSE, SHoldupDistributionSE,
			SGridDimensionSE> value{};

		SScriptEntry() = default;
		SScriptEntry(const SScriptEntryDescriptor& _descr) : SScriptEntryDescriptor{ _descr } {}
	};

	// Reads the string line from the input stream into SScriptEntry depending on the entry type.
	inline void ParseScriptEntry(SScriptEntry& _entry, std::istream& is)
	{
		switch (_entry.type)
		{
		case EEntryType::EMPTY:																									break;
		case EEntryType::BOOL:					_entry.value = StringFunctions::GetValueFromStream<bool>(is);					break;
		case EEntryType::INT:					_entry.value = StringFunctions::GetValueFromStream<int64_t>(is);				break;
		case EEntryType::UINT:					_entry.value = StringFunctions::GetValueFromStream<uint64_t>(is);				break;
		case EEntryType::DOUBLE:				_entry.value = StringFunctions::GetValueFromStream<double>(is);					break;
		case EEntryType::STRING:				_entry.value = StringFunctions::GetRestOfLine(is);								break;
		case EEntryType::PATH:					_entry.value = std::filesystem::path{ StringFunctions::GetRestOfLine(is) };		break;
		case EEntryType::NAME_OR_KEY:			_entry.value = StringFunctions::GetValueFromStream<SNameOrKey>(is);				break;
		case EEntryType::UNIT_PARAMETER:		_entry.value = StringFunctions::GetValueFromStream<SUnitParameterSE>(is);		break;
		case EEntryType::HOLDUP_DEPENDENT:		_entry.value = StringFunctions::GetValueFromStream<SHoldupDependentSE>(is);		break;
		case EEntryType::HOLDUP_COMPOUNDS:		_entry.value = StringFunctions::GetValueFromStream<SHoldupCompoundsSE>(is);		break;
		case EEntryType::HOLDUP_DISTRIBUTION:	_entry.value = StringFunctions::GetValueFromStream<SHoldupDistributionSE>(is);	break;
		case EEntryType::GRID_DIMENSION:	    _entry.value = StringFunctions::GetValueFromStream<SGridDimensionSE>(is);		break;
		}
	}

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
			// flowsheet settings
			MAKE_ARG(EScriptKeys::GRIDS_KEEP_EXISTING_VALUES  , EEntryType::BOOL),
			MAKE_ARG(EScriptKeys::DISTRIBUTION_GRID           , EEntryType::GRID_DIMENSION),
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
