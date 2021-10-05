/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ScriptRunner.h"
#include "BaseUnit.h"
#include "ScriptJob.h"
#include "DyssolStringConstants.h"
#include "DyssolUtilities.h"
#include <sstream>
#include <fstream>
#include <functional>

using namespace ScriptInterface;
using namespace StrConst;
namespace fs = std::filesystem;
namespace ch = std::chrono;

void CScriptRunner::RunJob(const CScriptJob& _job)
{
	const auto tStart = ch::steady_clock::now();

	Clear();
	bool success = true;
	if (success) success &= ConfigureFlowsheet(_job);
	if (success) success &= RunSimulation(_job);
	if (success) success &= ExportResults(_job);

	const auto tEnd = ch::steady_clock::now();
	std::cout << DyssolC_ScriptFinished(ch::duration_cast<ch::seconds>(tEnd - tStart).count()) << std::endl;
	if (!success)
		std::cout << DyssolC_ErrorFinish() << std::endl;
}

bool CScriptRunner::ConfigureFlowsheet(const CScriptJob& _job)
{
	bool success = true;
	if (success) success &= LoadFiles(_job);
	if (success) success &= SetupFlowsheet(_job);
	// TODO: put inside SetupFlowsheet
	if (success) success &= SetupFlowsheetParameters(_job);
	if (success) success &= SetupUnitParameters(_job);
	if (success) success &= SetupHoldups(_job);
	return success;
}

// TODO: rename to PrintError, add PrintInfo for consistency
// Outputs the message into console and return false.
bool PrintAndReturn(const std::string& _message)
{
	std::cout << _message << std::endl;
	return false;
}

void PrintWarning(const std::string& _message)
{
	std::cout << _message << std::endl;
}

bool CScriptRunner::LoadFiles(const CScriptJob& _job)
{
	const bool hasSrc     = _job.HasKey(EScriptKeys::SOURCE_FILE);
	const bool hasDst     = _job.HasKey(EScriptKeys::RESULT_FILE);
	const bool onlyExport = _job.HasKey(EScriptKeys::EXPORT_ONLY) && _job.GetValue<bool>(EScriptKeys::EXPORT_ONLY);
	if (!hasSrc && !hasDst)
		return PrintAndReturn(DyssolC_ErrorSrcDst(StrKey(EScriptKeys::SOURCE_FILE), StrKey(EScriptKeys::RESULT_FILE)));
	if (hasSrc && !hasDst && !onlyExport)
		std::cout << DyssolC_WriteSrc(StrKey(EScriptKeys::SOURCE_FILE), StrKey(EScriptKeys::RESULT_FILE)) << std::endl;

	// load materials database
	const auto MDBfile = _job.GetValue<fs::path>(EScriptKeys::MATERIALS_DATABASE);
	std::cout << DyssolC_LoadMDB(MDBfile.string()) << std::endl;
	if (!m_materialsDatabase.LoadFromFile(MDBfile))
		return PrintAndReturn(DyssolC_ErrorMDB());

	// set paths to models
	auto paths = _job.GetValues<fs::path>(EScriptKeys::MODELS_PATH);
	paths.insert(paths.begin(), fs::current_path()); // current path
	for (const auto& dir : paths)
	{
		std::cout << DyssolC_LoadModels(dir.string()) << std::endl;
		m_modelsManager.AddDir(dir);
	}

	// load flowsheet
	if (hasSrc)
	{
		const auto srcFile = _job.GetValue<fs::path>(EScriptKeys::SOURCE_FILE);
		std::cout << DyssolC_LoadFlowsheet(srcFile.string()) << std::endl;
		CH5Handler fileHandler;
		if (!m_flowsheet.LoadFromFile(fileHandler, srcFile))
			return PrintAndReturn(DyssolC_ErrorLoad());
	}

	return true;
}

bool CScriptRunner::SetupFlowsheet(const CScriptJob& _job)
{
	// TODO: put into calling function
	if (_job.HasKey(EScriptKeys::EXPORT_ONLY) && _job.GetValue<bool>(EScriptKeys::EXPORT_ONLY)) return true;

	// setup units
	// remove existing units
	if (_job.HasKey(EScriptKeys::KEEP_EXISTING_UNITS) && !_job.GetValue<bool>(EScriptKeys::KEEP_EXISTING_UNITS))
		for (const auto& u : m_flowsheet.GetAllUnits())
			m_flowsheet.DeleteUnit(u->GetKey());
	// add or set new units
	for (const auto& entry : _job.GetValues<std::vector<std::string>>(EScriptKeys::UNIT))
	{
		// check input
		if (entry.size() != 2)
			// TODO: output line numbers
			return PrintAndReturn(DyssolC_ErrorArgumentsNumberUnit(StrKey(EScriptKeys::UNIT)));
		// find model key
		const auto key = GetModelKey(entry[1]);
		if (key.empty())
			return PrintAndReturn(DyssolC_ErrorNoModel(StrKey(EScriptKeys::UNIT), entry[1]));
		// whether a unit already exists
		const bool exists = m_flowsheet.GetUnitByName(entry[0]);
		// pointer to unit to work with: existing or a new one
		CUnitContainer* unit = exists ? m_flowsheet.GetUnitByName(entry[0]) : m_flowsheet.AddUnit();
		// set data
		unit->SetName(entry[0]);
		unit->SetModel(key);
	}

	// setup streams
	for (const auto& entry : _job.GetValues<SStreamSE>(EScriptKeys::STREAM))
	{
		// get pointers to units and models
		auto [modelO, unitO, messageO] = TryGetModelPtr(entry.unitO, EScriptKeys::STREAM);
		if (!modelO)
			return PrintAndReturn(messageO);
		auto [modelI, unitI, messageI] = TryGetModelPtr(entry.unitI, EScriptKeys::STREAM);
		if (!modelI)
			return PrintAndReturn(messageI);
		auto* portO = GetPortPtr(*modelO, entry.portO);
		if (!portO)
			return PrintAndReturn(DyssolC_ErrorNoPort(StrKey(EScriptKeys::STREAM), unitO->GetName(), entry.portO.name, entry.portO.index));
		auto* portI = GetPortPtr(*modelI, entry.portI);
		if (!portI)
			return PrintAndReturn(DyssolC_ErrorNoPort(StrKey(EScriptKeys::STREAM), unitI->GetName(), entry.portI.name, entry.portI.index));
		// whether a stream between these ports already exists and connects these ports
		const bool connected = portO->GetStreamKey() == portI->GetStreamKey() && m_flowsheet.GetStream(portO->GetStreamKey());
		// whether a stream with the same name already exists
		const bool exists = m_flowsheet.GetStreamByName(entry.name);
		// pointer to stream to work with: existing or a new one
		auto* stream = connected ? m_flowsheet.GetStream(portO->GetStreamKey()) : exists ? m_flowsheet.GetStreamByName(entry.name) : m_flowsheet.AddStream();
		// remove old connected streams if necessary
		if (!connected)
		{
			m_flowsheet.DeleteStream(portO->GetStreamKey());
			m_flowsheet.DeleteStream(portI->GetStreamKey());
		}
		// set data
		stream->SetName(entry.name);
		portO->SetStreamKey(stream->GetKey());
		portI->SetStreamKey(stream->GetKey());
	}

	// setup compounds
	if (_job.HasKey(EScriptKeys::COMPOUNDS))
	{
		std::vector<std::string> compoundKeys;
		for (const auto& list : _job.GetValues<std::vector<std::string>>(EScriptKeys::COMPOUNDS))
		{
			for (const auto& val : list)
			{
				const auto* cmp = GetCompoundPtr(val);
				if (!cmp)
					return PrintAndReturn(DyssolC_ErrorNoCompounds(StrKey(EScriptKeys::COMPOUNDS), val));
				compoundKeys.push_back(cmp->GetKey());
			}
		}
		m_flowsheet.SetCompounds(compoundKeys);
	}

	// setup phases
	if (_job.HasKey(EScriptKeys::PHASES))
	{
		std::vector<SPhaseDescriptor> phases;
		for (const auto& entry : _job.GetValues<SPhasesSE>(EScriptKeys::PHASES))
			for (size_t i = 0; i < entry.types.size(); ++i)
				phases.push_back(SPhaseDescriptor{ static_cast<EPhase>(entry.types[i].key), entry.names[i] });
		m_flowsheet.SetPhases(phases);
	}

	// The grids may be cleaned before setting new values. Those grids, which are not mentioned in the script file, are not changed.
	// If cleaning is requested, on the first access to the grid's holder, clean it, store the key of the holder in this vector and do not clean any further.
	std::vector<std::string> processed;	// already processed grid's holders
	const bool keepG = !_job.HasKey(EScriptKeys::KEEP_EXISTING_GRIDS_VALUES) || _job.GetValue<bool>(EScriptKeys::KEEP_EXISTING_GRIDS_VALUES);	// keep or remove values in grids before setting new ones

	// setup distribution grids
	for (const auto& entry : _job.GetValues<SGridDimensionSE>(EScriptKeys::DISTRIBUTION_GRID))
	{
		// type of the grid
		const bool mainGrid = entry.unit.name == "GLOBAL";
		// get all pointers and values
		const auto [model, unit, message] = !mainGrid ? TryGetModelPtr(entry.unit, EScriptKeys::DISTRIBUTION_GRID) : std::make_tuple(nullptr, nullptr, "");
		if (!mainGrid && !model)
			return PrintAndReturn(message);
		CMultidimensionalGrid grid = mainGrid ? m_flowsheet.GetGrid() : model->GetGrid();
		// clean the grid if requested and if it is the first access to this grid
		if (const std::string key = mainGrid ? "GLOBAL" : unit->GetKey(); !keepG && !VectorContains(processed, key))
		{
			// remove all except compounds
			for (const auto& t : grid.GetDimensionsTypes())
				if (t != DISTR_COMPOUNDS)
					grid.RemoveDimension(t);
			// remember
			processed.push_back(key);
		}
		// get some values for the ease of use
		const auto& type = static_cast<EDistrTypes>(entry.distrType.key);
		const auto& entryType = static_cast<EGridEntry>(entry.entryType.key);
		const auto& function = static_cast<EGridFunction>(entry.function.key);
		// remove grid dimension if it already exists
		if (grid.HasDimension(type))
			grid.RemoveDimension(type);
		// set new grid dimension
		if (entryType == EGridEntry::GRID_NUMERIC)
		{
			// check number of arguments
			if (function != EGridFunction::GRID_FUN_MANUAL && entry.valuesNum.size() != 2 || function == EGridFunction::GRID_FUN_MANUAL && entry.valuesNum.size() != entry.classes + 1)
				// TODO: use DyssolC_ErrorArgumentsNumber
				return PrintAndReturn(DyssolC_ErrorArgumentsNumberGrid(StrKey(EScriptKeys::DISTRIBUTION_GRID), unit->GetName(), entry.distrType.name, entry.distrType.key));
			// create grid
			std::vector<double> res = function == EGridFunction::GRID_FUN_MANUAL ? entry.valuesNum : CreateGrid(function, entry.classes, entry.valuesNum[0], entry.valuesNum[1]);
			// convert volumes to diameters if required
			if (type == DISTR_SIZE && static_cast<EPSDGridType>(entry.psdMeans.key) == EPSDGridType::VOLUME)
				res = VolumeToDiameter(res);
			// add grid dimension
			grid.AddNumericDimension(type, res);
		}
		else if (entryType == EGridEntry::GRID_SYMBOLIC)
		{
			if (entry.valuesSym.size() != entry.classes)
				return PrintAndReturn(DyssolC_ErrorArgumentsNumberGrid(StrKey(EScriptKeys::DISTRIBUTION_GRID), unit->GetName(), entry.distrType.name, entry.distrType.key));
			// add grid dimension
			grid.AddSymbolicDimension(type, entry.valuesSym);
		}
		// set grid
		if (mainGrid)
			m_flowsheet.SetMainGrid(grid);
		else
			model->SetGrid(grid);
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
	// TODO: put into calling function
	if (_job.HasKey(EScriptKeys::EXPORT_ONLY) && _job.GetValue<bool>(EScriptKeys::EXPORT_ONLY)) return true;

	auto* params = m_flowsheet.GetParameters();

	// TODO: replace with code
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
	SET_PARAM(EScriptKeys::CONVERGENCE_METHOD          , SNamedEnum, ConvergenceMethod      , EConvergenceMethod  , key);
	SET_PARAM(EScriptKeys::EXTRAPOLATION_METHOD        , SNamedEnum, ExtrapolationMethod    , EExtrapolationMethod, key);
	return true;
}

bool CScriptRunner::SetupUnitParameters(const CScriptJob& _job)
{
	// TODO: put into calling function
	if (_job.HasKey(EScriptKeys::EXPORT_ONLY) && _job.GetValue<bool>(EScriptKeys::EXPORT_ONLY)) return true;

	for (const auto& entry : _job.GetValues<SUnitParameterSE>(EScriptKeys::UNIT_PARAMETER))
	{
		// TODO: write error message in TryGetModelPtr
		// get pointer to unit and model
		auto [model, unit, message] = TryGetModelPtr(entry.unit, EScriptKeys::UNIT_PARAMETER);
		if (!model)
			return PrintAndReturn(message);
		// get pointer to unit parameter
		auto* param = GetUnitParamPtr(*model, entry.param);
		if (!param)
			return PrintAndReturn(DyssolC_ErrorNoUP(StrKey(EScriptKeys::UNIT_PARAMETER), unit->GetName(), entry.param.name, entry.param.index));
		std::stringstream ss{ entry.values };	// create a stream with parameter values
		param->ValueFromStream(ss);				// read unit parameter values
	}

	return true;
}

// TODO: split
bool CScriptRunner::SetupHoldups(const CScriptJob& _job)
{
	// TODO: put into calling function
	if (_job.HasKey(EScriptKeys::EXPORT_ONLY) && _job.GetValue<bool>(EScriptKeys::EXPORT_ONLY)) return true;

	// The holdup may be cleaned before setting time-dependent values. Those holdups, which are not mentioned in the script file, are not changed.
	// If cleaning is requested, on the first access to the holdup, clean it, store in this vector and do not clean any further.
	std::vector<CBaseStream*> processed;	// already processed holdups
	const bool keepTP = !_job.HasKey(EScriptKeys::KEEP_EXISTING_HOLDUPS_VALUES) || _job.GetValue<bool>(EScriptKeys::KEEP_EXISTING_HOLDUPS_VALUES);	// keep or remove time points

	// Obtains the required holdup, cleans it if required and returns a pointer to it.
	const auto TryGetHoldupPtr = [&](const SNameOrIndex& _unit, const SNameOrIndex& _holdup, EScriptKeys _scriptKey) -> std::tuple<CBaseStream*, CUnitContainer*, std::string>
	{
		// get pointer to unit and model
		auto [model, unit, message] = TryGetModelPtr(_unit, _scriptKey);
		if (!model)		return std::make_tuple(nullptr, nullptr, message);
		// get pointer to holdup
		auto* holdup = GetHoldupPtr(*model, _holdup);
		if (!holdup)	return std::make_tuple(nullptr, nullptr, DyssolC_ErrorNoHoldup(StrKey(_scriptKey), unit->GetName(), _holdup.name, _holdup.index));
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
		auto [holdup, unit, message] = TryGetHoldupPtr(entry.unit, entry.holdup, EScriptKeys::HOLDUP_OVERALL);
		if (!holdup)
			return PrintAndReturn(message);
		// check the number of passed arguments
		if (entry.values.size() != m_flowsheet.GetOverallPropertiesNumber() && entry.values.size() % (m_flowsheet.GetOverallPropertiesNumber() + 1) != 0)
			return PrintAndReturn(DyssolC_ErrorArgumentsNumber(StrKey(EScriptKeys::HOLDUP_OVERALL), unit->GetName(), entry.holdup.name, entry.holdup.index));
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
		auto [holdup, unit, message] = TryGetHoldupPtr(entry.unit, entry.holdup, EScriptKeys::HOLDUP_PHASES);
		if (!holdup)
			return PrintAndReturn(message);
		// check the number of passed arguments
		if (entry.values.size() != m_flowsheet.GetPhasesNumber() && entry.values.size() % (m_flowsheet.GetPhasesNumber() + 1) != 0)
			return PrintAndReturn(DyssolC_ErrorArgumentsNumber(StrKey(EScriptKeys::HOLDUP_PHASES), unit->GetName(), entry.holdup.name, entry.holdup.index));
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
		auto [holdup, unit, message] = TryGetHoldupPtr(entry.unit, entry.holdup, EScriptKeys::HOLDUP_COMPOUNDS);
		if (!holdup)
			return PrintAndReturn(message);
		// check the number of passed arguments
		if (entry.values.size() != m_flowsheet.GetCompoundsNumber() && entry.values.size() % (m_flowsheet.GetCompoundsNumber() + 1) != 0)
			return PrintAndReturn(DyssolC_ErrorArgumentsNumber(StrKey(EScriptKeys::HOLDUP_COMPOUNDS), unit->GetName(), entry.holdup.name, entry.holdup.index, entry.phase.name, entry.phase.key));
		// check that all phases are defined
		if (!m_flowsheet.HasPhase(static_cast<EPhase>(entry.phase.key)))
			return PrintAndReturn(DyssolC_ErrorNoPhase(StrKey(EScriptKeys::HOLDUP_COMPOUNDS), unit->GetName(), entry.holdup.name, entry.holdup.index, entry.phase.name, entry.phase.key));
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
		auto [holdup, unit, message] = TryGetHoldupPtr(entry.unit, entry.holdup, EScriptKeys::HOLDUP_DISTRIBUTION);
		if (!holdup)
			return PrintAndReturn(message);
		// read required values for ease of use
		const auto distr = static_cast<EDistrTypes>(entry.distrType.key);					// distribution type
		const auto fun = static_cast<EDistrFunction>(entry.function.key);					// distribution function type
		const auto mean = static_cast<EPSDGridType>(entry.psdMeans.key);					// mean values type for PSD
		const auto psd = static_cast<EPSDTypes>(entry.psdType.key);							// PSD type
		const bool manual = fun == EDistrFunction::Manual;									// whether manual distribution defined
		const size_t len = entry.values.size();												// length of the values vector
		const auto grid = holdup->GetGrid();												// distributions grid
		if (!grid.HasDimension(distr))
			return PrintAndReturn(DyssolC_ErrorNoDistribution(StrKey(EScriptKeys::HOLDUP_DISTRIBUTION), unit->GetName(), entry.holdup.name, entry.holdup.index, entry.distrType.name, entry.distrType.key));
		const size_t classes = grid.GetGridDimension(distr)->ClassesNumber();				// number of classes in the distribution
		const auto means = distr != DISTR_SIZE ? grid.GetGridDimensionNumeric(distr)->GetClassesMeans() : grid.GetPSDMeans(mean); // mean valued for the PSD grid
		const bool hasTime = manual && len % (classes + 1) == 0 || !manual && len % 3 == 0;	// whether time is defined
		const bool mix = entry.compound == "MIXTURE";										// whether the distribution is defined for the total mixture of for a single compound
		// check the number of passed arguments
		if (manual && len != classes && len % (classes + 1) != 0 || !manual && len != 2 && len % 3 != 0)
			return PrintAndReturn(DyssolC_ErrorArgumentsNumber(StrKey(EScriptKeys::HOLDUP_DISTRIBUTION), unit->GetName(), entry.holdup.name, entry.holdup.index));
		// get and check compound key
		const auto* compound = m_materialsDatabase.GetCompound(entry.compound) ? m_materialsDatabase.GetCompound(entry.compound) : m_materialsDatabase.GetCompoundByName(entry.compound);
		const std::string key = compound ? compound->GetKey() : "";
		if (!mix && key.empty())
			return PrintAndReturn(DyssolC_ErrorNoCompound(StrKey(EScriptKeys::HOLDUP_DISTRIBUTION), unit->GetName(), entry.holdup.name, entry.holdup.index, key));
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
	// TODO: put into calling function
	if (_job.HasKey(EScriptKeys::EXPORT_ONLY) && _job.GetValue<bool>(EScriptKeys::EXPORT_ONLY)) return true;

	// initialize flowsheet
	std::cout << DyssolC_Initialize() << std::endl;
	const std::string error = m_flowsheet.Initialize();
	if (!error.empty())
		return PrintAndReturn(DyssolC_ErrorInit(error));

	// run simulation
	m_simulator.SetFlowsheet(&m_flowsheet);
	std::cout << DyssolC_Start() << std::endl;
	const auto tStart = ch::steady_clock::now();
	m_simulator.Simulate();
	const auto tEnd = ch::steady_clock::now();

	// save simulation results
	const auto dstFile = _job.HasKey(EScriptKeys::RESULT_FILE) ? _job.GetValue<fs::path>(EScriptKeys::RESULT_FILE) : _job.GetValue<fs::path>(EScriptKeys::SOURCE_FILE);
	std::cout << DyssolC_SaveFlowsheet(dstFile.string()) << std::endl;
	CH5Handler fileHandler;
	m_flowsheet.SaveToFile(fileHandler, dstFile);

	std::cout << DyssolC_SimFinished(ch::duration_cast<ch::seconds>(tEnd - tStart).count()) << std::endl;

	return true;
}

// TODO: split
bool CScriptRunner::ExportResults(const CScriptJob& _job)
{
	if (!_job.HasKey(EScriptKeys::EXPORT_FILE)) return true;

	const auto exportFile = _job.GetValue<fs::path>(EScriptKeys::EXPORT_FILE);
	std::cout << DyssolC_ExportResults(exportFile.string()) << std::endl;

	// open text file for export
	std::ofstream file(exportFile);
	if (!file)
		return PrintAndReturn(DyssolC_ErrorExportFile());

	// setup export
	if (_job.HasKey(EScriptKeys::EXPORT_PRECISION))
		file.precision(_job.GetValue<int64_t>(EScriptKeys::EXPORT_PRECISION));
	if (_job.HasKey(EScriptKeys::EXPORT_FIXED_POINT))
		file.setf(_job.GetValue<bool>(EScriptKeys::EXPORT_FIXED_POINT) ? std::ios::fixed : std::ios::scientific);
	const double limit = _job.HasKey(EScriptKeys::EXPORT_SIGNIFICANCE_LIMIT) ? std::abs(_job.GetValue<double>(EScriptKeys::EXPORT_SIGNIFICANCE_LIMIT)) : 0.0;
	// replaces all values less than the limit with zeros
	const auto Filter = [&](double v) { return limit == 0.0 ? v : std::abs(v) >= limit ? v : 0.0; };

	// helper function to export values
	const auto ExportValues = [&](const std::vector<double>& tp, const CBaseStream* s, const std::function<void(const CBaseStream*, double)>& fun)
	{
		for (const double t : !tp.empty() ? tp : s->GetAllTimePoints())
		{
			file << " " << t;
			fun(s, t);
		}
		file << std::endl;
	};

	// flag to return
	bool success{ true };

	// helper function to export streams data
	const auto ExportStreams = [&](const EScriptKeys& key, const std::string& tag, const std::function<void(const CBaseStream*, double)>& fun)
	{
		for (const auto& e : _job.GetValues<SExportStreamSE>(key))
		{
			// get pointer to stream
			const CStream* stream = GetStreamPtr(e.stream);
			if (!stream)
			{
				success = PrintAndReturn(DyssolC_ErrorNoStream(StrKey(key), e.stream.name, e.stream.index));
				continue;
			}
			// export
			file << tag << " " << StringFunctions::Quote(stream->GetName());
			ExportValues(e.times, stream, fun);
		}
	};

	// helper function to export holdups data
	const auto ExportHoldups = [&](const EScriptKeys& key, const std::string& tag, const std::function<void(const CBaseStream*, double)>& fun)
	{
		for (const auto& e : _job.GetValues<SExportHoldupSE>(key))
		{
			// get pointer to unit and model
			auto [model, unit, message] = TryGetModelPtr(e.unit, key);
			if (!model)
			{
				success = PrintAndReturn(DyssolC_ErrorNoUnit(StrKey(key), e.unit.name, e.unit.index));
				continue;
			}
			// get pointer to holdup
			const auto* holdup = GetHoldupPtr(*model, e.holdup);
			if (!holdup)
			{
				success = PrintAndReturn(DyssolC_ErrorNoHoldup(StrKey(key), unit->GetName(), e.holdup.name, e.holdup.index));
				continue;
			}
			// export
			file << tag << " " << StringFunctions::Quote(unit->GetName()) << " " << StringFunctions::Quote(holdup->GetName());
			ExportValues(e.times, holdup, fun);
		}
	};

	// helper functions to export specific data
	const auto ExportMass = [&](const CBaseStream* s, double t)
	{
		file << " " << Filter(s->GetMass(t));
	};
	const auto ExportTemperature = [&](const CBaseStream* s, double t)
	{
		file << " " << Filter(s->GetTemperature(t));
	};
	const auto ExportPressure = [&](const CBaseStream* s, double t)
	{
		file << " " << Filter(s->GetPressure(t));
	};
	const auto ExportOveralls = [&](const CBaseStream* s, double t)
	{
		for (const auto& o : m_flowsheet.GetOveralProperties())
			file << " " << Filter(s->GetOverallProperty(t, o.type));
	};
	const auto ExportPhases = [&](const CBaseStream* s, double t)
	{
		for (const auto& p : m_flowsheet.GetPhases())
			file << " " << Filter(s->GetPhaseFraction(t, p.state));
	};
	const auto ExportCompounds = [&](const CBaseStream* s, double t)
	{
		for (const auto& c : m_flowsheet.GetCompounds())
			file << " " << Filter(s->GetCompoundFraction(t, c));
	};
	const auto ExportPSD = [&](const CBaseStream* s, double t)
	{
		for (const double v : s->GetPSD(t, PSD_MassFrac))
			file << " " << Filter(v);
	};
	const auto ExportDistributions = [&](const CBaseStream* s, double t)
	{
		for (const auto& d : s->GetGrid().GetDimensionsTypes())
			for (const double v : s->GetDistribution(t, d))
				file << " " << Filter(v);
	};

	// export streams' data
	ExportStreams(EScriptKeys::EXPORT_STREAM_MASS               , "STREAM_MASS"         , ExportMass);
	ExportStreams(EScriptKeys::EXPORT_STREAM_TEMPERATURE        , "STREAM_TEMPERATURE"  , ExportTemperature);
	ExportStreams(EScriptKeys::EXPORT_STREAM_PRESSURE           , "STREAM_PRESSURE"     , ExportPressure);
	ExportStreams(EScriptKeys::EXPORT_STREAM_OVERALLS           , "STREAM_OVERALLS"     , ExportOveralls);
	ExportStreams(EScriptKeys::EXPORT_STREAM_PHASES_FRACTIONS   , "STREAM_PHASES"       , ExportPhases);
	ExportStreams(EScriptKeys::EXPORT_STREAM_COMPOUNDS_FRACTIONS, "STREAM_COMPOUNDS"    , ExportCompounds);
	ExportStreams(EScriptKeys::EXPORT_STREAM_PSD                , "STREAM_PSD"          , ExportPSD);
	ExportStreams(EScriptKeys::EXPORT_STREAM_DISTRIBUTIONS      , "STREAM_DISTRIBUTIONS", ExportDistributions);

	// export holdups' data
	ExportHoldups(EScriptKeys::EXPORT_HOLDUP_MASS               , "HOLDUP_MASS"         , ExportMass);
	ExportHoldups(EScriptKeys::EXPORT_HOLDUP_TEMPERATURE        , "HOLDUP_TEMPERATURE"  , ExportTemperature);
	ExportHoldups(EScriptKeys::EXPORT_HOLDUP_PRESSURE           , "HOLDUP_PRESSURE"     , ExportPressure);
	ExportHoldups(EScriptKeys::EXPORT_HOLDUP_OVERALLS           , "HOLDUP_OVERALLS"     , ExportOveralls);
	ExportHoldups(EScriptKeys::EXPORT_HOLDUP_PHASES_FRACTIONS   , "HOLDUP_PHASES"       , ExportPhases);
	ExportHoldups(EScriptKeys::EXPORT_HOLDUP_COMPOUNDS_FRACTIONS, "HOLDUP_COMPOUNDS"    , ExportCompounds);
	ExportHoldups(EScriptKeys::EXPORT_HOLDUP_PSD                , "HOLDUP_PSD"          , ExportPSD);
	ExportHoldups(EScriptKeys::EXPORT_HOLDUP_DISTRIBUTIONS      , "HOLDUP_DISTRIBUTIONS", ExportDistributions);

	// export state variables
	for (const auto& e : _job.GetValues<SExportStateVarSE>(EScriptKeys::EXPORT_UNIT_STATE_VARIABLE))
	{
		// get pointer to unit and model
		auto [model, unit, message] = TryGetModelPtr(e.unit, EScriptKeys::EXPORT_UNIT_STATE_VARIABLE);
		if (!model)
		{
			success = PrintAndReturn(DyssolC_ErrorNoUnit(StrKey(EScriptKeys::EXPORT_UNIT_STATE_VARIABLE), e.unit.name, e.unit.index));
			continue;
		}
		// get pointer to holdup
		const auto* variable = GetStateVarPtr(*model, e.variable);
		if (!variable)
		{
			success = PrintAndReturn(DyssolC_ErrorNoStateVar(StrKey(EScriptKeys::EXPORT_UNIT_STATE_VARIABLE), unit->GetName(), e.variable.name, e.variable.index));
			continue;
		}
		// export
		file << "UNIT_STATE_VAR" << " " << StringFunctions::Quote(variable->GetName());
		if (!variable->HasHistory())
			file << " " << variable->GetValue();
		else
			for (const auto& v : variable->GetHistory())
				file << " " << v.time << " " << v.value;
		file << std::endl;
	}

	// export plots
	for (const auto& e : _job.GetValues<SExportPlotSE>(EScriptKeys::EXPORT_UNIT_PLOT))
	{
		// get pointer to unit and model
		auto [model, unit, message] = TryGetModelPtr(e.unit, EScriptKeys::EXPORT_UNIT_PLOT);
		if (!model)
		{
			success = PrintAndReturn(DyssolC_ErrorNoUnit(StrKey(EScriptKeys::EXPORT_UNIT_PLOT), e.unit.name, e.unit.index));
			continue;
		}
		// get pointer to plot
		const auto* plot = GetPlotPtr(*model, e.plot);
		if (!plot)
		{
			success = PrintAndReturn(DyssolC_ErrorNoPlot(StrKey(EScriptKeys::EXPORT_UNIT_PLOT), unit->GetName(), e.plot.name, e.plot.index));
			continue;
		}
		// get pointer to curve
		const auto* curve = GetCurvePtr(*plot, e.curve);
		if (!curve)
		{
			success = PrintAndReturn(DyssolC_ErrorNoCurve(StrKey(EScriptKeys::EXPORT_UNIT_PLOT), unit->GetName(), plot->GetName(), e.plot.name, e.plot.index));
			continue;
		}
		// export
		file << "UNIT_PLOT" << " " << StringFunctions::Quote(plot->GetName()) << " " << StringFunctions::Quote(curve->GetName());
		for (const auto& p : curve->GetPoints())
			file << " " << p.x << " " << p.y;
		file << std::endl;
	}

	// close file before exit
	file.close();

	// export to graph file
	if (_job.HasKey(EScriptKeys::EXPORT_FLOWSHEET_GRAPH))
	{
		const auto graphFile = _job.GetValue<fs::path>(EScriptKeys::EXPORT_FLOWSHEET_GRAPH);
		std::cout << DyssolC_ExportGraph(graphFile.string()) << std::endl;

		// open text file for export
		std::ofstream graph(graphFile);
		if (!graph)
			return PrintAndReturn(DyssolC_ErrorGraphFile());

		// export graph
		graph << m_flowsheet.GenerateDOTFile();
		graph.close();
	}

	return success;
}

void CScriptRunner::Clear()
{
	m_flowsheet.Clear();
	m_modelsManager.Clear();
	m_materialsDatabase.Clear();
}

std::tuple<CBaseUnit*, CUnitContainer*, std::string> CScriptRunner::TryGetModelPtr(const SNameOrIndex& _unit, const EScriptKeys& _scriptKey)
{
	// get pointer to unit
	auto* unit = GetUnitPtr(_unit);
	if (!unit)		return std::make_tuple(nullptr, nullptr, DyssolC_ErrorNoUnit(StrKey(_scriptKey), _unit.name, _unit.index));
	// get pointer to unit's model
	auto* model = unit->GetModel();
	if (!model)		return std::make_tuple(nullptr, unit   , DyssolC_ErrorLoadModel(StrKey(_scriptKey), unit->GetName()));
	// return pointer to model and empty error message
	return std::make_tuple(model, unit, "");
}

CUnitContainer* CScriptRunner::GetUnitPtr(const SNameOrIndex& _nameOrIndex)
{
	auto* unit = m_flowsheet.GetUnitByName(_nameOrIndex.name);	// try to access by name
	if (!unit) unit = m_flowsheet.GetUnit(_nameOrIndex.index);	// try to access by index
	return unit;												// return pointer
}

CStream* CScriptRunner::GetStreamPtr(const SNameOrIndex& _nameOrIndex)
{
	auto* stream = m_flowsheet.GetStreamByName(_nameOrIndex.name);		// try to access by name
	if (!stream) stream = m_flowsheet.GetStream(_nameOrIndex.index);	// try to access by index
	return stream;														// return pointer
}

CBaseUnitParameter* CScriptRunner::GetUnitParamPtr(CBaseUnit& _model, const SNameOrIndex& _nameOrIndex)
{
	auto& manager = _model.GetUnitParametersManager();				// get manager
	auto* param = manager.GetParameter(_nameOrIndex.name);			// try to access by name
	if (!param) param = manager.GetParameter(_nameOrIndex.index);	// try to access by index
	return param;													// return pointer
}

CBaseStream* CScriptRunner::GetHoldupPtr(CBaseUnit& _model, const SNameOrIndex& _nameOrIndex)
{
	auto& manager = _model.GetStreamsManager();							// get manager
	auto* holdup = manager.GetObjectInit(_nameOrIndex.name);			// try to access by name
	if (!holdup) holdup = manager.GetObjectInit(_nameOrIndex.index);	// try to access by index
	return holdup;														// return pointer
}

CCompound* CScriptRunner::GetCompoundPtr(const std::string& _nameOrKey)
{
	auto* compound = m_materialsDatabase.GetCompound(_nameOrKey);		// try to access by key
	if (!compound)
		compound = m_materialsDatabase.GetCompoundByName(_nameOrKey);	// try to access by name
	return compound;
}

CUnitPort* CScriptRunner::GetPortPtr(CBaseUnit& _model, const SNameOrIndex& _nameOrIndex)
{
	auto& manager = _model.GetPortsManager();				// get manager
	auto* port = manager.GetPort(_nameOrIndex.name);		// try to access by name
	if (!port) port = manager.GetPort(_nameOrIndex.index);	// try to access by index
	return port;											// return pointer
}

CStateVariable* CScriptRunner::GetStateVarPtr(CBaseUnit& _model, const ScriptInterface::SNameOrIndex& _nameOrIndex)
{
	auto& manager = _model.GetStateVariablesManager();						// get manager
	auto* variable = manager.GetStateVariable(_nameOrIndex.name);			// try to access by name
	if (!variable) variable = manager.GetStateVariable(_nameOrIndex.index);	// try to access by index
	return variable;														// return pointer
}

const CPlot* CScriptRunner::GetPlotPtr(const CBaseUnit& _model, const ScriptInterface::SNameOrIndex& _nameOrIndex)
{
	const auto& manager = _model.GetPlotsManager();			// get manager
	const auto* plot = manager.GetPlot(_nameOrIndex.name);	// try to access by name
	if (!plot) plot = manager.GetPlot(_nameOrIndex.index);	// try to access by index
	return plot;											// return pointer
}

const CCurve* CScriptRunner::GetCurvePtr(const CPlot& _plot, const ScriptInterface::SNameOrIndex& _nameOrIndex)
{
	const auto* curve = _plot.GetCurve(_nameOrIndex.name);		// try to access by name
	if (!curve) curve = _plot.GetCurve(_nameOrIndex.index);		// try to access by index
	return curve;												// return pointer
}

std::string CScriptRunner::GetModelKey(const std::string& _value) const
{
	std::error_code ec;	// to use non-throwing version of fs::equivalent
	for (const auto& m : m_modelsManager.GetAvailableUnits())
	{
		if (m.uniqueID == _value ||									// try to access by ID
			m.name == _value ||										// try to access by name
			fs::equivalent(fs::path{ _value }, m.fileLocation, ec))	// try to access by file bath
			return m.uniqueID;
	}
	return {};
}
