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
	m_sUnitName = "Time delay";
	m_sAuthorName = "SPE TUHH";
	m_sUniqueID = "56D734DFB1EA441B859C2C70D6F43BBC";
}

void CTimeDelay::CreateStructure()
{
	/// Add ports ///
	AddPort("In", INPUT_PORT);
	AddPort("Out", OUTPUT_PORT);

	/// Add unit parameters ///
	AddConstParameter("Time delay", 0, 1e+6, 0, "s", "Unit time delay");

	/// Set this unit as user data of model ///
	m_Model.SetUserData(this);
}

void CTimeDelay::Initialize(double _dTime)
{
	m_timeDelay = GetConstParameterValue("Time delay");
	if (m_timeDelay < 0)
		RaiseError("Parameter 'Time delay' may not be negative.");

	const size_t nNumPhases = GetPhasesNumber();
	const size_t nNumDistr = GetDistributionsNumber();

	/// Clear all state variables in model ///
	m_Model.ClearVariables();

	/// Add state variables to the model ///
	m_Model.m_nMflow      = m_Model.AddDAEVariable(true, 0, 0, 0);
	m_Model.m_nNormMflow  = m_Model.AddDAEVariable(true, 1, 0, 0);
	m_Model.m_nNormT      = m_Model.AddDAEVariable(true, 1, 0, 0);
	m_Model.m_nNormP      = m_Model.AddDAEVariable(true, 1, 0, 0);
	m_Model.m_nNormPhases = m_Model.AddDAEVariable(true, 1, 0, 0);
	m_Model.m_vnNormPhaseCompounds.resize(nNumPhases);
	for (size_t i = 0; i < nNumPhases; ++i)
		m_Model.m_vnNormPhaseCompounds[i] = m_Model.AddDAEVariable(true, 1, 0, 0);
	m_Model.m_vnNormDistr.resize(nNumDistr);
	for (size_t k = 0; k < nNumDistr; ++k)
		m_Model.m_vnNormDistr[k] = m_Model.AddDAEVariable(true, 1, 0, 0);
	/// Set tolerances to the model ///
	m_Model.SetTolerance(1e-3, 1e-5);

	/// Set model to the solver ///
	if (!m_Solver.SetModel(&m_Model))
		RaiseError(m_Solver.GetError());
}

void CTimeDelay::Simulate(double _dStartTime, double _dEndTime)
{
	/// Run solver ///
	if (!m_Solver.Calculate(_dStartTime, _dEndTime))
		RaiseError(m_Solver.GetError());
}

void CTimeDelay::SaveState()
{
	/// Save solver's state ///
	m_Solver.SaveState();
}

void CTimeDelay::LoadState()
{
	/// Load solver's state ///
	m_Solver.LoadState();
}


//////////////////////////////////////////////////////////////////////////
/// Solver

void CMyDAEModel::ResultsHandler(double _dTime, double* _pVars, double* _pDerivs, void* _pUserData)
{
	/// General information ///
	const auto unit = static_cast<CTimeDelay*>(_pUserData);
	// Pointer to inlet and outlet stream
	CMaterialStream* pInStream = unit->GetPortStream("In");
	CMaterialStream* pOutStream = unit->GetPortStream("Out");

	/// Time delay of information ///
	//if (_dTime < unit->m_timeDelay)
	//{
	//	pOutStream->CopyFromStream(_dTime, pInStream, 0);
	//	pOutStream->SetMassFlow(_dTime, 0);
	//}
	//else
	//{
	//	pOutStream->CopyFromStream(_dTime, pInStream, _dTime - unit->m_timeDelay);
	//	pOutStream->SetMassFlow(_dTime, _pVars[m_nMflow]);
	//}

	pOutStream->CopyFromStream(_dTime, pInStream, _dTime - unit->m_timeDelay);
	pOutStream->SetMassFlow(_dTime, _pVars[m_nMflow]);
}

void CMyDAEModel::CalculateResiduals(double _dTime, double* _pVars, double* _pDers, double* _pRes, void* _pUserData)
{
	/// General information ///
	// Pointer to unit
	const auto unit = static_cast<CTimeDelay*>(_pUserData);
	// Pointer to inlet stream
	CMaterialStream* pInStream = unit->GetPortStream("In");
	// Previous time point of inlet stream
	const double dTimePrev = pInStream->GetPreviousTimePoint(_dTime);

	/// Flowsheet setup ///
	const unsigned nNumPhases = unit->GetPhasesNumber();
	const unsigned nNumCompounds = unit->GetCompoundsNumber();
	const unsigned nNumDistr = unit->GetDistributionsNumber();
	const std::vector<EDistrTypes> vDistrTypes = unit->GetDistributionsTypes();

	/// MTP values ///
	// Mass flows at last and current time points
	const double dMflowPrev = pInStream->GetMassFlow(dTimePrev);
	const double dMflow = pInStream->GetMassFlow(_dTime);
	const double dNormMflow_update = std::pow(dMflow - dMflowPrev, 2);
	// Temperatures at last and current time points
	const double dTPrev = pInStream->GetTemperature(dTimePrev);
	const double dT = pInStream->GetTemperature(_dTime);
	const double dNormT_update = std::pow(dT - dTPrev, 2);
	// Pressures at last and current time points
	const double dPPrev = pInStream->GetPressure(dTimePrev);
	const double dP = pInStream->GetPressure(_dTime);
	const double dNormP_update = std::pow(dP - dPPrev, 2);

	/// Phase & phase compound fractions ///
	// Initialize norm for phase fractions
	double dNormPhases_update = 0;
	// Declare norm vector for phase compound fractions
	std::vector<double> vNormPhaseCompounds_update(nNumCompounds, 0);
	for (unsigned i = 0; i < nNumPhases; ++i)
	{
		// Get state of aggregation of current phase
		const unsigned nPhaseSOA = unit->GetPhaseSOA(i);
		// Get phase fraction at last time point
		const double dTempPhaseFracPrev = pInStream->GetPhaseMassFlow(dTimePrev, nPhaseSOA) / dMflowPrev;
		// Get phase fraction at current time point
		const double dTempPhaseFrac = pInStream->GetPhaseMassFlow(_dTime, nPhaseSOA) / dMflow;
		// Squared difference of phase fractions
		dNormPhases_update += std::pow(dTempPhaseFracPrev - dTempPhaseFrac, 2);

		// Loop for phase compound fractions
		for (unsigned j = 0; j < nNumCompounds; ++j)
		{
			// Phase compound fractions at last time point
			const double dTempCompPhaseFracPrev = pInStream->GetCompoundPhaseFraction(dTimePrev, j, nPhaseSOA);
			// Phase compound fractions at current time point
			const double dTempCompPhaseFrac = pInStream->GetCompoundPhaseFraction(_dTime, j, nPhaseSOA);
			// Squared difference of phase compound fractions
			vNormPhaseCompounds_update[i] += std::pow(dTempCompPhaseFracPrev - dTempCompPhaseFrac, 2);
		}

		// Root of sum for 2-norm calculation for phase compound fractions
		vNormPhaseCompounds_update[i] = std::sqrt(vNormPhaseCompounds_update[i]);
	}
	// Root of sum for 2-norm calculation for phase fractions
	dNormPhases_update = std::sqrt(dNormPhases_update);


	/// Multidimensional distributions ///
	// Declare norm vector for distributions
	std::vector<double> vNormDistr_update(nNumDistr);
	for (size_t k = 0; k < nNumDistr; ++k)
	{
		// Distribution vector at previous time point
		const std::vector<double> vDistrPrev = pInStream->GetDistribution(dTimePrev, vDistrTypes[k]);
		// Distribution vector at current time point
		const std::vector<double> vDistr = pInStream->GetDistribution(_dTime, vDistrTypes[k]);

		if (vDistrPrev.size() != vDistr.size())
		{
			unit->RaiseError("Number of classes in distribution '" + std::vector<std::string>(DISTR_NAMES)[k] + "' not equal.");
			return;
		}

		// Calculate squared sum of differences between distribution vectors
		for (size_t l = 0; l < vDistr.size(); ++l)
			vNormDistr_update[k] += std::pow(vDistr[l] - vDistrPrev[l], 2);
		// Root of sum for 2-norm calculation for distributions
		vNormDistr_update[k] = std::sqrt(vNormDistr_update[k]);
	}

	/// Calculate residuals ///
	// No change if time is smaller than the time delay of the unit
	if (_dTime < unit->m_timeDelay)
	{
		_pRes[m_nMflow] = -_pDers[m_nMflow];
		_pRes[m_nNormMflow] = -_pDers[m_nNormMflow];
		_pRes[m_nNormT] = -_pDers[m_nNormT];
		_pRes[m_nNormP] = -_pDers[m_nNormP];

		_pRes[m_nNormPhases] = - _pDers[m_nNormPhases];
		for (size_t i = 0; i < nNumPhases; ++i)
			_pRes[m_vnNormPhaseCompounds[i]] = -_pDers[m_vnNormPhaseCompounds[i]];
		for (size_t k = 0; k < nNumDistr; ++k)
			_pRes[m_vnNormDistr[k]] = -_pDers[m_vnNormDistr[k]];
	}
	// Residuals of the derivatives equal the difference in the respective norms from from the last value of the
	else
	{
		_pRes[m_nMflow] = -_pDers[m_nMflow] + (pInStream->GetMassFlow(_dTime - unit->m_timeDelay) - _pVars[m_nMflow]);
		_pRes[m_nNormMflow] = -_pDers[m_nNormMflow] + (dNormMflow_update - _pVars[m_nNormMflow]);
		_pRes[m_nNormT] = -_pDers[m_nNormT] + (dNormT_update - _pVars[m_nNormT]);
		_pRes[m_nNormP] = -_pDers[m_nNormP] + (dNormP_update - _pVars[m_nNormP]);

		_pRes[m_nNormPhases] = -_pDers[m_nNormPhases] + (dNormPhases_update - _pVars[m_nNormPhases]);
		for (size_t i = 0; i < nNumPhases; ++i)
			_pRes[m_vnNormPhaseCompounds[i]] = -_pDers[m_vnNormPhaseCompounds[i]] + (vNormPhaseCompounds_update[i] - _pVars[m_vnNormPhaseCompounds[i]]);
		for (size_t k = 0; k < nNumDistr; ++k)
			_pRes[m_vnNormDistr[k]] = -_pDers[m_vnNormDistr[k]] + (vNormDistr_update[k] - _pVars[m_vnNormDistr[k]]);
	}
}
