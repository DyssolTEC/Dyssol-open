/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include "ScriptDefines.h"
#include <vector>
#include <memory>


// Description of each script job.
class CScriptJob
{
	using entry_t = std::unique_ptr<ScriptInterface::SScriptEntry>;

	std::vector<entry_t> m_entries; // List of all parsed script arguments.

public:
	// Adds new entry with the given key to the list and returns a pointer to it. If such key was not defined, does nothing and returns nullptr.
	ScriptInterface::SScriptEntry* AddEntry(const std::string& _key);

	// Determines if the argument with the given key exists in the job.
	[[nodiscard]] bool HasKey(ScriptInterface::EScriptKeys _key) const;

	// Returns value of the argument by its key. If there are several arguments with the same key, any of them may be returned.
	// Returns T{} if such key does not exist or a wrong type is requested for this key.
	template<typename T> [[nodiscard]] T GetValue(ScriptInterface::EScriptKeys _key) const
	{
		auto entry = std::find_if(m_entries.begin(), m_entries.end(), [&](const entry_t& _e) { return _e->key == _key; });
		if (entry == m_entries.end()) return {};
		if (!std::holds_alternative<T>(entry)) return {};
		return std::get<T>(entry->value);
	}

	// Returns values of all the defined arguments with the given key.
	// Returns an empty vector if such key does not exist or a wrong type is requested for this key.
	template<typename T> [[nodiscard]] std::vector<T> GetValues(ScriptInterface::EScriptKeys _key) const
	{
		std::vector<T> res;
		for (const auto& entry : m_entries)
			if (entry->key == _key && std::holds_alternative<T>(entry))
				res.push_back(std::get<T>(entry->value));
		return res;
	}
};
