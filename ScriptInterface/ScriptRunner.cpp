/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ScriptRunner.h"
#include "BaseUnit.h"
#include "ScriptJob.h"
#include "DyssolStringConstants.h"
#include "DyssolUtilities.h"
#include <sstream>

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
#define PRINT_AND_RETURN2(MESSAGE)     { std::cout << MESSAGE << std::endl; return false; }

bool CScriptRunner::LoadFiles(const CScriptJob& _job)
{
	const bool hasSrc = _job.HasKey(EScriptKeys::SOURCE_FILE);
	const bool hasDst = _job.HasKey(EScriptKeys::RESULT_FILE);
	if (!hasSrc && !hasDst)
		PRINT_AND_RETURN(DyssolC_ErrorSrcDst, StrKey(EScriptKeys::SOURCE_FILE), StrKey(EScriptKeys::RESULT_FILE))
	if (hasSrc && !hasDst)
		std::cout << DyssolC_WriteSrc(StrKey(EScriptKeys::SOURCE_FILE), StrKey(EScriptKeys::RESULT_FILE)) << std::endl;

	// load materials database
	const auto MDBfile = _job.GetValue<path>(EScriptKeys::MATERIALS_DATABASE);
	std::cout << DyssolC_LoadMDB(MDBfile.string()) << std::endl;
	if (!m_materialsDatabase.LoadFromFile(MDBfile))
		PRINT_AND_RETURN(DyssolC_ErrorMDB)

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
			PRINT_AND_RETURN(DyssolC_ErrorLoad)
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
		if (!unit)		PRINT_AND_RETURN(DyssolC_ErrorParseUnit, StrKey(EScriptKeys::UNIT_PARAMETER), entry.unit.name, entry.unit.index)
		// get pointer to unit's model
		auto* model = unit->GetModel();
		if (!model)		PRINT_AND_RETURN(DyssolC_ErrorParseModel, StrKey(EScriptKeys::UNIT_PARAMETER), unit->GetName())
		// get pointer to unit parameter
		auto* param = GetUnitParamPtr(*model, entry.param);
		if (!param)		PRINT_AND_RETURN(DyssolC_ErrorParseUP, StrKey(EScriptKeys::UNIT_PARAMETER), unit->GetName(), entry.param.name, entry.param.index)
		std::stringstream ss{ entry.values };	// create a stream with parameter values
		param->ValueFromStream(ss);				// read unit parameter values
	}

	return true;
}

bool CScriptRunner::SetupHoldups(const CScriptJob& _job)
{
	// The holdup may be cleaned before setting time-dependent values. But those holdups, which are not mentioned in the script file, must not be changed.
	// If cleaning is requested, on the first access to the holdup, clean it, store in this vector and do not clean any further.
	std::vector<CBaseStream*> processed;	// already processed holdups
	const bool keepTP = !_job.HasKey(EScriptKeys::HOLDUPS_KEEP_EXISTING_VALUES) || _job.GetValue<bool>(EScriptKeys::HOLDUPS_KEEP_EXISTING_VALUES);	// keep or remove time points

	// Obtains the required holdup, cleans it if required and returns a pointer to it.
	const auto ObtainHoldup = [&](const SNameOrIndex& _unit, const SNameOrIndex& _holdup, EScriptKeys _scriptKey) -> std::tuple<CBaseStream*, CUnitContainer*, std::string>
	{
		// get pointer to unit
		auto* unit = GetUnitPtr(_unit);
		if (!unit)		return std::make_tuple(nullptr, nullptr, DyssolC_ErrorParseUnit(StrKey(EScriptKeys::HOLDUP_OVERALL), _unit.name, _unit.index));
		// get pointer to unit's model
		auto* model = unit->GetModel();
		if (!model)		return std::make_tuple(nullptr, nullptr, DyssolC_ErrorParseModel(StrKey(EScriptKeys::HOLDUP_OVERALL), unit->GetName()));
		// get pointer to holdup
		auto* holdup = GetHoldupPtr(*model, _holdup);
		if (!holdup)	return std::make_tuple(nullptr, nullptr, DyssolC_ErrorParseHO(StrKey(EScriptKeys::HOLDUP_OVERALL), unit->GetName(), _holdup.name, _holdup.index));
		// remove all time points if requested and if it is the first access to this stream
		if (!keepTP && !VectorContains(processed, holdup))
		{
			holdup->RemoveAllTimePoints();
			processed.push_back(holdup);
		}
		// return pointer to holdup
		return std::make_tuple(holdup, unit, "");
	};

	// setup holdups overall parameters
	for (const auto& entry : _job.GetValues<SHoldupDependentSE>(EScriptKeys::HOLDUP_OVERALL))
	{
		// get pointer to holdup
		auto [holdup, unit, message] = ObtainHoldup(entry.unit, entry.holdup, EScriptKeys::HOLDUP_OVERALL);
		if (!holdup) PRINT_AND_RETURN2(message)
		// check the number of passed arguments
		if (entry.values.size() != m_flowsheet.GetOverallPropertiesNumber() && entry.values.size() % (m_flowsheet.GetOverallPropertiesNumber() + 1) != 0)
			PRINT_AND_RETURN(DyssolC_ErrorArgumentsNumber, StrKey(EScriptKeys::HOLDUP_OVERALL), unit->GetName(), entry.holdup.name, entry.holdup.index)
		// set values: only values for time point 0 without time are given
		if (entry.values.size() == m_flowsheet.GetOverallPropertiesNumber())
			for (size_t iOvr = 0; iOvr < m_flowsheet.GetOverallPropertiesNumber(); ++iOvr)
				holdup->SetOverallProperty(0.0, m_flowsheet.GetOveralProperties()[iOvr].type, entry.values[iOvr]);
		// set values: values with time points are given
		else
			for (size_t iTime = 0; iTime < entry.values.size(); iTime += m_flowsheet.GetOverallPropertiesNumber() + 1)
				for (size_t iOvr = 0; iOvr < m_flowsheet.GetOverallPropertiesNumber(); ++iOvr)
					holdup->SetOverallProperty(entry.values[iTime], m_flowsheet.GetOveralProperties()[iOvr].type, entry.values[iTime + iOvr + 1]);
	}

	// setup holdups phase fractions
	for (const auto& entry : _job.GetValues<SHoldupDependentSE>(EScriptKeys::HOLDUP_PHASES))
	{
		// get pointer to holdup
		auto [holdup, unit, message] = ObtainHoldup(entry.unit, entry.holdup, EScriptKeys::HOLDUP_PHASES);
		if (!holdup) PRINT_AND_RETURN2(message)
		// check the number of passed arguments
		if (entry.values.size() != m_flowsheet.GetPhasesNumber() && entry.values.size() % (m_flowsheet.GetPhasesNumber() + 1) != 0)
			PRINT_AND_RETURN(DyssolC_ErrorArgumentsNumber, StrKey(EScriptKeys::HOLDUP_PHASES), unit->GetName(), entry.holdup.name, entry.holdup.index)
		// set values: only values for time point 0 without time are given
		if (entry.values.size() == m_flowsheet.GetPhasesNumber())
			for (size_t iPhase = 0; iPhase < m_flowsheet.GetPhasesNumber(); ++iPhase)
				holdup->SetPhaseFraction(0.0, m_flowsheet.GetPhases()[iPhase].state, entry.values[iPhase]);
		// set values: values with time points are given
		else
			for (size_t iTime = 0; iTime < entry.values.size(); iTime += m_flowsheet.GetPhasesNumber() + 1)
				for (size_t iPhase = 0; iPhase < m_flowsheet.GetPhasesNumber(); ++iPhase)
					holdup->SetPhaseFraction(entry.values[iTime], m_flowsheet.GetPhases()[iPhase].state, entry.values[iTime + iPhase + 1]);
	}

	// setup holdups compound fractions
	for (const auto& entry : _job.GetValues<SHoldupCompoundsSE>(EScriptKeys::HOLDUP_COMPOUNDS))
	{
		// get pointer to holdup
		auto [holdup, unit, message] = ObtainHoldup(entry.unit, entry.holdup, EScriptKeys::HOLDUP_COMPOUNDS);
		if (!holdup) PRINT_AND_RETURN2(message)
		// check the number of passed arguments
		if (entry.values.size() != m_flowsheet.GetCompoundsNumber() && entry.values.size() % (m_flowsheet.GetCompoundsNumber() + 1) != 0)
			PRINT_AND_RETURN(DyssolC_ErrorArgumentsNumber, StrKey(EScriptKeys::HOLDUP_COMPOUNDS), unit->GetName(), entry.holdup.name, entry.holdup.index, entry.phase.name, entry.phase.key)
		// check that all phases are defined
		if (!m_flowsheet.HasPhase(static_cast<EPhase>(entry.phase.key)))
			PRINT_AND_RETURN(DyssolC_ErrorNoPhase, StrKey(EScriptKeys::HOLDUP_COMPOUNDS), unit->GetName(), entry.holdup.name, entry.holdup.index, entry.phase.name, entry.phase.key)
		// set values: only values for time point 0 without time are given
		if (entry.values.size() == m_flowsheet.GetCompoundsNumber())
			holdup->SetCompoundsFractions(0.0, static_cast<EPhase>(entry.phase.key), entry.values);
		// set values: values with time points are given
		else
			for (size_t iTime = 0; iTime < entry.values.size(); iTime += m_flowsheet.GetCompoundsNumber() + 1)
				holdup->SetCompoundsFractions(entry.values[iTime], static_cast<EPhase>(entry.phase.key), std::vector<double>{ entry.values.begin() + iTime + 1, entry.values.begin() + iTime + 1 + m_flowsheet.GetCompoundsNumber() });
	}

	// setup solids distributions
	for (const auto& entry : _job.GetValues<SHoldupDistributionSE>(EScriptKeys::HOLDUP_DISTRIBUTION))
	{
		// get pointer to holdup
		auto [holdup, unit, message] = ObtainHoldup(entry.unit, entry.holdup, EScriptKeys::HOLDUP_DISTRIBUTION);
		if (!holdup) PRINT_AND_RETURN2(message)
		// read required values for ease of use
		const auto distr = static_cast<EDistrTypes>(entry.distrType.key);					// distribution type
		const auto fun = static_cast<EDistrFunction>(entry.function.key);					// distribution function type
		const auto mean = static_cast<EPSDGridType>(entry.psdMeans.key);					// mean values type for PSD
		const auto psd = static_cast<EPSDTypes>(entry.psdType.key);							// PSD type
		const bool manual = fun == EDistrFunction::Manual;									// whether manual distribution defined
		const size_t& len = entry.values.size();											// length of the values vector
		// TODO: use holdup's grid instead of the global grid
		const auto& grid = m_flowsheet.GetGrid();											// distributions grid
		const size_t& classes = grid.GetGridDimension(distr)->ClassesNumber();				// number of classes in the distribution
		const auto means = distr != DISTR_SIZE ? grid.GetGridDimensionNumeric(distr)->GetClassesMeans() : grid.GetPSDMeans(mean); // mean valued for the PSD grid
		const bool hasTime = manual && len % (classes + 1) == 0 || !manual && len % 3 == 0;	// whether time is defined
		const bool mix = entry.compound.empty();											// whether the distribution is defined for the total mixture of for a single compound
		// check the number of passed arguments
		if (manual && len != classes && len % (classes + 1) != 0 || !manual && len != 2 && len % 3 != 0)
			PRINT_AND_RETURN(DyssolC_ErrorArgumentsNumber, StrKey(EScriptKeys::HOLDUP_DISTRIBUTION), unit->GetName(), entry.holdup.name, entry.holdup.index)
		if (!grid.HasDimension(distr))
			PRINT_AND_RETURN(DyssolC_ErrorNoDistribution, StrKey(EScriptKeys::HOLDUP_DISTRIBUTION), unit->GetName(), entry.holdup.name, entry.holdup.index, entry.distrType.name, entry.distrType.key)
		// get and check compound key
		const auto* compound = m_materialsDatabase.GetCompound(entry.compound) ? m_materialsDatabase.GetCompound(entry.compound) : m_materialsDatabase.GetCompoundByName(entry.compound);
		const std::string key = compound ? compound->GetKey() : "";
		if (!mix && key.empty())
			PRINT_AND_RETURN(DyssolC_ErrorNoCompound, StrKey(EScriptKeys::HOLDUP_DISTRIBUTION), unit->GetName(), entry.holdup.name, entry.holdup.index, key)
		// split times and values
		const size_t dT = manual ? classes : 2;		// step between time points
		const size_t offs = hasTime ? 1 : 0;		// additional offset due to the time point itself
		std::vector<double> times;					// all defined time points
		std::vector<std::vector<double>> values;	// defined distributed values for each time point
		for (size_t iTime = 0; iTime < entry.values.size(); iTime += dT + offs)
		{
			times.push_back(hasTime ? entry.values[iTime] : 0.0);
			values.emplace_back(entry.values.begin() + iTime + offs, entry.values.begin() + iTime + offs + dT);
		}
		// create functional distributions
		if (!manual)
			for (auto& value : values)
				value = CreateDistribution(fun, means, value[0], value[1]);
		// set values
		for (size_t i = 0; i < times.size(); ++i)
			if (distr == DISTR_SIZE)	holdup->SetPSD(times[i], psd, key, values[i], mean);
			else if (mix)				holdup->SetDistribution(times[i], distr, values[i]);
				else					holdup->SetDistribution(times[i], distr, key, values[i]);
	}

	return true;
}

bool CScriptRunner::RunSimulation(const CScriptJob& _job)
{
	// initialize flowsheet
	std::cout << DyssolC_Initialize() << std::endl;
	const std::string error = m_flowsheet.Initialize();
	if (!error.empty())
		PRINT_AND_RETURN(DyssolC_ErrorInit, error)

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

