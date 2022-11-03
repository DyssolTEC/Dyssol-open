/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Bunker.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CBunker();
}

//////////////////////////////////////////////////////////////////////////
/// Unit

void CBunker::CreateBasicInfo()
{
	/// Basic unit's info ///
	SetUnitName("Solids bunker");
	SetAuthorName("SPE TUHH");
	SetUniqueID("A5D7F41322C949EC86C96C583A35501F");
}

void CBunker::CreateStructure()
{
	/// Add ports ///
	AddPort("Inflow", EUnitPort::INPUT);
	AddPort("Outflow", EUnitPort::OUTPUT);

	/// Add unit parameters ///
	m_upModel = AddComboParameter("Model", EModel::Adaptive, { EModel::Adaptive, EModel::Constant }, { "Adaptive", "Constant" }, "Outflow model");
	m_upMassFlow   = AddTDParameter       ("Output mass flow"  , 1     , "kg/s", "Output mass flow"                                               , 0.0);
	m_upTargetMass = AddConstRealParameter("Target mass"       , 100000, "kg"  , "Target mass of bunker."                                         , 0.0);
	m_upRTol       = AddConstRealParameter("Relative tolerance", 0.0   , "-"   , "Solver relative tolerance. Set to 0 to use flowsheet-wide value", 0.0);
	m_upATol       = AddConstRealParameter("Absolute tolerance", 0.0   , "-"   , "Solver absolute tolerance. Set to 0 to use flowsheet-wide value", 0.0);

	// group unit parameters
	AddParametersToGroup("Model", "Constant", { m_upMassFlow->GetName() });

	/// Add holdups ///
	m_holdup = AddHoldup("Holdup");

	/// Add internal streams ///
	m_inSolid  = AddStream("InflowSolid");
	m_inBypass = AddStream("InflowBypass");

	/// Set this unit as user data of model ///
	m_model.SetUserData(this);
}

void CBunker::Initialize(double _time)
{
	/// Check presence of solid phase ///
	if (!IsPhaseDefined(EPhase::SOLID))
		RaiseError("Solid phase has not been defined.");

	// unit parameters
	m_outputModel = static_cast<EModel>(m_upModel->GetValue());
	m_targetMass  = m_upTargetMass->GetValue();

	// Pointers to streams
	m_inlet  = GetPortStream("Inflow");
	m_outlet = GetPortStream("Outflow");

	// Flowsheet settings
	m_compoundsNum  = GetCompoundsNumber();
	m_distrsNum     = GetDistributionsNumber();
	m_distributions = GetDistributionsTypes();

	/// Prepare holdup ///
	const std::vector<double> timePoints = m_holdup->GetAllTimePoints();
	if (timePoints.empty())
	{
		RaiseError("No initial state of bunker at time point 't = 0' found.");
		return;
	}
	if (timePoints[0] != 0.0)
	{
		RaiseError("No initial state of bunker at time point 't = 0' found.");
		return;
	}
	if (timePoints.size() > 1)
	{
		RaiseWarning("Neglecting all time points of bunker after 't = 0'.");
		m_holdup->RemoveTimePointsAfter(_time);
	}

	if (m_holdup->GetPhaseFraction(_time, EPhase::SOLID) != 1.0)
	{
		RaiseWarning("The holdup of the bunker model can only contain solids. Removing all other phases from the bunker holdup.");
		m_holdup->SetMass(_time, m_holdup->GetPhaseMass(_time, EPhase::SOLID));
		for (const auto& p : GetAllPhases())
			m_holdup->SetPhaseFraction(_time, p, p == EPhase::SOLID ? 1.0 : 0.0);
	}

	/// Clear all state variables in model ///
	m_model.ClearVariables();

	/// Add state variables to the model ///
	const double initMass = m_holdup->GetMass(_time);

	m_model.m_iMass          = m_model.AddDAEVariable(true , initMass, 0, 1.0);
	m_model.m_iMflowOut      = m_model.AddDAEVariable(false, 0       , 0, 1.0);
	m_model.m_iNormMflow     = m_model.AddDAEVariable(true , 1       , 0, 0.0);
	m_model.m_iNormT         = m_model.AddDAEVariable(true , 1       , 0, 0.0);
	m_model.m_iNormP         = m_model.AddDAEVariable(true , 1       , 0, 0.0);
	m_model.m_iNormCompounds = m_model.AddDAEVariable(true , 1       , 0, 0.0);
	m_model.m_iNormDistr     = m_model.AddDAEVariables(true, std::vector(m_distrsNum, 1.0), 0, 0);

	/// Set tolerances to the model ///
	const auto rtol = m_upRTol->GetValue();
	const auto atol = m_upATol->GetValue();
	m_model.SetTolerance(rtol != 0.0 ? rtol : GetRelTolerance(), atol != 0.0 ? atol : GetAbsTolerance());

	/// Set model to the solver ///
	if (!m_solver.SetModel(&m_model))
		RaiseError(m_solver.GetError());
}

void CBunker::Simulate(double _timeBeg, double _timeEnd)
{
	m_inSolid->RemoveTimePointsAfter(_timeBeg);
	m_inBypass->RemoveTimePointsAfter(_timeBeg);

	/// Bypass non solid phases ///
	for (const double time : m_inlet->GetTimePointsClosed(_timeBeg, _timeEnd))
	{
		m_inSolid->CopyFromStream(time, m_inlet);
		m_inSolid->SetMassFlow(time, m_inlet->GetPhaseMassFlow(time, EPhase::SOLID));
		for (const auto& p : GetAllPhases())
			m_inSolid->SetPhaseFraction(time, p, p == EPhase::SOLID ? 1.0 : 0.0);

		m_inBypass->CopyFromStream(time, m_inlet);
		m_inBypass->SetPhaseMassFlow(time, EPhase::SOLID, 0.0);
	}

	/// Run solver ///
	// iterate over all input time point to properly react on all signal changes
	const auto allTP = GetAllTimePointsClosed(_timeBeg, _timeEnd);
	for (size_t i = 0; i < allTP.size() - 1; ++i)
	{
		m_solver.SetMaxStep(0.25 * (allTP[i + 1] - allTP[i]));
		if (!m_solver.Calculate(allTP[i], allTP[i + 1]))
			RaiseError(m_solver.GetError());
	}
}

void CBunker::SaveState()
{
	/// Save solver's state ///
	m_solver.SaveState();
}

void CBunker::LoadState()
{
	/// Load solver's state ///
	m_solver.LoadState();
}

//////////////////////////////////////////////////////////////////////////
/// Solver

void CMyDAEModel::ResultsHandler(double _time, double* _vars, double* _ders, void* _unit)
{
	/// General information ///
	const auto* unit = static_cast<CBunker*>(_unit);

	if (unit->m_outputModel == CBunker::EModel::Constant && _vars[m_iMass] > unit->m_targetMass)
		static_cast<CBunker*>(_unit)->RaiseError("Bunker overflow at t = " + std::to_string(_time) + "s!");

	const double timePrev = unit->m_holdup->GetPreviousTimePoint(_time);

	unit->m_holdup->AddStream(timePrev, _time, unit->m_inSolid);
	unit->m_holdup->SetMass(_time, _vars[m_iMass]);

	unit->m_outlet->CopyFromHoldup(_time, unit->m_holdup, _vars[m_iMflowOut]);
	unit->m_outlet->AddStream(_time, unit->m_inBypass);
}

void CMyDAEModel::CalculateResiduals(double _time, double* _vars, double* _ders, double* _res, void* _unit)
{
	// Pointer to unit
	const auto* unit = static_cast<CBunker*>(_unit);

	// Previous time point of inlet stream
	const double timePrev = unit->m_inSolid->GetPreviousTimePoint(_time);

	/// Inflow ///
	const double MflowIn = unit->m_inSolid->GetMassFlow(_time);

	/// Outflow ///
	const double MflowOut = _vars[m_iMflowOut];

	/// MTP values ///
	// Bunker mass
	const double massBunker = _vars[m_iMass];
	// Mass flows at last and current time points
	const double MflowPrev       = unit->m_inSolid->GetMassFlow(timePrev);
	const double MflowCurr       = unit->m_inSolid->GetMassFlow(_time);
	const double normMflowUpdate = std::pow(MflowCurr - MflowPrev, 2);
	// Temperatures at last and current time points
	const double TPrev       = unit->m_inSolid->GetTemperature(timePrev);
	const double TCurr       = unit->m_inSolid->GetTemperature(_time);
	const double normTUpdate = std::pow(TCurr - TPrev, 2);
	// Pressures at last and current time points
	const double PPrev       = unit->m_inSolid->GetPressure(timePrev);
	const double PCurr       = unit->m_inSolid->GetPressure(_time);
	const double normPUpdate = std::pow(PCurr - PPrev, 2);

	/// Compound fractions ///
	// Declare norm vector for phase compound fractions
	double normCompoundsUpdate = 0.0;
	// Loop for phase compound fractions
	for (const auto& compound : unit->GetAllCompounds())
	{
		// Phase compound fractions at last time point
		const double compPhaseFracPrev = unit->m_inSolid->GetCompoundFraction(timePrev, compound, EPhase::SOLID);
		// Phase compound fractions at current time point
		const double compPhaseFracCurr = unit->m_inSolid->GetCompoundFraction(_time, compound, EPhase::SOLID);
		// Squared difference of phase compound fractions
		normCompoundsUpdate += std::pow(compPhaseFracCurr - compPhaseFracPrev, 2);
	}
	// Root of sum for 2-norm calculation for phase compound fractions
	normCompoundsUpdate = std::sqrt(normCompoundsUpdate);

	/// Multidimensional distributions ///
	// Declare norm vector for distributions
	std::vector<double> normDistrUpdate(unit->m_distrsNum);
	for (size_t i = 0; i < unit->m_distrsNum; ++i)
	{
		// Distribution vector at previous time point
		std::vector<double> distrPrev = unit->m_inSolid->GetDistribution(timePrev, unit->m_distributions[i]);
		// Distribution vector at current time point
		std::vector<double> distrCurr = unit->m_inSolid->GetDistribution(_time, unit->m_distributions[i]);
		// Calculate squared sum of differences between distribution vectors
		for (size_t j = 0; j < distrCurr.size(); ++j)
			normDistrUpdate[i] += std::pow(distrCurr[j] - distrPrev[j], 2);
		// Root of sum for 2-norm calculation for distributions
		normDistrUpdate[i] = std::sqrt(normDistrUpdate[i]);
	}

	/// Calculate residuals ///
	// Bunker mass
	_res[m_iMass] = _ders[m_iMass] - (MflowIn - MflowOut);

	// Outflow
	switch(unit->m_outputModel)
	{
		case CBunker::EModel::Adaptive: // Adaptive model
		{
			const double MFlowOut = std::pow(2 * massBunker / (massBunker + unit->m_targetMass), 2) * MflowIn;
			_res[m_iMflowOut] = _vars[m_iMflowOut] - MFlowOut;
			break;
		}
		case CBunker::EModel::Constant: // Constant model
		{
			const double mass_flow_requested = unit->m_upMassFlow->GetValue(_time);
			const auto dT = _time - unit->m_holdup->GetPreviousTimePoint(_time);

			// smoothing function
			const double smooth = std::max(std::tanh(50 * (massBunker - mass_flow_requested * dT)), 0.0);
			const double MFlowOut = std::max(smooth * mass_flow_requested + (1 - smooth) * std::min(MflowIn, mass_flow_requested), 0.0);

			_res[m_iMflowOut] = _vars[m_iMflowOut] - MFlowOut;

			break;
		}
	}

	// Residuals of the derivatives equal the difference in the respective norms
	_res[m_iNormMflow]     = _ders[m_iNormMflow]     - (normMflowUpdate     - _vars[m_iNormMflow]);
	_res[m_iNormT]         = _ders[m_iNormT]         - (normTUpdate         - _vars[m_iNormT]);
	_res[m_iNormP]         = _ders[m_iNormP]         - (normPUpdate         - _vars[m_iNormP]);
	_res[m_iNormCompounds] = _ders[m_iNormCompounds] - (normCompoundsUpdate - _vars[m_iNormCompounds]);

	for (size_t i = 0; i < unit->m_distrsNum; ++i)
		_res[m_iNormDistr + i] = _ders[m_iNormDistr + i] - (normDistrUpdate[i] - _vars[m_iNormDistr + i]);
}
