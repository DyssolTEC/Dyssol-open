/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ScriptTypes.h"
#include "StringFunctions.h"
#include "NameConverters.h"

using namespace StringFunctions;

std::istream& ScriptInterface::operator>>(std::istream& _s, ScriptInterface::SNameOrIndex& _obj) // Input stream operator.
{
	const auto str = StringFunctions::GetValueFromStream<std::string>(_s);
	_obj.name = StringFunctions::IsSimpleUInt(str) ? "" : str;
	_obj.index = StringFunctions::IsSimpleUInt(str) ? std::stoull(str) - 1 : -1;
	return _s;
}

std::istream& ScriptInterface::operator>>(std::istream& _s, SNameOrKey& _obj)
{
	const auto str = StringFunctions::GetValueFromStream<std::string>(_s);
	_obj.name = StringFunctions::IsSimpleUInt(str) ? "" : str;
	_obj.key = StringFunctions::IsSimpleUInt(str) ? std::stoull(str) : -1;
	return _s;
}

std::istream& ScriptInterface::operator>>(std::istream& _s, SUnitParameterSE& _obj)
{
	_obj.unit = StringFunctions::GetValueFromStream<SNameOrIndex>(_s);
	_obj.param = StringFunctions::GetValueFromStream<SNameOrIndex>(_s);
	// format depends on parameter type, but type resolution is only possible when the flowsheet is loaded, so postpone final parsing
	_obj.values = StringFunctions::GetRestOfLine(_s);
	return _s;
}

std::istream& ScriptInterface::operator>>(std::istream& _s, SHoldupDependentSE& _obj)
{
	_obj.unit = StringFunctions::GetValueFromStream<SNameOrIndex>(_s);
	_obj.holdup = StringFunctions::GetValueFromStream<SNameOrIndex>(_s);
	// format depends on the flowsheet settings, so postpone final parsing until the flowsheet is loaded
	_obj.values = StringFunctions::GetValueFromStream<std::vector<double>>(_s);
	return _s;
}

std::istream& ScriptInterface::operator>>(std::istream& _s, SHoldupCompoundsSE& _obj)
{
	_obj.unit = StringFunctions::GetValueFromStream<SNameOrIndex>(_s);
	_obj.holdup = StringFunctions::GetValueFromStream<SNameOrIndex>(_s);
	_obj.phase = Convert<EPhase>(StringFunctions::GetValueFromStream<SNameOrKey>(_s), &PhaseName2Enum);
	// format depends on the flowsheet settings, so postpone final parsing until the flowsheet is loaded
	_obj.values = StringFunctions::GetValueFromStream<std::vector<double>>(_s);
	return _s;
}

std::istream& ScriptInterface::operator>>(std::istream& _s, SHoldupDistributionSE& _obj)
{
	_obj.unit = StringFunctions::GetValueFromStream<SNameOrIndex>(_s);
	_obj.holdup = StringFunctions::GetValueFromStream<SNameOrIndex>(_s);
	_obj.distrType = Convert<EDistrTypes>(StringFunctions::GetValueFromStream<SNameOrKey>(_s), &DistributionName2Enum);
	_obj.compound = StringFunctions::GetValueFromStream<std::string>(_s);
	if (StringFunctions::ToUpperCase(_obj.compound) == "MIXTURE" || StringFunctions::ToUpperCase(_obj.compound) == "TOTAL_MIXTURE")
		_obj.compound.clear();
	// special treatment for PSD
	if (_obj.distrType.key == DISTR_SIZE)
	{
		_obj.psdType = Convert<EPSDTypes>(StringFunctions::GetValueFromStream<SNameOrKey>(_s), &PSDTypeName2Enum);
		_obj.psdMeans = Convert<EPSDGridType>(StringFunctions::GetValueFromStream<SNameOrKey>(_s), &PSDMeanName2Enum);
	}
	_obj.function = Convert<EDistrFunction>(StringFunctions::GetValueFromStream<SNameOrKey>(_s), &DistributionFunctionName2Enum);
	_obj.values = StringFunctions::GetValueFromStream<std::vector<double>>(_s);
	return _s;
}