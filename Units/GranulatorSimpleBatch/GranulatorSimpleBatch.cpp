/* Copyright (c) 2022, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "GranulatorSimpleBatch.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CGranulatorSimpleBatch();
}

//////////////////////////////////////////////////////////////////////////
/// Batch granulator with a suspension inlet and a fluidization gas inlet, and an outlet for exhaust gas
/// - Without heat and mass transfer
/// - Overspray rate set to zero, i.e. no spray drying and no solid dust in outlet
/// - All liquid going into the granulator will escape as exhaust gas
void CGranulatorSimpleBatch::CreateBasicInfo()
{
	/// Set basic unit info ///
	SetUnitName("Granulator simple batch");
	SetAuthorName("Xiye Zhou");
	SetUniqueID("2FAC6E6F-77D1-46EF-B28F-32D9F461976D");
}

void CGranulatorSimpleBatch::CreateStructure()
{
	/// Add ports ///
	AddPort("Suspension"      , EUnitPort::INPUT);
	AddPort("FluidizationGas" , EUnitPort::INPUT);
	AddPort("ExhaustGasOutput", EUnitPort::OUTPUT);

	/// Add unit parameters ///
	AddConstRealParameter("Relative tolerance", 0.0, "-", "Solver relative tolerance. Set to 0 to use flowsheet-wide value", 0);
	AddConstRealParameter("Absolute tolerance", 0.0, "-", "Solver absolute tolerance. Set to 0 to use flowsheet-wide value", 0);

	/// Add holdups ///
	AddHoldup("Holdup");

	/// Set this unit as user data of model ///
	m_model.SetUserData(this);
}

void CGranulatorSimpleBatch::Initialize(double _time)
{
	/// Check flowsheet parameters ///
	if (!IsPhaseDefined(EPhase::SOLID))		RaiseError("Solid phase has not been defined.");
	if (!IsPhaseDefined(EPhase::LIQUID))	RaiseError("Liquid phase has not been defined.");
	if (!IsPhaseDefined(EPhase::VAPOR))		RaiseError("Gas phase has not been defined.");
	if (!IsDistributionDefined(DISTR_SIZE))	RaiseError("Size distribution has not been defined.");

	/// Get holdup ///
	m_holdup = GetHoldup("Holdup");

	/// Get pointers to streams ///
	m_inSuspStream        = GetPortStream("Suspension");
	m_inGasStream         = GetPortStream("FluidizationGas");
	m_outExhaustGasStream = GetPortStream("ExhaustGasOutput");

	/// Get grid data for PSD ///
	m_classesNum = GetClassesNumber(DISTR_SIZE);	//n
	m_sizeGrid   = GetNumericGrid(DISTR_SIZE);		//d_min
	m_avgDiam    = GetClassesMeans(DISTR_SIZE);		//d_m,i
	m_classSize  = GetClassesSizes(DISTR_SIZE);		//Delta d

	/// Get initial PSD ///
	const std::vector<double> q3  = m_holdup->GetPSD(_time, PSD_q3);		// Mass-related density distribution
	const std::vector<double> PSD = m_holdup->GetPSD(_time, PSD_MassFrac);	// PSD as mass fractions

	/// Get initial mass in holdup ///
	const double initMass = m_holdup->GetMass(_time);

	/// Calculate initial diameter ratio for calculating G ///
	m_diamRatio.clear();
	m_diamRatio.push_back(0);
	for (size_t i = 1; i < m_classesNum; ++i)
	{
		m_diamRatio.push_back(std::pow((m_sizeGrid[i] + m_sizeGrid[i + 1]) / (m_sizeGrid[i - 1] + m_sizeGrid[i]), 3)); //at least 3 size grids, i.e. 2 classNums
	}

	/// Clear all state variables in model ///
	m_model.ClearVariables();

	/// Precalulate initial values for DAE
	const double suspSolDens = m_inSuspStream->GetPhaseProperty(_time, EPhase::SOLID, DENSITY);	// Density of the solid in the solution
	// Total surface of all particles (ATot)
	double temp = 0;
	for (size_t i = 0; i < m_classesNum; ++i)
		temp += q3[i] * m_classSize[i] / m_avgDiam[i];
	const double initATot = initMass * 6 * temp / suspSolDens;
	// Initial product Mtot* q3
	std::vector<double> initMTotq3 = ReservedVector<double>(m_classesNum);
	for (const double v : q3)
		initMTotq3.push_back(initMass * v);

	/// Add state variables to a model ///
	m_model.m_iAtot     = m_model.AddDAEVariable (false, initATot  , 0, 0  );	// Total surface of all particles in the Granulator
	m_model.m_iMtot     = m_model.AddDAEVariable (true , initMass  , 0, 0  );	// Total mass of all particles in the Granulator
	m_model.m_iMFlowExh = m_model.AddDAEVariable (false, 0         , 0, 0  );	// Output mass flow of exhaust gas
	m_model.m_iG        = m_model.AddDAEVariable (false, 1e-8      , 0, 0  );	// Growth rate
	m_model.m_iMtotq3   = m_model.AddDAEVariables(true , initMTotq3, 0, 1.0);	// Product Mtot*q3

	AddStateVariable("Atot"       , initATot);
	AddStateVariable("Mtot"       , initMass);
	AddStateVariable("MExhaustGas", 0);
	AddStateVariable("G"          , 1e-8);
	for (size_t i = 0; i < m_classesNum; ++i)
		AddStateVariable("q3 of " + std::to_string(m_avgDiam[i] * 1e6) + " mu", q3[i]);
	for (size_t i = 0; i < m_classesNum; ++i)
		AddStateVariable("Mass fraction of " + std::to_string(m_avgDiam[i] * 1e6) + " mu", PSD[i]);

	/// Set tolerances to model ///
	const auto rtol = GetConstRealParameterValue("Relative tolerance");
	const auto atol = GetConstRealParameterValue("Absolute tolerance");
	m_model.SetTolerance(rtol != 0.0 ? rtol : GetRelTolerance(), atol != 0.0 ? atol : GetAbsTolerance());

	/// Set model to a solver ///
	if (!m_solver.SetModel(&m_model))
		RaiseError(m_solver.GetError());
}

void CGranulatorSimpleBatch::SaveState()
{
	m_solver.SaveState();
}

void CGranulatorSimpleBatch::LoadState()
{
	m_solver.LoadState();
}

void CGranulatorSimpleBatch::Simulate(double _timeBeg, double _timeEnd)
{
	if (!m_solver.Calculate(_timeBeg, _timeEnd))
		RaiseError(m_solver.GetError());
}

void CUnitDAEModel::CalculateResiduals(double _time, double* _vars, double* _ders, double* _res, void* _unit)
{
	const auto* unit = static_cast<CGranulatorSimpleBatch*>(_unit);

	const double ATot            = _vars[m_iAtot];
	const double mFlowExhaustGas = _vars[m_iMFlowExh];
	const double G               = _vars[m_iG];

	const double totGasMassFlow  = unit->m_inGasStream->GetMassFlow(_time);								// Mass flow of fluidization gas
	const double mFlowSuspSolid  = unit->m_inSuspStream->GetPhaseMassFlow(_time, EPhase::SOLID);		// Mass flow of solid phase in solution
	const double mFlowSuspLiquid = unit->m_inSuspStream->GetMassFlow(_time) - mFlowSuspSolid;			// Mass flow of all phases except solid in solution
	const double mEff = mFlowSuspSolid;																	// Effective mass stream of the injected solution
	const double suspSolDens = unit->m_inSuspStream->GetPhaseProperty(_time, EPhase::SOLID, DENSITY);	// Density of the solid in the solution

	// Total surface of all particles (ATot)
	double sum = 0;
	for (size_t i = 0; i < unit->m_classesNum; ++i)
	{
		sum += _vars[m_iMtotq3[i]] * unit->m_classSize[i] / unit->m_avgDiam[i];
	}
	const double varAtot = 6 * sum / suspSolDens;
	_res[m_iAtot] = ATot - varAtot;

	// Total mass of all particles (MTot)
	const double derMtot = mEff;
	_res[m_iMtot] = _ders[m_iMtot] - derMtot;

	// Output mass flow of exhaust gas (MFlowExhaustGas)
	const double varMFlowExhaustGas = mFlowSuspLiquid + totGasMassFlow;
	_res[m_iMFlowExh] = mFlowExhaustGas - varMFlowExhaustGas;

	// Growth rate (G)
	const double varG = 2 * mEff / (suspSolDens * ATot);
	if (ATot != 0.0)
	{
		_res[m_iG] = G - varG;
	}
	else
	{
		_res[m_iG] = 0;
	}

	// Product (Mtot*q3)
	const double derMtotq3_0 = -G / unit->m_classSize[0] * _vars[m_iMtotq3[0]];
	_res[m_iMtotq3[0]] = _ders[m_iMtotq3[0]] - derMtotq3_0;
	for (size_t i = 1; i < unit->m_classesNum; ++i)
	{
		const double derMtotq3 = -G / unit->m_classSize[i] * (_vars[m_iMtotq3[i]] - _vars[m_iMtotq3[i - 1]] * unit->m_diamRatio[i]);
		_res[m_iMtotq3[i]] = _ders[m_iMtotq3[i]] - derMtotq3;
	}
}

void CUnitDAEModel::ResultsHandler(double _time, double* _vars, double* _ders, void* _unit)
{
	auto* unit = static_cast<CGranulatorSimpleBatch*>(_unit);

	unit->m_holdup->AddTimePoint(_time);

	std::vector<double> q3(unit->m_classesNum);
	for (size_t i = 0; i < unit->m_classesNum; ++i)
	{
		q3[i] = _vars[m_iMtotq3[i]] / _vars[m_iMtot];
	}
	const std::vector<double> PSD = Convertq3ToMassFractions(unit->m_sizeGrid, q3);

	unit->m_holdup->SetMass(_time, _vars[m_iMtot]);
	unit->m_holdup->SetPSD(_time, PSD_q3, q3);

	unit->m_outExhaustGasStream->CopyFromStream(_time, unit->m_inGasStream);
	unit->m_outExhaustGasStream->SetMassFlow(_time, _vars[m_iMFlowExh]);
	unit->m_outExhaustGasStream->SetPhaseFraction(_time, EPhase::SOLID, 0);
	unit->m_outExhaustGasStream->SetPhaseFraction(_time, EPhase::LIQUID, 0);
	unit->m_outExhaustGasStream->SetPhaseFraction(_time, EPhase::VAPOR, 1);

	unit->SetStateVariable("Atot"       , _vars[m_iAtot]    , _time);
	unit->SetStateVariable("Mtot"       , _vars[m_iMtot]    , _time);
	unit->SetStateVariable("MExhaustGas", _vars[m_iMFlowExh], _time);
	unit->SetStateVariable("G"          , _vars[m_iG]       , _time);
	for (size_t i = 0; i < unit->m_classesNum; ++i)
	{
		unit->SetStateVariable("q3 of " + std::to_string(unit->m_avgDiam[i] * 1e6) + " mu", q3[i], _time);
		unit->SetStateVariable("Mass fraction of " + std::to_string(unit->m_avgDiam[i] * 1e6) + " mu", PSD[i], _time);
	}
}
