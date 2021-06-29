#include "ScriptParser.h"
#include "ContainerFunctions.h"
#include "DyssolStringConstants.h"
#include "DyssolUtilities.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace StringFunctions;
using namespace ScriptInterface;

CScriptParser::CScriptParser(const std::filesystem::path& _script)
{
	Parse(_script);
}

size_t CScriptParser::JobsCount() const
{
	return m_jobs.size();
}

std::vector<const CScriptJob*> CScriptParser::Jobs() const
{
	std::vector<const CScriptJob*> res;
	for (const auto& j : m_jobs)
		res.push_back(j.get());
	return res;
}

void CScriptParser::Parse(const std::filesystem::path& _script)
{
	std::ifstream scriptFile{ _script };
	if (!scriptFile.good())
	{
		std::cout << StrConst::DyssolC_ErrorNoScript() << std::endl;
		return;
	}

	std::string line;
	while (std::getline(scriptFile, line))
	{
		TrimFromSymbols(line, StrConst::COMMENT_SYMBOL);	// remove comments
		ProcessLine(line);									// process
	}
	scriptFile.close();

	// Convert names to keys/indices
	NamesToKeys();
}

void CScriptParser::ProcessLine(const std::string& _line)
{
	if (_line.empty()) return;
	std::stringstream ss{ _line };

	// current script key
	const auto key = ToUpperCase(GetValueFromStream<std::string>(ss));

	// check that this key is allowed
	if (!VectorContains(m_allKeys, key))
	{
		std::cout << StrConst::DyssolC_WarningUnknown(key) << std::endl;
		return;
	}

	// special treatment for JOB key
	if (key == "JOB" || m_jobs.empty() && key != "JOB")
		m_jobs.emplace_back(std::make_unique<CScriptJob>());
	if (key == "JOB")
		return;

	// add new entry with the given key to the job with
	SScriptEntry* entry = m_jobs.back()->AddEntry(key);

	// parse the line and write the value to this entry
	switch (entry->type)
	{
	case EEntryType::EMPTY:																			break;
	case EEntryType::BOOL:				entry->value = GetValueFromStream<bool>(ss);				break;
	case EEntryType::INT:				entry->value = GetValueFromStream<int64_t>(ss);				break;
	case EEntryType::UINT:				entry->value = GetValueFromStream<uint64_t>(ss);			break;
	case EEntryType::DOUBLE:			entry->value = GetValueFromStream<double>(ss);				break;
	case EEntryType::STRING:			entry->value = GetRestOfLine(ss);							break;
	case EEntryType::PATH:				entry->value = std::filesystem::path{ GetRestOfLine(ss) };	break;
	case EEntryType::NAME_OR_KEY:		entry->value = GetValueFromStream<SNameOrKey>(ss);			break;
	case EEntryType::UNIT_PARAM:		entry->value = ReadUnitParameterFromStream(ss);				break;
	case EEntryType::HOLDUP_OVERALL:	entry->value = ReadHoldupOverallFromStream(ss);				break;
	}
}

void CScriptParser::NamesToKeys()
{
	// Convergence method
	const auto ConvergenceKey = [](const std::string& _name)
	{
		if (_name == "DIRECT_SUBSTITUTION")	return EConvergenceMethod::DIRECT_SUBSTITUTION;
		if (_name == "WEGSTEIN")			return EConvergenceMethod::WEGSTEIN;
		if (_name == "STEFFENSEN")			return EConvergenceMethod::STEFFENSEN;
		std::cout << StrConst::DyssolC_WarningUnknown(_name) << std::endl;
		return EConvergenceMethod::WEGSTEIN;
	};

	// Extrapolation method
	const auto ExtrapolationKey = [](const std::string& _name)
	{
		if (_name == "LINEAR")				return EExtrapolationMethod::LINEAR;
		if (_name == "CUBIC_SPLINE")		return EExtrapolationMethod::SPLINE;
		if (_name == "NEAREST_NEIGHBOR")	return EExtrapolationMethod::NEAREST;
		std::cout << StrConst::DyssolC_WarningUnknown(_name) << std::endl;
		return EExtrapolationMethod::LINEAR;
	};

	// Overall stream parameters
	const auto OverallKey = [](const std::string& _name)
	{
		if (_name == "MASS" || _name == "MASS_FLOW")	return EOverall::OVERALL_MASS;
		if (_name == "TEMPERATURE")						return EOverall::OVERALL_TEMPERATURE;
		if (_name == "PRESSURE")						return EOverall::OVERALL_PRESSURE;
		std::cout << StrConst::DyssolC_WarningUnknown(_name) << std::endl;
		return static_cast<EOverall>(-1);
	};

	for (auto& job : m_jobs)
	{
		for (auto& param : job->GetValuesPtr<SNameOrKey>(EScriptKeys::CONVERGENCE_METHOD))
			if (!param->HasKey())
				param->key = E2I(ConvergenceKey(ToUpperCase(param->name)));
		for (auto& param : job->GetValuesPtr<SNameOrKey>(EScriptKeys::EXTRAPOLATION_METHOD))
			if (!param->HasKey())
				param->key = E2I(ExtrapolationKey(ToUpperCase(param->name)));
		for (auto& entry : job->GetValuesPtr<SHoldupOverallSE>(EScriptKeys::HOLDUP_OVERALL))
			if (!entry->param.HasKey())
				entry->param.key = E2I(OverallKey(ToUpperCase(entry->param.name)));
	}
}

// TODO: move to corresponding friend stream functions
SUnitParameterSE CScriptParser::ReadUnitParameterFromStream(std::istream& _s) const
{
	SUnitParameterSE res;
	res.unit   = GetValueFromStream<SNameOrIndex>(_s);
	res.param  = GetValueFromStream<SNameOrIndex>(_s);
	res.values = GetRestOfLine(_s);	// format depends on parameter type, but type resolution is only possible when the flowsheet is loaded, so postpone final parsing
	return res;
}

SHoldupOverallSE CScriptParser::ReadHoldupOverallFromStream(std::istream& _s) const
{
	SHoldupOverallSE res;
	res.unit   = GetValueFromStream<SNameOrIndex>(_s);
	res.holdup = GetValueFromStream<SNameOrIndex>(_s);
	res.param  = GetValueFromStream<SNameOrKey>(_s);
	res.values = GetValueFromStream<CDependentValues>(_s);
	return res;
}
