/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "TimeDelay.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CTimeDelay();
}

//////////////////////////////////////////////////////////////////////////
/// Unit

void CTimeDelay::CreateBasicInfo()
{
	/// Basic unit's info ///
	SetUnitName("Time delay");
	SetAuthorName("SPE TUHH");
	SetUniqueID("56D734DFB1EA441B859C2C70D6F43BBC");
}

void CTimeDelay::CreateStructure()
{
	/// Add ports ///
	AddPort("In", EUnitPort::INPUT);
	AddPort("Out", EUnitPort::OUTPUT);

	/// Add unit parameters ///
	AddComboParameter("Model", EModel::NORM_BASED, { EModel::NORM_BASED, EModel::SIMPLE_SHIFT }, { "Norm-based", "Simple shift" }, "Time delay model");
	AddConstRealParameter("Time delay"        , 0.0, "s", "Unit time delay"                                                , 0, 1e+6);
	AddConstRealParameter("Relative tolerance", 0.0, "-", "Solver relative tolerance. Set to 0 to use flowsheet-wide value", 0      );
	AddConstRealParameter("Absolute tolerance", 0.0, "-", "Solver absolute tolerance. Set to 0 to use flowsheet-wide value", 0      );

	AddParametersToGroup("Model", "Norm-based", { "Relative tolerance", "Absolute tolerance" });
}

void CTimeDelay::Initialize(double _time)
{
	m_inlet  = GetPortStream("In");
	m_outlet = GetPortStream("Out");

	m_timeDelay = GetConstRealParameterValue("Time delay");
	if (m_timeDelay < 0)
		RaiseError("Parameter 'Time delay' may not be negative.");

	m_model = static_cast<EModel>(GetComboParameterValue("Model"));

	switch (m_model)
	{
	case NORM_BASED:   InitializeNormBased(_time);   break;
	case SIMPLE_SHIFT: InitializeSimpleShift(_time); break;
	}
}

void CTimeDelay::Simulate(double _timeBeg, double _timeEnd)
{
	switch (m_model)
	{
	case NORM_BASED:   SimulateNormBased(_timeBeg, _timeEnd);   break;
	case SIMPLE_SHIFT: SimulateSimpleShift(_timeBeg, _timeEnd); break;
	}
}

void CTimeDelay::SaveState()
{
	switch (m_model)
	{
	case NORM_BASED:   SaveStateNormBased();   break;
	case SIMPLE_SHIFT: SaveStateSimpleShift(); break;
	}
}

void CTimeDelay::LoadState()
{
	switch (m_model)
	{
	case NORM_BASED:   LoadStateNormBased();   break;
	case SIMPLE_SHIFT: LoadStateSimpleShift(); break;
	}
}

void CTimeDelay::InitializeSimpleShift(double _time)
{
	m_stream = AddStream("stream");
}

void CTimeDelay::SimulateSimpleShift(double _timeBeg, double _timeEnd) const
{
	// store inlet internally
	m_stream->CopyFromStream(_timeBeg, _timeEnd, m_inlet);
	// get all time points on shifted time interval
	std::vector<double> tp_shifted = m_stream->GetTimePointsClosed(_timeBeg - m_timeDelay, _timeEnd - m_timeDelay);
	// remove possible negative time points
	while (!tp_shifted.empty() && tp_shifted.front() < 0)
		tp_shifted.erase(tp_shifted.begin());

	// always create time point 0
	if (_timeBeg == 0.0)
	{
		m_outlet->CopyFromStream(0.0, m_stream);
		if (m_timeDelay != 0.0)
			m_outlet->SetMassFlow(0.0, 0.0);
	}

	// create additional point close to the first active one for proper interpolation
	if (!tp_shifted.empty() && m_timeDelay != 0.0)
	{
		const auto t1 = m_timeDelay - m_timeDelay / 100;
		m_outlet->CopyFromStream(t1, m_stream, 0.0);
		m_outlet->SetMassFlow(t1, 0.0);
	}

	// set outlet data
	for (const double t : tp_shifted)
		m_outlet->CopyFromStream(t + m_timeDelay, m_stream, t);
}

void CTimeDelay::SaveStateSimpleShift()
{
}

void CTimeDelay::LoadStateSimpleShift()
{
}

void CTimeDelay::InitializeNormBased(double _time)
{
	m_compoundsNum  = GetCompoundsNumber();
	m_phasesNum     = GetPhasesNumber();
	m_distrsNum     = GetDistributionsNumber();
	m_compounds     = GetAllCompounds();
	m_phases        = GetAllPhases();
	m_distributions = GetDistributionsTypes();

	/// Clear all state variables in model ///
	m_DAEModel.ClearVariables();

	/// Add state variables to the model ///
	m_DAEModel.m_iMflow              = m_DAEModel.AddDAEVariable(true, 0, 0, 0);
	m_DAEModel.m_iNormMflow          = m_DAEModel.AddDAEVariable(true, 1, 0, 0);
	m_DAEModel.m_iNormT              = m_DAEModel.AddDAEVariable(true, 1, 0, 0);
	m_DAEModel.m_iNormP              = m_DAEModel.AddDAEVariable(true, 1, 0, 0);
	m_DAEModel.m_iNormPhases         = m_DAEModel.AddDAEVariable(true, 1, 0, 0);
	m_DAEModel.m_iNormPhaseCompounds = m_DAEModel.AddDAEVariables(true, std::vector<double>(m_phasesNum, 1), 0, 0);
	m_DAEModel.m_iNormDistr          = m_DAEModel.AddDAEVariables(true, std::vector<double>(m_distrsNum, 1), 0, 0);

	/// Set tolerances to the model ///
	const auto rtol = GetConstRealParameterValue("Relative tolerance");
	const auto atol = GetConstRealParameterValue("Absolute tolerance");
	m_DAEModel.SetTolerance(rtol != 0.0 ? rtol : GetRelTolerance(), atol != 0.0 ? atol : GetAbsTolerance());

	/// Set model to the solver ///
	if (!m_DAESolver.SetModel(&m_DAEModel))
		RaiseError(m_DAESolver.GetError());

	/// Set this unit as user data of model ///
	m_DAEModel.SetUserData(this);
}

void CTimeDelay::SimulateNormBased(double _timeBeg, double _timeEnd)
{
	/// Run solver ///
	if (!m_DAESolver.Calculate(_timeBeg, _timeEnd))
		RaiseError(m_DAESolver.GetError());
}

void CTimeDelay::SaveStateNormBased()
{
	/// Save solver's state ///
	m_DAESolver.SaveState();
}

void CTimeDelay::LoadStateNormBased()
{
	/// Load solver's state ///
	m_DAESolver.LoadState();
}

//////////////////////////////////////////////////////////////////////////
/// Solver

void CMyDAEModel::ResultsHandler(double _time, double* _vars, double* _ders, void* _unit)
{
	/// General information ///
	const auto* unit = static_cast<CTimeDelay*>(_unit);

	/// Time delay of information ///
	unit->m_outlet->CopyFromStream(_time, unit->m_inlet, _time - unit->m_timeDelay);
	unit->m_outlet->SetMassFlow(_time, _vars[m_iMflow]);
}

void CMyDAEModel::CalculateResiduals(double _time, double* _vars, double* _ders, double* _res, void* _unit)
{
	/// General information ///
	// Pointer to unit
	const auto* unit = static_cast<CTimeDelay*>(_unit);
	// Previous time point of inlet stream
	const double timePrev = unit->m_inlet->GetPreviousTimePoint(_time);

	/// MTP values ///
	// Mass flows at last and current time points
	const double MflowPrev = unit->m_inlet->GetMassFlow(timePrev);
	const double Mflow     = unit->m_inlet->GetMassFlow(_time);
	const double normMflow_update = std::pow(Mflow - MflowPrev, 2);
	// Temperatures at last and current time points
	const double TPrev = unit->m_inlet->GetTemperature(timePrev);
	const double T     = unit->m_inlet->GetTemperature(_time);
	const double normT_update = std::pow(T - TPrev, 2);
	// Pressures at last and current time points
	const double PPrev = unit->m_inlet->GetPressure(timePrev);
	const double P     = unit->m_inlet->GetPressure(_time);
	const double normP_update = std::pow(P - PPrev, 2);

	/// Phase & phase compound fractions ///
	// Initialize norm for phase fractions
	double normPhases_update = 0;
	// Declare norm vector for phase compound fractions
	std::vector<double> normPhaseCompounds_update(unit->m_compoundsNum, 0);
	for (size_t i = 0; i < unit->m_phasesNum; ++i)
	{
		// Get phase fraction at last time point
		const double tempPhaseFracPrev = unit->m_inlet->GetPhaseFraction(timePrev, unit->m_phases[i]);
		// Get phase fraction at current time point
		const double tempPhaseFrac = unit->m_inlet->GetPhaseFraction(_time, unit->m_phases[i]);
		// Squared difference of phase fractions
		normPhases_update += std::pow(tempPhaseFrac - tempPhaseFracPrev, 2);

		// Loop for phase compound fractions
		for (const auto& compound : unit->m_compounds)
		{
			// Phase compound fractions at last time point
			const double tempCompPhaseFracPrev = unit->m_inlet->GetCompoundFraction(timePrev, compound, unit->m_phases[i]);
			// Phase compound fractions at current time point
			const double tempCompPhaseFrac = unit->m_inlet->GetCompoundFraction(_time, compound, unit->m_phases[i]);
			// Squared difference of phase compound fractions
			normPhaseCompounds_update[i] += std::pow(tempCompPhaseFracPrev - tempCompPhaseFrac, 2);
		}

		// Root of sum for 2-norm calculation for phase compound fractions
		normPhaseCompounds_update[i] = std::sqrt(normPhaseCompounds_update[i]);
	}
	// Root of sum for 2-norm calculation for phase fractions
	normPhases_update = std::sqrt(normPhases_update);

	/// Multidimensional distributions ///
	// Declare norm vector for distributions
	std::vector<double> normDistr_update(unit->m_distrsNum);
	for (size_t i = 0; i < unit->m_distrsNum; ++i)
	{
		// Distribution vector at previous time point
		const std::vector<double> distrPrev = unit->m_inlet->GetDistribution(timePrev, unit->m_distributions[i]);
		// Distribution vector at current time point
		const std::vector<double> distr = unit->m_inlet->GetDistribution(_time, unit->m_distributions[i]);
		// Calculate squared sum of differences between distribution vectors
		for (size_t j = 0; j < distr.size(); ++j)
			normDistr_update[i] += std::pow(distr[j] - distrPrev[j], 2);
		// Root of sum for 2-norm calculation for distributions
		normDistr_update[i] = std::sqrt(normDistr_update[i]);
	}

	/// Calculate residuals ///
	// No change if time is smaller than the time delay of the unit
	if (_time < unit->m_timeDelay)
	{
		_res[m_iMflow]      = _ders[m_iMflow];
		_res[m_iNormMflow]  = _ders[m_iNormMflow];
		_res[m_iNormT]      = _ders[m_iNormT];
		_res[m_iNormP]      = _ders[m_iNormP];
		_res[m_iNormPhases] = _ders[m_iNormPhases];
		for (size_t i = 0; i < unit->m_phasesNum; ++i) _res[m_iNormPhaseCompounds + i] = _ders[m_iNormPhaseCompounds + i];
		for (size_t i = 0; i < unit->m_distrsNum; ++i) _res[m_iNormDistr          + i] = _ders[m_iNormDistr          + i];
	}
	// Residuals of the derivatives equal the difference in the respective norms from from the last value of the
	else
	{
		_res[m_iMflow]      = _ders[m_iMflow]      - (unit->m_inlet->GetMassFlow(_time - unit->m_timeDelay) - _vars[m_iMflow]);
		_res[m_iNormMflow]  = _ders[m_iNormMflow]  - (normMflow_update  - _vars[m_iNormMflow]);
		_res[m_iNormT]      = _ders[m_iNormT]      - (normT_update      - _vars[m_iNormT]);
		_res[m_iNormP]      = _ders[m_iNormP]      - (normP_update      - _vars[m_iNormP]);
		_res[m_iNormPhases] = _ders[m_iNormPhases] - (normPhases_update - _vars[m_iNormPhases]);
		for (size_t i = 0; i < unit->m_phasesNum; ++i) _res[m_iNormPhaseCompounds + i] = _ders[m_iNormPhaseCompounds + i] - (normPhaseCompounds_update[i] - _vars[m_iNormPhaseCompounds + i]);
		for (size_t i = 0; i < unit->m_distrsNum; ++i) _res[m_iNormDistr          + i] = _ders[m_iNormDistr          + i] - (normDistr_update[i]          - _vars[m_iNormDistr          + i]);
	}
}
