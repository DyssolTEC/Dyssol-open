/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ArgumentsParser.h"
#include "ScriptParser.h"
#include "ScriptRunner.h"
#include "DyssolStringConstants.h"
#include "ConfigFileParser.h"
#include "ParametersHolder.h"
#include "Simulator.h"
#include "Flowsheet.h"
#include "ModelsManager.h"
#include "BaseUnit.h"
#include "MaterialsDatabase.h"
#include "MultidimensionalGrid.h"
#include "DyssolUtilities.h"
#include "StringFunctions.h"
#include "ThreadPool.h"
#include "DyssolSystemDefines.h"
#include "FileSystem.h"
#include <chrono>
#include <filesystem>
#include <fstream>

void ExportResults(const CConfigFileParser& _parser, const CFlowsheet& _flowsheet)
{
	if (!_parser.IsValueDefined(EArguments::TEXT_EXPORT_FILE)) return;

	const auto FindStreamByName = [&](const std::string& _name) -> const CStream*
	{
		const auto streams = _flowsheet.GetAllStreams();
		const auto it = std::find_if(streams.begin(), streams.end(), [&](const CStream* _s) { return _s->GetName() == _name; });
		if (it == streams.end()) return {};
		return *it;
	};

	// open export text file
	std::ofstream file(StringFunctions::UnicodePath(_parser.GetValue<std::wstring>(EArguments::TEXT_EXPORT_FILE)));
	// set precision
	if (_parser.IsValueDefined(EArguments::TEXT_EXPORT_PRECISION))
		file.precision(_parser.GetValue<unsigned>(EArguments::TEXT_EXPORT_PRECISION));
	if (_parser.IsValueDefined(EArguments::TEXT_EXPORT_FIXED_POINT))
		file.setf(std::ios::fixed);

	double limit = 0.0;
	if (_parser.IsValueDefined(EArguments::TEXT_EXPORT_SIGNIFICANCE_LIMIT))
		limit = std::abs(_parser.GetValue<double>(EArguments::TEXT_EXPORT_SIGNIFICANCE_LIMIT));

	const auto Limit = [&](double _v)
	{
		return limit == 0.0 ? _v : std::abs(_v) >= limit ? _v : 0.0;
	};

	// export mass flows
	for (const auto& e : _parser.GetValue<std::vector<SExportStreamDataMass>>(EArguments::EXPORT_MASS))
	{
		const CStream* stream = FindStreamByName(e.streamName);
		if (!stream) continue;
		file << "MASS " << stream->GetName();
		for (const double t : !e.timePoints.empty() ? e.timePoints : stream->GetAllTimePoints())
			file << " " << t << " " << Limit(stream->GetMass(t));
		file << std::endl;
	}

	// export PSDs
	for (const auto& e : _parser.GetValue<std::vector<SExportStreamDataMass>>(EArguments::EXPORT_PSD))
	{
		const CStream* stream = FindStreamByName(e.streamName);
		if (!stream) continue;
		file << "PSD " << stream->GetName();
		for (const double t : !e.timePoints.empty() ? e.timePoints : stream->GetAllTimePoints())
		{
			file << " " << t;
			for (const double v : stream->GetPSD(t, PSD_MassFrac))
				file << " " << Limit(v);
		}
		file << std::endl;
	}
}

// Prints information about command line arguments.
void PrintArgumentsInfo()
{
	// TODO: build in into ArgumentsParser
	std::cout << "Usage: DyssolC -key[=value] [-key[=value]]..." << std::endl;
	std::cout << std::endl;
	std::cout << "Mandatory arguments to start simulation:" << std::endl;
	std::cout << "-s, --script          path to script file" << std::endl;
	std::cout << std::endl;
	std::cout << "Information:" << std::endl;
	std::cout << "-v,  --version        print information about current version" << std::endl;
	std::cout << "-m,  --models         print information about available models" << std::endl;
	std::cout << "-mp, --models_path    additional path to look for available models" << std::endl;
}

// Prints information about the current version.
void PrintVersionInfo()
{
	std::cout << "Version: " << CURRENT_VERSION_STR << std::endl;
}

// Helper function to print a formatted entry justified left and filled with spaces until the given length.
template<typename T> void PrintEntry(const T& _entry, std::streamsize _width)
{
	std::cout << std::left << std::setw(_width) << std::setfill(' ') << _entry;
}

// Prints information about available models.
void PrintModelsInfo(const std::vector<std::filesystem::path>& _modelPaths)
{
	// create models manager
	CModelsManager manager;
	manager.AddDir(L".");				// add current directory as a path to units/solvers
	for (const auto& dir : _modelPaths)	// add other directories as a path to units/solvers
		manager.AddDir(dir);

	// get available models
	const auto units   = manager.GetAvailableUnits();
	const auto solvers = manager.GetAvailableSolvers();

	// print info
	const size_t nameLen = 50;
	const size_t typeLen = 30;
	const size_t authLen = 50;
	PrintEntry("Name"  , nameLen);
	PrintEntry("Type"  , typeLen);
	PrintEntry("Author", authLen);
	std::cout << std::endl;
	for (const auto& m : units)
	{
		PrintEntry(m.name, nameLen);
		PrintEntry(m.isDynamic ? StrConst::MMT_Dynamic : StrConst::MMT_SteadyState, typeLen);
		PrintEntry(m.author, authLen);
		std::cout << std::endl;
	}
	for (const auto& m : solvers)
	{
		PrintEntry(m.name, nameLen);
		PrintEntry(std::vector<std::string>{ SOLVERS_TYPE_NAMES }[static_cast<unsigned>(m.solverType)] + " " + StrConst::MMT_Solver, typeLen);
		PrintEntry(m.author, authLen);
		std::cout << std::endl;
	}
}

void RunSimulation(const CConfigFileParser& _parser)
{
	const std::wstring sSrcFile = _parser.GetValue<std::wstring>(EArguments::SOURCE_FILE);
	const std::wstring sDstFile = _parser.IsValueDefined(EArguments::RESULT_FILE) ? _parser.GetValue<std::wstring>(EArguments::RESULT_FILE) : sSrcFile;
	const std::wstring sMDBFile = _parser.GetValue<std::wstring>(EArguments::MATERIALS_DATABASE);

	// create material database
	CMaterialsDatabase materialsDB;
	if (!materialsDB.LoadFromFile(sMDBFile))
	{
		std::cout << "Error: The specified materials database file can not be loaded: " << StringFunctions::WString2String(sMDBFile) << std::endl;
		return;
	}

	// create models manager
	CModelsManager modelsManager;
	std::cout << "Loading available models..." << std::endl;
	modelsManager.AddDir(L".");		// add current directory as path to units/solvers
	for (const auto& dir : _parser.GetValue<std::vector<std::wstring>>(EArguments::MODELS_PATH))
		modelsManager.AddDir(dir);

	// create flowsheet
	CFlowsheet flowsheet{ modelsManager, materialsDB };

	// load flowsheet
	std::cout << "Loading flowsheet..." << std::endl;
	CH5Handler fileHandler;
	if (!flowsheet.LoadFromFile(fileHandler, sSrcFile))
	{
		std::cout << "Error: The specified flowsheet can not be loaded: " << StringFunctions::WString2String(sSrcFile) << std::endl;
		return;
	}

	// set parameters
	auto* params = flowsheet.GetParameters();
	if (_parser.IsValueDefined(EArguments::SIMULATION_TIME))	params->EndSimulationTime(_parser.GetValue<double>(EArguments::SIMULATION_TIME));
	if (_parser.IsValueDefined(EArguments::RELATIVE_TOLERANCE))	params->RelTol(_parser.GetValue<double>(EArguments::RELATIVE_TOLERANCE));
	if (_parser.IsValueDefined(EArguments::ABSOLUTE_TOLERANCE))	params->AbsTol(_parser.GetValue<double>(EArguments::ABSOLUTE_TOLERANCE));
	if (_parser.IsValueDefined(EArguments::MINIMAL_FRACTION))	params->MinFraction(_parser.GetValue<double>(EArguments::MINIMAL_FRACTION));
	if (_parser.IsValueDefined(EArguments::INIT_TIME_WINDOW))	params->InitTimeWindow(_parser.GetValue<double>(EArguments::INIT_TIME_WINDOW));
	if (_parser.IsValueDefined(EArguments::MIN_TIME_WINDOW))	params->MinTimeWindow(_parser.GetValue<double>(EArguments::MIN_TIME_WINDOW));
	if (_parser.IsValueDefined(EArguments::MAX_TIME_WINDOW))	params->MaxTimeWindow(_parser.GetValue<double>(EArguments::MAX_TIME_WINDOW));
	if (_parser.IsValueDefined(EArguments::MAX_ITERATIONS_NUM))	params->MaxItersNumber(_parser.GetValue<unsigned>(EArguments::MAX_ITERATIONS_NUM));
	if (_parser.IsValueDefined(EArguments::WINDOW_CHANGE_RATE))	params->MagnificationRatio(_parser.GetValue<double>(EArguments::WINDOW_CHANGE_RATE));
	if (_parser.IsValueDefined(EArguments::ITER_UPPER_LIMIT))	params->ItersUpperLimit(_parser.GetValue<unsigned>(EArguments::ITER_UPPER_LIMIT));
	if (_parser.IsValueDefined(EArguments::ITER_LOWER_LIMIT))	params->ItersLowerLimit(_parser.GetValue<unsigned>(EArguments::ITER_LOWER_LIMIT));
	if (_parser.IsValueDefined(EArguments::ITER_UPPER_LIMIT_1))	params->Iters1stUpperLimit(_parser.GetValue<unsigned>(EArguments::ITER_UPPER_LIMIT_1));
	if (_parser.IsValueDefined(EArguments::CONVERGENCE_METHOD))	params->ConvergenceMethod(static_cast<EConvergenceMethod>(_parser.GetValue<unsigned>(EArguments::CONVERGENCE_METHOD)));
	if (_parser.IsValueDefined(EArguments::ACCEL_PARAMETER))	params->WegsteinAccelParam(_parser.GetValue<double>(EArguments::ACCEL_PARAMETER));
	if (_parser.IsValueDefined(EArguments::RELAX_PARAMETER))	params->RelaxationParam(_parser.GetValue<double>(EArguments::RELAX_PARAMETER));
	if (_parser.IsValueDefined(EArguments::EXTRAPOL_METHOD))	params->ExtrapolationMethod(static_cast<EExtrapolationMethod>(_parser.GetValue<unsigned>(EArguments::EXTRAPOL_METHOD)));

	// setup grid
	if (_parser.IsValueDefined(EArguments::DISTRIBUTION_GRID))
	{
		CMultidimensionalGrid grid;
		for (auto g : _parser.GetValue<std::vector<SGridDimensionEx>>(EArguments::DISTRIBUTION_GRID))
		{
			if (g.gridType == EGridEntry::GRID_NUMERIC)
				grid.AddNumericDimension(flowsheet.GetGrid().GetDimensionsTypes()[g.iGrid], g.vNumGrid);
			else
				grid.AddSymbolicDimension(flowsheet.GetGrid().GetDimensionsTypes()[g.iGrid], g.vStrGrid);
		}
		flowsheet.SetMainGrid(grid);
	}

	auto units = flowsheet.GetAllUnits();
	const auto phases = flowsheet.GetPhases();
	const auto compounds = flowsheet.GetCompounds();

	// setup unit parameters
	if (_parser.IsValueDefined(EArguments::UNIT_PARAMETER))
	{
		for (const auto& u : _parser.GetValue<std::vector<SUnitParameterEx>>(EArguments::UNIT_PARAMETER))
		{
			CUnitContainer* pModel = units[u.iUnit];
			if (!pModel || !pModel->GetModel()) continue;
			CBaseUnitParameter* pParam = pModel->GetModel()->GetUnitParametersManager().GetParameter(u.iParam);
			if (!pParam) continue;
			pParam->Clear();
			switch (pParam->GetType())
			{
			case EUnitParameter::CONSTANT:
			case EUnitParameter::CONSTANT_DOUBLE:
				dynamic_cast<CConstRealUnitParameter*>(pParam)->SetValue(u.dValue);
				break;
			case EUnitParameter::CONSTANT_INT64:
				dynamic_cast<CConstIntUnitParameter*>(pParam)->SetValue(static_cast<int64_t>(u.dValue));
				break;
			case EUnitParameter::CONSTANT_UINT64:
				dynamic_cast<CConstUIntUnitParameter*>(pParam)->SetValue(static_cast<int64_t>(u.dValue));
				break;
			case EUnitParameter::TIME_DEPENDENT:
				for (const auto td : u.tdValue)
					dynamic_cast<CTDUnitParameter*>(pParam)->SetValue(td.time, td.value);
				break;
			case EUnitParameter::STRING:
				dynamic_cast<CStringUnitParameter*>(pParam)->SetValue(u.sValue);
				break;
			case EUnitParameter::CHECKBOX:
				dynamic_cast<CCheckBoxUnitParameter*>(pParam)->SetChecked(u.dValue != 0);
				break;
			case EUnitParameter::SOLVER:
				dynamic_cast<CSolverUnitParameter*>(pParam)->SetKey(modelsManager.GetSolverDescriptor(StringFunctions::String2WString(u.sValue)).uniqueID);
				break;
			case EUnitParameter::COMBO:
			case EUnitParameter::GROUP:
				dynamic_cast<CComboUnitParameter*>(pParam)->SetValue(static_cast<size_t>(u.dValue));
				break;
			case EUnitParameter::COMPOUND:
				dynamic_cast<CCompoundUnitParameter*>(pParam)->SetCompound(u.sValue);
				break;
			case EUnitParameter::MDB_COMPOUND:
				dynamic_cast<CMDBCompoundUnitParameter*>(pParam)->SetCompound(u.sValue);
				break;
			case EUnitParameter::REACTION:		break;	// TODO: implement
			case EUnitParameter::LIST_DOUBLE:	break;	// TODO: implement
			case EUnitParameter::LIST_UINT64:	break;	// TODO: implement
			case EUnitParameter::LIST_INT64:	break;	// TODO: implement
			case EUnitParameter::UNKNOWN:  break;
			}
		}
	}

	// setup holdups MTP
	for (auto h : _parser.GetValue<std::vector<SHoldupParam>>(EArguments::UNIT_HOLDUP_MTP))
	{
		CUnitContainer* pModel = units[h.iUnit];
		if (!pModel || !pModel->GetModel()) continue;
		auto holdups = pModel->GetModel()->GetStreamsManager().GetAllInit();
		if (h.iHoldup >= holdups.size()) continue;
		CBaseStream* pHoldup = holdups[h.iHoldup];
		if (!pHoldup) continue;
		std::vector<double> values = h.vValues;
		values.resize(3); // to ensure it is of required length
		std::vector<double> vTPs = pHoldup->GetAllTimePoints();
		if (h.iTimePoint >= vTPs.size()) continue;
		pHoldup->SetMass(vTPs[h.iTimePoint], values[0]);
		pHoldup->SetTemperature(vTPs[h.iTimePoint], values[1]);
		pHoldup->SetPressure(vTPs[h.iTimePoint], values[2]);
	}

	// setup holdups phase fractions
	for (auto h : _parser.GetValue<std::vector<SHoldupParam>>(EArguments::UNIT_HOLDUP_PHASES))
	{
		CUnitContainer* pModel = units[h.iUnit];
		if (!pModel || !pModel->GetModel()) continue;
		auto holdups = pModel->GetModel()->GetStreamsManager().GetAllInit();
		if (h.iHoldup >= holdups.size()) continue;
		CBaseStream* pHoldup = holdups[h.iHoldup];
		if (!pHoldup) continue;
		std::vector<double> values = h.vValues;
		values.resize(flowsheet.GetPhasesNumber()); // to ensure it is of required length
		std::vector<double> vTPs = pHoldup->GetAllTimePoints();
		if (h.iTimePoint >= vTPs.size()) continue;
		for (unsigned i = 0; i < phases.size(); ++i)
			pHoldup->SetPhaseFraction(vTPs[h.iTimePoint], phases[i].state, values[i]);
	}

	// setup holdups compound fractions
	for (auto h : _parser.GetValue<std::vector<SHoldupParam>>(EArguments::UNIT_HOLDUP_COMP))
	{
		CUnitContainer* pModel = units[h.iUnit];
		if (!pModel || !pModel->GetModel()) continue;
		auto holdups = pModel->GetModel()->GetStreamsManager().GetAllInit();
		if (h.iHoldup >= holdups.size()) continue;
		CBaseStream* pHoldup = holdups[h.iHoldup];
		if (!pHoldup) continue;
		if (h.iPhase >= flowsheet.GetPhasesNumber()) continue;
		std::vector<double> values = h.vValues;
		values.resize(flowsheet.GetCompoundsNumber()); // to ensure it is of required length
		std::vector<double> vTPs = pHoldup->GetAllTimePoints();
		if (h.iTimePoint >= vTPs.size()) continue;
		for (unsigned i = 0; i < compounds.size(); ++i)
			pHoldup->SetCompoundFraction(vTPs[h.iTimePoint], compounds[i], phases[h.iPhase].state, values[i]);
	}

	// setup holdups solids distributions
	for (auto h : _parser.GetValue<std::vector<SHoldupParam>>(EArguments::UNIT_HOLDUP_SOLID))
	{
		CUnitContainer* pModel = units[h.iUnit];
		if (!pModel || !pModel->GetModel()) continue;
		auto holdups = pModel->GetModel()->GetStreamsManager().GetAllInit();
		if (h.iHoldup >= holdups.size()) continue;
		CBaseStream* pHoldup = holdups[h.iHoldup];
		if (!pHoldup) continue;
		if (!flowsheet.HasPhase(EPhase::SOLID)) continue;
		const CMultidimensionalGrid& pGrid = flowsheet.GetGrid();
		if (h.iDistribution >= pGrid.GetDimensionsNumber()) continue;
		const std::vector<double> vTPs = pHoldup->GetAllTimePoints();
		if (h.iTimePoint >= vTPs.size()) continue;
		const auto distrType = flowsheet.GetGrid().GetDimensionsTypes()[h.iDistribution];
		const EPSDGridType sizeType = distrType == DISTR_SIZE ? h.psdGridType : EPSDGridType::DIAMETER;
		// TODO: if symbolic grid - will fail
		const std::vector<double> vMedians = distrType == DISTR_SIZE ? pGrid.GetPSDMeans(sizeType) : (pGrid.GetGridDimension(distrType)->GridType() == EGridEntry::GRID_NUMERIC ? pGrid.GetGridDimensionNumeric(distrType)->GetClassesMeans() : std::vector<double>{});
		std::vector<double> values(pGrid.GetGridDimension(distrType)->ClassesNumber());
		switch (h.distrFun)
		{
		case EDistrFunction::Manual:								values = h.vValues;															break;
		case EDistrFunction::Normal:	if (h.vValues.size() >= 2)	values = CreateDistributionNormal(vMedians, h.vValues[0], h.vValues[1]);	break;
		case EDistrFunction::LogNormal:	if (h.vValues.size() >= 2)	values = CreateDistributionLogNormal(vMedians, h.vValues[0], h.vValues[1]);	break;
		case EDistrFunction::RRSB:		if (h.vValues.size() >= 2)	values = CreateDistributionRRSB(vMedians, h.vValues[0], h.vValues[1]);		break;
		case EDistrFunction::GGS:		if (h.vValues.size() >= 2)	values = CreateDistributionGGS(vMedians, h.vValues[0], h.vValues[1]);		break;
		}

		values.resize(pGrid.GetGridDimension(distrType)->ClassesNumber()); // to ensure it is of required length
		if (distrType == DISTR_SIZE)
			if (h.iCompound < flowsheet.GetCompoundsNumber())
				pHoldup->SetPSD(vTPs[h.iTimePoint], h.psdType, compounds[h.iCompound], values);
			else
				pHoldup->SetPSD(vTPs[h.iTimePoint], h.psdType, values);
		else
			if (h.iCompound < flowsheet.GetCompoundsNumber())
				pHoldup->SetDistribution(vTPs[h.iTimePoint], distrType, compounds[h.iCompound], values);
			else
				pHoldup->SetDistribution(vTPs[h.iTimePoint], distrType, values);
	}

	// initialize flowsheet
	std::cout << "Initializing flowsheet..." << std::endl;
	std::string sInitError = flowsheet.Initialize();
	if (!sInitError.empty())
	{
		std::cout << "Error during initialization: " << sInitError << std::endl;
		return;
	}

	// create and setup simulator
	CSimulator simulator;
	simulator.SetFlowsheet(&flowsheet);

	std::cout << "Number of simulation threads: " << ThreadPool::CThreadPool::GetAvailableThreadsNumber() << std::endl;

	// run simulation
	std::cout << "Starting simulation..." << std::endl;
	const auto tStart = std::chrono::steady_clock::now();
	simulator.Simulate();
	const auto tEnd = std::chrono::steady_clock::now();

	// save simulation results
	std::cout << "Saving flowsheet..." << std::endl;
	flowsheet.SaveToFile(fileHandler, sDstFile);

	std::cout << "Simulation finished in " << std::chrono::duration_cast<std::chrono::seconds>(tEnd - tStart).count() << " [s]" << std::endl;

	// export results to text files if necessary
	ExportResults(_parser, flowsheet);

	// export to graph file
	if (_parser.IsValueDefined(EArguments::EXPORT_GRAPH))
	{
		std::ofstream file(StringFunctions::UnicodePath(_parser.GetValue<std::wstring>(EArguments::EXPORT_GRAPH)));
		file << flowsheet.GenerateDOTFile();
		file.close();
	}
}

void RunDyssol(const std::filesystem::path& _script)
{
	InitializeThreadPool();

	std::cout << "Parsing script file: " << _script << std::endl;

	const CScriptParser parser{ _script };
	std::cout << "Jobs found: " << parser.JobsCount() << std::endl;

	CScriptRunner runner;
	for (const auto& job : parser.Jobs())
		runner.RunJob(*job);
}

int main(int argc, const char *argv[])
{
	// TODO: just run the simulation
	// possible keys with aliases
	std::vector<std::vector<std::string>> keys;
	keys.push_back({ "v" , "version"     });
	keys.push_back({ "m" , "models"      });
	keys.push_back({ "mp", "models_path" });
	keys.push_back({ "s" , "script"      });

	const CArgumentsParser parser(argc, argv, keys);

	if (parser.TokensCount() == 0)
	{
		PrintArgumentsInfo();
		return 1;
	}

	if (parser.HasKey("v"))
		PrintVersionInfo();
	if (parser.HasKey("m"))
	{
		std::vector<std::filesystem::path> fsPaths;
		for (const auto& p : parser.GetValues("mp"))
			fsPaths.emplace_back(p);
		PrintModelsInfo(fsPaths);
	}
	if (parser.HasKey("s"))
		RunDyssol(parser.GetValue("s"));

	return 0;
}
