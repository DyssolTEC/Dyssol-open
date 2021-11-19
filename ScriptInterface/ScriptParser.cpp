/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ScriptParser.h"
#include "DyssolStringConstants.h"
#include <fstream>
#include <sstream>

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
	if (!scriptFile)
	{
		std::cout << StrConst::DyssolC_ErrorNoScript() << std::endl;
		return;
	}

	std::string line;
	while (std::getline(scriptFile, line))
	{
		StringFunctions::TrimFromSymbols(line, StrConst::COMMENT_SYMBOL);	// remove comments
		ProcessLine(line);													// process
	}
	scriptFile.close();

	// convert names to keys
	NamesToKeys();
}

void CScriptParser::ProcessLine(const std::string& _line)
{
	if (_line.empty()) return;
	std::stringstream ss{ _line };

	// current script key
	const auto key = StringFunctions::ToUpperCase(StringFunctions::GetValueFromStream<std::string>(ss));

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
	ReadScriptEntry(*entry, ss);
}

void CScriptParser::NamesToKeys()
{
	for (const auto& job : m_jobs)
	{
		for (auto* param : job->GetValuesPtr<SNamedEnum>(EScriptKeys::CONVERGENCE_METHOD))
			param->FillAndCheck<EConvergenceMethod>();
		for (auto* param : job->GetValuesPtr<SNamedEnum>(EScriptKeys::EXTRAPOLATION_METHOD))
			param->FillAndCheck<EExtrapolationMethod>();
	}
}
