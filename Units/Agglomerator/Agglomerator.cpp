/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Agglomerator.h"
#include "DistributionsFunctions.h"
#include "DyssolUtilities.h"
#include "ContainerFunctions.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CAgglomerator();
}

void CAgglomerator::CreateBasicInfo()
{
	/// Set basic unit info ///
	SetUnitName("Agglomerator");
	SetAuthorName("SPE TUHH");
	SetUniqueID("9F37215AA74D4B1699B7EC648F366219");
	SetHelpLink("003_models/unit_agglomerator.html");
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
	AddComboParameter("Kernel", E2I(CAgglomerationSolver::EKernels::BROWNIAN),
		E2I({ CAgglomerationSolver::EKernels::CONSTANT, CAgglomerationSolver::EKernels::SUM, CAgglomerationSolver::EKernels::PRODUCT, CAgglomerationSolver::EKernels::BROWNIAN, CAgglomerationSolver::EKernels::SHEAR, CAgglomerationSolver::EKernels::PEGLOW, CAgglomerationSolver::EKernels::COAGULATION, CAgglomerationSolver::EKernels::GRAVITATIONAL, CAgglomerationSolver::EKernels::EKE, CAgglomerationSolver::EKernels::THOMPSON }),
		{ "Constant","Sum","Product","Brownian","Shear","Peglow","Coagulation","Gravitational","Kinetic energy","Thompson" },
		"Agglomeration kernel");
	AddConstUIntParameter("Rank", 3, "", "Rank of the kernel (for FFT solver)", 1, 10);
	AddConstRealParameter("Relative tolerance", 0.0, "-", "Solver relative tolerance. Set to 0 to use flowsheet-wide value", 0.0);
	AddConstRealParameter("Absolute tolerance", 0.0, "-", "Solver absolute tolerance. Set to 0 to use flowsheet-wide value", 0.0);

	/// Add holdups ///
	AddHoldup("Holdup");

	/// Set this unit as user data of model ///
	m_model.SetUserData(this);
}

void CAgglomerator::Initialize(double _time)
{
	/// Check flowsheet parameters ///
	if (!IsPhaseDefined(EPhase::SOLID))		RaiseError("Solid phase has not been defined.");
	if (!IsDistributionDefined(DISTR_SIZE))	RaiseError("Size distribution has not been defined.");

	/// Get pointers to streams and holdups ///
	m_holdup = GetHoldup("Holdup");
	m_inStream = GetPortStream("Input");
	m_outStream = GetPortStream("Output");

	/// Get number of classes for PSD ///
	m_classesNum = GetClassesNumber(DISTR_SIZE);
	/// Get grid of PSD ///
	m_sizeGrid = GetNumericGrid(DISTR_SIZE);
	/// Get particles' mean diameters ///
	m_sizes = GetClassesSizes(DISTR_SIZE);

	/// Clear all state variables in model ///
	m_model.ClearVariables();

	std::vector<double> Ninlet = m_holdup->GetPSD(_time, PSD_Number);

	/// Add state variables to a model ///
	m_model.m_iq0 = m_model.AddDAEVariables(true, Ninlet, 0); // Initial PSD

	/// Set tolerances to model ///
	const auto rtol = GetConstRealParameterValue("Relative tolerance");
	const auto atol = GetConstRealParameterValue("Absolute tolerance");
	m_model.SetTolerance(rtol != 0.0 ? rtol : GetRelTolerance(), atol != 0.0 ? atol : GetAbsTolerance());

	/// Set model to a solver ///
	const double maxStep = GetConstRealParameterValue("Step");
	if (maxStep != 0.0)
		m_solver.SetMaxStep(maxStep);
	if (!m_solver.SetModel(&m_model))
		RaiseError(m_solver.GetError());

	/// Initialize agglomeration calculator ///
	m_aggSolver = GetSolverAgglomeration("Solver");
	if (!m_aggSolver)
	{
		RaiseError("Cannot load Solver");
		return;
	}
	/// Set parameters ///
	m_aggSolver->Initialize(m_sizeGrid, GetConstRealParameterValue("Beta0"),
		V2E<CAgglomerationSolver::EKernels>(GetComboParameterValue("Kernel")),
		{ static_cast<double>(GetConstUIntParameterValue("Rank")) });
}

void CAgglomerator::SaveState()
{
	m_solver.SaveState();
}

void CAgglomerator::LoadState()
{
	m_solver.LoadState();
}

void CAgglomerator::Simulate(double _timeBeg, double _timeEnd)
{
	if (!m_solver.Calculate(_timeBeg, _timeEnd))
		RaiseError(m_solver.GetError());
}

void CUnitDAEModel::ResultsHandler(double _time, double* _vars, double* _ders, void* _unit)
{
	auto* unit = static_cast<CAgglomerator*>(_unit);

	unit->m_holdup->AddTimePoint(_time);

	const double holdupMass = unit->m_holdup->GetMass(_time);
	unit->m_holdup->AddStream(std::max(unit->m_inStream->GetPreviousTimePoint(_time), unit->m_holdup->GetPreviousTimePoint(_time)), _time, unit->m_inStream);
	unit->m_holdup->RemoveTimePointsAfter(_time);
	unit->m_holdup->SetMass(_time, holdupMass);

	unit->m_holdup->SetPSD(_time, PSD_MassFrac, ConvertNumbersToMassFractions(unit->m_sizeGrid, Slice(_vars, m_iq0)));

	const double outMass = unit->m_inStream->GetMassFlow(_time); // equal to in mass flow
	unit->m_outStream->CopyFromHoldup(_time, unit->m_holdup, outMass);
}

void CUnitDAEModel::CalculateResiduals(double _time, double* _vars, double* _ders, double* _res, void* _unit)
{
	const auto* unit = static_cast<CAgglomerator*>(_unit);

	const double inMass = unit->m_inStream->GetMassFlow(_time);
	const double holdupMass = unit->m_holdup->GetMass(_time);
	const double outMass = inMass;

	std::vector<double> Ninlet = unit->m_inStream->GetPSD(_time, PSD_Number);

	// Call agglomeration function
	auto [BRate, DRate] = unit->m_aggSolver->Calculate(std::vector<double>(_vars, _vars + unit->m_classesNum));

	// Calculate derivatives
	for (size_t i = 0; i < unit->m_classesNum; ++i)
	{
		const double der = BRate[i] - DRate[i] + Ninlet[i] - _vars[i] / holdupMass * outMass;
		_res[m_iq0[i]] = _ders[m_iq0[i]] - der;
	}
}
