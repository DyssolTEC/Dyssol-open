#include "ScriptParser.h"
#include "ContainerFunctions.h"
#include "DyssolStringConstants.h"
#include "DyssolUtilities.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <functional>

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
	case EEntryType::EMPTY:																					break;
	case EEntryType::BOOL:					entry->value = GetValueFromStream<bool>(ss);					break;
	case EEntryType::INT:					entry->value = GetValueFromStream<int64_t>(ss);					break;
	case EEntryType::UINT:					entry->value = GetValueFromStream<uint64_t>(ss);				break;
	case EEntryType::DOUBLE:				entry->value = GetValueFromStream<double>(ss);					break;
	case EEntryType::STRING:				entry->value = GetRestOfLine(ss);								break;
	case EEntryType::PATH:					entry->value = std::filesystem::path{ GetRestOfLine(ss) };		break;
	case EEntryType::NAME_OR_KEY:			entry->value = GetValueFromStream<SNameOrKey>(ss);				break;
	case EEntryType::UNIT_PARAMETER:		entry->value = GetValueFromStream<SUnitParameterSE>(ss);		break;
	case EEntryType::HOLDUP_DEPENDENT:		entry->value = GetValueFromStream<SHoldupDependentSE>(ss);		break;
	case EEntryType::HOLDUP_COMPOUNDS:		entry->value = GetValueFromStream<SHoldupCompoundsSE>(ss);		break;
	case EEntryType::HOLDUP_DISTRIBUTION:	entry->value = GetValueFromStream<SHoldupDistributionSE>(ss);	break;
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

	// Phases
	const auto PhaseKey = [](const std::string& _name)
	{
		if (_name == "SOLID")					return EPhase::SOLID;
		if (_name == "LIQUID")					return EPhase::LIQUID;
		if (_name == "GAS" || _name == "VAPOR")	return EPhase::VAPOR;
		std::cout << StrConst::DyssolC_WarningUnknown(_name) << std::endl;
		return EPhase::UNDEFINED;
	};

	// Distribution types
	const auto DistributionType = [](const std::string& _name)
	{
		if (_name == "COMPOUNDS")			return EDistrTypes::DISTR_COMPOUNDS;
		if (_name == "SIZE")				return EDistrTypes::DISTR_SIZE;
		if (_name == "PARTICLE_POROSITY")	return EDistrTypes::DISTR_PART_POROSITY;
		if (_name == "FORM_FACTOR")			return EDistrTypes::DISTR_FORM_FACTOR;
		if (_name == "COLOR")				return EDistrTypes::DISTR_COLOR;
		if (_name == "USER_DEFINED_01")		return EDistrTypes::DISTR_USER_DEFINED_01;
		if (_name == "USER_DEFINED_02")		return EDistrTypes::DISTR_USER_DEFINED_02;
		if (_name == "USER_DEFINED_03")		return EDistrTypes::DISTR_USER_DEFINED_03;
		if (_name == "USER_DEFINED_04")		return EDistrTypes::DISTR_USER_DEFINED_04;
		if (_name == "USER_DEFINED_05")		return EDistrTypes::DISTR_USER_DEFINED_05;
		if (_name == "USER_DEFINED_06")		return EDistrTypes::DISTR_USER_DEFINED_06;
		if (_name == "USER_DEFINED_07")		return EDistrTypes::DISTR_USER_DEFINED_07;
		if (_name == "USER_DEFINED_08")		return EDistrTypes::DISTR_USER_DEFINED_08;
		if (_name == "USER_DEFINED_09")		return EDistrTypes::DISTR_USER_DEFINED_09;
		if (_name == "USER_DEFINED_10")		return EDistrTypes::DISTR_USER_DEFINED_10;
		std::cout << StrConst::DyssolC_WarningUnknown(_name) << std::endl;
		return EDistrTypes::DISTR_UNDEFINED;
	};

	// PSD types
	const auto PSDType = [](const std::string& _name)
	{
		if (_name == "MASS_FRACTION")	return EPSDTypes::PSD_MassFrac;
		if (_name == "NUMBER")			return EPSDTypes::PSD_Number;
		if (_name == "Q0_DENSITY")		return EPSDTypes::PSD_q0;
		if (_name == "Q0_CUMULATIVE")	return EPSDTypes::PSD_Q0;
		if (_name == "Q2_DENSITY")		return EPSDTypes::PSD_q2;
		if (_name == "Q2_CUMULATIVE")	return EPSDTypes::PSD_Q2;
		if (_name == "Q3_DENSITY")		return EPSDTypes::PSD_q3;
		if (_name == "Q3_CUMULATIVE")	return EPSDTypes::PSD_Q3;
		std::cout << StrConst::DyssolC_WarningUnknown(_name) << std::endl;
		return EPSDTypes::PSD_MassFrac;
	};

	// Types of PSD mean values
	const auto PSDMean = [](const std::string& _name)
	{
		if (_name == "DIAMETER")	return EPSDGridType::DIAMETER;
		if (_name == "VOLUME")		return EPSDGridType::VOLUME;
		std::cout << StrConst::DyssolC_WarningUnknown(_name) << std::endl;
		return EPSDGridType::DIAMETER;
	};

	// Types of PSD mean values
	const auto DistributionFunction = [](const std::string& _name)
	{
		if (_name == "MANUAL")		return EDistrFunction::Manual;
		if (_name == "NORMAL")		return EDistrFunction::Normal;
		if (_name == "LOG_NORMAL")	return EDistrFunction::LogNormal;
		if (_name == "RRSB")		return EDistrFunction::RRSB;
		if (_name == "GGS")			return EDistrFunction::GGS;
		std::cout << StrConst::DyssolC_WarningUnknown(_name) << std::endl;
		return EDistrFunction::Manual;
	};

	// Applies one of the above functions to convert a name to a key
	const auto& Convert = [](SNameOrKey& _entry, auto func)
	{
		if (!_entry.HasKey())
			_entry.key = E2I(func(ToUpperCase(_entry.name)));
	};

	for (auto& job : m_jobs)
	{
		for (auto& param : job->GetValuesPtr<SNameOrKey>(EScriptKeys::CONVERGENCE_METHOD))
			Convert(*param, ConvergenceKey);
		for (auto& param : job->GetValuesPtr<SNameOrKey>(EScriptKeys::EXTRAPOLATION_METHOD))
			Convert(*param, ExtrapolationKey);
		for (auto& entry : job->GetValuesPtr<SHoldupCompoundsSE>(EScriptKeys::HOLDUP_COMPOUNDS))
			Convert(entry->phase, PhaseKey);
		for (auto& entry : job->GetValuesPtr<SHoldupDistributionSE>(EScriptKeys::HOLDUP_DISTRIBUTION))
		{
			Convert(entry->distrType, DistributionType);
			Convert(entry->function , DistributionFunction);
			if (static_cast<EDistrTypes>(entry->distrType.key) == EDistrTypes::DISTR_SIZE)
			{
				Convert(entry->psdType , PSDType);
				Convert(entry->psdMeans, PSDMean);
			}
			if (ToUpperCase(entry->compound) == "MIXTURE" || ToUpperCase(entry->compound) == "TOTAL_MIXTURE")
				entry->compound.clear();
		}
	}
}
