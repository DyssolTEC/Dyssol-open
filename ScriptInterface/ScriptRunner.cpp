/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ScriptRunner.h"
#include "BaseUnit.h"
#include "ScriptJob.h"
#include "StringFunctions.h"
#include <functional>

using namespace ScriptInterface;
namespace fs = std::filesystem;

// TODO: move text to DyssolStringConstants.h

void CScriptRunner::RunJob(const CScriptJob& _job)
{
	const auto tStart = std::chrono::steady_clock::now();

	Clear();
	bool success = true;
	if (success) success &= ConfigureFlowsheet(_job);
	if (success) success &= RunSimulation(_job);

	const auto tEnd = std::chrono::steady_clock::now();
	std::cout << "Script job finished in " << std::chrono::duration_cast<std::chrono::seconds>(tEnd - tStart).count() << " [s]" << std::endl;
	if (!success)
		std::cout << "Finished with errors!" << std::endl;
}

bool CScriptRunner::ConfigureFlowsheet(const CScriptJob& _job)
{
	bool success = true;
	if (success) success &= LoadFiles(_job);
	if (success) success &= SetupFlowsheetParameters(_job);
	if (success) success &= SetupUnits(_job);
	return success;
}

bool CScriptRunner::LoadFiles(const CScriptJob& _job)
{
	const bool hasSrc = _job.HasKey(EScriptKeys::SOURCE_FILE);
	const bool hasDst = _job.HasKey(EScriptKeys::RESULT_FILE);
	if (!hasSrc && !hasDst)
	{
		std::cout << "Error: Neither " << ARG2STR(EScriptKeys::SOURCE_FILE) << " nor " << ARG2STR(EScriptKeys::RESULT_FILE) << " is specified. Aborting" << std::endl;
		return false;
	}
	if (hasSrc && !hasDst)
		std::cout << ARG2STR(EScriptKeys::RESULT_FILE) << " is not specified. " << ARG2STR(EScriptKeys::SOURCE_FILE) << " will be used to store simulation results " << std::endl;

	// load materials database
	const auto MDBfile = _job.GetValue<fs::path>(EScriptKeys::MATERIALS_DATABASE);
	std::cout << "Loading materials database file: \n\t" << MDBfile << std::endl;
	if (!m_materialsDatabase.LoadFromFile(MDBfile))
	{
		std::cout << "Error: Materials database file can not be loaded" << std::endl;
		return false;
	}

	// set paths to models
	std::cout << "Loading models from: \n\t" << fs::current_path() << std::endl;
	m_modelsManager.AddDir(L".");		// add current directory as path to units/solvers
	for (const auto& dir : _job.GetValues<fs::path>(EScriptKeys::MODELS_PATH))
	{
		std::cout << "Loading models from: \n\t" << dir << std::endl;
		m_modelsManager.AddDir(dir);
	}

	// load flowsheet
	if (hasSrc)
	{
		const auto srcFile = _job.GetValue<fs::path>(EScriptKeys::SOURCE_FILE);
		std::cout << "Loading flowsheet file: \n\t" << srcFile << std::endl;
		CH5Handler fileHandler;
		if (!m_flowsheet.LoadFromFile(fileHandler, srcFile))
		{
			std::cout << "Error: The flowsheet file can not be loaded" << std::endl;
			return false;
		}
	}

	return true;
}

#define _EXPAND(x) x
#define _RESOLVE_MACRO(_1,_2,_3,_4,NAME,...) NAME
#define SET_PARAM3(KEY, TYPE, FUN) if (_job.HasKey(KEY)) params->FUN(_job.GetValue<TYPE>(KEY))
#define SET_PARAM4(KEY, TYPE, FUN, TYPE_CAST) if (_job.HasKey(KEY)) params->FUN(static_cast<TYPE_CAST>(_job.GetValue<TYPE>(KEY)))
#define SET_PARAM(...) _EXPAND(_RESOLVE_MACRO(__VA_ARGS__, SET_PARAM4, SET_PARAM3)(__VA_ARGS__))

bool CScriptRunner::SetupFlowsheetParameters(const CScriptJob& _job)
{
	auto* params = m_flowsheet.GetParameters();

	SET_PARAM(EScriptKeys::SIMULATION_TIME             , double  , EndSimulationTime                     );
	SET_PARAM(EScriptKeys::RELATIVE_TOLERANCE          , double  , RelTol                                );
	SET_PARAM(EScriptKeys::ABSOLUTE_TOLERANCE          , double  , AbsTol                                );
	SET_PARAM(EScriptKeys::MINIMAL_FRACTION            , double  , MinFraction                           );
	SET_PARAM(EScriptKeys::INIT_TIME_WINDOW            , double  , InitTimeWindow                        );
	SET_PARAM(EScriptKeys::SAVE_TIME_STEP_HINT         , double  , SaveTimeStep                          );
	SET_PARAM(EScriptKeys::SAVE_FLAG_FOR_HOLDUPS       , bool    , SaveTimeStepFlagHoldups               );
	SET_PARAM(EScriptKeys::THERMO_TEMPERATURE_MIN      , double  , EnthalpyMinT                          );
	SET_PARAM(EScriptKeys::THERMO_TEMPERATURE_MAX      , double  , EnthalpyMaxT                          );
	SET_PARAM(EScriptKeys::THERMO_TEMPERATURE_INTERVALS, uint64_t, EnthalpyInt                           );
	SET_PARAM(EScriptKeys::MIN_TIME_WINDOW             , double  , MinTimeWindow                         );
	SET_PARAM(EScriptKeys::MAX_TIME_WINDOW             , double  , MaxTimeWindow                         );
	SET_PARAM(EScriptKeys::MAX_ITERATIONS_NUMBER       , uint64_t, MaxItersNumber         , uint32_t     );
	SET_PARAM(EScriptKeys::WINDOW_CHANGE_RATE          , double  , MagnificationRatio                    );
	SET_PARAM(EScriptKeys::ITERATIONS_UPPER_LIMIT      , uint64_t, ItersUpperLimit        , uint32_t     );
	SET_PARAM(EScriptKeys::ITERATIONS_LOWER_LIMIT      , uint64_t, ItersLowerLimit        , uint32_t     );
	SET_PARAM(EScriptKeys::ITERATIONS_UPPER_LIMIT_1ST  , uint64_t, Iters1stUpperLimit     , uint32_t     );
	SET_PARAM(EScriptKeys::RELAXATION_PARAMETER        , double  , RelaxationParam                       );
	SET_PARAM(EScriptKeys::ACCELERATION_LIMIT          , double  , WegsteinAccelParam                    );
	// TODO: make indexing from 1 or name them
	SET_PARAM(EScriptKeys::CONVERGENCE_METHOD          , uint64_t, ConvergenceMethod      , EConvMethod  );
	SET_PARAM(EScriptKeys::EXTRAPOLATION_METHOD        , uint64_t, ExtrapolationMethod    , EExtrapMethod);

	return true;
}

bool CScriptRunner::SetupUnits(const CScriptJob& _job)
{
	// Returns a pointer to unit by its name or index.
	const auto GetUnitPtr = [&](const SUnitParameterScriptEntry& _entry)
	{
		auto* unit = m_flowsheet.GetUnitByName(_entry.unitName);	// try to access by name
		if (!unit) unit = m_flowsheet.GetUnit(_entry.unitIndex);	// try to access by index
		return unit;												// return pointer
	};

	// Returns a pointer to unit parameter by its name or index.
	const auto GetParamPtr = [&](CBaseUnit& _model, const SUnitParameterScriptEntry& _entry)
	{
		auto& manager = _model.GetUnitParametersManager();				// get unit parameters manager
		auto* param = manager.GetParameter(_entry.paramName);			// try to access by name
		if (!param) param = manager.GetParameter(_entry.paramIndex);	// try to access by index
		return param;													// return pointer
	};

	for (const auto& entry : _job.GetValues<SUnitParameterScriptEntry>(EScriptKeys::UNIT_PARAMETER))
	{
		auto* unit = GetUnitPtr(entry);				// get pointer to unit
		if (!unit)
		{
			std::cout << "Error while applying " << ARG2STR(EScriptKeys::UNIT_PARAMETER) << ": \n\t"
				<< "Cannot load a unit neither by its name " << StringFunctions::Quote(entry.unitName)
				<< " nor by its index " << StringFunctions::Quote(std::to_string(entry.unitIndex + 1)) << std::endl;
			return false;
		}
		auto* model = unit->GetModel();				// get pointer to unit's model
		if (!model)
		{
			std::cout << "Error while applying " << ARG2STR(EScriptKeys::UNIT_PARAMETER) << ": \n\t"
				<< "Cannot load a model for unit " << StringFunctions::Quote(unit->GetName()) << std::endl;
			return false;
		}
		auto* param = GetParamPtr(*model, entry);	// get pointer to unit parameter
		if (!param)
		{
			std::cout << "Error while applying " << ARG2STR(EScriptKeys::UNIT_PARAMETER) << ": \n\t"
				<< "Cannot load a parameter of unit " << StringFunctions::Quote(unit->GetName()) << " neither by its name " << StringFunctions::Quote(entry.paramName)
				<< " nor by its index " << StringFunctions::Quote(std::to_string(entry.paramIndex + 1)) << std::endl;
			return false;
		}
		std::stringstream ss{ entry.values };		// create a stream with parameter values
		param->ValueFromStream(ss);					// read unit parameter values
	}

	return true;
}

bool CScriptRunner::RunSimulation(const CScriptJob& _job)
{
	// TODO: move text to DyssolStringConstants.h
	// initialize flowsheet
	std::cout << "Initializing flowsheet" << std::endl;
	const std::string error = m_flowsheet.Initialize();
	if (!error.empty())
	{
		std::cout << "Error during initialization: " << error << std::endl;
		return false;
	}

	// run simulation
	m_simulator.SetFlowsheet(&m_flowsheet);
	std::cout << "Starting simulation" << std::endl;
	const auto tStart = std::chrono::steady_clock::now();
	m_simulator.Simulate();
	const auto tEnd = std::chrono::steady_clock::now();

	// save simulation results
	const auto dstFile = _job.HasKey(EScriptKeys::RESULT_FILE) ? _job.GetValue<fs::path>(EScriptKeys::RESULT_FILE) : _job.GetValue<fs::path>(EScriptKeys::SOURCE_FILE);
	std::cout << "Saving flowsheet to: " << dstFile << std::endl;
	CH5Handler fileHandler;
	m_flowsheet.SaveToFile(fileHandler, dstFile);

	std::cout << "Simulation finished in " << std::chrono::duration_cast<std::chrono::seconds>(tEnd - tStart).count() << " [s]" << std::endl;

	return true;
}

void CScriptRunner::Clear()
{
	m_flowsheet.Clear();
	m_modelsManager.Clear();
	m_materialsDatabase.Clear();
}

