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
	SetUnitName("Bunker");
	SetAuthorName("SPE TUHH");
	SetUniqueID("A5D7F41322C949EC86C96C583A35501F");
}

void CBunker::CreateStructure()
{
	/// Add ports ///
	AddPort("Inflow", EUnitPort::INPUT);
	AddPort("Outflow", EUnitPort::OUTPUT);

	/// Add unit parameters ///
	AddConstRealParameter("Target mass"       , 100000, "kg", "Target mass of bunker."                                         , 0.0);
	AddConstRealParameter("Relative tolerance", 0.0   , "-" , "Solver relative tolerance. Set to 0 to use flowsheet-wide value", 0.0);
	AddConstRealParameter("Absolute tolerance", 0.0   , "-" , "Solver absolute tolerance. Set to 0 to use flowsheet-wide value", 0.0);

	/// Add holdups ///
	AddHoldup("Holdup");

	/// Add Internal streams ///
	AddStream("InflowSolid");
	AddStream("InflowBypass");

	/// Set this unit as user data of model ///
	m_Model.SetUserData(this);
}

void CBunker::Initialize(double _dTime)
{
	/// Check presence of solid phase ///
	if (!IsPhaseDefined(EPhase::SOLID))
		RaiseError("Solid phase has not been defined.");
	// Pointer to holdup stream
	CHoldup* pHoldup = GetHoldup("Holdup");

	const size_t nNumDistr = GetDistributionsNumber();

	/// Prepare holdup ///
	std::vector<double> vTimePoints = pHoldup->GetAllTimePoints();
	if (vTimePoints.empty())
	{
		RaiseError("No initial state of bunker at time point 't = 0' found.");
		return;
	}
	if (vTimePoints[0] != 0.0)
	{
		RaiseError("No initial state of bunker at time point 't = 0' found.");
		return;
	}
	if (vTimePoints.size() > 1)
	{
		RaiseWarning("Neglecting all time points of bunker after 't = 0'.");
		pHoldup->RemoveTimePointsAfter(_dTime);
	}

	const double dMass = pHoldup->GetMass(_dTime);
	const double dMassSolid = pHoldup->GetPhaseMass(_dTime, EPhase::SOLID);
	if (dMass != dMassSolid)
	{
		RaiseWarning("The holdup of the bunker model can only contain solids. Removing all other phases from bunker holdup than the solid phase.");
		pHoldup->SetMass(_dTime, 0);
		pHoldup->SetPhaseMass(_dTime, EPhase::SOLID, dMassSolid);
	}

	/// Clear all state variables in model ///
	m_Model.ClearVariables();

	/// Add state variables to the model ///
	const double dMassBunker_Init = pHoldup->GetMass(_dTime);

	m_Model.m_nMassBunker = m_Model.AddDAEVariable(true, dMassBunker_Init, 0, 0);
	m_Model.m_nMflow_Out = m_Model.AddDAEVariable(false, 0, 0, 0);
	m_Model.m_nNormMflow = m_Model.AddDAEVariable(true, 1, 0, 0);
	m_Model.m_nNormT = m_Model.AddDAEVariable(true, 1, 0, 0);
	m_Model.m_nNormP = m_Model.AddDAEVariable(true, 1, 0, 0);
	m_Model.m_nNormCompounds = m_Model.AddDAEVariable(true, 1, 0, 0);

	m_Model.m_vnNormDistr.resize(nNumDistr);
	for (size_t k = 0; k < nNumDistr; ++k)
		m_Model.m_vnNormDistr[k] = m_Model.AddDAEVariable(true, 1, 0, 0);

	/// Set tolerances to the model ///
	const auto rtol = GetConstRealParameterValue("Relative tolerance");
	const auto atol = GetConstRealParameterValue("Absolute tolerance");
	m_Model.SetTolerance(rtol != 0.0 ? rtol : GetRelTolerance(), atol != 0.0 ? atol : GetAbsTolerance());

	/// Set model to the solver ///
	if (!m_Solver.SetModel(&m_Model))
		RaiseError(m_Solver.GetError());
}

void CBunker::Simulate(double _dStartTime, double _dEndTime)
{
	// Pointer to inlet and outlet stream
	CStream* pInStream = GetPortStream("Inflow");

	// Pointer to internal stream
	CStream* pInSolid = GetStream("InflowSolid");
	CStream* pInBypass = GetStream("InflowBypass");

	pInSolid->RemoveTimePointsAfter(_dStartTime);
	pInBypass->RemoveTimePointsAfter(_dStartTime);

	/// Bypass non solid phases ///
	std::vector<double> vTimePoints = pInStream->GetTimePointsClosed(_dStartTime, _dEndTime);

	for (double time : vTimePoints)
	{
		const double dMflow_Solid = pInStream->GetPhaseMassFlow(time, EPhase::SOLID);

		pInSolid->CopyFromStream(time, pInStream);
		pInSolid->SetMassFlow(time, 0.0);
		pInSolid->SetPhaseMassFlow(time, EPhase::SOLID, dMflow_Solid);

		pInBypass->CopyFromStream(time, pInStream);
		pInBypass->SetMassFlow(time, 0);

		if (IsPhaseDefined(EPhase::VAPOR))
			pInBypass->SetPhaseMassFlow(time, EPhase::VAPOR, pInStream->GetPhaseMassFlow(time, EPhase::VAPOR));
		if (IsPhaseDefined(EPhase::LIQUID))
			pInBypass->SetPhaseMassFlow(time, EPhase::LIQUID, pInStream->GetPhaseMassFlow(time, EPhase::LIQUID));
	}

	/// Run solver ///
	if (!m_Solver.Calculate(_dStartTime, _dEndTime))
		RaiseError(m_Solver.GetError());
}

void CBunker::SaveState()
{
	/// Save solver's state ///
	m_Solver.SaveState();
}

void CBunker::LoadState()
{
	/// Load solver's state ///
	m_Solver.LoadState();
}

//////////////////////////////////////////////////////////////////////////
/// Solver

void CMyDAEModel::ResultsHandler(double _time, double* _pVars, double* _pDerivs, void* _pUserData)
{
	/// General information ///
	auto* unit = static_cast<CBunker*>(_pUserData);

	// Pointer to inlet and outlet stream
	CStream* pOutStream = unit->GetPortStream("Outflow");

	// Pointer to internal stream
	CStream* pInSolid = unit->GetStream("InflowSolid");
	CStream* pInBypass = unit->GetStream("InflowBypass");

	// Pointer to holdup stream
	CHoldup* pHoldup = unit->GetHoldup("Holdup");

	// Previous time point of inlet stream
	double dTimePrev = 0;
	if (_time > 0)
	{
		dTimePrev = pHoldup->GetPreviousTimePoint(_time);
		pHoldup->RemoveTimePointsAfter(dTimePrev);
		/// Add input to bunker ///
		pHoldup->AddStream(dTimePrev, _time, pInSolid);
	}

	std::vector<double> vTimePoints = pHoldup->GetTimePointsClosed(dTimePrev, _time);

	for (size_t i = 1; i < vTimePoints.size() - 1; ++i)
		pHoldup->RemoveTimePoint(vTimePoints[i]);

	pHoldup->SetMass(_time, _pVars[m_nMassBunker]);

	pOutStream->CopyFromHoldup(_time, pHoldup, _pVars[m_nMflow_Out]);
	pOutStream->AddStream(_time, pInBypass);
}

void CMyDAEModel::CalculateResiduals(double _time, double* _pVars, double* _pDers, double* _pRes, void* _pUserData)
{
	/// General information ///
	// Pointer to unit
	auto* unit = static_cast<CBunker*>(_pUserData);
	// Parameter for bunker capacity
	const double dMassBunker_Target = unit->GetConstRealParameterValue("Target mass");
	// Pointer to internal stream
	CStream* pInSolid = unit->GetStream("InflowSolid");

	// Previous time point of inlet stream
	const double dTimePrev = pInSolid->GetPreviousTimePoint(_time);

	/// Flowsheet setup ///
	const size_t nNumCompounds = unit->GetCompoundsNumber();
	const size_t nNumDistr = unit->GetDistributionsNumber();
	const std::vector<EDistrTypes> vDistrTypes = unit->GetDistributionsTypes();

	/// Inflow ///
	const double dMflow_In = pInSolid->GetMassFlow(_time);

	/// Outflow ///
	const double dMflow_Out = _pVars[m_nMflow_Out];

	/// MTP values ///
	// Bunker mass
	const double dMassBunker = _pVars[m_nMassBunker];
	// Mass flows at last and current time points
	const double dMflowPrev = pInSolid->GetMassFlow(dTimePrev);
	const double dMflow = pInSolid->GetMassFlow(_time);
	const double dNormMflow_update = std::pow(dMflow - dMflowPrev, 2);
	// Temperatures at last and current time points
	const double dTPrev = pInSolid->GetTemperature(dTimePrev);
	const double dT = pInSolid->GetTemperature(_time);
	const double dNormT_update = std::pow(dT - dTPrev, 2);
	// Pressures at last and current time points
	const double dPPrev = pInSolid->GetPressure(dTimePrev);
	const double dP = pInSolid->GetPressure(_time);
	const double dNormP_update = std::pow(dP - dPPrev, 2);

	/// Compound fractions ///
	// Declare norm vector for phase compound fractions
	double dNormCompounds_update = 0;
	// Loop for phase compound fractions
	for (size_t j = 0; j < nNumCompounds; ++j)
	for (const auto& compound : unit->GetAllCompounds())
	{
		// Phase compound fractions at last time point
		const double dTempCompPhaseFracPrev = pInSolid->GetCompoundFraction(dTimePrev, compound, EPhase::SOLID);
		// Phase compound fractions at current time point
		const double dTempCompPhaseFrac = pInSolid->GetCompoundFraction(_time, compound, EPhase::SOLID);
		// Squared difference of phase compound fractions
		dNormCompounds_update += std::pow(dTempCompPhaseFracPrev - dTempCompPhaseFrac, 2);
	}
	// Root of sum for 2-norm calculation for phase compound fractions
	dNormCompounds_update = std::sqrt(dNormCompounds_update);

	/// Multidimensional distributions ///
	// Declare norm vector for distributions
	std::vector<double> vNormDistr_update(nNumDistr);
	for (size_t k = 0; k < nNumDistr; ++k)
	{
		// Number of classes of current distribution
		const size_t nNumClasses = unit->GetClassesNumber(vDistrTypes[k]);
		// Distribution vector at previous time point
		std::vector<double> vDistrPrev = pInSolid->GetDistribution(dTimePrev, vDistrTypes[k]);
		// Distribution vector at current time point
		std::vector<double> vDistr = pInSolid->GetDistribution(_time, vDistrTypes[k]);

		// Calculate squared sum of differences between distribution vectors
		for (size_t l = 0; l < nNumClasses; ++l)
			vNormDistr_update[k] += std::pow(vDistr[l] - vDistrPrev[l], 2);

		// Root of sum for 2-norm calculation for distributions
		vNormDistr_update[k] = std::sqrt(vNormDistr_update[k]);
	}

	/// Calculate residuals ///
	// Bunker mass
	_pRes[m_nMassBunker] = -_pDers[m_nMassBunker] + dMflow_In - dMflow_Out;
	_pRes[m_nMflow_Out] = -_pVars[m_nMflow_Out] + std::pow(2 * dMassBunker / (dMassBunker + dMassBunker_Target),2) * dMflow_In;

	// Residuals of the derivatives equal the difference in the respective norms from from the last value of the
	_pRes[m_nNormMflow] = -_pDers[m_nNormMflow] + (dNormMflow_update - _pVars[m_nNormMflow]);
	_pRes[m_nNormT] = -_pDers[m_nNormT] + (dNormT_update - _pVars[m_nNormT]);
	_pRes[m_nNormP] = -_pDers[m_nNormP] + (dNormP_update - _pVars[m_nNormP]);

	_pRes[m_nNormCompounds] = -_pDers[m_nNormCompounds] + (dNormCompounds_update - _pVars[m_nNormCompounds]);

	for (size_t k = 0; k < nNumDistr; ++k)
		_pRes[m_vnNormDistr[k]] = -_pDers[m_vnNormDistr[k]] + (vNormDistr_update[k] - _pVars[m_vnNormDistr[k]]);
}
