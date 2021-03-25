/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Agglomerator.h"
#include "DistributionsFunctions.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CAgglomerator();
}

void CAgglomerator::CreateBasicInfo()
{
	/// Set basic unit info ///
	SetUnitName  ("Agglomerator");
	SetAuthorName("SPE TUHH");
	SetUniqueID  ("9F37215AA74D4B1699B7EC648F366219");
}

void CAgglomerator::CreateStructure()
{
	/// Add ports ///
	AddPort("Input" , EUnitPort::INPUT);
	AddPort("Output", EUnitPort::OUTPUT);

	/// Add unit parameters ///
	AddConstRealParameter("Beta0", 1, "-", "Rate factor", 0);
	AddConstRealParameter("Step", 0, "s", "Max time step in DAE solver", 0);
	/// Add possibility to choose external agglomeration calculator ///
	AddSolverAgglomeration("Solver", "Agglomeration solver");
	AddComboParameter("Kernel", CAgglomerationSolver::EKernels::BROWNIAN,
		{ CAgglomerationSolver::EKernels::CONSTANT, CAgglomerationSolver::EKernels::SUM, CAgglomerationSolver::EKernels::PRODUCT, CAgglomerationSolver::EKernels::BROWNIAN, CAgglomerationSolver::EKernels::SHEAR, CAgglomerationSolver::EKernels::PEGLOW, CAgglomerationSolver::EKernels::COAGULATION, CAgglomerationSolver::EKernels::GRAVITATIONAL, CAgglomerationSolver::EKernels::EKE, CAgglomerationSolver::EKernels::THOMPSON },
		{ "Constant","Sum","Product","Brownian","Shear","Peglow","Coagulation","Gravitational","Kinetic energy","Thompson" },
		"Agglomeration kernel");
	AddConstUIntParameter("Rank", 3, "", "Rank of the kernel (for FFT solver)", 1, 10);

	/// Add holdups ///
	AddHoldup("Holdup");

	/// Set this unit as user data of model ///
	m_Model.SetUserData(this);
}

void CAgglomerator::Initialize(double _dTime)
{
	/// Check flowsheet parameters ///
	if (!IsPhaseDefined(EPhase::SOLID))		RaiseError("Solid phase has not been defined.");
	if (!IsDistributionDefined(DISTR_SIZE))	RaiseError("Size distribution has not been defined.");

	/// Get pointers to streams and holdups ///
	m_pHoldup = GetHoldup("Holdup");
	m_pInStream = GetPortStream("Input");
	m_pOutStream = GetPortStream("Output");

	/// Get number of classes for PSD ///
	m_nClassesNum = GetClassesNumber(DISTR_SIZE);
	/// Get grid of PSD ///
	m_vSizeGrid = GetNumericGrid(DISTR_SIZE);
	/// Get particles' mean diameters ///
	m_vSizes = GetClassesSizes(DISTR_SIZE);

	/// Clear all state variables in model ///
	m_Model.ClearVariables();

	std::vector<double> vNinlet = m_pHoldup->GetPSD(_dTime, PSD_Number);

	/// Add state variables to a model ///
	for (size_t i = 0; i < m_nClassesNum; ++i)	// Initial PSD
		m_Model.AddDAEVariable(true, vNinlet[i], 0);
	m_Model.m_nq0 = 0;

	/// Set tolerances to model ///
	m_Model.SetTolerance(GetRelTolerance(), GetAbsTolerance());

	/// Set model to a solver ///
	const double dMaxStep = GetConstRealParameterValue("Step");
	if (dMaxStep != 0.0)
		m_Solver.SetMaxStep(dMaxStep);
	if (!m_Solver.SetModel(&m_Model))
		RaiseError(m_Solver.GetError());

	/// Initialize agglomeration calculator ///
	m_pAggSolver = GetSolverAgglomeration("Solver");
	if (!m_pAggSolver)
	{
		RaiseError("Cannot load Solver");
		return;
	}
	/// Set parameters ///
	m_pAggSolver->Initialize(m_vSizeGrid, GetConstRealParameterValue("Beta0"),
							static_cast<CAgglomerationSolver::EKernels>(GetComboParameterValue("Kernel")),
							GetConstUIntParameterValue("Rank"));
}

void CAgglomerator::SaveState()
{
	m_Solver.SaveState();
}

void CAgglomerator::LoadState()
{
	m_Solver.LoadState();
}

void CAgglomerator::Simulate(double _dStartTime, double _dEndTime)
{
	if (!m_Solver.Calculate(_dStartTime, _dEndTime))
		RaiseError(m_Solver.GetError());
}

void CUnitDAEModel::ResultsHandler(double _dTime, double* _pVars, double* _pDerivs, void* _pUserData)
{
	auto* unit = static_cast<CAgglomerator*>(_pUserData);

	unit->m_pHoldup->AddTimePoint(_dTime);

	const double dHoldupMass = unit->m_pHoldup->GetMass(_dTime);
	unit->m_pHoldup->AddStream(std::max(unit->m_pInStream->GetPreviousTimePoint(_dTime), unit->m_pHoldup->GetPreviousTimePoint(_dTime)), _dTime, unit->m_pInStream);
	unit->m_pHoldup->RemoveTimePointsAfter(_dTime);
	unit->m_pHoldup->SetMass(_dTime, dHoldupMass);

	unit->m_pHoldup->SetPSD(_dTime, PSD_MassFrac, ConvertNumbersToMassFractions(unit->m_vSizeGrid, std::vector<double>(_pVars + m_nq0, _pVars + m_nq0 + unit->m_nClassesNum)));

	const double dOutMass = unit->m_pInStream->GetMassFlow(_dTime); // equal to in mass flow
	unit->m_pOutStream->CopyFromHoldup(_dTime, unit->m_pHoldup, dOutMass);
}

void CUnitDAEModel::CalculateResiduals(double _dTime, double* _pVars, double* _pDers, double* _pRes, void* _pUserData)
{
	const auto* unit = static_cast<CAgglomerator*>(_pUserData);

	const double dInMass = unit->m_pInStream->GetMassFlow(_dTime);
	const double dHoldupMass = unit->m_pHoldup->GetMass(_dTime);
	const double dOutMass = dInMass;

	std::vector<double> vNinlet = unit->m_pInStream->GetPSD(_dTime, PSD_Number);

	// Call agglomeration function
	unit->m_pAggSolver->Calculate(std::vector<double>(_pVars, _pVars + unit->m_nClassesNum), m_vBRate, m_vDRate);

	// Calculate derivatives
	for (size_t i = 0; i < unit->m_nClassesNum; ++i)
	{
		const double dDer = m_vBRate[i] - m_vDRate[i] + vNinlet[i] - _pVars[i] / dHoldupMass * dOutMass;
		_pRes[m_nq0 + i] = _pDers[m_nq0 + i] - dDer;
	}
}
