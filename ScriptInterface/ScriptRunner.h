/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include "Flowsheet.h"
#include "MaterialsDatabase.h"
#include "ModelsManager.h"
#include "Simulator.h"

class CBaseStream;
class CBaseUnitParameter;
namespace ScriptInterface
{
	enum class EScriptKeys;
	struct SNameOrIndex;
	struct SNameOrKey;
}

class CScriptJob;

/* Executes a script job. */
class CScriptRunner
{
	CMaterialsDatabase m_materialsDatabase{};							// Database of materials.
	CModelsManager m_modelsManager{};									// Units and solvers manager.
	CFlowsheet m_flowsheet{ m_modelsManager, m_materialsDatabase };		// Flowsheet.
	CSimulator m_simulator{};											// Simulator.

public:
	// Executes the _job.
	void RunJob(const CScriptJob& _job);

private:
	// Reads the simulation settings from _job and creates a flowsheet. Returns success flag.
	bool ConfigureFlowsheet(const CScriptJob& _job);

	// Loads the flowsheet, materials database and models paths. Returns success flag.
	bool LoadFiles(const CScriptJob& _job);
	// Sets flowsheet main settings from the _job. Returns success flag.
	bool SetupFlowsheet(const CScriptJob& _job);
	// Sets flowsheet parameters from the _job. Returns success flag.
	bool SetupFlowsheetParameters(const CScriptJob& _job);
	// Sets units parameters from the _job. Returns success flag.
	bool SetupUnitParameters(const CScriptJob& _job);
	// Sets holdups parameters from the _job. Returns success flag.
	bool SetupHoldups(const CScriptJob& _job);

	// Performs the simulation. Returns success flag.
	bool RunSimulation(const CScriptJob& _job);
	// Exports results from file.
	bool ExportResults(const CScriptJob& _job);

	// Clears current state of the runner.
	void Clear();

	// Tries to obtain a pointer to a model and unit. Return the pointers on success or an error message on fail.
	std::tuple<CBaseUnit*, CUnitContainer*, std::string> TryGetModelPtr(const ScriptInterface::SNameOrIndex& _unit, const ScriptInterface::EScriptKeys& _scriptKey);

	// Returns a pointer to a unit by its name or index. Returns nullptr if the search fails.
	CUnitContainer* GetUnitPtr(const ScriptInterface::SNameOrIndex& _nameOrIndex);
	// Returns a pointer to a stream by its name or index. Returns nullptr if the search fails.
	CStream* GetStreamPtr(const ScriptInterface::SNameOrIndex& _nameOrIndex);
	// Returns a pointer to a unit parameter by its name or index. Returns nullptr if the search fails.
	CBaseUnitParameter* GetUnitParamPtr(CBaseUnit& _model, const ScriptInterface::SNameOrIndex& _nameOrIndex);
	// Returns a pointer to a holdup by its name or index. Returns nullptr if the search fails.
	CBaseStream* GetHoldupPtr(CBaseUnit& _model, const ScriptInterface::SNameOrIndex& _nameOrIndex);
	// Returns a pointer to a compound by its name or key. Returns nullptr if the search fails.
	CCompound* GetCompoundPtr(const std::string& _nameOrKey);
	// Returns a pointer to a unit port by its name or index. Returns nullptr if the search fails.
	CUnitPort* GetPortPtr(CBaseUnit& _model, const ScriptInterface::SNameOrIndex& _nameOrIndex);
	// Returns a pointer to a state variable by its name or index. Returns nullptr if the search fails.
	CStateVariable* GetStateVarPtr(CBaseUnit& _model, const ScriptInterface::SNameOrIndex& _nameOrIndex);
	// Returns a pointer to a plot by its name or index. Returns nullptr if the search fails.
	const CPlot* GetPlotPtr(const CBaseUnit& _model, const ScriptInterface::SNameOrIndex& _nameOrIndex);
	// Returns a pointer to a curve by its name or index. Returns nullptr if the search fails.
	const CCurve* GetCurvePtr(const CPlot& _plot, const ScriptInterface::SNameOrIndex& _nameOrIndex);
	// Returns a unique key of a model, trying to find it by its ID, name and file path. Returns empty string if the search fails.
	std::string GetModelKey(const std::string& _value) const;
};
