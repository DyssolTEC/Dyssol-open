/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ScriptTypes.h"
#include "StringFunctions.h"
#include "NameConverters.h"

using namespace StringFunctions;

std::istream& ScriptInterface::operator>>(std::istream& _s, ScriptInterface::SNameOrIndex& _obj) // Input stream operator.
{
	const auto str = GetValueFromStream<std::string>(_s);
	_obj.name  = IsSimpleUInt(str) ? "" : str;
	_obj.index = IsSimpleUInt(str) ? std::stoull(str) - 1 : -1;
	return _s;
}

std::istream& ScriptInterface::operator>>(std::istream& _s, SNameOrKey& _obj)
{
	const auto str = GetValueFromStream<std::string>(_s);
	_obj.name = IsSimpleUInt(str) ? "" : str;
	_obj.key  = IsSimpleUInt(str) ? std::stoull(str) : -1;
	return _s;
}

std::istream& ScriptInterface::operator>>(std::istream& _s, SUnitParameterSE& _obj)
{
	_obj.unit   = GetValueFromStream<SNameOrIndex>(_s);
	_obj.param  = GetValueFromStream<SNameOrIndex>(_s);
	_obj.values = GetRestOfLine(_s); // format depends on parameter type, so postpone final parsing until the flowsheet is loaded
	return _s;
}

std::istream& ScriptInterface::operator>>(std::istream& _s, SHoldupDependentSE& _obj)
{
	_obj.unit   = GetValueFromStream<SNameOrIndex>(_s);
	_obj.holdup = GetValueFromStream<SNameOrIndex>(_s);
	_obj.values = GetValueFromStream<std::vector<double>>(_s); // format depends on the flowsheet settings, so postpone final parsing until the flowsheet is loaded
	return _s;
}

std::istream& ScriptInterface::operator>>(std::istream& _s, SHoldupCompoundsSE& _obj)
{
	_obj.unit   = GetValueFromStream<SNameOrIndex>(_s);
	_obj.holdup = GetValueFromStream<SNameOrIndex>(_s);
	_obj.phase  = Convert<EPhase>(GetValueFromStream<SNameOrKey>(_s), &PhaseName2Enum);
	_obj.values = GetValueFromStream<std::vector<double>>(_s); // format depends on the flowsheet settings, so postpone final parsing until the flowsheet is loaded
	return _s;
}

std::istream& ScriptInterface::operator>>(std::istream& _s, SHoldupDistributionSE& _obj)
{
	_obj.unit         = GetValueFromStream<SNameOrIndex>(_s);
	_obj.holdup       = GetValueFromStream<SNameOrIndex>(_s);
	_obj.distrType    = Convert<EDistrTypes>(GetValueFromStream<SNameOrKey>(_s), &DistributionName2Enum);
	_obj.compound     = GetValueFromStream<std::string>(_s);
	// special treatment for PSD
	if (_obj.distrType.key == DISTR_SIZE)
	{
		_obj.psdType  = Convert<EPSDTypes>(GetValueFromStream<SNameOrKey>(_s), &PSDTypeName2Enum);
		_obj.psdMeans = Convert<EPSDGridType>(GetValueFromStream<SNameOrKey>(_s), &PSDMeanName2Enum);
	}
	_obj.function     = Convert<EDistrFunction>(GetValueFromStream<SNameOrKey>(_s), &DistributionFunctionName2Enum);
	_obj.values       = GetValueFromStream<std::vector<double>>(_s);
	return _s;
}

std::istream& ScriptInterface::operator>>(std::istream& _s, SGridDimensionSE& _obj)
{
	_obj.unit             = GetValueFromStream<SNameOrIndex>(_s);
	_obj.distrType        = Convert<EDistrTypes>(GetValueFromStream<SNameOrKey>(_s), &DistributionName2Enum);
	_obj.entryType        = Convert<EGridEntry>(GetValueFromStream<SNameOrKey>(_s), &GridEntryName2Enum);
	if (static_cast<EGridEntry>(_obj.entryType.key) == EGridEntry::GRID_NUMERIC)
	{
		_obj.function     = Convert<EGridFunction>(GetValueFromStream<SNameOrKey>(_s), &GridFunctionName2Enum);
		if (static_cast<EDistrTypes>(_obj.distrType.key) == DISTR_SIZE)
			_obj.psdMeans = Convert<EPSDGridType>(GetValueFromStream<SNameOrKey>(_s), &PSDMeanName2Enum);
	}
	_obj.classes          = GetValueFromStream<size_t>(_s);
	if (static_cast<EGridEntry>(_obj.entryType.key) == EGridEntry::GRID_NUMERIC)
		_obj.valuesNum    = GetValueFromStream<std::vector<double>>(_s);
	else if (static_cast<EGridEntry>(_obj.entryType.key) == EGridEntry::GRID_SYMBOLIC)
		_obj.valuesSym    = GetValueFromStream<std::vector<std::string>>(_s);
	return _s;
}

std::istream& ScriptInterface::operator>>(std::istream& _s, SPhasesSE& _obj)
{
	_obj.names.clear();
	_obj.types.clear();
	while (!_s.eof())
	{
		_obj.names.push_back(GetValueFromStream<std::string>(_s));
		_obj.types.push_back(Convert<EPhase>(GetValueFromStream<SNameOrKey>(_s), &PhaseName2Enum));
	}
	return _s;
}

std::istream& ScriptInterface::operator>>(std::istream& _s, SStreamSE& _obj)
{
	_obj.name  = GetValueFromStream<std::string>(_s);
	_obj.unitO = GetValueFromStream<SNameOrIndex>(_s);
	_obj.portO = GetValueFromStream<SNameOrIndex>(_s);
	_obj.unitI = GetValueFromStream<SNameOrIndex>(_s);
	_obj.portI = GetValueFromStream<SNameOrIndex>(_s);
	return _s;
}