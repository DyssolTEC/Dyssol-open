/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolTypes.h"
#include <algorithm>
#include <vector>

class CFlowsheet;
class CModelsManager;
class CMaterialsDatabase;

struct SGridDimensionEx
{
	unsigned iGrid{};					                        // Grid index.
	EGridEntry gridType{ EGridEntry::GRID_NUMERIC };            // Type of grid entries (GRID_NUMERIC | GRID_SYMBOLIC).
	EGridFunction gridFun{ EGridFunction::GRID_FUN_UNDEFINED }; // Function, that was use to define the grid (GRID_FUN_MANUAL | GRID_FUN_EQUIDISTANT |...).
	unsigned nClasses{};				                        // Number of classes.
	std::vector<double> vNumGrid;		                        // Grid for GRID_NUMERIC.
	std::vector<std::string> vStrGrid;	                        // Grid for GRID_SYMBOLIC.
};

struct SUnitParameterEx
{
	size_t iUnit{};                // Index of unit.
	size_t iParam{};               // Index of parameter in unit.
	double dValue{};               // Numeric value.
	std::vector<STDValue> tdValue; // Time dependent value.
	std::string sValue;            // String value.
};

struct SHoldupParam
{
	size_t iUnit{};                                     // Index of unit.
	size_t iHoldup{};                                   // Index of holdup in unit.
	size_t iTimePoint{};                                // Index of time point in holdup.
	size_t iPhase{};                                    // Index of phase.
	size_t iCompound{};                                 // Index of compound.
	size_t iDistribution{};                             // Index of distribution.
	EPSDTypes psdType{ PSD_MassFrac };                  // Type of PSD distribution.
	EDistrFunction distrFun{ EDistrFunction::Manual };  // Type of distribution function (Manual | Normal | RRSB | GGS).
	EPSDGridType psdGridType{ EPSDGridType::DIAMETER }; // Type of grid for PSD distribution (DIAMETER | VOLUME).
	std::vector<double> vValues;                        // Grid values.
};

struct SExportStreamDataMass
{
	std::string streamName;
	std::vector<double> timePoints;
};

class CConfigFileParser
{
	enum class EArgType { argDOUBLE, argUNSIGNED, argSTRING, argSTRINGS, argGRIDS, argUNITS, argHLDP_DISTRS, argHLDP_COMPS, argHLDP_SOLIDS, argEXPORT_STREAM_DATA };
	struct SArgument
	{
		EArguments name;     // Argument (SOURCE_FILE | SIMULATION_TIME | UNIT_PARAMETER |...).
		std::string nameStr; // A string version of argument.
		EArgType type;       // Type of the argument (argDOUBLE | argUNSIGNED | argSTRING |...).
		void* value;         // The value of the argument. Can be a single value (e.g. for argDOUBLE) or a vector of values of the same type (e.g. for argUNITS). If set to nullptr, the particular argument is not loaded from config file.
	};

	std::vector<SArgument> m_arguments; // List of all possible arguments.

public:
	CConfigFileParser();
	~CConfigFileParser();
	CConfigFileParser(const CConfigFileParser& _other)                = delete;
	CConfigFileParser(CConfigFileParser&& _other) noexcept            = delete;
	CConfigFileParser& operator=(const CConfigFileParser& _other)     = delete;
	CConfigFileParser& operator=(CConfigFileParser&& _other) noexcept = delete;

	// Saves the flowsheet file as a config file.
	static void SaveConfigFile(const std::wstring& _fileName, const std::wstring& _flowsheetFile, const CFlowsheet& _flowsheet, const CModelsManager& _modelsManager, const CMaterialsDatabase& _materialsDB);

	bool Parse(const std::string& _sFile);				// Parses config file and returns true on success.
	bool IsValueDefined(const EArguments& _key) const;	// Returns true if specified argument was set by user.

	// Returns a specified value.
	template<typename T>
	T GetValue(const EArguments& _key) const
	{
		auto arg = std::find_if(m_arguments.begin(), m_arguments.end(), [_key](const SArgument& a)->bool { return a.name == _key; });
		if (arg != m_arguments.end() && arg->value)
			return *static_cast<T*>(arg->value);
		return T{};
	}

private:
	SGridDimensionEx CreateGridFromSS(std::stringstream& _ss) const;
	SUnitParameterEx CreateUnitFromSS(std::stringstream& _ss) const;
	SHoldupParam CreateDistrFromSS(std::stringstream& _ss) const;
	SHoldupParam CreateCompoundDistrFromSS(std::stringstream& _ss) const;
	SHoldupParam CreateSolidDistrFromSS(std::stringstream& _ss) const;
	SExportStreamDataMass ExportStreamDataFromSS(std::stringstream& _ss) const;
	void ClearArguments();

	static void AllocateMemory(SArgument* _arg);
	static void DeallocateMemory(SArgument* _arg);
};
