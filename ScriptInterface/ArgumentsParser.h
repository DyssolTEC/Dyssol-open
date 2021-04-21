/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include <string>
#include <map>
#include <vector>

/* Parses input arguments in format -key=value / -key="value" / --key=value / --key="value" and stores them in m_tokens.
 * If m_allowedKeys is set, parses only theses keys, discarding all others.
 * m_allowedKeys also defines aliases for keys. */
class CArgumentsParser
{
	const char m_separator = '=';
	const std::string m_keySign = "-";

	std::vector<std::vector<std::string>> m_allowedKeys;	// List of allowed keys with aliases.
	std::multimap<std::string, std::string> m_tokens;		// Parsed <key,value> tokens.

public:
	// Parses all the given arguments.
	CArgumentsParser(int _argc, const char** _argv);
	// Parses the arguments from the given list.
	CArgumentsParser(int _argc, const char** _argv, std::vector<std::vector<std::string>> _allowedKeys);

	// Returns total number of parsed arguments.
	[[nodiscard]] size_t TokensNumber() const;
	// Determines if the argument with the given key or its alias exists.
	[[nodiscard]] bool HasKey(const std::string& _key) const;
	// Returns value of the argument by its key. If there are several arguments with the same key, any of them may be returned. Returns an empty string if such key does not exist.
	[[nodiscard]] std::string GetValue(const std::string& _key) const;
	// Returns values of all the defined arguments with the given key. Returns an empty vector if such key does not exist.
	[[nodiscard]] std::vector<std::string> GetValues(const std::string& _key) const;

private:
	// Parses argument returning a key and a value.
	[[nodiscard]] std::pair<std::string, std::string> ParseArgument(const std::string& _argument) const;
	// Removes leading m_keySign.
	[[nodiscard]] std::string RemoveSign(const std::string& _str) const;
	// Returns all aliases for the given key.
	[[nodiscard]] std::vector<std::string> KeyAliases(const std::string& _key) const;
};
