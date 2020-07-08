/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ConfigFileParser.h"
#include "FlowsheetParameters.h"
#include "Simulator.h"
#include "ModelsManager.h"
#include "DyssolUtilities.h"
#include "StringFunctions.h"
#include "ThreadPool.h"
#include "DyssolSystemDefines.h"
#include <chrono>

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
	CFlowsheet flowsheet;
	// set models manager
	flowsheet.SetModelsManager(&modelsManager);
	// set material database
	flowsheet.SetMaterialsDatabase(&materialsDB);

	// load flowsheet
	std::cout << "Loading flowsheet..." << std::endl;
	CH5Handler fileHandler;
	if (!flowsheet.LoadFromFile(fileHandler, sSrcFile))
	{
		std::cout << "Error: The specified flowsheet can not be loaded: " << StringFunctions::WString2String(sSrcFile) << std::endl;
		return;
	}

	// set parameters
	if (_parser.IsValueDefined(EArguments::SIMULATION_TIME))	flowsheet.SetSimulationTime(_parser.GetValue<double>(EArguments::SIMULATION_TIME));
	if (_parser.IsValueDefined(EArguments::RELATIVE_TOLERANCE))	flowsheet.m_pParams->RelTol(_parser.GetValue<double>(EArguments::RELATIVE_TOLERANCE));
	if (_parser.IsValueDefined(EArguments::ABSOLUTE_TOLERANCE))	flowsheet.m_pParams->AbsTol(_parser.GetValue<double>(EArguments::ABSOLUTE_TOLERANCE));
	if (_parser.IsValueDefined(EArguments::MINIMAL_FRACTION))	flowsheet.m_pParams->MinFraction(_parser.GetValue<double>(EArguments::MINIMAL_FRACTION));
	if (_parser.IsValueDefined(EArguments::INIT_TIME_WINDOW))	flowsheet.m_pParams->InitTimeWindow(_parser.GetValue<double>(EArguments::INIT_TIME_WINDOW));
	if (_parser.IsValueDefined(EArguments::MIN_TIME_WINDOW))	flowsheet.m_pParams->MinTimeWindow(_parser.GetValue<double>(EArguments::MIN_TIME_WINDOW));
	if (_parser.IsValueDefined(EArguments::MAX_TIME_WINDOW))	flowsheet.m_pParams->MaxTimeWindow(_parser.GetValue<double>(EArguments::MAX_TIME_WINDOW));
	if (_parser.IsValueDefined(EArguments::MAX_ITERATIONS_NUM))	flowsheet.m_pParams->MaxItersNumber(_parser.GetValue<unsigned>(EArguments::MAX_ITERATIONS_NUM));
	if (_parser.IsValueDefined(EArguments::WINDOW_CHANGE_RATE))	flowsheet.m_pParams->MagnificationRatio(_parser.GetValue<double>(EArguments::WINDOW_CHANGE_RATE));
	if (_parser.IsValueDefined(EArguments::ITER_UPPER_LIMIT))	flowsheet.m_pParams->ItersUpperLimit(_parser.GetValue<unsigned>(EArguments::ITER_UPPER_LIMIT));
	if (_parser.IsValueDefined(EArguments::ITER_LOWER_LIMIT))	flowsheet.m_pParams->ItersLowerLimit(_parser.GetValue<unsigned>(EArguments::ITER_LOWER_LIMIT));
	if (_parser.IsValueDefined(EArguments::ITER_UPPER_LIMIT_1))	flowsheet.m_pParams->Iters1stUpperLimit(_parser.GetValue<unsigned>(EArguments::ITER_UPPER_LIMIT_1));
	if (_parser.IsValueDefined(EArguments::CONVERGENCE_METHOD))	flowsheet.m_pParams->ConvergenceMethod(static_cast<EConvMethod>(_parser.GetValue<unsigned>(EArguments::CONVERGENCE_METHOD)));
	if (_parser.IsValueDefined(EArguments::ACCEL_PARAMETER))	flowsheet.m_pParams->WegsteinAccelParam(_parser.GetValue<double>(EArguments::ACCEL_PARAMETER));
	if (_parser.IsValueDefined(EArguments::RELAX_PARAMETER))	flowsheet.m_pParams->RelaxationParam(_parser.GetValue<double>(EArguments::RELAX_PARAMETER));
	if (_parser.IsValueDefined(EArguments::EXTRAPOL_METHOD))	flowsheet.m_pParams->ExtrapolationMethod(static_cast<EExtrapMethod>(_parser.GetValue<unsigned>(EArguments::EXTRAPOL_METHOD)));

	// setup grid
	if (_parser.IsValueDefined(EArguments::DISTRIBUTION_GRID))
	{
		for (auto g : _parser.GetValue<std::vector<SGridDimensionEx>>(EArguments::DISTRIBUTION_GRID))
		{
			SGridDimension dim = flowsheet.GetDistributionsGrid()->GetDimension(g.iGrid);
			dim.gridFun = g.gridFun;
			dim.gridEntry = g.gridType;
			dim.classes = g.nClasses;
			dim.numGrid = g.vNumGrid;
			dim.strGrid = g.vStrGrid;
			flowsheet.GetDistributionsGrid()->SetDimension(dim);
		}
		flowsheet.SetDistributionsGrid();
	}

	// setup unit parameters
	if (_parser.IsValueDefined(EArguments::UNIT_PARAMETER))
	{
		for (const auto& u : _parser.GetValue<std::vector<SUnitParameterEx>>(EArguments::UNIT_PARAMETER))
		{
			CBaseModel* pModel = flowsheet.GetModel(u.iUnit);
			if (!pModel) continue;
			CBaseUnitParameter* pParam = pModel->GetUnitParametersManager()->GetParameter(u.iParam);
			if (!pParam) continue;
			pParam->Clear();
			switch (pParam->GetType())
			{
			case EUnitParameter::CONSTANT:
				dynamic_cast<CConstUnitParameter*>(pParam)->SetValue(u.dValue);
				break;
			case EUnitParameter::TIME_DEPENDENT:
				for (const auto td : u.tdValue)
					dynamic_cast<CTDUnitParameter*>(pParam)->SetValue(td.dTime, td.dValue);
				break;
			case EUnitParameter::STRING:
				dynamic_cast<CStringUnitParameter*>(pParam)->SetValue(u.sValue);
				break;
			case EUnitParameter::CHECKBOX:
				dynamic_cast<CCheckboxUnitParameter*>(pParam)->SetChecked(u.dValue != 0);
				break;
			case EUnitParameter::SOLVER:
				dynamic_cast<CSolverUnitParameter*>(pParam)->SetKey(modelsManager.GetSolverDescriptor(StringFunctions::String2WString(u.sValue)).uniqueID);
				break;
			case EUnitParameter::COMBO:
				dynamic_cast<CComboUnitParameter*>(pParam)->SetValue(static_cast<size_t>(u.dValue));
				break;
			case EUnitParameter::GROUP:
				dynamic_cast<CGroupUnitParameter*>(pParam)->SetValue(static_cast<size_t>(u.dValue));
				break;
			case EUnitParameter::COMPOUND:
				dynamic_cast<CCompoundUnitParameter*>(pParam)->SetCompound(u.sValue);
				break;
			case EUnitParameter::UNKNOWN:  break;
			}
		}
	}

	// setup holdups MTP
	for (auto h : _parser.GetValue<std::vector<SHoldupParam>>(EArguments::UNIT_HOLDUP_MTP))
	{
		CBaseModel* pModel = flowsheet.GetModel(h.iUnit);
		if (!pModel) continue;
		if (h.iHoldup >= pModel->GetHoldupsCount()) continue;
		CHoldup* pHoldup = pModel->GetHoldupInit(h.iHoldup);
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
		CBaseModel* pModel = flowsheet.GetModel(h.iUnit);
		if (!pModel) continue;
		if (h.iHoldup >= pModel->GetHoldupsCount()) continue;
		CHoldup* pHoldup = pModel->GetHoldupInit(h.iHoldup);
		if (!pHoldup) continue;
		std::vector<double> values = h.vValues;
		values.resize(flowsheet.GetPhasesNumber()); // to ensure it is of required length
		std::vector<double> vTPs = pHoldup->GetAllTimePoints();
		if (h.iTimePoint >= vTPs.size()) continue;
		for (unsigned i = 0; i < flowsheet.GetPhasesNumber(); ++i)
			pHoldup->SetSinglePhaseProp(vTPs[h.iTimePoint], PHASE_FRACTION, flowsheet.GetPhaseAggregationState(i), values[i]);
	}

	// setup holdups compound fractions
	for (auto h : _parser.GetValue<std::vector<SHoldupParam>>(EArguments::UNIT_HOLDUP_COMP))
	{
		CBaseModel* pModel = flowsheet.GetModel(h.iUnit);
		if (!pModel) continue;
		if (h.iHoldup >= pModel->GetHoldupsCount()) continue;
		CHoldup* pHoldup = pModel->GetHoldupInit(h.iHoldup);
		if (!pHoldup) continue;
		if (h.iPhase >= flowsheet.GetPhasesNumber()) continue;
		std::vector<double> values = h.vValues;
		values.resize(flowsheet.GetCompoundsNumber()); // to ensure it is of required length
		std::vector<double> vTPs = pHoldup->GetAllTimePoints();
		if (h.iTimePoint >= vTPs.size()) continue;
		for (unsigned i = 0; i < flowsheet.GetCompoundsNumber(); ++i)
			pHoldup->SetCompoundPhaseFraction(vTPs[h.iTimePoint], flowsheet.GetCompoundKey(i), flowsheet.GetPhaseAggregationState((unsigned)h.iPhase), values[i]);
	}

	// setup holdups solids distributions
	for (auto h : _parser.GetValue<std::vector<SHoldupParam>>(EArguments::UNIT_HOLDUP_SOLID))
	{
		CBaseModel* pModel = flowsheet.GetModel(h.iUnit);
		if (!pModel) continue;
		if (h.iHoldup >= pModel->GetHoldupsCount()) continue;
		CHoldup* pHoldup = pModel->GetHoldupInit(h.iHoldup);
		if (!pHoldup) continue;
		if (!flowsheet.IsPhaseDefined(SOA_SOLID)) continue;
		const CDistributionsGrid* pGrid = flowsheet.GetDistributionsGrid();
		if (h.iDistribution >= pGrid->GetDistributionsNumber()) continue;
		const std::vector<double> vTPs = pHoldup->GetAllTimePoints();
		if (h.iTimePoint >= vTPs.size()) continue;
		const EPSDGridType sizeType = pGrid->GetDistrType(h.iDistribution) == DISTR_SIZE ? h.psdGridType : EPSDGridType::DIAMETER;
		const std::vector<double> vMedians = pGrid->GetDistrType(h.iDistribution) == DISTR_SIZE ? pGrid->GetPSDMeans(sizeType) : pGrid->GetClassMeansByIndex(h.iDistribution);
		std::vector<double> values(pGrid->GetClassesByIndex(h.iDistribution));
		switch (h.distrFun)
		{
		case EDistrFunction::Manual:								values = h.vValues;															break;
		case EDistrFunction::Normal:	if (h.vValues.size() >= 2)	values = CreateDistributionNormal(vMedians, h.vValues[0], h.vValues[1]);	break;
		case EDistrFunction::LogNormal:	if (h.vValues.size() >= 2)	values = CreateDistributionLogNormal(vMedians, h.vValues[0], h.vValues[1]);	break;
		case EDistrFunction::RRSB:		if (h.vValues.size() >= 2)	values = CreateDistributionRRSB(vMedians, h.vValues[0], h.vValues[1]);		break;
		case EDistrFunction::GGS:		if (h.vValues.size() >= 2)	values = CreateDistributionGGS(vMedians, h.vValues[0], h.vValues[1]);		break;
		}

		values.resize(pGrid->GetClassesByIndex(h.iDistribution)); // to ensure it is of required length
		if (pGrid->GetDistrType(h.iDistribution) == DISTR_SIZE)
			if (h.iCompound < flowsheet.GetCompoundsNumber())
				pHoldup->SetPSD(vTPs[h.iTimePoint], h.psdType, flowsheet.GetCompoundKey(h.iCompound), values);
			else
				pHoldup->SetPSD(vTPs[h.iTimePoint], h.psdType, values);
		else
			if (h.iCompound < flowsheet.GetCompoundsNumber())
				pHoldup->SetDistribution(vTPs[h.iTimePoint], pGrid->GetDistrType(h.iDistribution), flowsheet.GetCompoundKey(h.iCompound), values);
			else
				pHoldup->SetDistribution(vTPs[h.iTimePoint], pGrid->GetDistrType(h.iDistribution), values);
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
}

int main(int argc, const char *argv[])
{
	if (argc < 2)
	{
		std::cout << "Error: Path to a config file is not specified." << std::endl;
		return 0;
	}

	const std::string sParam(argv[1]);
	if (sParam == "-models" || sParam == "-m")
	{
		// TODO
	}
	else if (sParam == "-version" || sParam == "-v")
	{
		std::cout << "Version: " << CURRENT_VERSION_STR << std::endl;
		//std::cout << "Build: " << CURRENT_BUILD_VERSION << std::endl;
	}
	else
	{
		CConfigFileParser parser;
		const bool bParsed = parser.Parse(sParam);
		if (!bParsed)
		{
			std::cout << "Error: The specified config file can not be parsed or contains errors." << std::endl;
			return 0;
		}

		RunSimulation(parser);
	}
}
