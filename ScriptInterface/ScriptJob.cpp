#include "ScriptJob.h"

using namespace ScriptInterface;

SScriptEntry* CScriptJob::AddEntry(const std::string& _key)
{
	const auto descriptor = Key2Descriptor(_key);
	if (descriptor.keyStr.empty()) return {};
	return m_entries.emplace_back(std::make_unique<SScriptEntry>(descriptor)).get();
}

bool CScriptJob::HasKey(EScriptKeys _key) const
{
	return std::any_of(m_entries.begin(), m_entries.end(), [&](const std::unique_ptr<SScriptEntry>& _e) { return _e->key == _key; });
}
