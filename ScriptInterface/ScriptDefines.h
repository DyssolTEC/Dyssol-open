/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include "DyssolHelperDefines.h"
#include "ContainerFunctions.h"
#include <string>
#include <utility>
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
	};

	// All possible types of script entries.
	enum class EEntryType
	{
		EMPTY,		// Key without value
		DOUBLE,		// double
		STRING,		// std::string
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
		std::variant<double, std::string> value{};

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
	 * Name of the key from EScriptKeys is converted to a string and used as a key for script file. */
	inline std::vector<SScriptEntryDescriptor> AllScriptArguments()
	{
		return {
			MAKE_ARG(EScriptKeys::JOB               , EEntryType::EMPTY),
			MAKE_ARG(EScriptKeys::SOURCE_FILE       , EEntryType::STRING),
			MAKE_ARG(EScriptKeys::RESULT_FILE       , EEntryType::STRING),
			MAKE_ARG(EScriptKeys::MATERIALS_DATABASE, EEntryType::STRING),
			MAKE_ARG(EScriptKeys::MODELS_PATH       , EEntryType::STRING),
			MAKE_ARG(EScriptKeys::SIMULATION_TIME   , EEntryType::DOUBLE),
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
