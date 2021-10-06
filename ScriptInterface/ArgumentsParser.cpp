/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ArgumentsParser.h"
#include "ContainerFunctions.h"
#include "StringFunctions.h"

namespace SF = StringFunctions;

CArgumentsParser::CArgumentsParser(int _argc, const char** _argv)
	: CArgumentsParser{ _argc, _argv, {} }
{
}

CArgumentsParser::CArgumentsParser(int _argc, const char** _argv, std::vector<SKey> _allowedKeys)
	: m_allowedKeys{ std::move(_allowedKeys) }
{
	// parse all keys
	for (int i = 1; i < _argc; ++i)
		m_tokens.push_back(ParseArgument(std::string{ _argv[i] }));

	// handle allowed keys
	if (!m_allowedKeys.empty())
	{
		// remove possible signs in allowed keys
		for (auto& allowed : m_allowedKeys)
		{
			for (auto& k : allowed.keysL) k = RemoveSignL(k);
			for (auto& k : allowed.keysS) k = RemoveSignS(k);
		}
		// remove not allowed tokens
		FilterTokens();
	}
}

std::vector<CArgumentsParser::SKey> CArgumentsParser::AllAllowedKeys() const
{
	return m_allowedKeys;
}

size_t CArgumentsParser::TokensCount() const
{
	return m_tokens.size();
}

bool CArgumentsParser::HasKey(const std::string& _key) const
{
	const auto aliases = KeyAliases(_key);
	return std::any_of(aliases.begin(), aliases.end(), [&](const std::string& a)
		{ return VectorContains(m_tokens, [&](const SToken& t) { return RemoveSigns(t.key) == a; }); });
}

std::string CArgumentsParser::GetValue(const std::string& _key) const
{
	for (const auto& alias : KeyAliases(_key))
	{
		const size_t i = VectorFind(m_tokens, [&](const SToken& t) { return RemoveSigns(t.key) == alias; });
		if (i != static_cast<size_t>(-1))
			return m_tokens[i].value;
	}
	return {};
}

std::vector<std::string> CArgumentsParser::GetValues(const std::string& _key) const
{
	std::vector<std::string> res;
	for (const auto& alias : KeyAliases(_key))
		for (const auto& token : m_tokens)
			if (RemoveSigns(token.key) == alias)
				res.push_back(token.value);
	return res;
}

CArgumentsParser::SToken CArgumentsParser::ParseArgument(const std::string& _argument) const
{
	const auto parts = SF::SplitString(_argument, m_separator);
	if (parts.empty()) return {};
	// only a key or only a value
	if (parts.size() == 1)
	{
		// a key without value
		if (IsKey(parts[0]))
			return { parts[0], {} };
		// a value without key
		else
			return { {}, SF::RemoveQuotes(parts.front()) };
	}
	// key=value
	if (IsKey(parts[0]))
		return { parts[0], SF::RemoveQuotes(parts[1]) };
	// wrong format
	return {};
}

void CArgumentsParser::FilterTokens()
{
	// gather all allowed long keys
	std::vector<std::string> keysL;
	for (const auto& v : m_allowedKeys)
		keysL.insert(keysL.end(), v.keysL.begin(), v.keysL.end());
	// gather all allowed short keys
	std::vector<std::string> keysS;
	for (const auto& v : m_allowedKeys)
		keysS.insert(keysS.end(), v.keysS.begin(), v.keysS.end());

	// remove tokens with not allowed keys
	m_tokens.erase(std::remove_if(m_tokens.begin(), m_tokens.end(), [&](const SToken& t)
		{ return !VectorContains(keysL, RemoveSignL(t.key)) && !VectorContains(keysS, RemoveSignS(t.key)); }), m_tokens.end());
}

bool CArgumentsParser::IsKeyL(const std::string& _str) const
{
	return _str.rfind(m_keySignL, 0) == 0;
}

bool CArgumentsParser::IsKeyS(const std::string& _str) const
{
	return _str.rfind(m_keySignS, 0) == 0;
}

bool CArgumentsParser::IsKey(const std::string& _str) const
{
	return IsKeyL(_str) || IsKeyS(_str);
}

std::string CArgumentsParser::RemoveSignL(std::string _str) const
{
	if (IsKeyL(_str))
		_str.erase(0, m_keySignL.size());
	return _str;
}

std::string CArgumentsParser::RemoveSignS(std::string _str) const
{
	if (IsKeyS(_str))
		_str.erase(0, m_keySignS.size());
	return _str;
}

std::string CArgumentsParser::RemoveSigns(const std::string& _str) const
{
	return RemoveSignS(RemoveSignL(_str));
}

std::vector<std::string> CArgumentsParser::KeyAliases(const std::string& _key) const
{
	for (const auto& allowed : m_allowedKeys)
	{
		const auto cleanKey = RemoveSigns(_key);
		if (VectorContains(allowed.keysL, cleanKey) || VectorContains(allowed.keysS, cleanKey))
		{
			std::vector<std::string> res;
			res.insert(res.end(), allowed.keysL.begin(), allowed.keysL.end());
			res.insert(res.end(), allowed.keysS.begin(), allowed.keysS.end());
			return res;
		}
	}
	return{ _key }; // no aliases defined
}
