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
	AddConstRealParameter("Time delay"        , 0.0, "s", "Unit time delay"                                                , 0, 1e+6);
	AddConstRealParameter("Relative tolerance", 0.0, "-", "Solver relative tolerance. Set to 0 to use flowsheet-wide value", 0      );
	AddConstRealParameter("Absolute tolerance", 0.0, "-", "Solver absolute tolerance. Set to 0 to use flowsheet-wide value", 0      );

	/// Set this unit as user data of model ///
	m_model.SetUserData(this);
}

void CTimeDelay::Initialize(double _time)
{
	m_timeDelay = GetConstRealParameterValue("Time delay");
	if (m_timeDelay < 0)
		RaiseError("Parameter 'Time delay' may not be negative.");

	const size_t numPhases = GetPhasesNumber();
	const size_t numDistr  = GetDistributionsNumber();

	/// Clear all state variables in model ///
	m_model.ClearVariables();

	/// Add state variables to the model ///
	m_model.m_iMflow              = m_model.AddDAEVariable(true, 0, 0, 0);
	m_model.m_iNormMflow          = m_model.AddDAEVariable(true, 1, 0, 0);
	m_model.m_iNormT              = m_model.AddDAEVariable(true, 1, 0, 0);
	m_model.m_iNormP              = m_model.AddDAEVariable(true, 1, 0, 0);
	m_model.m_iNormPhases         = m_model.AddDAEVariable(true, 1, 0, 0);
	m_model.m_iNormPhaseCompounds = m_model.AddDAEVariables(true, std::vector<double>(numPhases, 1), 0, 0);
	m_model.m_iNormDistr          = m_model.AddDAEVariables(true, std::vector<double>(numDistr, 1), 0, 0);

	/// Set tolerances to the model ///
	const auto rtol = GetConstRealParameterValue("Relative tolerance");
	const auto atol = GetConstRealParameterValue("Absolute tolerance");
	m_model.SetTolerance(rtol != 0.0 ? rtol : GetRelTolerance(), atol != 0.0 ? atol : GetAbsTolerance());

	/// Set model to the solver ///
	if (!m_solver.SetModel(&m_model))
		RaiseError(m_solver.GetError());
}

void CTimeDelay::Simulate(double _timeBeg, double _timeEnd)
{
	/// Run solver ///
	if (!m_solver.Calculate(_timeBeg, _timeEnd))
		RaiseError(m_solver.GetError());
}

void CTimeDelay::SaveState()
{
	/// Save solver's state ///
	m_solver.SaveState();
}

void CTimeDelay::LoadState()
{
	/// Load solver's state ///
	m_solver.LoadState();
}

//////////////////////////////////////////////////////////////////////////
/// Solver

void CMyDAEModel::ResultsHandler(double _time, double* _vars, double* _ders, void* _unit)
{
	/// General information ///
	auto* unit = static_cast<CTimeDelay*>(_unit);
	// Pointer to inlet and outlet stream
	CMaterialStream* inStream  = unit->GetPortStream("In");
	CMaterialStream* outStream = unit->GetPortStream("Out");

	/// Time delay of information ///
	outStream->CopyFromStream(_time, inStream, _time - unit->m_timeDelay);
	outStream->SetMassFlow(_time, _vars[m_iMflow]);
}

void CMyDAEModel::CalculateResiduals(double _time, double* _vars, double* _ders, double* _res, void* _unit)
{
	/// General information ///
	// Pointer to unit
	auto* unit = static_cast<CTimeDelay*>(_unit);
	// Pointer to inlet stream
	CMaterialStream* inStream = unit->GetPortStream("In");
	// Previous time point of inlet stream
	const double timePrev = inStream->GetPreviousTimePoint(_time);

	/// Flowsheet setup ///
	const size_t numPhases = unit->GetPhasesNumber();
	const size_t numCompounds = unit->GetCompoundsNumber();
	const size_t numDistr = unit->GetDistributionsNumber();
	const std::vector<EDistrTypes> distrTypes = unit->GetDistributionsTypes();

	/// MTP values ///
	// Mass flows at last and current time points
	const double MflowPrev = inStream->GetMassFlow(timePrev);
	const double Mflow = inStream->GetMassFlow(_time);
	const double normMflow_update = std::pow(Mflow - MflowPrev, 2);
	// Temperatures at last and current time points
	const double TPrev = inStream->GetTemperature(timePrev);
	const double T = inStream->GetTemperature(_time);
	const double normT_update = std::pow(T - TPrev, 2);
	// Pressures at last and current time points
	const double PPrev = inStream->GetPressure(timePrev);
	const double P = inStream->GetPressure(_time);
	const double normP_update = std::pow(P - PPrev, 2);

	/// Phase & phase compound fractions ///
	// Initialize norm for phase fractions
	double normPhases_update = 0;
	// Declare norm vector for phase compound fractions
	std::vector<double> normPhaseCompounds_update(numCompounds, 0);
	for (size_t i = 0; i < numPhases; ++i)
	{
		// Get state of aggregation of current phase
		const auto phase = unit->GetPhaseType(i);
		// Get phase fraction at last time point
		const double tempPhaseFracPrev = inStream->GetPhaseMassFlow(timePrev, phase) / MflowPrev;
		// Get phase fraction at current time point
		const double tempPhaseFrac = inStream->GetPhaseMassFlow(_time, phase) / Mflow;
		// Squared difference of phase fractions
		normPhases_update += std::pow(tempPhaseFracPrev - tempPhaseFrac, 2);

		// Loop for phase compound fractions
		for (const auto& compound : unit->GetAllCompounds())
		{
			// Phase compound fractions at last time point
			const double tempCompPhaseFracPrev = inStream->GetCompoundFraction(timePrev, compound, phase);
			// Phase compound fractions at current time point
			const double tempCompPhaseFrac = inStream->GetCompoundFraction(_time, compound, phase);
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
	std::vector<double> normDistr_update(numDistr);
	for (size_t k = 0; k < numDistr; ++k)
	{
		// Distribution vector at previous time point
		const std::vector<double> distrPrev = inStream->GetDistribution(timePrev, distrTypes[k]);
		// Distribution vector at current time point
		const std::vector<double> distr = inStream->GetDistribution(_time, distrTypes[k]);

		if (distrPrev.size() != distr.size())
		{
			unit->RaiseError("Number of classes in distribution '" + std::to_string(k) + "' not equal.");
			return;
		}

		// Calculate squared sum of differences between distribution vectors
		for (size_t l = 0; l < distr.size(); ++l)
			normDistr_update[k] += std::pow(distr[l] - distrPrev[l], 2);
		// Root of sum for 2-norm calculation for distributions
		normDistr_update[k] = std::sqrt(normDistr_update[k]);
	}

	/// Calculate residuals ///
	// No change if time is smaller than the time delay of the unit
	if (_time < unit->m_timeDelay)
	{
		_res[m_iMflow] = -_ders[m_iMflow];
		_res[m_iNormMflow] = -_ders[m_iNormMflow];
		_res[m_iNormT] = -_ders[m_iNormT];
		_res[m_iNormP] = -_ders[m_iNormP];

		_res[m_iNormPhases] = - _ders[m_iNormPhases];
		for (size_t i = 0; i < numPhases; ++i)
			_res[m_iNormPhaseCompounds + i] = -_ders[m_iNormPhaseCompounds + i];
		for (size_t k = 0; k < numDistr; ++k)
			_res[m_iNormDistr + k] = -_ders[m_iNormDistr + k];
	}
	// Residuals of the derivatives equal the difference in the respective norms from from the last value of the
	else
	{
		_res[m_iMflow] = -_ders[m_iMflow] + (inStream->GetMassFlow(_time - unit->m_timeDelay) - _vars[m_iMflow]);
		_res[m_iNormMflow] = -_ders[m_iNormMflow] + (normMflow_update - _vars[m_iNormMflow]);
		_res[m_iNormT] = -_ders[m_iNormT] + (normT_update - _vars[m_iNormT]);
		_res[m_iNormP] = -_ders[m_iNormP] + (normP_update - _vars[m_iNormP]);

		_res[m_iNormPhases] = -_ders[m_iNormPhases] + (normPhases_update - _vars[m_iNormPhases]);
		for (size_t i = 0; i < numPhases; ++i)
			_res[m_iNormPhaseCompounds + i] = -_ders[m_iNormPhaseCompounds + i] + (normPhaseCompounds_update[i] - _vars[m_iNormPhaseCompounds + i]);
		for (size_t k = 0; k < numDistr; ++k)
			_res[m_iNormDistr + k] = -_ders[m_iNormDistr + k] + (normDistr_update[k] - _vars[m_iNormDistr + k]);
	}
}
