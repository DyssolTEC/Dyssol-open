/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Granulator.h"
#include "DistributionsFunctions.h"
#include "TransformMatrix.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CSimpleGranulator();
}

void CSimpleGranulator::CreateBasicInfo()
{
	/// Set basic unit info ///
	m_sUnitName = "Granulator";
	m_sAuthorName = "SPE TUHH";
	m_sUniqueID = "560E86013C6B4647A32A3AE346D5DB75";
}

void CSimpleGranulator::CreateStructure()
{
	/// Add ports ///
	AddPort("Suspension", INPUT_PORT);
	AddPort("ExternalNuclei", INPUT_PORT);
	AddPort("FluidizationGas", INPUT_PORT);
	AddPort("Output", OUTPUT_PORT);
	AddPort("DustOutput", OUTPUT_PORT);

	/// Add unit parameters ///
	AddTDParameter("Kos", 0.0, 1.0, 0.0, "-", "Overspray part of suspension");
	AddConstParameter("RTol", 0.0, 1.0, 1e-6, "-", "Relative tolerance");
	AddConstParameter("ATol", 0.0, 1.0, 1e-8, "-", "Absolute tolerance");

	/// Add holdups ///
	AddHoldup("HoldupMaterial");

	/// Set this unit as user data of model ///
	m_Model.SetUserData(this);
}

void CSimpleGranulator::Initialize(double _dTime)
{
	/// Check flowsheet parameters ///
	if (!IsPhaseDefined(SOA_SOLID))			RaiseError("Solid phase has not been defined.");
	if (!IsPhaseDefined(SOA_LIQUID))		RaiseError("Liquid phase has not been defined.");
	if (!IsPhaseDefined(SOA_VAPOR))			RaiseError("Gas phase has not been defined.");
	if (!IsDistributionDefined(DISTR_SIZE))	RaiseError("Size distribution has not been defined.");

	/// Get holdup ///
	m_pHoldup = GetHoldup("HoldupMaterial");

	/// Get pointers to streams ///
	m_pInSuspStream = GetPortStream("Suspension");
	m_pInNuclStream = GetPortStream("ExternalNuclei");
	m_pInGasStream = GetPortStream("FluidizationGas");
	m_pOutNuclStream = GetPortStream("Output");
	m_pOutDustStream = GetPortStream("DustOutput");

	/// Get number of classes for PSD ///
	m_nClassesNum = GetClassesNumber(DISTR_SIZE);
	/// Get grid of PSD ///
	m_vSizeGrid = GetNumericGrid(DISTR_SIZE);
	m_vAverDiam = GetClassesMeans(DISTR_SIZE);
	m_vClassSize = GetClassesSizes(DISTR_SIZE);
	/// Get initial PSD ///
	std::vector<double> vPSD = m_pHoldup->GetPSD(_dTime, PSD_q3);

	/// Get initial mass in holdup ///
	m_dInitMass = m_pHoldup->GetPhaseMass(_dTime, SOA_SOLID);

	/// Calculate some initials ///
	m_dPreCalc.clear();
	m_dPreCalc.push_back(0);
	for (unsigned i = 1; i < m_nClassesNum; ++i)
		m_dPreCalc.push_back(std::pow((m_vSizeGrid[i] + m_vSizeGrid[i + 1]) / (m_vSizeGrid[i - 1] + m_vSizeGrid[i]), 3));

	/// Clear all state variables in model ///
	m_Model.ClearVariables();

	/// Add state variables to a model ///
	m_Model.m_nAtot = m_Model.AddDAEVariable(false, 1, 0, 0);							// Total surface of all particles in the Granulator
	m_Model.m_nMtot = m_Model.AddDAEVariable(false, m_pHoldup->GetMass(_dTime), 0, 0);	// Total mass of all particles in the Granulator
	m_Model.m_nMout = m_Model.AddDAEVariable(false, 0, 0, 0);							// Output mass flow of nuclei
	m_Model.m_nMdust = m_Model.AddDAEVariable(false, 0, 0, 0);							// Output mass flow of dust
	m_Model.m_nG = m_Model.AddDAEVariable(false, 1e-8, 0, 0);							// Growth rate
	for (unsigned i = 0; i < m_nClassesNum; ++i)										// Initial PSD
		m_Model.AddDAEVariable(true, vPSD[i], 0, 1.0);
	m_Model.m_nq3 = m_Model.m_nG + 1;

	AddStateVariable("Atot", 1, true);
	AddStateVariable("Mtot", m_pHoldup->GetMass(_dTime), true);
	AddStateVariable("Mout", 0, true);
	AddStateVariable("Mdust", 0, true);
	AddStateVariable("G", 1e-8, true);
	for (unsigned i = 0; i < m_nClassesNum; ++i)
		AddStateVariable("PSD" + std::to_string(i), vPSD[i], true);

	/// Set tolerances to model ///
	m_Model.SetTolerance(GetConstParameterValue("RTol"), GetConstParameterValue("ATol"));

	/// Set model to a solver ///
	if (!m_Solver.SetModel(&m_Model))
		RaiseError(m_Solver.GetError());
}

void CSimpleGranulator::SaveState()
{
	m_Solver.SaveState();
}

void CSimpleGranulator::LoadState()
{
	m_Solver.LoadState();
}

void CSimpleGranulator::Simulate(double _dStartTime, double _dEndTime)
{
	if (!m_Solver.Calculate(_dStartTime, _dEndTime))
		RaiseError(m_Solver.GetError());
}

void CUnitDAEModel::ResultsHandler(double _dTime, double* _pVars, double* _pDerivs, void *_pUserData)
{
	auto unit = static_cast<CSimpleGranulator*>(_pUserData);

	const double dMSusp = unit->m_pInSuspStream->GetPhaseMassFlow(_dTime, SOA_SOLID);	// Mass flow of solid phase in suspension for current time
	const double dKos = unit->GetTDParameterValue("Kos", _dTime);						// Overspray part in suspension for current time
	const double dMe = dMSusp * (1 - dKos);												// Effective mass stream of the injected suspension for current time
	const double dMInNucl = unit->m_pInNuclStream->GetPhaseMassFlow(_dTime, SOA_SOLID); // Mass of input nuclei - solid part
	const double dMoutTemp = dMInNucl + dMe;

	const double dDustMass = _pVars[m_nMdust];

	unit->m_pHoldup->AddTimePoint(_dTime);

	std::vector<double> vTemp(unit->m_nClassesNum, 0);
	for (unsigned i = 0; i < unit->m_nClassesNum; ++i)
		vTemp[i] = _pVars[m_nq3 + i] < 0 ? 0 : _pVars[m_nq3 + i];

	// calculate and apply transformation matrix
	const std::vector<double> vInDistr = unit->m_pHoldup->GetDistribution(_dTime, DISTR_SIZE);
	const std::vector<double> vOutDistr = Convertq3ToMassFractions(unit->m_vSizeGrid, vTemp);
	CTransformMatrix TM;
	CSimpleGranulator::CalculateTM(DISTR_SIZE, vInDistr, vOutDistr, TM);
	unit->m_pHoldup->ApplyTM(_dTime, TM);

	const double dHoldupMass = unit->m_pHoldup->GetMass(_dTime);

	static_cast<CBaseStream*>(unit->m_pHoldup)->AddStream_Base(*static_cast<CBaseStream*>(unit->m_pInSuspStream), _dTime);	// hack to handle holdup as material stream
	static_cast<CBaseStream*>(unit->m_pHoldup)->AddStream_Base(*static_cast<CBaseStream*>(unit->m_pInNuclStream), _dTime);

	unit->m_pHoldup->RemoveTimePointsAfter(_dTime);

	unit->m_pHoldup->SetMass(_dTime, dHoldupMass);

	unit->m_pOutNuclStream->CopyFromHoldup(unit->m_pHoldup, _dTime, dMoutTemp);
	unit->m_pOutNuclStream->SetSinglePhaseProp(_dTime, FRACTION, SOA_SOLID, 1);
	unit->m_pOutNuclStream->SetSinglePhaseProp(_dTime, FRACTION, SOA_LIQUID, 0);
	unit->m_pOutNuclStream->SetSinglePhaseProp(_dTime, FRACTION, SOA_VAPOR, 0);

	unit->m_pOutDustStream->CopyFromHoldup(unit->m_pHoldup, _dTime, unit->m_pHoldup->GetMass(_dTime));
	unit->m_pOutDustStream->AddStream(unit->m_pInGasStream, _dTime);
	unit->m_pOutDustStream->SetMassFlow(_dTime, dDustMass);
	unit->m_pOutDustStream->SetSinglePhaseProp(_dTime, FRACTION, SOA_SOLID, 0);
	const double dFrL = unit->m_pOutDustStream->GetSinglePhaseProp(_dTime, FRACTION, SOA_LIQUID);
	const double dFrV = unit->m_pOutDustStream->GetSinglePhaseProp(_dTime, FRACTION, SOA_VAPOR);
	unit->m_pOutDustStream->SetSinglePhaseProp(_dTime, FRACTION, SOA_LIQUID, dFrL / (dFrL + dFrV));
	unit->m_pOutDustStream->SetSinglePhaseProp(_dTime, FRACTION, SOA_VAPOR, dFrV / (dFrL + dFrV));

	unit->SetStateVariable(unsigned(m_nAtot), _pVars[m_nAtot]);
	unit->SetStateVariable(unsigned(m_nMtot), _pVars[m_nMtot]);
	unit->SetStateVariable(unsigned(m_nMout), _pVars[m_nMout]);
	unit->SetStateVariable(unsigned(m_nMdust), _pVars[m_nMdust]);
	unit->SetStateVariable(unsigned(m_nG), _pVars[m_nG]);
	for (unsigned i = 0; i < unit->m_nClassesNum; ++i)
		unit->SetStateVariable(unsigned(m_nq3) + i, _pVars[m_nq3 + i]);
	unit->SaveStateVariables(_dTime);
}

void CUnitDAEModel::CalculateResiduals(double _dTime, double* _pVars, double* _pDers, double* _pRes, void* _pUserData)
{
	const auto unit = static_cast<CSimpleGranulator*>(_pUserData);

	const double dATot = _pVars[m_nAtot];
	const double dMTot = _pVars[m_nMtot];
	const double dMOut = _pVars[m_nMout];
	const double dMDust = _pVars[m_nMdust];
	const double dG = _pVars[m_nG];

	const double dTotGasMass = unit->m_pInGasStream->GetMassFlow(_dTime);
	const double dMSusp = unit->m_pInSuspStream->GetPhaseMassFlow(_dTime, SOA_SOLID);				// Mass flow of solid phase in suspension for current time
	const double dMNotSol = unit->m_pInSuspStream->GetMassFlow(_dTime) - dMSusp;					// Mass flow of all phases except solid in suspension for current time
	const double dKos = unit->GetTDParameterValue("Kos", _dTime);									// Overspray part in suspension for current time
	const double dMe = dMSusp * (1 - dKos);															// Effective mass stream of the injected suspension for current time
	const double dSuspSolDens = unit->m_pInSuspStream->GetPhaseTPDProp(_dTime, DENSITY, SOA_SOLID);	// Density of the solid in the suspension
	const double dMInNucl = unit->m_pInNuclStream->GetPhaseMassFlow(_dTime, SOA_SOLID);				// Mass of input nuclei - solid part
	const double dMInNuclNotSol = unit->m_pInNuclStream->GetMassFlow(_dTime) - dMInNucl;			// Mass of input nuclei - not solid part
	const std::vector<double> vPSD = unit->m_pInNuclStream->GetPSD(_dTime, PSD_q3);					// PSD of input stream

	// Total surface of all particles (ATot)
	double dTemp = 0;
	for (unsigned i = 0; i < unit->m_nClassesNum; ++i)
		dTemp += _pVars[m_nq3 + i] * unit->m_vClassSize[i] / unit->m_vAverDiam[i];
	_pRes[m_nAtot] = dATot - dMTot * 6 * dTemp / dSuspSolDens;

	// Total mass of all particles (MTot)
	_pRes[m_nMtot] = dMTot - unit->m_dInitMass;

	// Output mass flow of nuclei (MOut)
	_pRes[m_nMout] = dMOut - (dMInNucl + dMe);

	// Output mass flow of dust (MDust)
	_pRes[m_nMdust] = dMDust - (dMInNuclNotSol + dMSusp*dKos + dMNotSol + dTotGasMass);

	// Growth rate (G)
	if (dATot != 0)
		_pRes[m_nG] = dG - (2 * dMe / (dSuspSolDens * dATot));
	else
		_pRes[m_nG] = 0;

	// PSD (q3)
	_pRes[m_nq3 + 0] = _pDers[m_nq3 + 0] - 0;
	for (unsigned i = 1; i < unit->m_nClassesNum; ++i)
	{
		const double dDer = -dG / unit->m_vClassSize[i] * (_pVars[m_nq3 + i] - _pVars[m_nq3 + i - 1] * unit->m_dPreCalc[i]) + 1 / dMTot * (dMInNucl * vPSD[i] - dMOut * _pVars[m_nq3 + i]);
		_pRes[m_nq3 + i] = _pDers[m_nq3 + i] - dDer;
	}
}
