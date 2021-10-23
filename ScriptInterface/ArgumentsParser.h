/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include <string>
#include <vector>

/*
 * Parses input arguments in format --key=value / --key="value" / -k=value / -k="value" and stores them in m_tokens.
 * If m_allowedKeys is set, parses only theses keys, discarding all others.
 * m_allowedKeys may also define aliases for keys.
 */
class CArgumentsParser
{
public:
	/*
	 * Describes command line keys.
	 */
	struct SKey
	{
		std::vector<std::string> keysL; // Full keys, that can be given as --key.
		std::vector<std::string> keysS; // Short keys, that can be given -k.
		std::string description;        // User-friendly description of the key.
	};

private:
	/*
	 * Tokens parsed from command line arguments.
	 */
	struct SToken
	{
		std::string key;	// Parsed key as "--key" or "-k".
		std::string value;	// Parsed value without quotes.
	};

	const char m_separator = '=';        // Separator between key and value.
	const std::string m_keySignS = "-";	 // Sign of a short key.
	const std::string m_keySignL = "--"; // Sign of a long key.

	std::vector<SKey> m_allowedKeys;	// List of allowed keys with aliases. If empty, all keys are allowed.
	std::vector<SToken> m_tokens;		// Parsed <--key,value> and <-k,value> tokens.

public:
	// Parses all the given arguments.
	CArgumentsParser(int _argc, const char** _argv);
	// Parses the arguments from the given list.
	CArgumentsParser(int _argc, const char** _argv, std::vector<SKey> _allowedKeys);

	// Returns all allowed keys.
	std::vector<SKey> AllAllowedKeys() const;

	// Returns total number of parsed arguments.
	[[nodiscard]] size_t TokensCount() const;
	// Determines if the argument with the given key or its alias exists.
	[[nodiscard]] bool HasKey(const std::string& _key) const;
	// Returns value of the argument by its key. If there are several arguments with the same key, any of them may be returned. Returns an empty string if such key does not exist.
	[[nodiscard]] std::string GetValue(const std::string& _key) const;
	// Returns values of all the defined arguments with the given key. Returns an empty vector if such key does not exist.
	[[nodiscard]] std::vector<std::string> GetValues(const std::string& _key) const;

private:
	// Parses argument returning a key and a value.
	[[nodiscard]] SToken ParseArgument(const std::string& _argument) const;
	// Filters parsed tokens removing those with not allowed keys.
	void FilterTokens();

	// Checks if the given string represents a long key.
	[[nodiscard]] bool IsKeyL(const std::string& _str) const;
	// Checks if the given string represents a short key.
	[[nodiscard]] bool IsKeyS(const std::string& _str) const;
	// Checks if the given string represents a long or short key.
	[[nodiscard]] bool IsKey(const std::string& _str) const;

	// Removes leading m_keySignL.
	[[nodiscard]] std::string RemoveSignL(std::string _str) const;
	// Removes leading m_keySignS.
	[[nodiscard]] std::string RemoveSignS(std::string _str) const;
	// Removes leading m_keySignS and m_keySignL.
	[[nodiscard]] std::string RemoveSigns(const std::string& _str) const;

	// Returns all aliases for the given key.
	[[nodiscard]] std::vector<std::string> KeyAliases(const std::string& _key) const;
};
