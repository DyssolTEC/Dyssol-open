/* Copyright (c) 2021, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include "DyssolFilesystem.h"

class CModelsManager;
class CMaterialsDatabase;
class CFlowsheet;

namespace ScriptInterface
{
	// Exports script to the specified file. Returns true on success.
	bool ExportScript(const std::filesystem::path& _scriptFile, const CFlowsheet& _flowsheet, const CModelsManager& _modelsManager, const CMaterialsDatabase& _materialsDB);
};

