#include "ScriptParser.h"
#include "ContainerFunctions.h"
#include "DyssolStringConstants.h"
#include <fstream>
#include <iostream>

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
	const auto key = ToUpperCase(GetValueFromStream<std::string>(ss));

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
	SScriptEntry* entry = m_jobs.back()->AddEntry(key);

	// parse the line and write the value to this entry
	switch (entry->type)
	{
	case EEntryType::EMPTY:																		break;
	case EEntryType::BOOL:			entry->value = GetValueFromStream<bool>(ss);				break;
	case EEntryType::INT:			entry->value = GetValueFromStream<int64_t>(ss);				break;
	case EEntryType::UINT:			entry->value = GetValueFromStream<uint64_t>(ss);			break;
	case EEntryType::DOUBLE:		entry->value = GetValueFromStream<double>(ss);				break;
	case EEntryType::STRING:		entry->value = GetRestOfLine(ss);							break;
	case EEntryType::PATH:			entry->value = std::filesystem::path{ GetRestOfLine(ss) };	break;
	case EEntryType::UNIT_PARAM:	entry->value = ReadUnitParameterFromStream(ss);				break;
	}
}

// Extracts name and index from the string.
std::pair<std::string, size_t> ParseNameAndIndex(const std::string& _str)
{
	const std::string name = IsSimpleUInt(_str) ? "" : _str;
	const size_t index     = IsSimpleUInt(_str) ? std::stoull(_str) - 1 : -1;
	return { name, index };
}

SUnitParameterScriptEntry CScriptParser::ReadUnitParameterFromStream(std::istream& _s) const
{
	SUnitParameterScriptEntry res;
	std::tie(res.unitName , res.unitIndex ) = ParseNameAndIndex(GetValueFromStream<std::string>(_s));
	std::tie(res.paramName, res.paramIndex) = ParseNameAndIndex(GetValueFromStream<std::string>(_s));
	res.values = GetRestOfLine(_s);
	return res;
}
