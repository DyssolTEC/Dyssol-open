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

	// Process a single line of a script file.
	void ProcessLine(const std::string& _line);

	// Returns the number of parsed jobs.
	size_t JobsCount() const;
	// Returns all parsed jobs.
	std::vector<const CScriptJob*> Jobs() const;

};

