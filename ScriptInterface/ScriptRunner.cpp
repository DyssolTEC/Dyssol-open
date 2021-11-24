/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ScriptRunner.h"
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

bool CScriptRunner::RunJob(const CScriptJob& _job)
{
	const auto tStart = ch::steady_clock::now();

	Clear();
	bool success = true;
	if (success) success &= CreateFlowsheet(_job);
	if (success) success &= RunSimulation(_job);
	if (success) success &= ExportResults(_job);

	const auto tEnd = ch::steady_clock::now();

	PrintMessage(DyssolC_ScriptFinished(ch::duration_cast<ch::seconds>(tEnd - tStart).count()));
	if (!success)
		PrintMessage(DyssolC_ErrorFinish());

	return success;
}

bool CScriptRunner::CreateFlowsheet(const CScriptJob& _job)
{
	bool success = true;

	if (success) success &= LoadFiles(_job);
	if (success) success &= SetupFlowsheet(_job);

	return success;
}

bool CScriptRunner::LoadFiles(const CScriptJob& _job)
{
	// check that source and/or result files are defined
	const bool hasSrc     = _job.HasKey(EScriptKeys::SOURCE_FILE);
	const bool hasDst     = _job.HasKey(EScriptKeys::RESULT_FILE);
	const bool onlyExport = _job.HasKey(EScriptKeys::EXPORT_ONLY) && _job.GetValue<bool>(EScriptKeys::EXPORT_ONLY);
	if (!hasSrc && !hasDst)
		return PrintMessage(DyssolC_ErrorSrcDst(StrKey(EScriptKeys::SOURCE_FILE), StrKey(EScriptKeys::RESULT_FILE)));
	if (hasSrc && !hasDst && !onlyExport)
		PrintMessage(DyssolC_WriteSrc(StrKey(EScriptKeys::SOURCE_FILE), StrKey(EScriptKeys::RESULT_FILE)));

	// load materials database
	const auto MDBfile = _job.GetValue<fs::path>(EScriptKeys::MATERIALS_DATABASE);
	PrintMessage(DyssolC_LoadMDB(MDBfile.string()));
	if (!m_materialsDatabase.LoadFromFile(MDBfile))
		return PrintMessage(DyssolC_ErrorMDB());

	// set paths to models
	auto modelsPaths = _job.GetValues<fs::path>(EScriptKeys::MODELS_PATH);
	modelsPaths.insert(modelsPaths.begin(), fs::current_path()); // add current path
	for (const auto& dir : modelsPaths)
	{
		PrintMessage(DyssolC_LoadModels(dir.string()));
		m_modelsManager.AddDir(dir);
	}

	// load flowsheet
	if (hasSrc)
	{
		const auto srcFile = _job.GetValue<fs::path>(EScriptKeys::SOURCE_FILE);
		PrintMessage(DyssolC_LoadFlowsheet(srcFile.string()));
		CH5Handler fileHandler;
		if (!m_flowsheet.LoadFromFile(fileHandler, srcFile))
			return PrintMessage(DyssolC_ErrorLoad());
	}

	return true;
}

bool CScriptRunner::SetupFlowsheet(const CScriptJob& _job)
{
	if (_job.HasKey(EScriptKeys::EXPORT_ONLY) && _job.GetValue<bool>(EScriptKeys::EXPORT_ONLY)) return true;

	bool success = true;

	if (success) success &= SetupFlowsheetParameters(_job);
	if (success) success &= SetupGrids(_job);
	if (success) success &= SetupCompounds(_job);
	if (success) success &= SetupPhases(_job);
	if (success) success &= SetupUnits(_job);
	if (success) success &= SetupStreams(_job);
	if (success) success &= SetupUnitParameters(_job);
	if (success) success &= SetupHoldups(_job);

	return success;
}

bool CScriptRunner::SetupUnits(const CScriptJob& _job)
{
	// remove existing units
	if (_job.HasKey(EScriptKeys::KEEP_EXISTING_UNITS) && !_job.GetValue<bool>(EScriptKeys::KEEP_EXISTING_UNITS))
		for (const auto& u : m_flowsheet.GetAllUnits())
			m_flowsheet.DeleteUnit(u->GetKey());

	// add or set new units
	for (const auto& entry : _job.GetValues<std::vector<std::string>>(EScriptKeys::UNIT))
	{
		// check input
		if (entry.size() != 2)
			return PrintMessage(DyssolC_ErrorArgumentsNumberUnit(StrKey(EScriptKeys::UNIT)));
		// find model key
		const auto key = TryGetModelKey(EScriptKeys::UNIT, entry[1]);
		if (key.empty()) return false;
		// whether a unit already exists
		const bool exists = m_flowsheet.GetUnitByName(entry[0]);
		// pointer to unit to work with: existing or a new one
		CUnitContainer* unit = exists ? m_flowsheet.GetUnitByName(entry[0]) : m_flowsheet.AddUnit();
		// set data
		unit->SetName(entry[0]);
		unit->SetModel(key);
	}

	return true;
}

bool CScriptRunner::SetupStreams(const CScriptJob& _job)
{
	// remove existing streams
	if (_job.HasKey(EScriptKeys::KEEP_EXISTING_STREAMS) && !_job.GetValue<bool>(EScriptKeys::KEEP_EXISTING_STREAMS))
		for (const auto& s : m_flowsheet.GetAllStreams())
			m_flowsheet.DeleteStream(s->GetKey());

	for (const auto& entry : _job.GetValues<SStreamSE>(EScriptKeys::STREAM))
	{
		auto* portO = TryGetPortPtr(EScriptKeys::STREAM, entry.unitO, entry.portO);
		if (!portO) return false;
		auto* portI = TryGetPortPtr(EScriptKeys::STREAM, entry.unitI, entry.portI);
		if (!portI) return false;
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

	return true;
}

bool CScriptRunner::SetupCompounds(const CScriptJob& _job)
{
	if (!_job.HasKey(EScriptKeys::COMPOUNDS)) return true;

	std::vector<std::string> keys;
	for (const auto& entry : _job.GetValues<std::vector<std::string>>(EScriptKeys::COMPOUNDS))
		for (const auto& key : entry)
		{
			const auto* compound = TryGetCompoundPtr(EScriptKeys::COMPOUNDS, key);
			if (!compound) return false;
			keys.push_back(compound->GetKey());
		}
	m_flowsheet.SetCompounds(keys);

	return true;
}

bool CScriptRunner::SetupPhases(const CScriptJob& _job)
{
	if (!_job.HasKey(EScriptKeys::PHASES)) return true;

	std::vector<SPhaseDescriptor> phases;
	for (const auto& entry : _job.GetValues<SPhasesSE>(EScriptKeys::PHASES))
		for (size_t i = 0; i < entry.types.size(); ++i)
			phases.push_back(SPhaseDescriptor{ static_cast<EPhase>(entry.types[i].key), entry.names[i] });
	m_flowsheet.SetPhases(phases);

	return true;
}

bool CScriptRunner::SetupGrids(const CScriptJob& _job)
{
	// The grids may be cleaned before setting new values. Those grids, which are not mentioned in the script file, are not changed.
	// If cleaning is requested, on the first access to the grid's holder, clean it, store the key of the holder in this vector and do not clean any further.
	std::vector<std::string> processed;	// already processed grid's holders
	const bool keep = !_job.HasKey(EScriptKeys::KEEP_EXISTING_GRIDS_VALUES) || _job.GetValue<bool>(EScriptKeys::KEEP_EXISTING_GRIDS_VALUES);	// keep or remove values in grids before setting new ones

	// setup distribution grids
	for (const auto& entry : _job.GetValues<SGridDimensionSE>(EScriptKeys::DISTRIBUTION_GRID))
	{
		// type of the grid
		const bool mainGrid = entry.unit.name == "GLOBAL";
		// get all pointers and values
		const auto [model, unit] = !mainGrid ? TryGetUnitAndModelPtr(EScriptKeys::DISTRIBUTION_GRID, entry.unit) : std::make_tuple(nullptr, nullptr);
		if (!mainGrid && !model) return false;
		CMultidimensionalGrid grid = mainGrid ? m_flowsheet.GetGrid() : model->GetGrid();
		// clean the grid if requested and if it is the first access to this grid
		if (const std::string key = mainGrid ? "GLOBAL" : unit->GetKey(); !keep && !VectorContains(processed, key))
		{
			// remove all except compounds
			for (const auto& t : grid.GetDimensionsTypes())
				if (t != DISTR_COMPOUNDS)
					grid.RemoveDimension(t);
			// remember
			processed.push_back(key);
		}
		// get some values for the ease of use
		const auto& type      = static_cast<EDistrTypes>(entry.distrType.key);
		const auto& entryType = static_cast<EGridEntry>(entry.entryType.key);
		const auto& function  = static_cast<EGridFunction>(entry.function.key);
		// remove grid dimension if it already exists
		if (grid.HasDimension(type))
			grid.RemoveDimension(type);
		// set new grid dimension
		if (entryType == EGridEntry::GRID_NUMERIC)
		{
			// check number of arguments
			if (function != EGridFunction::GRID_FUN_MANUAL && entry.valuesNum.size() != 2 || function == EGridFunction::GRID_FUN_MANUAL && entry.valuesNum.size() != entry.classes + 1)
				return PrintMessage(DyssolC_ErrorArgumentsNumberGrid(StrKey(EScriptKeys::DISTRIBUTION_GRID), unit->GetName(), entry.distrType.name, entry.distrType.key));
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
				return PrintMessage(DyssolC_ErrorArgumentsNumberGrid(StrKey(EScriptKeys::DISTRIBUTION_GRID), unit->GetName(), entry.distrType.name, entry.distrType.key));
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

bool CScriptRunner::SetupFlowsheetParameters(const CScriptJob& _job)
{
	auto* params = m_flowsheet.GetParameters();

	if (_job.HasKey(EScriptKeys::SIMULATION_TIME))              params->EndSimulationTime                                    (_job.GetValue<double  >  (EScriptKeys::SIMULATION_TIME              ));
	if (_job.HasKey(EScriptKeys::RELATIVE_TOLERANCE))           params->RelTol                                               (_job.GetValue<double  >  (EScriptKeys::RELATIVE_TOLERANCE           ));
	if (_job.HasKey(EScriptKeys::ABSOLUTE_TOLERANCE))           params->AbsTol                                               (_job.GetValue<double  >  (EScriptKeys::ABSOLUTE_TOLERANCE           ));
	if (_job.HasKey(EScriptKeys::MINIMAL_FRACTION))             params->MinFraction                                          (_job.GetValue<double  >  (EScriptKeys::MINIMAL_FRACTION             ));
	if (_job.HasKey(EScriptKeys::INIT_TIME_WINDOW))             params->InitTimeWindow                                       (_job.GetValue<double  >  (EScriptKeys::INIT_TIME_WINDOW             ));
	if (_job.HasKey(EScriptKeys::SAVE_TIME_STEP_HINT))          params->SaveTimeStep                                         (_job.GetValue<double  >  (EScriptKeys::SAVE_TIME_STEP_HINT          ));
	if (_job.HasKey(EScriptKeys::SAVE_FLAG_FOR_HOLDUPS))        params->SaveTimeStepFlagHoldups                              (_job.GetValue<bool    >  (EScriptKeys::SAVE_FLAG_FOR_HOLDUPS        ));
	if (_job.HasKey(EScriptKeys::THERMO_TEMPERATURE_MIN))       params->EnthalpyMinT                                         (_job.GetValue<double  >  (EScriptKeys::THERMO_TEMPERATURE_MIN       ));
	if (_job.HasKey(EScriptKeys::THERMO_TEMPERATURE_MAX))       params->EnthalpyMaxT                                         (_job.GetValue<double  >  (EScriptKeys::THERMO_TEMPERATURE_MAX       ));
	if (_job.HasKey(EScriptKeys::MIN_TIME_WINDOW))              params->MinTimeWindow                                        (_job.GetValue<double  >  (EScriptKeys::MIN_TIME_WINDOW              ));
	if (_job.HasKey(EScriptKeys::MAX_TIME_WINDOW))              params->MaxTimeWindow                                        (_job.GetValue<double  >  (EScriptKeys::MAX_TIME_WINDOW              ));
	if (_job.HasKey(EScriptKeys::WINDOW_CHANGE_RATE))           params->MagnificationRatio                                   (_job.GetValue<double  >  (EScriptKeys::WINDOW_CHANGE_RATE           ));
	if (_job.HasKey(EScriptKeys::RELAXATION_PARAMETER))         params->RelaxationParam                                      (_job.GetValue<double  >  (EScriptKeys::RELAXATION_PARAMETER         ));
	if (_job.HasKey(EScriptKeys::ACCELERATION_LIMIT))           params->WegsteinAccelParam                                   (_job.GetValue<double  >  (EScriptKeys::ACCELERATION_LIMIT           ));
	if (_job.HasKey(EScriptKeys::THERMO_TEMPERATURE_INTERVALS)) params->EnthalpyInt        (static_cast<uint32_t>            (_job.GetValue<uint64_t>  (EScriptKeys::THERMO_TEMPERATURE_INTERVALS)));
	if (_job.HasKey(EScriptKeys::MAX_ITERATIONS_NUMBER))        params->MaxItersNumber     (static_cast<uint32_t>            (_job.GetValue<uint64_t>  (EScriptKeys::MAX_ITERATIONS_NUMBER)       ));
	if (_job.HasKey(EScriptKeys::ITERATIONS_UPPER_LIMIT))       params->ItersUpperLimit    (static_cast<uint32_t>            (_job.GetValue<uint64_t>  (EScriptKeys::ITERATIONS_UPPER_LIMIT)      ));
	if (_job.HasKey(EScriptKeys::ITERATIONS_LOWER_LIMIT))       params->ItersLowerLimit    (static_cast<uint32_t>            (_job.GetValue<uint64_t>  (EScriptKeys::ITERATIONS_LOWER_LIMIT)      ));
	if (_job.HasKey(EScriptKeys::ITERATIONS_UPPER_LIMIT_1ST))   params->Iters1stUpperLimit (static_cast<uint32_t>            (_job.GetValue<uint64_t>  (EScriptKeys::ITERATIONS_UPPER_LIMIT_1ST)  ));
	if (_job.HasKey(EScriptKeys::CONVERGENCE_METHOD))           params->ConvergenceMethod  (static_cast<EConvergenceMethod>  (_job.GetValue<SNamedEnum>(EScriptKeys::CONVERGENCE_METHOD).key      ));
	if (_job.HasKey(EScriptKeys::EXTRAPOLATION_METHOD))         params->ExtrapolationMethod(static_cast<EExtrapolationMethod>(_job.GetValue<SNamedEnum>(EScriptKeys::EXTRAPOLATION_METHOD).key    ));

	m_flowsheet.UpdateToleranceSettings();
	m_flowsheet.UpdateThermodynamicsSettings();

	return true;
}

bool CScriptRunner::SetupUnitParameters(const CScriptJob& _job)
{
	for (const auto& entry : _job.GetValues<SUnitParameterSE>(EScriptKeys::UNIT_PARAMETER))
	{
		// get pointer to unit parameter
		auto* param = TryGetUnitParamPtr(EScriptKeys::UNIT_PARAMETER, entry.unit, entry.param);
		if (!param) return false;
		std::stringstream ss{ entry.values };	// create a stream with parameter values
		param->ValueFromStream(ss);				// read unit parameter values
	}

	return true;
}

bool CScriptRunner::SetupHoldups(const CScriptJob& _job)
{
	// The holdup may be cleaned before setting time-dependent values. Those holdups, which are not mentioned in the script file, are not changed.
	// If cleaning is requested, on the first access to the holdup, clean it, store in this vector and do not clean any further.
	std::vector<CBaseStream*> processed;	// already processed holdups
	const bool keep = !_job.HasKey(EScriptKeys::KEEP_EXISTING_HOLDUPS_VALUES) || _job.GetValue<bool>(EScriptKeys::KEEP_EXISTING_HOLDUPS_VALUES);	// keep or remove time points

	// Cleans holdup if required.
	const auto CleanUp = [&](CBaseStream* s)
	{
		if (s && !keep && !VectorContains(processed, s))
		{
			s->RemoveAllTimePoints();
			processed.push_back(s);
		}
	};

	// remove all time points if requested for required holdups
	for (const auto& entry : _job.GetValues<SHoldupDependentSE>(EScriptKeys::HOLDUP_OVERALL))
		CleanUp(GetHoldupInitPtr(GetModelPtr(GetUnitPtr(entry.unit)), entry.holdup));
	for (const auto& entry : _job.GetValues<SHoldupDependentSE>(EScriptKeys::HOLDUP_PHASES))
		CleanUp(GetHoldupInitPtr(GetModelPtr(GetUnitPtr(entry.unit)), entry.holdup));
	for (const auto& entry : _job.GetValues<SHoldupCompoundsSE>(EScriptKeys::HOLDUP_COMPOUNDS))
		CleanUp(GetHoldupInitPtr(GetModelPtr(GetUnitPtr(entry.unit)), entry.holdup));
	for (const auto& entry : _job.GetValues<SHoldupDistributionSE>(EScriptKeys::HOLDUP_DISTRIBUTION))
		CleanUp(GetHoldupInitPtr(GetModelPtr(GetUnitPtr(entry.unit)), entry.holdup));

	bool success = true;

	if (success) success &= SetupHoldupsOverall(_job);
	if (success) success &= SetupHoldupsPhases(_job);
	if (success) success &= SetupHoldupsCompounds(_job);
	if (success) success &= SetupHoldupsDistributions(_job);

	return success;
}

bool CScriptRunner::SetupHoldupsOverall(const CScriptJob& _job)
{
	for (const auto& entry : _job.GetValues<SHoldupDependentSE>(EScriptKeys::HOLDUP_OVERALL))
	{
		// get pointer to holdup
		auto [holdup, unit] = TryGetHoldupInitPtr(EScriptKeys::HOLDUP_OVERALL, entry.unit, entry.holdup);
		if (!holdup) return false;
		// check the number of passed arguments
		if (entry.values.size() != m_flowsheet.GetOverallPropertiesNumber() && entry.values.size() % (m_flowsheet.GetOverallPropertiesNumber() + 1) != 0)
			return PrintMessage(DyssolC_ErrorArgumentsNumber(StrKey(EScriptKeys::HOLDUP_OVERALL), unit->GetName(), entry.holdup.name, entry.holdup.index));
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

	return true;
}

bool CScriptRunner::SetupHoldupsPhases(const CScriptJob& _job)
{
	for (const auto& entry : _job.GetValues<SHoldupDependentSE>(EScriptKeys::HOLDUP_PHASES))
	{
		// get pointer to holdup
		auto [holdup, unit] = TryGetHoldupInitPtr(EScriptKeys::HOLDUP_PHASES, entry.unit, entry.holdup);
		if (!holdup) return false;
		// check the number of passed arguments
		if (entry.values.size() != m_flowsheet.GetPhasesNumber() && entry.values.size() % (m_flowsheet.GetPhasesNumber() + 1) != 0)
			return PrintMessage(DyssolC_ErrorArgumentsNumber(StrKey(EScriptKeys::HOLDUP_PHASES), unit->GetName(), entry.holdup.name, entry.holdup.index));
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

	return true;
}

bool CScriptRunner::SetupHoldupsCompounds(const CScriptJob& _job)
{
	for (const auto& entry : _job.GetValues<SHoldupCompoundsSE>(EScriptKeys::HOLDUP_COMPOUNDS))
	{
		// get pointer to holdup
		auto [holdup, unit] = TryGetHoldupInitPtr(EScriptKeys::HOLDUP_COMPOUNDS, entry.unit, entry.holdup);
		if (!holdup) return false;
		// check the number of passed arguments
		if (entry.values.size() != m_flowsheet.GetCompoundsNumber() && entry.values.size() % (m_flowsheet.GetCompoundsNumber() + 1) != 0)
			return PrintMessage(DyssolC_ErrorArgumentsNumber(StrKey(EScriptKeys::HOLDUP_COMPOUNDS), unit->GetName(), entry.holdup.name, entry.holdup.index, entry.phase.name, entry.phase.key));
		// check that all phases are defined
		if (!m_flowsheet.HasPhase(static_cast<EPhase>(entry.phase.key)))
			return PrintMessage(DyssolC_ErrorNoPhase(StrKey(EScriptKeys::HOLDUP_COMPOUNDS), unit->GetName(), entry.holdup.name, entry.holdup.index, entry.phase.name, entry.phase.key));
		// set values: only values for time point 0 without time are given
		if (entry.values.size() == m_flowsheet.GetCompoundsNumber())
			holdup->SetCompoundsFractions(0.0, static_cast<EPhase>(entry.phase.key), entry.values);
		// set values: values with time points are given
		else
			for (size_t iTime = 0; iTime < entry.values.size(); iTime += m_flowsheet.GetCompoundsNumber() + 1)
				holdup->SetCompoundsFractions(entry.values[iTime], static_cast<EPhase>(entry.phase.key), std::vector<double>{ entry.values.begin() + iTime + 1, entry.values.begin() + iTime + 1 + m_flowsheet.GetCompoundsNumber() });
	}

	return true;
}

bool CScriptRunner::SetupHoldupsDistributions(const CScriptJob& _job)
{
	for (const auto& entry : _job.GetValues<SHoldupDistributionSE>(EScriptKeys::HOLDUP_DISTRIBUTION))
	{
		// get pointer to holdup
		auto [holdup, unit] = TryGetHoldupInitPtr(EScriptKeys::HOLDUP_DISTRIBUTION, entry.unit, entry.holdup);
		if (!holdup) return false;
		// read required values for ease of use
		const auto distr = static_cast<EDistrTypes>(entry.distrType.key);					// distribution type
		const auto fun = static_cast<EDistrFunction>(entry.function.key);					// distribution function type
		const auto mean = static_cast<EPSDGridType>(entry.psdMeans.key);					// mean values type for PSD
		const auto psd = static_cast<EPSDTypes>(entry.psdType.key);							// PSD type
		const bool manual = fun == EDistrFunction::Manual;									// whether manual distribution defined
		const size_t len = entry.values.size();												// length of the values vector
		const auto grid = holdup->GetGrid();												// distributions grid
		if (!grid.HasDimension(distr))
			return PrintMessage(DyssolC_ErrorNoDistribution(StrKey(EScriptKeys::HOLDUP_DISTRIBUTION), unit->GetName(), entry.holdup.name, entry.holdup.index, entry.distrType.name, entry.distrType.key));
		const size_t classes = grid.GetGridDimension(distr)->ClassesNumber();				// number of classes in the distribution
		const auto means = distr != DISTR_SIZE ? grid.GetGridDimensionNumeric(distr)->GetClassesMeans() : grid.GetPSDMeans(mean); // mean valued for the PSD grid
		const bool hasTime = manual && len % (classes + 1) == 0 || !manual && len % 3 == 0;	// whether time is defined
		const bool mix = entry.compound == "MIXTURE";										// whether the distribution is defined for the total mixture of for a single compound
		// check the number of passed arguments
		if (manual && len != classes && len % (classes + 1) != 0 || !manual && len != 2 && len % 3 != 0)
			return PrintMessage(DyssolC_ErrorArgumentsNumber(StrKey(EScriptKeys::HOLDUP_DISTRIBUTION), unit->GetName(), entry.holdup.name, entry.holdup.index));
		// get and check compound key
		const auto* compound = m_materialsDatabase.GetCompound(entry.compound) ? m_materialsDatabase.GetCompound(entry.compound) : m_materialsDatabase.GetCompoundByName(entry.compound);
		const std::string key = compound ? compound->GetKey() : "";
		if (!mix && key.empty())
			return PrintMessage(DyssolC_ErrorNoCompound(StrKey(EScriptKeys::HOLDUP_DISTRIBUTION), unit->GetName(), entry.holdup.name, entry.holdup.index, key));
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
	if (_job.HasKey(EScriptKeys::EXPORT_ONLY) && _job.GetValue<bool>(EScriptKeys::EXPORT_ONLY)) return true;

	// initialize flowsheet
	PrintMessage(DyssolC_Initialize());
	const std::string error = m_flowsheet.Initialize();
	if (!error.empty())
		return PrintMessage(DyssolC_ErrorInit(error));

	// run simulation
	m_simulator.SetFlowsheet(&m_flowsheet);
	PrintMessage(DyssolC_Start());
	const auto tStart = ch::steady_clock::now();
	m_simulator.Simulate();
	const auto tEnd = ch::steady_clock::now();

	// save simulation results
	const auto dstFile = _job.HasKey(EScriptKeys::RESULT_FILE) ? _job.GetValue<fs::path>(EScriptKeys::RESULT_FILE) : _job.GetValue<fs::path>(EScriptKeys::SOURCE_FILE);
	PrintMessage(DyssolC_SaveFlowsheet(dstFile.string()));
	CH5Handler fileHandler;
	m_flowsheet.SaveToFile(fileHandler, dstFile);

	PrintMessage(DyssolC_SimFinished(ch::duration_cast<ch::seconds>(tEnd - tStart).count()));

	return true;
}

// TODO: split
bool CScriptRunner::ExportResults(const CScriptJob& _job)
{
	if (!_job.HasKey(EScriptKeys::EXPORT_FILE)) return true;

	const auto exportFile = _job.GetValue<fs::path>(EScriptKeys::EXPORT_FILE);
	PrintMessage(DyssolC_ExportResults(exportFile.string()));

	// open text file for export
	std::ofstream file(exportFile);
	if (!file)
		return PrintMessage(DyssolC_ErrorExportFile());

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
			const CBaseStream* stream = TryGetStreamPtr(key, e.stream);
			success &= stream != nullptr;
			if (!stream) continue;
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
			// get pointer to holdup
			auto [holdup, unit] = TryGetHoldupWorkPtr(key , e.unit, e.holdup);
			success &= holdup != nullptr;
			if (!holdup) continue;
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
		// get pointer to holdup
		const auto* variable = TryGetStateVarPtr(EScriptKeys::EXPORT_UNIT_STATE_VARIABLE , e.unit, e.variable);
		success &= variable != nullptr;
		if (!variable) continue;
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
		// get pointer to curve
		auto [plot, curve] = TryGetCurvePtr(EScriptKeys::EXPORT_UNIT_PLOT, e.unit, e.plot, e.curve);
		success &= curve != nullptr;
		if (!curve) continue;
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
		PrintMessage(DyssolC_ExportGraph(graphFile.string()));

		// open text file for export
		std::ofstream graph(graphFile);
		if (!graph)
			return PrintMessage(DyssolC_ErrorGraphFile());

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

CUnitContainer* CScriptRunner::TryGetUnitPtr(EScriptKeys _sk, const SNameOrIndex& _unit)
{
	auto* unit = GetUnitPtr(_unit);
	if (!unit) PrintMessage(DyssolC_ErrorNoUnit(StrKey(_sk), _unit.name, _unit.index));
	return unit;
}

CBaseUnit* CScriptRunner::TryGetModelPtr(EScriptKeys _sk, CUnitContainer* _unit)
{
	auto* model = GetModelPtr(_unit);
	if (!model && _unit) PrintMessage(DyssolC_ErrorLoadModel(StrKey(_sk), _unit->GetName()));
	return model;
}

std::tuple<CBaseUnit*, CUnitContainer*> CScriptRunner::TryGetUnitAndModelPtr(EScriptKeys _sk, const SNameOrIndex& _unit)
{
	auto* unit = TryGetUnitPtr(_sk, _unit);
	auto* model = TryGetModelPtr(_sk, unit);
	return std::make_tuple(model, unit);
}

CBaseStream* CScriptRunner::TryGetStreamPtr(EScriptKeys _sk, const SNameOrIndex& _stream)
{
	auto* stream = GetStreamPtr(_stream);
	if (!stream) PrintMessage(DyssolC_ErrorNoStream(StrKey(_sk), _stream.name, _stream.index));
	return stream;
}

CBaseUnitParameter* CScriptRunner::TryGetUnitParamPtr(EScriptKeys _sk, const SNameOrIndex& _unit, const SNameOrIndex& _param)
{
	auto [model, unit] = TryGetUnitAndModelPtr(_sk, _unit);
	auto* param = GetUnitParamPtr(model, _param);
	if (!param && model && unit) PrintMessage(DyssolC_ErrorNoUP(StrKey(_sk), unit->GetName(), _param.name, _param.index));
	return param;
}

std::tuple<CBaseStream*, CUnitContainer*> CScriptRunner::TryGetHoldupInitPtr(EScriptKeys _sk, const SNameOrIndex& _unit, const SNameOrIndex& _holdup)
{
	auto [model, unit] = TryGetUnitAndModelPtr(_sk, _unit);
	auto* holdup = GetHoldupInitPtr(model, _holdup);
	if (!holdup && model && unit) PrintMessage(DyssolC_ErrorNoHoldup(StrKey(_sk), unit->GetName(), _holdup.name, _holdup.index));
	return std::make_tuple(holdup, unit);
}

std::tuple<CBaseStream*, CUnitContainer*> CScriptRunner::TryGetHoldupWorkPtr(EScriptKeys _sk, const SNameOrIndex& _unit, const SNameOrIndex& _holdup)
{
	auto [model, unit] = TryGetUnitAndModelPtr(_sk, _unit);
	auto* holdup = GetHoldupWorkPtr(model, _holdup);
	if (!holdup && model && unit) PrintMessage(DyssolC_ErrorNoHoldup(StrKey(_sk), unit->GetName(), _holdup.name, _holdup.index));
	return std::make_tuple(holdup, unit);
}

CCompound* CScriptRunner::TryGetCompoundPtr(EScriptKeys _sk, const std::string& _compound)
{
	auto* compound = GetCompoundPtr(_compound);
	if (!compound) PrintMessage(DyssolC_ErrorNoCompounds(StrKey(_sk), _compound));
	return compound;
}

CUnitPort* CScriptRunner::TryGetPortPtr(EScriptKeys _sk, const SNameOrIndex& _unit, const SNameOrIndex& _port)
{
	auto [model, unit] = TryGetUnitAndModelPtr(_sk, _unit);
	auto* port = GetPortPtr(model, _port);
	if (!port && model && unit) PrintMessage(DyssolC_ErrorNoPort(StrKey(_sk), unit->GetName(), _port.name, _port.index));
	return port;
}

CStateVariable* CScriptRunner::TryGetStateVarPtr(EScriptKeys _sk, const SNameOrIndex& _unit, const SNameOrIndex& _var)
{
	auto [model, unit] = TryGetUnitAndModelPtr(_sk, _unit);
	auto* var = GetStateVarPtr(model, _var);
	if (!var && model && unit) PrintMessage(DyssolC_ErrorNoStateVar(StrKey(_sk), unit->GetName(), _var.name, _var.index));
	return var;
}

std::tuple<const CPlot*, const CCurve*> CScriptRunner::TryGetCurvePtr(EScriptKeys _sk, const SNameOrIndex& _unit, const SNameOrIndex& _plot, const SNameOrIndex& _curve)
{
	auto [model, unit] = TryGetUnitAndModelPtr(_sk, _unit);
	auto* plot = GetPlotPtr(model, _plot);
	if (!plot && model && unit) PrintMessage(DyssolC_ErrorNoPlot(StrKey(_sk), unit->GetName(), _plot.name, _plot.index));
	auto* curve = GetCurvePtr(plot, _curve);
	if (!curve && plot && model && unit) PrintMessage(DyssolC_ErrorNoCurve(StrKey(_sk), unit->GetName(), plot->GetName(), _plot.name, _plot.index));
	return std::make_tuple(plot, curve);
}

std::string CScriptRunner::TryGetModelKey(EScriptKeys _sk, const std::string& _value) const
{
	auto key = GetModelKey(_value);
	if (key.empty()) PrintMessage(DyssolC_ErrorNoModel(StrKey(_sk), _value));
	return key;
}

CUnitContainer* CScriptRunner::GetUnitPtr(const SNameOrIndex& _nameOrIndex)
{
	auto* unit = m_flowsheet.GetUnitByName(_nameOrIndex.name);	// try to access by name
	if (!unit) unit = m_flowsheet.GetUnit(_nameOrIndex.index);	// try to access by index
	return unit;												// return pointer
}

CBaseUnit* CScriptRunner::GetModelPtr(CUnitContainer* _unit)
{
	if (!_unit) return {};
	return _unit->GetModel();
}

CBaseStream* CScriptRunner::GetStreamPtr(const SNameOrIndex& _nameOrIndex)
{
	auto* stream = m_flowsheet.GetStreamByName(_nameOrIndex.name);		// try to access by name
	if (!stream) stream = m_flowsheet.GetStream(_nameOrIndex.index);	// try to access by index
	return stream;														// return pointer
}

CBaseUnitParameter* CScriptRunner::GetUnitParamPtr(CBaseUnit* _model, const SNameOrIndex& _nameOrIndex)
{
	if (!_model) return {};
	auto& manager = _model->GetUnitParametersManager();				// get manager
	auto* param = manager.GetParameter(_nameOrIndex.name);			// try to access by name
	if (!param) param = manager.GetParameter(_nameOrIndex.index);	// try to access by index
	return param;													// return pointer
}

CBaseStream* CScriptRunner::GetHoldupInitPtr(CBaseUnit* _model, const SNameOrIndex& _nameOrIndex)
{
	if (!_model) return {};
	auto& manager = _model->GetStreamsManager();						// get manager
	auto* holdup = manager.GetObjectInit(_nameOrIndex.name);			// try to access by name
	if (!holdup) holdup = manager.GetObjectInit(_nameOrIndex.index);	// try to access by index
	return holdup;														// return pointer
}

CBaseStream* CScriptRunner::GetHoldupWorkPtr(CBaseUnit* _model, const SNameOrIndex& _nameOrIndex)
{
	if (!_model) return {};
	auto& manager = _model->GetStreamsManager();						// get manager
	auto* holdup = manager.GetObjectWork(_nameOrIndex.name);			// try to access by name
	if (!holdup) holdup = manager.GetObjectWork(_nameOrIndex.index);	// try to access by index
	return holdup;														// return pointer
}

CCompound* CScriptRunner::GetCompoundPtr(const std::string& _nameOrKey)
{
	auto* compound = m_materialsDatabase.GetCompound(_nameOrKey);					// try to access by key
	if (!compound) compound = m_materialsDatabase.GetCompoundByName(_nameOrKey);	// try to access by name
	return compound;
}

CUnitPort* CScriptRunner::GetPortPtr(CBaseUnit* _model, const SNameOrIndex& _nameOrIndex)
{
	if (!_model) return {};
	auto& manager = _model->GetPortsManager();				// get manager
	auto* port = manager.GetPort(_nameOrIndex.name);		// try to access by name
	if (!port) port = manager.GetPort(_nameOrIndex.index);	// try to access by index
	return port;											// return pointer
}

CStateVariable* CScriptRunner::GetStateVarPtr(CBaseUnit* _model, const SNameOrIndex& _nameOrIndex)
{
	if (!_model) return {};
	auto& manager = _model->GetStateVariablesManager();						// get manager
	auto* variable = manager.GetStateVariable(_nameOrIndex.name);			// try to access by name
	if (!variable) variable = manager.GetStateVariable(_nameOrIndex.index);	// try to access by index
	return variable;														// return pointer
}

const CPlot* CScriptRunner::GetPlotPtr(const CBaseUnit* _model, const SNameOrIndex& _nameOrIndex)
{
	if (!_model) return {};
	const auto& manager = _model->GetPlotsManager();		// get manager
	const auto* plot = manager.GetPlot(_nameOrIndex.name);	// try to access by name
	if (!plot) plot = manager.GetPlot(_nameOrIndex.index);	// try to access by index
	return plot;											// return pointer
}

const CCurve* CScriptRunner::GetCurvePtr(const CPlot* _plot, const SNameOrIndex& _nameOrIndex)
{
	if (!_plot) return {};
	const auto* curve = _plot->GetCurve(_nameOrIndex.name);		// try to access by name
	if (!curve) curve = _plot->GetCurve(_nameOrIndex.index);	// try to access by index
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

bool CScriptRunner::PrintMessage(const std::string& _message)
{
	std::cout << _message << std::endl;
	return false;
}
