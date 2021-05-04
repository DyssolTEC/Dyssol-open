/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include "Flowsheet.h"
#include "MaterialsDatabase.h"
#include "ModelsManager.h"
#include "Simulator.h"

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
	// Sets flowsheet parameters from the _job. Returns success flag.
	bool SetupFlowsheetParameters(const CScriptJob& _job);
	// Sets unit parameters from the _job. Returns success flag.
	bool SetupUnits(const CScriptJob& _job);

	// Performs the simulation. Returns success flag.
	bool RunSimulation(const CScriptJob& _job);

	// Clears current state of the runner.
	void Clear();
};

