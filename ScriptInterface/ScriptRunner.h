/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include "Flowsheet.h"
#include "MaterialsDatabase.h"
#include "ModelsManager.h"
#include "Simulator.h"

class CBaseStream;
class CBaseUnitParameter;
class CScriptJob;
namespace ScriptInterface
{
	enum class EScriptKeys;
	struct SNameOrIndex;
}

/* Executes a script job. */
class CScriptRunner
{
	CMaterialsDatabase m_materialsDatabase{};							// Database of materials.
	CModelsManager m_modelsManager{};									// Units and solvers manager.
	CFlowsheet m_flowsheet{ &m_modelsManager, &m_materialsDatabase };	// Flowsheet.
	CSimulator m_simulator{};											// Simulator.

public:
	// Executes the job. Returns success flag.
	bool RunJob(const CScriptJob& _job);

private:
	// Reads the simulation settings from the job and creates a flowsheet. Returns success flag.
	bool CreateFlowsheet(const CScriptJob& _job);

	// Loads the flowsheet, materials database and models. Returns success flag.
	bool LoadFiles(const CScriptJob& _job);
	// Sets all flowsheet settings. Returns success flag.
	bool SetupFlowsheet(const CScriptJob& _job);
	// Sets flowsheet units. Returns success flag.
	bool SetupUnits(const CScriptJob& _job);
	// Sets flowsheet streams. Returns success flag.
	bool SetupStreams(const CScriptJob& _job);
	// Sets flowsheet compounds. Returns success flag.
	bool SetupCompounds(const CScriptJob& _job);
	// Sets flowsheet phases. Returns success flag.
	bool SetupPhases(const CScriptJob& _job);
	// Sets flowsheet and units' distribution grids. Returns success flag.
	bool SetupGrids(const CScriptJob& _job);
	// Sets flowsheet parameters. Returns success flag.
	bool SetupFlowsheetParameters(const CScriptJob& _job);
	// Sets units parameters. Returns success flag.
	bool SetupUnitParameters(const CScriptJob& _job);
	// Sets holdups parameters. Returns success flag.
	bool SetupHoldups(const CScriptJob& _job);
	// Sets holdups overall parameters. Returns success flag.
	bool SetupHoldupsOverall(const CScriptJob& _job);
	// Sets holdups phases fractions. Returns success flag.
	bool SetupHoldupsPhases(const CScriptJob& _job);
	// Sets holdups compounds fractions. Returns success flag.
	bool SetupHoldupsCompounds(const CScriptJob& _job);
	// Sets holdups distributed parameters. Returns success flag.
	bool SetupHoldupsDistributions(const CScriptJob& _job);

	// Performs the simulation. Returns success flag.
	bool RunSimulation(const CScriptJob& _job);
	// Exports results from file.
	bool ExportResults(const CScriptJob& _job);

	// Clears current state of the runner.
	void Clear();

	// Tries to obtain a pointer to a required unit. Prints error message and returns nullptr if the search fails.
	CUnitContainer* TryGetUnitPtr(ScriptInterface::EScriptKeys _sk, const ScriptInterface::SNameOrIndex& _unit);
	// Tries to obtain a pointer to a required model. Prints error message and returns nullptr if the search fails.
	static CBaseUnit* TryGetModelPtr(ScriptInterface::EScriptKeys _sk, CUnitContainer* _unit);
	// Tries to obtain a pointer to a required model and unit. Prints error message and returns nullptr if the search fails.
	std::tuple<CBaseUnit*, CUnitContainer*> TryGetUnitAndModelPtr(ScriptInterface::EScriptKeys _sk, const ScriptInterface::SNameOrIndex& _unit);
	// Tries to obtain a pointer to a required stream. Prints error message and returns nullptr if the search fails.
	CBaseStream* TryGetStreamPtr(ScriptInterface::EScriptKeys _sk, const ScriptInterface::SNameOrIndex& _stream);
	// Tries to obtain a pointer to a required unit parameter. Prints error message and returns nullptr if the search fails.
	CBaseUnitParameter* TryGetUnitParamPtr(ScriptInterface::EScriptKeys _sk, const ScriptInterface::SNameOrIndex& _unit, const ScriptInterface::SNameOrIndex& _param);
	// Tries to obtain a pointer to a required holdup with initial values. Prints error message and returns nullptr if the search fails.
	std::tuple<CBaseStream*, CUnitContainer*> TryGetHoldupInitPtr(ScriptInterface::EScriptKeys _sk, const ScriptInterface::SNameOrIndex& _unit, const ScriptInterface::SNameOrIndex& _holdup);
	// Tries to obtain a pointer to a required holdup with final values. Prints error message and returns nullptr if the search fails.
	std::tuple<CBaseStream*, CUnitContainer*> TryGetHoldupWorkPtr(ScriptInterface::EScriptKeys _sk, const ScriptInterface::SNameOrIndex& _unit, const ScriptInterface::SNameOrIndex& _holdup);
	// Tries to obtain a pointer to required compound. Prints error message and returns nullptr if the search fails.
	CCompound* TryGetCompoundPtr(ScriptInterface::EScriptKeys _sk, const std::string& _compound);
	// Tries to obtain a pointer to required port. Prints error message and returns nullptr if the search fails.
	CUnitPort* TryGetPortPtr(ScriptInterface::EScriptKeys _sk, const ScriptInterface::SNameOrIndex& _unit, const ScriptInterface::SNameOrIndex& _port);
	// Tries to obtain a pointer to required state variable. Prints error message and returns nullptr if the search fails.
	CStateVariable* TryGetStateVarPtr(ScriptInterface::EScriptKeys _sk, const ScriptInterface::SNameOrIndex& _unit, const ScriptInterface::SNameOrIndex& _var);
	// Tries to obtain a pointer to required curve. Prints error message and returns nullptr if the search fails.
	std::tuple<const CPlot*, const CCurve*> TryGetCurvePtr(ScriptInterface::EScriptKeys _sk, const ScriptInterface::SNameOrIndex& _unit, const ScriptInterface::SNameOrIndex& _plot, const ScriptInterface::SNameOrIndex& _curve);
	// Tries to obtain a unique key of a required model, trying to find it by its ID, name and file path. Prints error message and returns empty string if the search fails.
	std::string TryGetModelKey(ScriptInterface::EScriptKeys _sk, const std::string& _value) const;

	// Returns a pointer to a unit by its name or index. Returns nullptr if the search fails.
	CUnitContainer* GetUnitPtr(const ScriptInterface::SNameOrIndex& _nameOrIndex);
	// Returns a pointer to a model set in the unit. Returns nullptr if the search fails.
	static CBaseUnit* GetModelPtr(CUnitContainer* _unit);
	// Returns a pointer to a stream by its name or index. Returns nullptr if the search fails.
	CBaseStream* GetStreamPtr(const ScriptInterface::SNameOrIndex& _nameOrIndex);
	// Returns a pointer to a unit parameter by its name or index. Returns nullptr if the search fails.
	static CBaseUnitParameter* GetUnitParamPtr(CBaseUnit* _model, const ScriptInterface::SNameOrIndex& _nameOrIndex);
	// Returns a pointer to a holdup with initial values by its name or index. Returns nullptr if the search fails.
	static CBaseStream* GetHoldupInitPtr(CBaseUnit* _model, const ScriptInterface::SNameOrIndex& _nameOrIndex);
	// Returns a pointer to a holdup with final values by its name or index. Returns nullptr if the search fails.
	static CBaseStream* GetHoldupWorkPtr(CBaseUnit* _model, const ScriptInterface::SNameOrIndex& _nameOrIndex);
	// Returns a pointer to a compound by its name or key. Returns nullptr if the search fails.
	CCompound* GetCompoundPtr(const std::string& _nameOrKey);
	// Returns a pointer to a unit port by its name or index. Returns nullptr if the search fails.
	static CUnitPort* GetPortPtr(CBaseUnit* _model, const ScriptInterface::SNameOrIndex& _nameOrIndex);
	// Returns a pointer to a state variable by its name or index. Returns nullptr if the search fails.
	static CStateVariable* GetStateVarPtr(CBaseUnit* _model, const ScriptInterface::SNameOrIndex& _nameOrIndex);
	// Returns a pointer to a plot by its name or index. Returns nullptr if the search fails.
	static const CPlot* GetPlotPtr(const CBaseUnit* _model, const ScriptInterface::SNameOrIndex& _nameOrIndex);
	// Returns a pointer to a curve by its name or index. Returns nullptr if the search fails.
	static const CCurve* GetCurvePtr(const CPlot* _plot, const ScriptInterface::SNameOrIndex& _nameOrIndex);
	// Returns a unique key of a model, trying to find it by its ID, name and file path. Returns empty string if the search fails.
	std::string GetModelKey(const std::string& _value) const;

	// Prints the message to the console and returns false.
	static bool PrintMessage(const std::string& _message);
};
