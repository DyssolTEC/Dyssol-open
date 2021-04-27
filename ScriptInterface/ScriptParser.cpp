#include "ScriptParser.h"
#include "ContainerFunctions.h"
#include "DyssolStringConstants.h"
#include <fstream>
#include <iostream>

using namespace StringFunctions;

CScriptParser::CScriptParser(const std::filesystem::path& _script)
{
	std::ifstream scriptFile{ _script };
	if (!scriptFile.good())
	{
		std::cout << "Error: Script file can not be opened" << std::endl;
		return;
	}

	std::string line;
	while (std::getline(scriptFile, line))
	{
		TrimFromSymbols(line, StrConst::COMMENT_SYMBOL);	// remove comments
		ProcessLine(line);									// process
	}
	scriptFile.close();
}

void CScriptParser::ProcessLine(const std::string& _line)
{
	if (_line.empty()) return;
	std::stringstream ss{ _line };

	// current script key
	const auto key = ToUpperCase(GetValueFromStream<std::string>(&ss));

	// check that this key is allowed
	if (!VectorContains(m_allKeys, key))
	{
		std::cout << "Warning: Unknown key found: " << key << std::endl;
		return;
	}

	// special treatment for JOB key
	if (key == "JOB" || m_jobs.empty() && key != "JOB")
		m_jobs.emplace_back(std::make_unique<CScriptJob>());
	if (key == "JOB")
		return;

	// add new entry with the given key to the job with
	ScriptInterface::SScriptEntry* entry = m_jobs.back()->AddEntry(key);

	// parse the line and write the value to this entry
	switch (entry->type)
	{
	case ScriptInterface::EEntryType::EMPTY:
		break;
	case ScriptInterface::EEntryType::BOOL:
		entry->value = GetValueFromStream<bool>(&ss);
		break;
	case ScriptInterface::EEntryType::INT:
		entry->value = GetValueFromStream<int64_t>(&ss);
		break;
	case ScriptInterface::EEntryType::UINT:
		entry->value = GetValueFromStream<uint64_t>(&ss);
		break;
	case ScriptInterface::EEntryType::DOUBLE:
		entry->value = GetValueFromStream<double>(&ss);
		break;
	case ScriptInterface::EEntryType::STRING:
		entry->value = GetRestOfLine(&ss);
		break;
	case ScriptInterface::EEntryType::PATH:
		entry->value = std::filesystem::path{ GetRestOfLine(&ss) };
		break;
	}
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
