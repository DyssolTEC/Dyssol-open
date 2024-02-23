/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ScriptTypes.h"
#include "StringFunctions.h"

using namespace StringFunctions;

namespace ScriptInterface
{
	std::istream& operator>>(std::istream& _s, SNameOrIndex& _obj)
	{
		const auto str = GetValueFromStream<std::string>(_s);
		_obj.name  = IsSimpleUInt(str) ? "" : str;
		_obj.index = IsSimpleUInt(str) ? static_cast<size_t>(std::stoull(str) - 1) : -1;
		return _s;
	}
	std::ostream& operator<<(std::ostream& _s, const SNameOrIndex& _obj)
	{
		_s << Quote(_obj.name);
		return _s;
	}

	std::istream& operator>>(std::istream& _s, SNamedEnum& _obj)
	{
		const auto str = GetValueFromStream<std::string>(_s);
		_obj.name = IsSimpleUInt(str) ? "" : str;
		_obj.key  = IsSimpleUInt(str) ? std::stoll(str) : -1;
		return _s;
	}
	std::ostream& operator<<(std::ostream& _s, const SNamedEnum& _obj)
	{
		_s << _obj.name;
		return _s;
	}

	std::istream& operator>>(std::istream& _s, SUnitParameterSE& _obj)
	{
		_obj.unit   = GetValueFromStream<SNameOrIndex>(_s);
		_obj.param  = GetValueFromStream<SNameOrIndex>(_s);
		_obj.values = GetRestOfLine(_s); // format depends on parameter type, so postpone final parsing until the flowsheet is loaded
		return _s;
	}
	std::ostream& operator<<(std::ostream& _s, const SUnitParameterSE& _obj)
	{
		_s << _obj.unit << " " << _obj.param << " " << _obj.values;
		return _s;
	}

	std::istream& operator>>(std::istream& _s, SHoldupDependentSE& _obj)
	{
		_obj.unit   = GetValueFromStream<SNameOrIndex>(_s);
		_obj.holdup = GetValueFromStream<SNameOrIndex>(_s);
		_obj.values = GetValueFromStream<std::vector<double>>(_s); // format depends on the flowsheet settings, so postpone final parsing until the flowsheet is loaded
		return _s;
	}
	std::ostream& operator<<(std::ostream& _s, const SHoldupDependentSE& _obj)
	{
		_s << _obj.unit << " " << _obj.holdup;
		for (const auto& v : _obj.values)
			_s << " " << v;
		return _s;
	}

	std::istream& operator>>(std::istream& _s, SHoldupCompoundsSE& _obj)
	{
		_obj.unit   = GetValueFromStream<SNameOrIndex>(_s);
		_obj.holdup = GetValueFromStream<SNameOrIndex>(_s);
		_obj.phase  = GetValueFromStream<SNamedEnum>(_s).FillAndWarn<EPhase>();
		_obj.values = GetValueFromStream<std::vector<double>>(_s); // format depends on the flowsheet settings, so postpone final parsing until the flowsheet is loaded
		return _s;
	}
	std::ostream& operator<<(std::ostream& _s, const SHoldupCompoundsSE& _obj)
	{
		_s << _obj.unit << " " << _obj.holdup << " " << _obj.phase;
		for (const auto& v : _obj.values)
			_s << " " << v;
		return _s;
	}

	std::istream& operator>>(std::istream& _s, SHoldupDistributionSE& _obj)
	{
		_obj.unit      = GetValueFromStream<SNameOrIndex>(_s);
		_obj.holdup    = GetValueFromStream<SNameOrIndex>(_s);
		_obj.distrType = GetValueFromStream<SNamedEnum>(_s).FillAndWarn<EDistrTypes>();
		_obj.compound  = GetValueFromStream<std::string>(_s);
		// special treatment for PSD
		if (_obj.distrType.key == DISTR_SIZE)
		{
			_obj.psdType  = GetValueFromStream<SNamedEnum>(_s).FillAndWarn<EPSDTypes>();
			_obj.psdMeans = GetValueFromStream<SNamedEnum>(_s).FillAndWarn<EPSDGridType>();
		}
		_obj.function = GetValueFromStream<SNamedEnum>(_s).FillAndWarn<EDistrFunction>();
		_obj.values   = GetValueFromStream<std::vector<double>>(_s);
		return _s;
	}
	std::ostream& operator<<(std::ostream& _s, const SHoldupDistributionSE& _obj)
	{
		_s << _obj.unit << " " << _obj.holdup << " " << _obj.distrType << " " << Quote(_obj.compound);
		if (_obj.distrType.key == DISTR_SIZE)
			_s << " " << _obj.psdType << " " << _obj.psdMeans;
		_s << " " << _obj.function;
		for (const auto& v : _obj.values)
			_s << " " << v;
		return _s;
	}

	std::istream& operator>>(std::istream& _s, SGridDimensionSE& _obj)
	{
		_obj.unit      = GetValueFromStream<SNameOrIndex>(_s);
		_obj.distrType = GetValueFromStream<SNamedEnum>(_s).FillAndWarn<EDistrTypes>();
		_obj.entryType = GetValueFromStream<SNamedEnum>(_s).FillAndWarn<EGridEntry>();
		if (static_cast<EGridEntry>(_obj.entryType.key) == EGridEntry::GRID_NUMERIC)
		{
			_obj.function = GetValueFromStream<SNamedEnum>(_s).FillAndWarn<EGridFunction>();
			if (static_cast<EDistrTypes>(_obj.distrType.key) == DISTR_SIZE)
				_obj.psdMeans = GetValueFromStream<SNamedEnum>(_s).FillAndWarn<EPSDGridType>();
		}
		_obj.classes = GetValueFromStream<size_t>(_s);
		if (static_cast<EGridEntry>(_obj.entryType.key) == EGridEntry::GRID_NUMERIC)
			_obj.valuesNum = GetValueFromStream<std::vector<double>>(_s);
		else if (static_cast<EGridEntry>(_obj.entryType.key) == EGridEntry::GRID_SYMBOLIC)
			_obj.valuesSym = GetValueFromStream<std::vector<std::string>>(_s);
		return _s;
	}
	std::ostream& operator<<(std::ostream& _s, const SGridDimensionSE& _obj)
	{
		_s << _obj.unit << " " << _obj.distrType << " " << _obj.entryType;
		if (static_cast<EGridEntry>(_obj.entryType.key) == EGridEntry::GRID_NUMERIC)
		{
			_s << " " << _obj.function;
			if (static_cast<EDistrTypes>(_obj.distrType.key) == DISTR_SIZE)
				_s << " " << _obj.psdMeans;
		}
		_s << " " << _obj.classes;
		if (static_cast<EGridEntry>(_obj.entryType.key) == EGridEntry::GRID_NUMERIC)
			for (const auto& v : _obj.valuesNum)
				_s << " " << v;
		else if (static_cast<EGridEntry>(_obj.entryType.key) == EGridEntry::GRID_SYMBOLIC)
			for (const auto& v : _obj.valuesSym)
				_s << " " << Quote(v);
		return _s;
	}

	std::istream& operator>>(std::istream& _s, SPhasesSE& _obj)
	{
		_obj.names.clear();
		_obj.types.clear();
		while (!_s.eof())
		{
			_obj.names.push_back(GetValueFromStream<std::string>(_s));
			_obj.types.push_back(GetValueFromStream<SNamedEnum>(_s).FillAndWarn<EPhase>());
		}
		return _s;
	}
	std::ostream& operator<<(std::ostream& _s, const SPhasesSE& _obj)
	{
		for (size_t i = 0; i < _obj.names.size() && i < _obj.types.size(); ++i)
			_s << Quote(_obj.names[i]) << " " << _obj.types[i] << " ";
		return _s;
	}

	std::istream& operator>>(std::istream& _s, SStreamSE& _obj)
	{
		_obj.name  = GetValueFromStream<std::string>(_s);
		_obj.unitO = GetValueFromStream<SNameOrIndex>(_s);
		_obj.portO = GetValueFromStream<SNameOrIndex>(_s);
		_obj.unitI = GetValueFromStream<SNameOrIndex>(_s);
		_obj.portI = GetValueFromStream<SNameOrIndex>(_s);
		return _s;
	}
	std::ostream& operator<<(std::ostream& _s, const SStreamSE& _obj)
	{
		_s << Quote(_obj.name) << " " << _obj.unitO << " " << _obj.portO << " " << _obj.unitI << " " << _obj.portI;
		return _s;
	}

	std::istream& operator>>(std::istream& _s, SExportStreamSE& _obj)
	{
		_obj.stream = GetValueFromStream<SNameOrIndex>(_s);
		_obj.times  = GetValueFromStream<std::vector<double>>(_s);
		return _s;
	}
	std::ostream& operator<<(std::ostream& _s, const SExportStreamSE& _obj)
	{
		_s << _obj.stream;
		for (const auto& v : _obj.times)
			_s << " " << v;
		return _s;
	}

	std::istream& operator>>(std::istream& _s, SExportHoldupSE& _obj)
	{
		_obj.unit   = GetValueFromStream<SNameOrIndex>(_s);
		_obj.holdup = GetValueFromStream<SNameOrIndex>(_s);
		_obj.times  = GetValueFromStream<std::vector<double>>(_s);
		return _s;
	}
	std::ostream& operator<<(std::ostream& _s, const SExportHoldupSE& _obj)
	{
		_s << _obj.unit << " " << _obj.holdup;
		for (const auto& v : _obj.times)
			_s << " " << v;
		return _s;
	}

	std::istream& operator>>(std::istream& _s, SExportStateVarSE& _obj)
	{
		_obj.unit     = GetValueFromStream<SNameOrIndex>(_s);
		_obj.variable = GetValueFromStream<SNameOrIndex>(_s);
		_obj.times    = GetValueFromStream<std::vector<double>>(_s);
		return _s;
	}
	std::ostream& operator<<(std::ostream& _s, const SExportStateVarSE& _obj)
	{
		_s << _obj.unit << " " << _obj.variable;
		for (const auto& v : _obj.times)
			_s << " " << v;
		return _s;
	}

	std::istream& operator>>(std::istream& _s, SExportPlotSE& _obj)
	{
		_obj.unit  = GetValueFromStream<SNameOrIndex>(_s);
		_obj.plot  = GetValueFromStream<SNameOrIndex>(_s);
		_obj.curve = GetValueFromStream<SNameOrIndex>(_s);
		return _s;
	}
	std::ostream& operator<<(std::ostream& _s, const SExportPlotSE& _obj)
	{
		_s << _obj.unit << " " << _obj.plot << " " << _obj.curve;
		return _s;
	}
}