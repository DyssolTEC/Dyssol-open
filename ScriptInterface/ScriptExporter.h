/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include <filesystem>

class CModelsManager;
class CMaterialsDatabase;
class CFlowsheet;

class CScriptExporter
{
	const CFlowsheet& m_flowsheet;				// Reference to a flowsheet.
	const CModelsManager& m_modelsManager;		// Reference to a global models manager.
	const CMaterialsDatabase& m_materialsDB;	// Reference to a global database of materials.

public:
	CScriptExporter(const CFlowsheet& _flowsheet, const CModelsManager& _modelsManager, const CMaterialsDatabase& _materialsDB);

	// Exports script to the specified file. Returns true on success.
	bool Export(const std::filesystem::path& _scriptFile, const std::filesystem::path& _flowsheetFile);
};

