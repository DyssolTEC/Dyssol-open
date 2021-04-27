/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ArgumentsParser.h"
#include "ContainerFunctions.h"
#include "StringFunctions.h"
#include <utility>

using namespace StringFunctions;

CArgumentsParser::CArgumentsParser(int _argc, const char** _argv)
	: CArgumentsParser{ _argc, _argv, {} }
{
}

CArgumentsParser::CArgumentsParser(int _argc, const char** _argv, std::vector<std::vector<std::string>> _allowedKeys)
	: m_allowedKeys{ std::move(_allowedKeys) }
{
	// parse all keys
	for (int i = 1; i < _argc; ++i)
		m_tokens.insert(ParseArgument(std::string{ _argv[i] }));
	// filter parsed keys
	if (!m_allowedKeys.empty())
	{
		// gather all allowed keys
		std::vector<std::string> allAllowedKeys;
		for (const auto& v : m_allowedKeys)
			allAllowedKeys.insert(allAllowedKeys.end(), v.begin(), v.end());
		// remove tokens with not allowed keys
		for (auto i = m_tokens.begin(), last = m_tokens.end(); i != last; )
			if (!VectorContains(allAllowedKeys, i->first))
				i = m_tokens.erase(i);
			else
				++i;
	}
}

size_t CArgumentsParser::TokensCount() const
{
	return m_tokens.size();
}

bool CArgumentsParser::HasKey(const std::string& _key) const
{
	for (const auto& alias : KeyAliases(_key))
		if (MapContainsKey(m_tokens, alias))
			return true;
	return false;
}

std::string CArgumentsParser::GetValue(const std::string& _key) const
{
	for (const auto& alias : KeyAliases(_key))
		if (MapContainsKey(m_tokens, alias))
			return m_tokens.find(alias)->second;
	return {};
}

std::vector<std::string> CArgumentsParser::GetValues(const std::string& _key) const
{
	std::vector<std::string> res;
	const auto [beg, end] = m_tokens.equal_range(_key);
	for (auto it = beg; it != end; ++it)
		res.push_back(it->second);
	return res;
}

std::pair<std::string, std::string> CArgumentsParser::ParseArgument(const std::string& _argument) const
{
	const auto parts = SplitString(_argument, m_separator);
	if (parts.empty())
		return {};
	if (parts.size() == 1)
	{
		if (Contains(m_keySign, parts[0].front()))		// a key without value
			return { ToLowerCase(RemoveSign(parts[0])), "" };
		else
			return { "", RemoveQuotes(parts.front()) }; // a value without key
	}
	return { ToLowerCase(RemoveSign(parts[0])), RemoveQuotes(parts[1]) };
}

std::string CArgumentsParser::RemoveSign(const std::string& _str) const
{
	std::string res{ _str };
	while (!res.empty() && Contains(m_keySign, res.front()))
		res.erase(0, 1);
	return res;
}

std::vector<std::string> CArgumentsParser::KeyAliases(const std::string& _key) const
{
	for (const auto& v : m_allowedKeys)
		if (VectorContains(v, _key))
			return v;
	return{ _key }; // no aliases defined
}
