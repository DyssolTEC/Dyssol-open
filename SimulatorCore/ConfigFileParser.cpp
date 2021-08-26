/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ConfigFileParser.h"
#include "MultidimensionalGrid.h"
#include "ModelsManager.h"
#include "Flowsheet.h"
#include "BaseUnit.h"
#include "MaterialsDatabase.h"
#include "StringFunctions.h"
#include "FileSystem.h"
#include "DyssolUtilities.h"
#include "DyssolHelperDefines.h"
#include "DyssolStringConstants.h"
#include <fstream>
#include <sstream>

using namespace StringFunctions;

#define MAKE_ARGUMENT(ARGUM_NAME, ARGUM_TYPE) { ARGUM_NAME, StringFunctions::TrimWhitespaces(std::string(TO_ARG_STR(ARGUM_NAME))), ARGUM_TYPE, nullptr }

CConfigFileParser::CConfigFileParser()
{
	m_arguments = {
		MAKE_ARGUMENT(EArguments::SOURCE_FILE,			          EArgType::argSTRING),
		MAKE_ARGUMENT(EArguments::RESULT_FILE,			          EArgType::argSTRING),
		MAKE_ARGUMENT(EArguments::MATERIALS_DATABASE,	          EArgType::argSTRING),
		MAKE_ARGUMENT(EArguments::MODELS_PATH,			          EArgType::argSTRINGS),
		MAKE_ARGUMENT(EArguments::SIMULATION_TIME,		          EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::RELATIVE_TOLERANCE,	          EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::ABSOLUTE_TOLERANCE,	          EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::MINIMAL_FRACTION,		          EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::INIT_TIME_WINDOW,		          EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::MIN_TIME_WINDOW,		          EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::MAX_TIME_WINDOW,		          EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::MAX_ITERATIONS_NUM,	          EArgType::argUNSIGNED),
		MAKE_ARGUMENT(EArguments::WINDOW_CHANGE_RATE,	          EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::ITER_UPPER_LIMIT,		          EArgType::argUNSIGNED),
		MAKE_ARGUMENT(EArguments::ITER_LOWER_LIMIT,		          EArgType::argUNSIGNED),
		MAKE_ARGUMENT(EArguments::ITER_UPPER_LIMIT_1,	          EArgType::argUNSIGNED),
		MAKE_ARGUMENT(EArguments::CONVERGENCE_METHOD,	          EArgType::argUNSIGNED),
		MAKE_ARGUMENT(EArguments::ACCEL_PARAMETER,		          EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::RELAX_PARAMETER,		          EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::EXTRAPOL_METHOD,		          EArgType::argUNSIGNED),
		MAKE_ARGUMENT(EArguments::DISTRIBUTION_GRID,	          EArgType::argGRIDS),
		MAKE_ARGUMENT(EArguments::UNIT_PARAMETER,		          EArgType::argUNITS),
		MAKE_ARGUMENT(EArguments::UNIT_HOLDUP_MTP,		          EArgType::argHLDP_DISTRS),
		MAKE_ARGUMENT(EArguments::UNIT_HOLDUP_PHASES,	          EArgType::argHLDP_DISTRS),
		MAKE_ARGUMENT(EArguments::UNIT_HOLDUP_COMP,		          EArgType::argHLDP_COMPS),
		MAKE_ARGUMENT(EArguments::UNIT_HOLDUP_SOLID,	          EArgType::argHLDP_SOLIDS),
		MAKE_ARGUMENT(EArguments::TEXT_EXPORT_FILE,	              EArgType::argSTRING),
		MAKE_ARGUMENT(EArguments::TEXT_EXPORT_PRECISION,          EArgType::argUNSIGNED),
		MAKE_ARGUMENT(EArguments::TEXT_EXPORT_FIXED_POINT,        EArgType::argUNSIGNED),
		MAKE_ARGUMENT(EArguments::TEXT_EXPORT_SIGNIFICANCE_LIMIT, EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::EXPORT_MASS,	                  EArgType::argEXPORT_STREAM_DATA),
		MAKE_ARGUMENT(EArguments::EXPORT_PSD,	                  EArgType::argEXPORT_STREAM_DATA),
		MAKE_ARGUMENT(EArguments::EXPORT_GRAPH,	                  EArgType::argSTRING)
	};
}

CConfigFileParser::~CConfigFileParser()
{
	ClearArguments();
}

void CConfigFileParser::SaveConfigFile(const std::wstring& _fileName, const std::wstring& _flowsheetFile, const CFlowsheet& _flowsheet, const CModelsManager& _modelsManager, const CMaterialsDatabase& _materialsDB)
{
	std::ofstream file(UnicodePath(_fileName));
	if (file.fail()) return;

	// file names
	file << TO_ARG_STR(EArguments::SOURCE_FILE) << " " << WString2String(_flowsheetFile) << std::endl;
	file << TO_ARG_STR(EArguments::RESULT_FILE) << " " << WString2String(FileSystem::FilePath(_flowsheetFile) + L"/" + FileSystem::FileName(_flowsheetFile) + L"_res." + FileSystem::FileExtension(_flowsheetFile)) << std::endl;

	// models manager
	for (size_t i = 0; i < _modelsManager.DirsNumber(); ++i)
		if (_modelsManager.GetDirActivity(i))
			file << TO_ARG_STR(EArguments::MODELS_PATH) << " " << WString2String(_modelsManager.GetDirPath(i)) << std::endl;

	// materials database
	file << TO_ARG_STR(EArguments::MATERIALS_DATABASE) << " " << WString2String(_materialsDB.GetFileName()) << std::endl;
	file << std::endl;

	// simulation options
	const auto& parmas = _flowsheet.GetParameters();
	file << TO_ARG_STR(EArguments::SIMULATION_TIME)    << " " << parmas->endSimulationTime << std::endl;
	file << TO_ARG_STR(EArguments::RELATIVE_TOLERANCE) << " " << parmas->relTol << std::endl;
	file << TO_ARG_STR(EArguments::ABSOLUTE_TOLERANCE) << " " << parmas->absTol << std::endl;
	file << TO_ARG_STR(EArguments::MINIMAL_FRACTION)   << " " << parmas->minFraction << std::endl;
	file << TO_ARG_STR(EArguments::INIT_TIME_WINDOW)   << " " << parmas->initTimeWindow << std::endl;
	file << TO_ARG_STR(EArguments::MIN_TIME_WINDOW)    << " " << parmas->minTimeWindow << std::endl;
	file << TO_ARG_STR(EArguments::MAX_TIME_WINDOW)    << " " << parmas->maxTimeWindow << std::endl;
	file << TO_ARG_STR(EArguments::MAX_ITERATIONS_NUM) << " " << parmas->maxItersNumber << std::endl;
	file << TO_ARG_STR(EArguments::WINDOW_CHANGE_RATE) << " " << parmas->magnificationRatio << std::endl;
	file << TO_ARG_STR(EArguments::ITER_UPPER_LIMIT)   << " " << parmas->itersUpperLimit << std::endl;
	file << TO_ARG_STR(EArguments::ITER_LOWER_LIMIT)   << " " << parmas->itersLowerLimit << std::endl;
	file << TO_ARG_STR(EArguments::ITER_UPPER_LIMIT_1) << " " << parmas->iters1stUpperLimit << std::endl;
	file << TO_ARG_STR(EArguments::CONVERGENCE_METHOD) << " " << parmas->convergenceMethod << std::endl;
	file << TO_ARG_STR(EArguments::ACCEL_PARAMETER)    << " " << parmas->wegsteinAccelParam << std::endl;
	file << TO_ARG_STR(EArguments::RELAX_PARAMETER)    << " " << parmas->relaxationParam << std::endl;
	file << TO_ARG_STR(EArguments::EXTRAPOL_METHOD)    << " " << E2I(static_cast<EExtrapMethod>(parmas->extrapolationMethod)) << std::endl;
	file << std::endl;

	// distributions grid
	const auto& grid = _flowsheet.GetGrid();
	int i = 0;
	for (const auto& gridDim : grid.GetGridDimensions())
	{
		const EGridEntry type = gridDim->GridType();
		file << TO_ARG_STR(EArguments::DISTRIBUTION_GRID) << " " << i + 1 << " " << E2I(type) << " " << gridDim->ClassesNumber() << " ";
		switch (type)
		{
		case EGridEntry::GRID_NUMERIC:
			file << E2I(EGridFunction::GRID_FUN_MANUAL) << " ";
			for (double v : dynamic_cast<const CGridDimensionNumeric*>(gridDim)->Grid())
				file << " " << v;
			break;
		case EGridEntry::GRID_SYMBOLIC:
			for (const std::string& v : dynamic_cast<const CGridDimensionSymbolic*>(gridDim)->Grid())
				file << " " << v;
			break;
		}
		file << "\t" << StrConst::COMMENT_SYMBOL << " " << std::vector<std::string>{ DISTR_NAMES }[GetDistributionTypeIndex(gridDim->DimensionType())] << std::endl;
		i++;
	}
	file << std::endl;

	// units parameters
	const auto& units = _flowsheet.GetAllUnits();
	for (size_t iUnit = 0; iUnit < units.size(); ++iUnit)
	{
		if (!units[iUnit]->GetModel()) continue;
		const auto& params = units[iUnit]->GetModel()->GetUnitParametersManager().GetParameters();
		for (size_t iParam = 0; iParam < params.size(); ++iParam)
		{
			file << TO_ARG_STR(EArguments::UNIT_PARAMETER) << " " << iUnit + 1 << " " << iParam + 1;
			switch (params[iParam]->GetType())
			{
			case EUnitParameter::CONSTANT:			[[fallthrough]];
			case EUnitParameter::CONSTANT_DOUBLE:	file << " " << dynamic_cast<const CConstRealUnitParameter*>(params[iParam])->GetValue();	break;
			case EUnitParameter::CONSTANT_INT64:	file << " " << dynamic_cast<const CConstIntUnitParameter*>(params[iParam])->GetValue();		break;
			case EUnitParameter::CONSTANT_UINT64:	file << " " << dynamic_cast<const CConstUIntUnitParameter*>(params[iParam])->GetValue();	break;
			case EUnitParameter::TIME_DEPENDENT:	file << " " << dynamic_cast<const CTDUnitParameter*>(params[iParam])->GetTDData();			break;
			case EUnitParameter::STRING:			file << " " << dynamic_cast<const CStringUnitParameter*>(params[iParam])->GetValue();		break;
			case EUnitParameter::COMBO:				[[fallthrough]];
			case EUnitParameter::GROUP:				file << " " << dynamic_cast<const CComboUnitParameter*>(params[iParam])->GetValue();		break;
			case EUnitParameter::CHECKBOX:			file << " " << dynamic_cast<const CCheckBoxUnitParameter*>(params[iParam])->IsChecked();	break;
			case EUnitParameter::COMPOUND:			file << " " << dynamic_cast<const CCompoundUnitParameter*>(params[iParam])->GetCompound();	break;
			case EUnitParameter::MDB_COMPOUND:		file << " " << dynamic_cast<const CMDBCompoundUnitParameter*>(params[iParam])->GetCompound();	break;
			case EUnitParameter::SOLVER:			file << " " << WString2String(_modelsManager.GetSolverLibName(dynamic_cast<const CSolverUnitParameter*>(params[iParam])->GetKey()));	break;
			case EUnitParameter::REACTION:			break; // TODO: implement
			case EUnitParameter::LIST_DOUBLE:		break; // TODO: implement
			case EUnitParameter::LIST_INT64:		break; // TODO: implement
			case EUnitParameter::LIST_UINT64:		break; // TODO: implement
			case EUnitParameter::UNKNOWN:			break;
			}
			file << "\t" << StrConst::COMMENT_SYMBOL << " " << units[iUnit]->GetName() << " - " << params[iParam]->GetName() << " - <Values>" << std::endl;
		}
	}
	file << std::endl;

	// TODO: save also other overall properties
	// mass/temperature/pressure of initial holdups/feeds
	for (size_t iUnit = 0; iUnit < units.size(); ++iUnit)
	{
		if (!units[iUnit]->GetModel()) continue;
		const auto& holdups = units[iUnit]->GetModel()->GetStreamsManager().GetAllInit();
		for (size_t iHoldup = 0; iHoldup < holdups.size(); ++iHoldup)
		{
			const std::vector<double> tp = holdups[iHoldup]->GetAllTimePoints();
			for (size_t iTime = 0; iTime < tp.size(); ++iTime)
				file << TO_ARG_STR(EArguments::UNIT_HOLDUP_MTP) << " " << iUnit + 1 << " " << iHoldup + 1 << " " << iTime + 1 << " " <<
				holdups[iHoldup]->GetMass(tp[iTime]) << " " << holdups[iHoldup]->GetTemperature(tp[iTime]) << " " << holdups[iHoldup]->GetPressure(tp[iTime]);
			file << "\t" << StrConst::COMMENT_SYMBOL << " " << units[iUnit]->GetName() << " - " << holdups[iHoldup]->GetName() << " - <Mass> - <Temperature> - <Pressure>" << std::endl;
		}
	}
	file << std::endl;

	// phase fractions of initial holdups/feeds
	for (size_t iUnit = 0; iUnit < units.size(); ++iUnit)
	{
		if (!units[iUnit]->GetModel()) continue;
		const auto& holdups = units[iUnit]->GetModel()->GetStreamsManager().GetAllInit();
		for (size_t iHoldup = 0; iHoldup < holdups.size(); ++iHoldup)
		{
			const std::vector<double> tp = holdups[iHoldup]->GetAllTimePoints();
			for (size_t iTime = 0; iTime < tp.size(); ++iTime)
			{
				file << TO_ARG_STR(EArguments::UNIT_HOLDUP_PHASES) << " " << iUnit + 1 << " " << iHoldup + 1 << " " << iTime + 1;
				for (const auto& phase : _flowsheet.GetPhases())
					file << " " << holdups[iHoldup]->GetPhaseFraction(tp[iTime], phase.state);
				file << "\t" << StrConst::COMMENT_SYMBOL << " " << units[iUnit]->GetName() << " - " << holdups[iHoldup]->GetName() << " - " << tp[iTime] << "[s]";
				for (const auto& phase : _flowsheet.GetPhases())
					file << " - " << phase.name;
				file << std::endl;
			}
		}
	}
	file << std::endl;

	// compounds fractions of initial holdups/feeds
	for (size_t iUnit = 0; iUnit < units.size(); ++iUnit)
	{
		if (!units[iUnit]->GetModel()) continue;
		const auto& holdups = units[iUnit]->GetModel()->GetStreamsManager().GetAllInit();
		for (size_t iHoldup = 0; iHoldup < holdups.size(); ++iHoldup)
		{
			const std::vector<double> tp = holdups[iHoldup]->GetAllTimePoints();
			const auto& phases = _flowsheet.GetPhases();
			for (size_t iPhase = 0; iPhase < phases.size(); ++iPhase)
				for (size_t iTime = 0; iTime < tp.size(); ++iTime)
				{
					file << TO_ARG_STR(EArguments::UNIT_HOLDUP_COMP) << " " << iUnit + 1 << " " << iHoldup + 1 << " " << iPhase + 1 << " " << iTime + 1;
					for (const auto& compoundKey : _flowsheet.GetCompounds())
						file << " " << holdups[iHoldup]->GetCompoundFraction(tp[iTime], compoundKey, phases[iPhase].state);
					file << "\t" << StrConst::COMMENT_SYMBOL << " " << units[iUnit]->GetName() << " - " << holdups[iHoldup]->GetName() << " - " <<
						phases[iPhase].name << " - " << tp[iTime] << "[s]";
					for (const auto& compoundKey : _flowsheet.GetCompounds())
						file << " - " << (_materialsDB.GetCompound(compoundKey) ? _materialsDB.GetCompound(compoundKey)->GetName() : "");
					file << std::endl;
				}
		}
	}
	file << std::endl;

	// distributions of initial holdups/feeds
	for (size_t iUnit = 0; iUnit < units.size(); ++iUnit)
	{
		if (!units[iUnit]->GetModel()) continue;
		const auto& holdups = units[iUnit]->GetModel()->GetStreamsManager().GetAllInit();
		for (size_t iHoldup = 0; iHoldup < holdups.size(); ++iHoldup)
		{
			const std::vector<double> tp = holdups[iHoldup]->GetAllTimePoints();
			const std::vector<EDistrTypes> distrs = grid.GetDimensionsTypes();
			for (size_t iDistr = 1; iDistr < distrs.size(); ++iDistr)
			{
				const auto& compoundKey = _flowsheet.GetCompounds();
				for (size_t iComp = 0; iComp < compoundKey.size(); ++iComp)
					for (size_t iTime = 0; iTime < tp.size(); ++iTime)
					{
						file << TO_ARG_STR(EArguments::UNIT_HOLDUP_SOLID) << " " << iUnit + 1 << " " << iHoldup + 1 << " " << iDistr + 1 << " " << iComp + 1 << " " << iTime + 1 << " " <<
							PSD_MassFrac << " " << E2I(EDistrFunction::Manual) << " " << E2I(EPSDGridType::DIAMETER);
						for (auto v : holdups[iHoldup]->GetDistribution(tp[iTime], distrs[iDistr], compoundKey[iComp]))
							file << " " << v;
						file << "\t" << StrConst::COMMENT_SYMBOL << " " << units[iUnit]->GetName() << " - " << holdups[iHoldup]->GetName() << " - " <<
							(_materialsDB.GetCompound(compoundKey[iComp]) ? _materialsDB.GetCompound(compoundKey[iComp])->GetName() : "") << " - " << tp[iTime] << "[s] - PSD_MassFrac - Manual - DIAMETER - <Values>" << std::endl;
					}
			}
		}
	}
}

bool CConfigFileParser::Parse(const std::string& _sFile)
{
	std::cout << "Starting parsing configuration file " << _sFile << std::endl;
	ClearArguments();

	std::ifstream configFile(UnicodePath(_sFile));

	if (!configFile.good()) return false;

	std::string line;
	while (std::getline(configFile, line))
	{
		std::stringstream ss(line);
		const std::string sKey = GetValueFromStream<std::string>(&ss);
		auto arg = std::find_if(m_arguments.begin(), m_arguments.end(), [sKey](const SArgument& a)->bool { return a.nameStr == sKey; });
		if (arg == m_arguments.end())
		{
			if (!sKey.empty() && (sKey.size() < StrConst::COMMENT_SYMBOL.size() || sKey.substr(0, StrConst::COMMENT_SYMBOL.size()) != StrConst::COMMENT_SYMBOL))
				std::cout << "Warning: Unknown key found: " << sKey << std::endl;
			continue;
		}

		if (!arg->value)
			AllocateMemory(&*arg);
		switch (arg->type)
		{
		case EArgType::argDOUBLE:
			*static_cast<double*>(arg->value) = GetValueFromStream<double>(&ss);
			break;
		case EArgType::argUNSIGNED:
			*static_cast<unsigned*>(arg->value) = GetValueFromStream<unsigned>(&ss);
			break;
		case EArgType::argSTRING:
			*static_cast<std::wstring*>(arg->value) = String2WString(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL));
			break;
		case EArgType::argSTRINGS:
			static_cast<std::vector<std::wstring>*>(arg->value)->push_back(String2WString(TrimFromSymbols(GetRestOfLine(&ss), StrConst::COMMENT_SYMBOL)));
			break;
		case EArgType::argGRIDS:
			static_cast<std::vector<SGridDimensionEx>*>(arg->value)->push_back(CreateGridFromSS(ss));
			break;
		case EArgType::argUNITS:
			static_cast<std::vector<SUnitParameterEx>*>(arg->value)->push_back(CreateUnitFromSS(ss));
			break;
		case EArgType::argHLDP_DISTRS:
			static_cast<std::vector<SHoldupParam>*>(arg->value)->push_back(CreateDistrFromSS(ss));
			break;
		case EArgType::argHLDP_COMPS:
			static_cast<std::vector<SHoldupParam>*>(arg->value)->push_back(CreateCompoundDistrFromSS(ss));
			break;
		case EArgType::argHLDP_SOLIDS:
			static_cast<std::vector<SHoldupParam>*>(arg->value)->push_back(CreateSolidDistrFromSS(ss));
			break;
		case EArgType::argEXPORT_STREAM_DATA:
			static_cast<std::vector<SExportStreamDataMass>*>(arg->value)->push_back(ExportStreamDataFromSS(ss));
			break;
		}
	}
	configFile.close();

	return true;
}

bool CConfigFileParser::IsValueDefined(const EArguments& _key) const
{
	const auto arg = std::find_if(m_arguments.begin(), m_arguments.end(), [_key](const SArgument& a)->bool { return a.name == _key; });
	return arg != m_arguments.end() && arg->value;
}

SGridDimensionEx CConfigFileParser::CreateGridFromSS(std::stringstream& _ss) const
{
	SGridDimensionEx grid;
	grid.iGrid = GetValueFromStream<unsigned>(&_ss) - 1;
	grid.gridType = static_cast<EGridEntry>(GetValueFromStream<unsigned>(&_ss));
	grid.nClasses = GetValueFromStream<unsigned>(&_ss);
	if (grid.gridType == EGridEntry::GRID_NUMERIC)
		grid.gridFun = static_cast<EGridFunction>(GetValueFromStream<unsigned>(&_ss));
	std::stringstream ss2(TrimFromSymbols(GetRestOfLine(&_ss), StrConst::COMMENT_SYMBOL));
	if (grid.gridType == EGridEntry::GRID_NUMERIC && grid.gridFun == EGridFunction::GRID_FUN_MANUAL)
	{
		const size_t len = grid.gridType == EGridEntry::GRID_NUMERIC ? grid.nClasses + 1 : grid.nClasses * 2;
		grid.vNumGrid.resize(len, 0);
		size_t ind = 0;
		while (!ss2.eof() && ss2.good() && ind < len)
			ss2 >> grid.vNumGrid[ind++];
	}
	else if (grid.gridType == EGridEntry::GRID_SYMBOLIC)
	{
		grid.vStrGrid.resize(grid.nClasses, "");
		size_t ind = 0;
		while (!ss2.eof() && ss2.good() && (ind < grid.nClasses))
			ss2 >> grid.vStrGrid[ind++];
	}
	else
		grid.vNumGrid = CreateGrid(grid.gridFun, grid.nClasses, GetValueFromStream<double>(&_ss), GetValueFromStream<double>(&_ss));

	return grid;
}

SUnitParameterEx CConfigFileParser::CreateUnitFromSS(std::stringstream& _ss) const
{
	SUnitParameterEx unit;
	unit.iUnit  = GetValueFromStream<size_t>(&_ss) - 1;
	unit.iParam = GetValueFromStream<size_t>(&_ss) - 1;
	unit.sValue = TrimFromSymbols(GetRestOfLine(&_ss), StrConst::COMMENT_SYMBOL);
	std::stringstream ss1(unit.sValue);
	unit.dValue = GetValueFromStream<double>(&ss1);
	if (ss1.eof())
		unit.tdValue.emplace_back(0, unit.dValue);
	else
	{
		std::stringstream ss2(unit.sValue);
		while (!ss2.eof() && ss2.good())
			unit.tdValue.emplace_back(GetValueFromStream<double>(&ss2), GetValueFromStream<double>(&ss2));
	}
	return unit;
}

SHoldupParam CConfigFileParser::CreateDistrFromSS(std::stringstream& _ss) const
{
	SHoldupParam holdup;
	holdup.iUnit      = GetValueFromStream<size_t>(&_ss) - 1;
	holdup.iHoldup    = GetValueFromStream<size_t>(&_ss) - 1;
	holdup.iTimePoint = GetValueFromStream<size_t>(&_ss) - 1;
	std::stringstream ss2(TrimFromSymbols(GetRestOfLine(&_ss), StrConst::COMMENT_SYMBOL));
	while (!ss2.eof() && ss2.good())
		holdup.vValues.push_back(GetValueFromStream<double>(&ss2));
	return holdup;
}

SHoldupParam CConfigFileParser::CreateCompoundDistrFromSS(std::stringstream& _ss) const
{
	SHoldupParam holdup;
	holdup.iUnit      = GetValueFromStream<size_t>(&_ss) - 1;
	holdup.iHoldup    = GetValueFromStream<size_t>(&_ss) - 1;
	holdup.iPhase     = GetValueFromStream<size_t>(&_ss) - 1;
	holdup.iTimePoint = GetValueFromStream<size_t>(&_ss) - 1;
	std::stringstream ss2(TrimFromSymbols(GetRestOfLine(&_ss), StrConst::COMMENT_SYMBOL));
	while (!ss2.eof() && ss2.good())
		holdup.vValues.push_back(GetValueFromStream<double>(&ss2));
	return holdup;
}

SHoldupParam CConfigFileParser::CreateSolidDistrFromSS(std::stringstream& _ss) const
{
	SHoldupParam holdup;
	holdup.iUnit         = GetValueFromStream<size_t>(&_ss) - 1;
	holdup.iHoldup       = GetValueFromStream<size_t>(&_ss) - 1;
	holdup.iDistribution = GetValueFromStream<size_t>(&_ss) - 1;
	holdup.iCompound     = GetValueFromStream<size_t>(&_ss) - 1;
	holdup.iTimePoint    = GetValueFromStream<size_t>(&_ss) - 1;
	holdup.psdType       = static_cast<EPSDTypes>(GetValueFromStream<unsigned>(&_ss));
	holdup.distrFun      = static_cast<EDistrFunction>(GetValueFromStream<unsigned>(&_ss));
	holdup.psdGridType   = static_cast<EPSDGridType>(GetValueFromStream<unsigned>(&_ss));
	std::stringstream ss2(TrimFromSymbols(GetRestOfLine(&_ss), StrConst::COMMENT_SYMBOL));
	while (!ss2.eof() && ss2.good())
		holdup.vValues.push_back(GetValueFromStream<double>(&ss2));
	return holdup;
}

SExportStreamDataMass CConfigFileParser::ExportStreamDataFromSS(std::stringstream& _ss) const
{
	SExportStreamDataMass res;
	res.streamName = GetValueFromStream<std::string>(&_ss);
	const auto times = TrimFromSymbols(GetRestOfLine(&_ss), StrConst::COMMENT_SYMBOL);
	if (!times.empty())
	{
		std::stringstream ss2(times);
		while (!ss2.eof() && ss2.good())
			res.timePoints.push_back(GetValueFromStream<double>(&ss2));
	}
	return res;
}

void CConfigFileParser::ClearArguments()
{
	for (SArgument& arg : m_arguments)
		if (arg.value)
			DeallocateMemory(&arg);
}

void CConfigFileParser::AllocateMemory(SArgument* _arg)
{
	if (_arg->value) return;
	switch (_arg->type)
	{
	case EArgType::argDOUBLE:		_arg->value = new double;							break;
	case EArgType::argUNSIGNED:		_arg->value = new unsigned;							break;
	case EArgType::argSTRING:		_arg->value = new std::wstring;						break;
	case EArgType::argSTRINGS:		_arg->value = new std::vector<std::wstring>();		break;
	case EArgType::argGRIDS:		_arg->value = new std::vector<SGridDimensionEx>();	break;
	case EArgType::argUNITS:		_arg->value = new std::vector<SUnitParameterEx>();	break;
	case EArgType::argHLDP_DISTRS:	_arg->value = new std::vector<SHoldupParam>();		break;
	case EArgType::argHLDP_COMPS:	_arg->value = new std::vector<SHoldupParam>();		break;
	case EArgType::argHLDP_SOLIDS:	_arg->value = new std::vector<SHoldupParam>();		break;
	case EArgType::argEXPORT_STREAM_DATA:	_arg->value = new std::vector<SExportStreamDataMass>();		break;
	}
}

void CConfigFileParser::DeallocateMemory(SArgument* _arg)
{
	switch (_arg->type)
	{
	case EArgType::argDOUBLE:		delete static_cast<double*>(_arg->value);							break;
	case EArgType::argUNSIGNED:		delete static_cast<unsigned*>(_arg->value);							break;
	case EArgType::argSTRING:		delete static_cast<std::wstring*>(_arg->value);						break;
	case EArgType::argSTRINGS:		delete static_cast<std::vector<std::wstring>*>(_arg->value);		break;
	case EArgType::argUNITS:		delete static_cast<std::vector<SUnitParameterEx>*>(_arg->value);	break;
	case EArgType::argGRIDS:		delete static_cast<std::vector<SGridDimensionEx>*>(_arg->value);	break;
	case EArgType::argHLDP_DISTRS:	delete static_cast<std::vector<SHoldupParam>*>(_arg->value);		break;
	case EArgType::argHLDP_COMPS:	delete static_cast<std::vector<SHoldupParam>*>(_arg->value);		break;
	case EArgType::argHLDP_SOLIDS:	delete static_cast<std::vector<SHoldupParam>*>(_arg->value);		break;
	case EArgType::argEXPORT_STREAM_DATA:	delete static_cast<std::vector<SExportStreamDataMass>*>(_arg->value);			break;
	}
	_arg->value = nullptr;
}
