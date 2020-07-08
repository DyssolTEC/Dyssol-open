/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ConfigFileParser.h"
#include "DistributionsGrid.h"
#include "StringFunctions.h"
#include "DyssolHelperDefines.h"
#include "DyssolStringConstants.h"
#include <fstream>
#include <sstream>

using namespace StringFunctions;

#define MAKE_ARGUMENT(ARGUM_NAME, ARGUM_TYPE) { ARGUM_NAME, StringFunctions::TrimWhitespaces(std::string(TO_ARG_STR(ARGUM_NAME))), ARGUM_TYPE, nullptr }

CConfigFileParser::CConfigFileParser()
{
	m_arguments = {
		MAKE_ARGUMENT(EArguments::SOURCE_FILE,			EArgType::argSTRING),
		MAKE_ARGUMENT(EArguments::RESULT_FILE,			EArgType::argSTRING),
		MAKE_ARGUMENT(EArguments::MATERIALS_DATABASE,	EArgType::argSTRING),
		MAKE_ARGUMENT(EArguments::MODELS_PATH,			EArgType::argSTRINGS),
		MAKE_ARGUMENT(EArguments::SIMULATION_TIME,		EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::RELATIVE_TOLERANCE,	EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::ABSOLUTE_TOLERANCE,	EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::MINIMAL_FRACTION,		EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::INIT_TIME_WINDOW,		EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::MIN_TIME_WINDOW,		EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::MAX_TIME_WINDOW,		EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::MAX_ITERATIONS_NUM,	EArgType::argUNSIGNED),
		MAKE_ARGUMENT(EArguments::WINDOW_CHANGE_RATE,	EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::ITER_UPPER_LIMIT,		EArgType::argUNSIGNED),
		MAKE_ARGUMENT(EArguments::ITER_LOWER_LIMIT,		EArgType::argUNSIGNED),
		MAKE_ARGUMENT(EArguments::ITER_UPPER_LIMIT_1,	EArgType::argUNSIGNED),
		MAKE_ARGUMENT(EArguments::CONVERGENCE_METHOD,	EArgType::argUNSIGNED),
		MAKE_ARGUMENT(EArguments::ACCEL_PARAMETER,		EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::RELAX_PARAMETER,		EArgType::argDOUBLE),
		MAKE_ARGUMENT(EArguments::EXTRAPOL_METHOD,		EArgType::argUNSIGNED),
		MAKE_ARGUMENT(EArguments::DISTRIBUTION_GRID,	EArgType::argGRIDS),
		MAKE_ARGUMENT(EArguments::UNIT_PARAMETER,		EArgType::argUNITS),
		MAKE_ARGUMENT(EArguments::UNIT_HOLDUP_MTP,		EArgType::argHLDP_DISTRS),
		MAKE_ARGUMENT(EArguments::UNIT_HOLDUP_PHASES,	EArgType::argHLDP_DISTRS),
		MAKE_ARGUMENT(EArguments::UNIT_HOLDUP_COMP,		EArgType::argHLDP_COMPS),
		MAKE_ARGUMENT(EArguments::UNIT_HOLDUP_SOLID,	EArgType::argHLDP_SOLIDS)
	};
}

CConfigFileParser::~CConfigFileParser()
{
	ClearArguments();
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
		grid.vNumGrid = CDistributionsGrid::CalculateGrid(grid.gridFun, grid.nClasses, GetValueFromStream<double>(&_ss), GetValueFromStream<double>(&_ss));

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
	}
	_arg->value = nullptr;
}
