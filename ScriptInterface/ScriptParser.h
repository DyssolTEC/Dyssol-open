/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include "ScriptJob.h"
#include <filesystem>
#include <string>
#include <vector>

/* Parses script files. */
class CScriptParser
{
	const std::vector<std::string> m_allKeys{ ScriptInterface::AllScriptKeys() };	// All possible script keys.

	std::vector<std::unique_ptr<CScriptJob>> m_jobs;	// All parsed jobs.

public:
	// Opens a _script file and parses it.
	CScriptParser(const std::filesystem::path& _script);

	// Returns the number of parsed jobs.
	[[nodiscard]] size_t JobsCount() const;
	// Returns all parsed jobs.
	[[nodiscard]] std::vector<const CScriptJob*> Jobs() const;

private:
	// Parses the script.
	void Parse(const std::filesystem::path& _script);

	// Process a single line of a script file, cleared from comments.
	void ProcessLine(const std::string& _line);

	// Extracts unit parameter script entry from the stream.
	ScriptInterface::SUnitParameterScriptEntry ReadUnitParameterFromStream(std::istream& _s) const;
};

