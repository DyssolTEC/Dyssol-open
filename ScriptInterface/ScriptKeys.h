/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

/*
 * Main defines of the script interface.
 * All script keywords as they appear in the script are described in EScriptKeys.
 * Each value of EScriptKeys enum automatically obtains its string representation to work with.
 * EEntryType describes all possible types each line of the script can represent.
 * Each script line is read into SScriptEntry - it can hold any type defined in EEntryType.
 * How to read each type of EEntryType from a stream is given in ReadScriptEntry().
 * How to write each type of EEntryType to a stream is given in WriteScriptEntry().
 * To make these functions work, each type must define operator>>(std::istream&, T&) and operator<<(std::ostream&, const T&),
 * as it is given in ScriptTypes.h
 * The correspondence between EScriptKeys and EEntryType is set in allScriptArguments.
 * To add a new type:
 *   1. Define the new type in ScriptTypes.h.
 *   2. Implement operator>>(std::istream&, T&) and operator<<(std::ostream&, const T&) for this type in ScriptTypes.cpp.
 *   3. Add the type to std::variant of the SScriptEntry struct.
 *   4. Add the name of this type to EEntryType.
 *   5. Define the reading/writing rules for this name in ReadScriptEntry()/WriteScriptEntry().
 * To add a new script key of existing type:
 *   1. Add it to EScriptKeys.
 *   2. Set the correspondence in allScriptArguments.
 *   3. Use it in ScriptRunner.cpp, e.g. as job.GetValue<double>(EScriptKeys::SIMULATION_TIME).
 *	 4. Implement export of this key in ScriptExporter.cpp.
 */

#pragma once
#include "ScriptTypes.h"
#include "ContainerFunctions.h"
#include "DyssolHelperDefines.h"
#include <filesystem>
#include <variant>

namespace ScriptInterface
{
	// All possible script keys, as they appear in script files.
	enum class EScriptKeys
	{
		JOB                              ,
		SOURCE_FILE                      ,
		RESULT_FILE                      ,
		MATERIALS_DATABASE               ,
		MODELS_PATH                      ,
		SIMULATION_TIME                  ,
		RELATIVE_TOLERANCE               ,
		ABSOLUTE_TOLERANCE               ,
		MINIMAL_FRACTION                 ,
		SAVE_TIME_STEP_HINT              ,
		SAVE_FLAG_FOR_HOLDUPS            ,
		THERMO_TEMPERATURE_MIN           ,
		THERMO_TEMPERATURE_MAX           ,
		THERMO_TEMPERATURE_INTERVALS     ,
		INIT_TIME_WINDOW                 ,
		MIN_TIME_WINDOW                  ,
		MAX_TIME_WINDOW                  ,
		MAX_ITERATIONS_NUMBER            ,
		WINDOW_CHANGE_RATE               ,
		ITERATIONS_UPPER_LIMIT           ,
		ITERATIONS_LOWER_LIMIT           ,
		ITERATIONS_UPPER_LIMIT_1ST       ,
		CONVERGENCE_METHOD               ,
		RELAXATION_PARAMETER             ,
		ACCELERATION_LIMIT               ,
		EXTRAPOLATION_METHOD             ,
		COMPOUNDS                        ,
		PHASES                           ,
		KEEP_EXISTING_GRIDS_VALUES       ,
		DISTRIBUTION_GRID                ,
		KEEP_EXISTING_UNITS              ,
		UNIT                             ,
		KEEP_EXISTING_STREAMS            ,
		STREAM                           ,
		UNIT_PARAMETER                   ,
		KEEP_EXISTING_HOLDUPS_VALUES     ,
		HOLDUP_OVERALL                   ,
		HOLDUP_PHASES                    ,
		HOLDUP_COMPOUNDS                 ,
		HOLDUP_DISTRIBUTION              ,
		EXPORT_FILE                      ,
		EXPORT_PRECISION                 ,
		EXPORT_FIXED_POINT               ,
		EXPORT_SIGNIFICANCE_LIMIT        ,
		EXPORT_ONLY                      ,
		EXPORT_STREAM_MASS               ,
		EXPORT_STREAM_TEMPERATURE        ,
		EXPORT_STREAM_PRESSURE           ,
		EXPORT_STREAM_OVERALLS           ,
		EXPORT_STREAM_PHASES_FRACTIONS   ,
		EXPORT_STREAM_COMPOUNDS_FRACTIONS,
		EXPORT_STREAM_PSD                ,
		EXPORT_STREAM_DISTRIBUTIONS      ,
		EXPORT_HOLDUP_MASS               ,
		EXPORT_HOLDUP_TEMPERATURE        ,
		EXPORT_HOLDUP_PRESSURE           ,
		EXPORT_HOLDUP_OVERALLS           ,
		EXPORT_HOLDUP_PHASES_FRACTIONS   ,
		EXPORT_HOLDUP_COMPOUNDS_FRACTIONS,
		EXPORT_HOLDUP_PSD                ,
		EXPORT_HOLDUP_DISTRIBUTIONS      ,
		EXPORT_UNIT_STATE_VARIABLE       ,
		EXPORT_UNIT_PLOT                 ,
		EXPORT_FLOWSHEET_GRAPH           ,
	};

	// All possible types of script entries.
	enum class EEntryType
	{
		EMPTY              , // key without value
		BOOL               , // bool
		INT                , // int64_t
		UINT               , // uint64_t
		DOUBLE             , // double
		STRING             , // std::string
		STRINGS            , // std::vector<std::string>
		PATH               , // std::filesystem::path
		NAME_OR_KEY        , // SNamedEnum
		UNIT_PARAMETER     , // SUnitParameterSE
		HOLDUP_DEPENDENT   , // SHoldupDependentSE
		HOLDUP_COMPOUNDS   , // SHoldupCompoundSE
		HOLDUP_DISTRIBUTION, // SHoldupDistributionSE
		GRID_DIMENSION     , // SGridDimensionSE
		PHASES             , // SPhasesSE
		STREAM             , // SStreamSE
		EXPORT_STREAM      , // SExportStreamSE
		EXPORT_HOLDUP      , // SExportHoldupSE
		EXPORT_STATE_VAR   , // SExportStateVarSE
		EXPORT_PLOT        , // SExportPlotSE
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
		std::variant<bool, int64_t, uint64_t, double, std::string, std::vector<std::string>, std::filesystem::path,
			SNamedEnum, SUnitParameterSE, SHoldupDependentSE, SHoldupCompoundsSE, SHoldupDistributionSE,
			SGridDimensionSE, SPhasesSE, SStreamSE,
			SExportStreamSE, SExportHoldupSE, SExportStateVarSE, SExportPlotSE> value{};

		SScriptEntry() = default;
		SScriptEntry(const SScriptEntryDescriptor& _descr) : SScriptEntryDescriptor{ _descr } {}
	};

	// Reads the string line from the input stream into SScriptEntry depending on the entry type.
	inline void ReadScriptEntry(SScriptEntry& _entry, std::istream& is)
	{
		switch (_entry.type)
		{
		case EEntryType::EMPTY:																																break;
		case EEntryType::BOOL:					_entry.value = StringFunctions::GetValueFromStream<bool>(is);												break;
		case EEntryType::INT:					_entry.value = StringFunctions::GetValueFromStream<int64_t>(is);											break;
		case EEntryType::UINT:					_entry.value = StringFunctions::GetValueFromStream<uint64_t>(is);											break;
		case EEntryType::DOUBLE:				_entry.value = StringFunctions::GetValueFromStream<double>(is);												break;
		case EEntryType::STRING:				_entry.value = StringFunctions::RemoveQuotes(StringFunctions::GetRestOfLine(is));							break;
		case EEntryType::STRINGS:				_entry.value = StringFunctions::GetValueFromStream<std::vector<std::string>>(is);							break;
		case EEntryType::PATH:					_entry.value = std::filesystem::path{ StringFunctions::RemoveQuotes(StringFunctions::GetRestOfLine(is)) };	break;
		case EEntryType::NAME_OR_KEY:			_entry.value = StringFunctions::GetValueFromStream<SNamedEnum>(is);											break;
		case EEntryType::UNIT_PARAMETER:		_entry.value = StringFunctions::GetValueFromStream<SUnitParameterSE>(is);									break;
		case EEntryType::HOLDUP_DEPENDENT:		_entry.value = StringFunctions::GetValueFromStream<SHoldupDependentSE>(is);									break;
		case EEntryType::HOLDUP_COMPOUNDS:		_entry.value = StringFunctions::GetValueFromStream<SHoldupCompoundsSE>(is);									break;
		case EEntryType::HOLDUP_DISTRIBUTION:	_entry.value = StringFunctions::GetValueFromStream<SHoldupDistributionSE>(is);								break;
		case EEntryType::GRID_DIMENSION:	    _entry.value = StringFunctions::GetValueFromStream<SGridDimensionSE>(is);									break;
		case EEntryType::PHASES:				_entry.value = StringFunctions::GetValueFromStream<SPhasesSE>(is);											break;
		case EEntryType::STREAM:				_entry.value = StringFunctions::GetValueFromStream<SStreamSE>(is);											break;
		case EEntryType::EXPORT_STREAM:			_entry.value = StringFunctions::GetValueFromStream<SExportStreamSE>(is);									break;
		case EEntryType::EXPORT_HOLDUP:			_entry.value = StringFunctions::GetValueFromStream<SExportHoldupSE>(is);									break;
		case EEntryType::EXPORT_STATE_VAR:		_entry.value = StringFunctions::GetValueFromStream<SExportStateVarSE>(is);									break;
		case EEntryType::EXPORT_PLOT:			_entry.value = StringFunctions::GetValueFromStream<SExportPlotSE>(is);										break;
		}
	}

	// Writes SScriptEntry to the output stream depending on the entry type.
	inline void WriteScriptEntry(const SScriptEntry& _entry, std::ostream& os)
	{
		os << _entry.keyStr << " ";
		switch (_entry.type)
		{
		case EEntryType::EMPTY:					{ 																													break; }
		case EEntryType::BOOL:					{ os << (std::get<bool>(_entry.value) ? "YES" : "NO");																break; }
		case EEntryType::INT:					{ os << std::get<int64_t>(_entry.value);																			break; }
		case EEntryType::UINT:					{ os << std::get<uint64_t>(_entry.value);																			break; }
		case EEntryType::DOUBLE:				{ os << std::get<double>(_entry.value);																				break; }
		case EEntryType::STRING:				{ os << StringFunctions::Quote(std::get<std::string>(_entry.value));												break; }
		case EEntryType::STRINGS:				{ for (const auto& s : std::get<std::vector<std::string>>(_entry.value)) os << StringFunctions::Quote(s) << " ";	break; }
		case EEntryType::PATH:					{ os << std::get<std::filesystem::path>(_entry.value);																break; }
		case EEntryType::NAME_OR_KEY:			{ os << std::get<SNamedEnum>(_entry.value);																			break; }
		case EEntryType::UNIT_PARAMETER:		{ os << std::get<SUnitParameterSE>(_entry.value);																	break; }
		case EEntryType::HOLDUP_DEPENDENT:		{ os << std::get<SHoldupDependentSE>(_entry.value);																	break; }
		case EEntryType::HOLDUP_COMPOUNDS:		{ os << std::get<SHoldupCompoundsSE>(_entry.value);																	break; }
		case EEntryType::HOLDUP_DISTRIBUTION:	{ os << std::get<SHoldupDistributionSE>(_entry.value);																break; }
		case EEntryType::GRID_DIMENSION:	    { os << std::get<SGridDimensionSE>(_entry.value);																	break; }
		case EEntryType::PHASES:				{ os << std::get<SPhasesSE>(_entry.value);																			break; }
		case EEntryType::STREAM:				{ os << std::get<SStreamSE>(_entry.value);																			break; }
		case EEntryType::EXPORT_STREAM:			{ os << std::get<SExportStreamSE>(_entry.value);																	break; }
		case EEntryType::EXPORT_HOLDUP:			{ os << std::get<SExportHoldupSE>(_entry.value);																	break; }
		case EEntryType::EXPORT_STATE_VAR:		{ os << std::get<SExportStateVarSE>(_entry.value);																	break; }
		case EEntryType::EXPORT_PLOT:			{ os << std::get<SExportPlotSE>(_entry.value);																		break; }
		}
		os << std::endl;
	}

	// Number of symbols to discard: length of 'EScriptKeys::'.
	constexpr size_t DISCARD = 13;
	// Converts a fully resolved name (e.g. EScriptKeys::SOURCE_FILE to a string (e.g. "SOURCE_FILE").
	#define ARG2STR(X) std::string(MACRO_TOSTRING(X)).substr(DISCARD, std::string(MACRO_TOSTRING(X)).length())
	// Creates a new SScriptEntryDescriptor.
	#define MAKE_SED(ARG_NAME, ARG_TYPE) { ARG_NAME, std::string{ARG2STR(ARG_NAME)}, ARG_TYPE }

	/* Creates a vector of all possible script arguments and initializes them.
	 * Name of the key from EScriptKeys is converted to a string and used as a key for script file.
	 * To add a new key of existing type, extend EScriptKeys and add it with MAKE_SED macro.
	 * To add a new key of a new type, additionally extend EEntryType and extend SScriptEntry::value with the real value type. */
	static std::vector<SScriptEntryDescriptor> allScriptArguments
	{
		// paths
		MAKE_SED(EScriptKeys::JOB                              , EEntryType::EMPTY)              ,
		MAKE_SED(EScriptKeys::SOURCE_FILE                      , EEntryType::PATH)               ,
		MAKE_SED(EScriptKeys::RESULT_FILE                      , EEntryType::PATH)               ,
		MAKE_SED(EScriptKeys::MATERIALS_DATABASE               , EEntryType::PATH)               ,
		MAKE_SED(EScriptKeys::MODELS_PATH                      , EEntryType::PATH)               ,
		// flowsheet parameters
		MAKE_SED(EScriptKeys::SIMULATION_TIME                  , EEntryType::DOUBLE)             ,
		MAKE_SED(EScriptKeys::RELATIVE_TOLERANCE               , EEntryType::DOUBLE)             ,
		MAKE_SED(EScriptKeys::ABSOLUTE_TOLERANCE               , EEntryType::DOUBLE)             ,
		MAKE_SED(EScriptKeys::MINIMAL_FRACTION                 , EEntryType::DOUBLE)             ,
		MAKE_SED(EScriptKeys::SAVE_TIME_STEP_HINT              , EEntryType::DOUBLE)             ,
		MAKE_SED(EScriptKeys::SAVE_FLAG_FOR_HOLDUPS            , EEntryType::BOOL)               ,
		MAKE_SED(EScriptKeys::THERMO_TEMPERATURE_MIN           , EEntryType::DOUBLE)             ,
		MAKE_SED(EScriptKeys::THERMO_TEMPERATURE_MAX           , EEntryType::DOUBLE)             ,
		MAKE_SED(EScriptKeys::THERMO_TEMPERATURE_INTERVALS     , EEntryType::UINT)               ,
		MAKE_SED(EScriptKeys::INIT_TIME_WINDOW                 , EEntryType::DOUBLE)             ,
		MAKE_SED(EScriptKeys::MIN_TIME_WINDOW                  , EEntryType::DOUBLE)             ,
		MAKE_SED(EScriptKeys::MAX_TIME_WINDOW                  , EEntryType::DOUBLE)             ,
		MAKE_SED(EScriptKeys::MAX_ITERATIONS_NUMBER            , EEntryType::UINT)               ,
		MAKE_SED(EScriptKeys::WINDOW_CHANGE_RATE               , EEntryType::DOUBLE)             ,
		MAKE_SED(EScriptKeys::ITERATIONS_UPPER_LIMIT           , EEntryType::UINT)               ,
		MAKE_SED(EScriptKeys::ITERATIONS_LOWER_LIMIT           , EEntryType::UINT)               ,
		MAKE_SED(EScriptKeys::ITERATIONS_UPPER_LIMIT_1ST       , EEntryType::UINT)               ,
		MAKE_SED(EScriptKeys::CONVERGENCE_METHOD               , EEntryType::NAME_OR_KEY)        ,
		MAKE_SED(EScriptKeys::RELAXATION_PARAMETER             , EEntryType::DOUBLE)             ,
		MAKE_SED(EScriptKeys::ACCELERATION_LIMIT               , EEntryType::DOUBLE)             ,
		MAKE_SED(EScriptKeys::EXTRAPOLATION_METHOD             , EEntryType::NAME_OR_KEY)        ,
		// flowsheet settings
		MAKE_SED(EScriptKeys::COMPOUNDS                        , EEntryType::STRINGS)            ,
		MAKE_SED(EScriptKeys::PHASES                           , EEntryType::PHASES)             ,
		MAKE_SED(EScriptKeys::KEEP_EXISTING_GRIDS_VALUES       , EEntryType::BOOL)               ,
		MAKE_SED(EScriptKeys::DISTRIBUTION_GRID                , EEntryType::GRID_DIMENSION)     ,
		MAKE_SED(EScriptKeys::KEEP_EXISTING_UNITS              , EEntryType::BOOL)               ,
		MAKE_SED(EScriptKeys::UNIT                             , EEntryType::STRINGS)            ,
		MAKE_SED(EScriptKeys::KEEP_EXISTING_STREAMS            , EEntryType::BOOL)               ,
		MAKE_SED(EScriptKeys::STREAM                           , EEntryType::STREAM)             ,
		MAKE_SED(EScriptKeys::UNIT_PARAMETER                   , EEntryType::UNIT_PARAMETER)     ,
		// holdup and input streams parameters
		MAKE_SED(EScriptKeys::KEEP_EXISTING_HOLDUPS_VALUES     , EEntryType::BOOL)               ,
		MAKE_SED(EScriptKeys::HOLDUP_OVERALL                   , EEntryType::HOLDUP_DEPENDENT)   ,
		MAKE_SED(EScriptKeys::HOLDUP_PHASES                    , EEntryType::HOLDUP_DEPENDENT)   ,
		MAKE_SED(EScriptKeys::HOLDUP_COMPOUNDS                 , EEntryType::HOLDUP_COMPOUNDS)   ,
		MAKE_SED(EScriptKeys::HOLDUP_DISTRIBUTION              , EEntryType::HOLDUP_DISTRIBUTION),
		// export
		MAKE_SED(EScriptKeys::EXPORT_FILE                      , EEntryType::PATH)               ,
		MAKE_SED(EScriptKeys::EXPORT_PRECISION                 , EEntryType::INT)                ,
		MAKE_SED(EScriptKeys::EXPORT_FIXED_POINT               , EEntryType::BOOL)               ,
		MAKE_SED(EScriptKeys::EXPORT_SIGNIFICANCE_LIMIT        , EEntryType::DOUBLE)             ,
		MAKE_SED(EScriptKeys::EXPORT_ONLY                      , EEntryType::BOOL)               ,
		MAKE_SED(EScriptKeys::EXPORT_STREAM_MASS               , EEntryType::EXPORT_STREAM)      ,
		MAKE_SED(EScriptKeys::EXPORT_STREAM_TEMPERATURE        , EEntryType::EXPORT_STREAM)      ,
		MAKE_SED(EScriptKeys::EXPORT_STREAM_PRESSURE           , EEntryType::EXPORT_STREAM)      ,
		MAKE_SED(EScriptKeys::EXPORT_STREAM_OVERALLS           , EEntryType::EXPORT_STREAM)      ,
		MAKE_SED(EScriptKeys::EXPORT_STREAM_PHASES_FRACTIONS   , EEntryType::EXPORT_STREAM)      ,
		MAKE_SED(EScriptKeys::EXPORT_STREAM_COMPOUNDS_FRACTIONS, EEntryType::EXPORT_STREAM)      ,
		MAKE_SED(EScriptKeys::EXPORT_STREAM_PSD                , EEntryType::EXPORT_STREAM)      ,
		MAKE_SED(EScriptKeys::EXPORT_STREAM_DISTRIBUTIONS      , EEntryType::EXPORT_STREAM)      ,
		MAKE_SED(EScriptKeys::EXPORT_HOLDUP_MASS               , EEntryType::EXPORT_HOLDUP)      ,
		MAKE_SED(EScriptKeys::EXPORT_HOLDUP_TEMPERATURE        , EEntryType::EXPORT_HOLDUP)      ,
		MAKE_SED(EScriptKeys::EXPORT_HOLDUP_PRESSURE           , EEntryType::EXPORT_HOLDUP)      ,
		MAKE_SED(EScriptKeys::EXPORT_HOLDUP_OVERALLS           , EEntryType::EXPORT_HOLDUP)      ,
		MAKE_SED(EScriptKeys::EXPORT_HOLDUP_PHASES_FRACTIONS   , EEntryType::EXPORT_HOLDUP)      ,
		MAKE_SED(EScriptKeys::EXPORT_HOLDUP_COMPOUNDS_FRACTIONS, EEntryType::EXPORT_HOLDUP)      ,
		MAKE_SED(EScriptKeys::EXPORT_HOLDUP_PSD                , EEntryType::EXPORT_HOLDUP)      ,
		MAKE_SED(EScriptKeys::EXPORT_HOLDUP_DISTRIBUTIONS      , EEntryType::EXPORT_HOLDUP)      ,
		MAKE_SED(EScriptKeys::EXPORT_UNIT_STATE_VARIABLE       , EEntryType::EXPORT_STATE_VAR)   ,
		MAKE_SED(EScriptKeys::EXPORT_UNIT_PLOT                 , EEntryType::EXPORT_PLOT)        ,
		MAKE_SED(EScriptKeys::EXPORT_FLOWSHEET_GRAPH           , EEntryType::PATH)               ,
	};

	// Returns a vector of string representations all possible script keys.
	inline std::vector<std::string> AllScriptKeysStr()
	{
		auto res = ReservedVector<std::string>(allScriptArguments.size());
		for (const auto& a : allScriptArguments)
			res.push_back(a.keyStr);
		return res;
	}

	// Returns a vector of all possible script keys.
	inline std::vector<EScriptKeys> AllScriptKeys()
	{
		auto res = ReservedVector<EScriptKeys>(allScriptArguments.size());
		for (const auto& a : allScriptArguments)
			res.push_back(a.key);
		return res;
	}

	// Returns a script entry descriptor for the given key. If a descriptor for such key was not defined, returns a default descriptor with empty key.
	inline SScriptEntryDescriptor Key2Descriptor(const std::string& _key)
	{
		const size_t index = VectorFind(allScriptArguments, [&](const SScriptEntryDescriptor& _d) { return _d.keyStr == _key; });
		if (index == static_cast<size_t>(-1)) return {};
		return allScriptArguments[index];
	}

	// Converts a script key to its string representation.
	inline std::string StrKey(const EScriptKeys& _key)
	{
		const size_t index = VectorFind(allScriptArguments, [&](const SScriptEntryDescriptor& _d) { return _d.key == _key; });
		if (index == static_cast<size_t>(-1)) return {};
		return allScriptArguments[index].keyStr;
	}
}
