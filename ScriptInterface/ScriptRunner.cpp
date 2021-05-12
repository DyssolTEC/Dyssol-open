/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ScriptRunner.h"
#include "BaseUnit.h"
#include "ScriptJob.h"
#include "DyssolStringConstants.h"
#include "DyssolUtilities.h"
#include <functional>

using namespace ScriptInterface;
using namespace StrConst;
using namespace std::filesystem;
using namespace std::chrono;

void CScriptRunner::RunJob(const CScriptJob& _job)
{
	const auto tStart = steady_clock::now();

	Clear();
	bool success = true;
	if (success) success &= ConfigureFlowsheet(_job);
	if (success) success &= RunSimulation(_job);

	const auto tEnd = steady_clock::now();
	std::cout << DyssolC_ScriptFinished(duration_cast<seconds>(tEnd - tStart).count()) << std::endl;
	if (!success)
		std::cout << DyssolC_ErrorFinish() << std::endl;
}

bool CScriptRunner::ConfigureFlowsheet(const CScriptJob& _job)
{
	bool success = true;
	if (success) success &= LoadFiles(_job);
	if (success) success &= SetupFlowsheetParameters(_job);
	if (success) success &= SetupUnitParameters(_job);
	if (success) success &= SetupHoldups(_job);
	return success;
}

#define PRINT_AND_RETURN(MESSAGE, ...) { std::cout << MESSAGE(__VA_ARGS__) << std::endl; return false; }

bool CScriptRunner::LoadFiles(const CScriptJob& _job)
{
	const bool hasSrc = _job.HasKey(EScriptKeys::SOURCE_FILE);
	const bool hasDst = _job.HasKey(EScriptKeys::RESULT_FILE);
	if (!hasSrc && !hasDst)
		PRINT_AND_RETURN(DyssolC_ErrorSrcDst, StrKey(EScriptKeys::SOURCE_FILE), StrKey(EScriptKeys::RESULT_FILE));
	if (hasSrc && !hasDst)
		std::cout << DyssolC_WriteSrc(StrKey(EScriptKeys::SOURCE_FILE), StrKey(EScriptKeys::RESULT_FILE)) << std::endl;

	// load materials database
	const auto MDBfile = _job.GetValue<path>(EScriptKeys::MATERIALS_DATABASE);
	std::cout << DyssolC_LoadMDB(MDBfile.string()) << std::endl;
	if (!m_materialsDatabase.LoadFromFile(MDBfile))
		PRINT_AND_RETURN(DyssolC_ErrorMDB);

	// set paths to models
	std::cout << DyssolC_LoadModels(current_path().string()) << std::endl;
	m_modelsManager.AddDir(L".");		// add current directory as path to units/solvers
	for (const auto& dir : _job.GetValues<path>(EScriptKeys::MODELS_PATH))
	{
		std::cout << DyssolC_LoadModels(dir.string()) << std::endl;
		m_modelsManager.AddDir(dir);
	}

	// load flowsheet
	if (hasSrc)
	{
		const auto srcFile = _job.GetValue<path>(EScriptKeys::SOURCE_FILE);
		std::cout << DyssolC_LoadFlowsheet(srcFile.string()) << std::endl;
		CH5Handler fileHandler;
		if (!m_flowsheet.LoadFromFile(fileHandler, srcFile))
			PRINT_AND_RETURN(DyssolC_ErrorLoad);
	}

	return true;
}

// Help macros to simplify setting of flowsheet parameters
#define EXPAND_MACRO(x) x
#define RESOLVE_MACRO(_1,_2,_3,_4,_5,NAME,...) NAME
#define SET_PARAM3(KEY, TYPE, FUN) if (_job.HasKey(KEY)) params->FUN(_job.GetValue<TYPE>(KEY))
#define SET_PARAM4(KEY, TYPE, FUN, TYPE_CAST) if (_job.HasKey(KEY)) params->FUN(static_cast<TYPE_CAST>(_job.GetValue<TYPE>(KEY)))
#define SET_PARAM5(KEY, TYPE, FUN, TYPE_CAST, VAR) if (_job.HasKey(KEY)) params->FUN(static_cast<TYPE_CAST>(_job.GetValue<TYPE>(KEY).VAR))
#define SET_PARAM(...) EXPAND_MACRO(RESOLVE_MACRO(__VA_ARGS__, SET_PARAM5, SET_PARAM4, SET_PARAM3)(__VA_ARGS__))

bool CScriptRunner::SetupFlowsheetParameters(const CScriptJob& _job)
{
	auto* params = m_flowsheet.GetParameters();

	SET_PARAM(EScriptKeys::SIMULATION_TIME             , double    , EndSimulationTime                          );
	SET_PARAM(EScriptKeys::RELATIVE_TOLERANCE          , double    , RelTol                                     );
	SET_PARAM(EScriptKeys::ABSOLUTE_TOLERANCE          , double    , AbsTol                                     );
	SET_PARAM(EScriptKeys::MINIMAL_FRACTION            , double    , MinFraction                                );
	SET_PARAM(EScriptKeys::INIT_TIME_WINDOW            , double    , InitTimeWindow                             );
	SET_PARAM(EScriptKeys::SAVE_TIME_STEP_HINT         , double    , SaveTimeStep                               );
	SET_PARAM(EScriptKeys::SAVE_FLAG_FOR_HOLDUPS       , bool      , SaveTimeStepFlagHoldups                    );
	SET_PARAM(EScriptKeys::THERMO_TEMPERATURE_MIN      , double    , EnthalpyMinT                               );
	SET_PARAM(EScriptKeys::THERMO_TEMPERATURE_MAX      , double    , EnthalpyMaxT                               );
	SET_PARAM(EScriptKeys::THERMO_TEMPERATURE_INTERVALS, uint64_t  , EnthalpyInt                                );
	SET_PARAM(EScriptKeys::MIN_TIME_WINDOW             , double    , MinTimeWindow                              );
	SET_PARAM(EScriptKeys::MAX_TIME_WINDOW             , double    , MaxTimeWindow                              );
	SET_PARAM(EScriptKeys::MAX_ITERATIONS_NUMBER       , uint64_t  , MaxItersNumber         , uint32_t          );
	SET_PARAM(EScriptKeys::WINDOW_CHANGE_RATE          , double    , MagnificationRatio                         );
	SET_PARAM(EScriptKeys::ITERATIONS_UPPER_LIMIT      , uint64_t  , ItersUpperLimit        , uint32_t          );
	SET_PARAM(EScriptKeys::ITERATIONS_LOWER_LIMIT      , uint64_t  , ItersLowerLimit        , uint32_t          );
	SET_PARAM(EScriptKeys::ITERATIONS_UPPER_LIMIT_1ST  , uint64_t  , Iters1stUpperLimit     , uint32_t          );
	SET_PARAM(EScriptKeys::RELAXATION_PARAMETER        , double    , RelaxationParam                            );
	SET_PARAM(EScriptKeys::ACCELERATION_LIMIT          , double    , WegsteinAccelParam                         );
	SET_PARAM(EScriptKeys::CONVERGENCE_METHOD          , SNameOrKey, ConvergenceMethod      , EConvergenceMethod  , key);
	SET_PARAM(EScriptKeys::EXTRAPOLATION_METHOD        , SNameOrKey, ExtrapolationMethod    , EExtrapolationMethod, key);
	return true;
}

bool CScriptRunner::SetupUnitParameters(const CScriptJob& _job)
{
	for (const auto& entry : _job.GetValues<SUnitParameterSE>(EScriptKeys::UNIT_PARAMETER))
	{
		// get pointer to unit
		auto* unit = GetUnitPtr(entry.unit);
		if (!unit)
			PRINT_AND_RETURN(DyssolC_ErrorParseUnit, StrKey(EScriptKeys::UNIT_PARAMETER), entry.unit.name, entry.unit.index);
		// get pointer to unit's model
		auto* model = unit->GetModel();
		if (!model)
			PRINT_AND_RETURN(DyssolC_ErrorParseModel, StrKey(EScriptKeys::UNIT_PARAMETER), unit->GetName());
		// get pointer to unit parameter
		auto* param = GetUnitParamPtr(*model, entry.param);
		if (!param)
			PRINT_AND_RETURN(DyssolC_ErrorParseUP, StrKey(EScriptKeys::UNIT_PARAMETER), unit->GetName(), entry.param.name, entry.param.index);
		std::stringstream ss{ entry.values };	// create a stream with parameter values
		param->ValueFromStream(ss);				// read unit parameter values
	}

	return true;
}

bool CScriptRunner::SetupHoldups(const CScriptJob& _job)
{
	// The holdup may be cleaned before setting time-dependent values. But those holdups, which are not mentioned in the script file, must not be changed.
	// If cleaning is requested, on the first access to the holdup, clean it, store in this vector and do not clean any further.
	std::vector<CBaseStream*> processed;												// already processed holdups
	const bool keepTP = !_job.HasKey(EScriptKeys::HOLDUPS_KEEP_EXISTING_VALUES) || _job.GetValue<bool>(EScriptKeys::HOLDUPS_KEEP_EXISTING_VALUES);	// keep or remove time points

	// setup holdups overall parameters
	for (const auto& entry : _job.GetValues<SHoldupOverallSE>(EScriptKeys::HOLDUP_OVERALL))
	{
		// get pointer to unit
		auto* unit = GetUnitPtr(entry.unit);
		if (!unit)
			PRINT_AND_RETURN(DyssolC_ErrorParseUnit, StrKey(EScriptKeys::HOLDUP_OVERALL), entry.unit.name, entry.unit.index);
		// get pointer to unit's model
		auto* model = unit->GetModel();
		if (!model)
			PRINT_AND_RETURN(DyssolC_ErrorParseModel, StrKey(EScriptKeys::HOLDUP_OVERALL), unit->GetName())
		// get pointer to holdup
		auto* holdup = GetHoldupPtr(*model, entry.holdup);
		if (!holdup)
			PRINT_AND_RETURN(DyssolC_ErrorParseHO, StrKey(EScriptKeys::HOLDUP_OVERALL), unit->GetName(), entry.holdup.name, entry.holdup.index);
		// remove all time points if requested and if it is the first access to this stream
		if (!keepTP && !VectorContains(processed, holdup))
		{
			holdup->RemoveAllTimePoints();
			processed.push_back(holdup);
		}
		// set values
		if (m_flowsheet.HasOverallProperty(static_cast<EOverall>(entry.param.key)))
			for (const auto& [time, value] : entry.values)
				holdup->SetOverallProperty(time, static_cast<EOverall>(entry.param.key), value);
		else
			std::cout << DyssolC_WarningNoOverall(StrKey(EScriptKeys::HOLDUP_OVERALL), entry.param.key) << std::endl;
	}

	return true;
}

bool CScriptRunner::RunSimulation(const CScriptJob& _job)
{
	// initialize flowsheet
	std::cout << DyssolC_Initialize() << std::endl;
	const std::string error = m_flowsheet.Initialize();
	if (!error.empty())
		PRINT_AND_RETURN(DyssolC_ErrorInit, error);

	// run simulation
	m_simulator.SetFlowsheet(&m_flowsheet);
	std::cout << DyssolC_Start() << std::endl;
	const auto tStart = steady_clock::now();
	m_simulator.Simulate();
	const auto tEnd = steady_clock::now();

	// save simulation results
	const auto dstFile = _job.HasKey(EScriptKeys::RESULT_FILE) ? _job.GetValue<path>(EScriptKeys::RESULT_FILE) : _job.GetValue<path>(EScriptKeys::SOURCE_FILE);
	std::cout << DyssolC_SaveFlowsheet(dstFile.string()) << std::endl;
	CH5Handler fileHandler;
	m_flowsheet.SaveToFile(fileHandler, dstFile);

	std::cout << DyssolC_SimFinished(duration_cast<seconds>(tEnd - tStart).count()) << std::endl;

	return true;
}

void CScriptRunner::Clear()
{
	m_flowsheet.Clear();
	m_modelsManager.Clear();
	m_materialsDatabase.Clear();
}

CUnitContainer* CScriptRunner::GetUnitPtr(const SNameOrIndex& _nameOrIndex)
{
	auto* unit = m_flowsheet.GetUnitByName(_nameOrIndex.name);	// try to access by name
	if (!unit) unit = m_flowsheet.GetUnit(_nameOrIndex.index);	// try to access by index
	return unit;												// return pointer
}

CBaseUnitParameter* CScriptRunner::GetUnitParamPtr(CBaseUnit& _model, const SNameOrIndex& _nameOrIndex)
{
	auto& manager = _model.GetUnitParametersManager();				// get unit parameters manager
	auto* param = manager.GetParameter(_nameOrIndex.name);			// try to access by name
	if (!param) param = manager.GetParameter(_nameOrIndex.index);	// try to access by index
	return param;													// return pointer
}

CBaseStream* CScriptRunner::GetHoldupPtr(CBaseUnit& _model, const SNameOrIndex& _nameOrIndex)
{
	auto& manager = _model.GetStreamsManager();							// get streams manager
	auto* holdup = manager.GetObjectInit(_nameOrIndex.name);			// try to access by name
	if (!holdup) holdup = manager.GetObjectInit(_nameOrIndex.index);	// try to access by index
	return holdup;														// return pointer
}

