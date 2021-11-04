/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Granulator.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CSimpleGranulator();
}

void CSimpleGranulator::CreateBasicInfo()
{
	/// Set basic unit info ///
	SetUnitName("Granulator");
	SetAuthorName("SPE TUHH");
	SetUniqueID("560E86013C6B4647A32A3AE346D5DB75");
}

void CSimpleGranulator::CreateStructure()
{
	/// Add ports ///
	AddPort("Solution", EUnitPort::INPUT);
	AddPort("ExternalNuclei", EUnitPort::INPUT);
	AddPort("FluidizationGas", EUnitPort::INPUT);
	AddPort("Output", EUnitPort::OUTPUT);
	AddPort("DustOutput", EUnitPort::OUTPUT);

	/// Add unit parameters ///
	AddTDParameter       ("Kos" , 0.0 , "-", "Overspray part of solution", 0.0, 1.0);
	AddConstRealParameter("RTol", 1e-6, "-", "Relative tolerance"          , 0.0, 1.0);
	AddConstRealParameter("ATol", 1e-8, "-", "Absolute tolerance"          , 0.0, 1.0);

	/// Add holdups ///
	AddHoldup("HoldupMaterial");

	/// Set this unit as user data of model ///
	m_model.SetUserData(this);
}

void CSimpleGranulator::Initialize(double _time)
{
	/// Check flowsheet parameters ///
	if (!IsPhaseDefined(EPhase::SOLID))		RaiseError("Solid phase has not been defined.");
	if (!IsPhaseDefined(EPhase::LIQUID))	RaiseError("Liquid phase has not been defined.");
	if (!IsPhaseDefined(EPhase::VAPOR))		RaiseError("Gas phase has not been defined.");
	if (!IsDistributionDefined(DISTR_SIZE))	RaiseError("Size distribution has not been defined.");

	/// Get holdup ///
	m_holdup = GetHoldup("HoldupMaterial");

	/// Get pointers to streams ///
	m_inSolutStream = GetPortStream("Solution");
	m_inNuclStream = GetPortStream("ExternalNuclei");
	m_inGasStream = GetPortStream("FluidizationGas");
	m_outNuclStream = GetPortStream("Output");
	m_outDustStream = GetPortStream("DustOutput");

	/// Get number of classes for PSD ///
	m_classesNum = GetClassesNumber(DISTR_SIZE);
	/// Get grid of PSD ///
	m_sizeGrid = GetNumericGrid(DISTR_SIZE);
	m_averDiam = GetClassesMeans(DISTR_SIZE);
	m_classSize = GetClassesSizes(DISTR_SIZE);
	/// Get initial PSD ///
	std::vector<double> vPSD = m_holdup->GetPSD(_time, PSD_q3);

	/// Get initial mass in holdup ///
	m_initMass = m_holdup->GetPhaseMass(_time, EPhase::SOLID);

	/// Calculate some initials ///
	m_preCalc.clear();
	m_preCalc.push_back(0);
	for (size_t i = 1; i < m_classesNum; ++i)
		m_preCalc.push_back(std::pow((m_sizeGrid[i] + m_sizeGrid[i + 1]) / (m_sizeGrid[i - 1] + m_sizeGrid[i]), 3));

	/// Clear all state variables in model ///
	m_model.ClearVariables();

	/// Add state variables to a model ///
	m_model.m_iAtot = m_model.AddDAEVariable(false, 1, 0, 0);							// Total surface of all particles in the Granulator
	m_model.m_iMtot = m_model.AddDAEVariable(false, m_holdup->GetMass(_time), 0, 0);	// Total mass of all particles in the Granulator
	m_model.m_iMout = m_model.AddDAEVariable(false, 0, 0, 0);							// Output mass flow of nuclei
	m_model.m_iMdust = m_model.AddDAEVariable(false, 0, 0, 0);							// Output mass flow of dust
	m_model.m_iG = m_model.AddDAEVariable(false, 1e-8, 0, 0);							// Growth rate
	for (size_t i = 0; i < m_classesNum; ++i)											// Initial PSD
		m_model.AddDAEVariable(true, vPSD[i], 0, 1.0);
	m_model.m_iq3 = m_model.m_iG + 1;

	AddStateVariable("Atot", 1);
	AddStateVariable("Mtot", m_holdup->GetMass(_time));
	AddStateVariable("Mout", 0);
	AddStateVariable("Mdust", 0);
	AddStateVariable("G", 1e-8);
	for (size_t i = 0; i < m_classesNum; ++i)
		AddStateVariable("PSD" + std::to_string(i), vPSD[i]);

	/// Set tolerances to model ///
	m_model.SetTolerance(GetConstRealParameterValue("RTol"), GetConstRealParameterValue("ATol"));

	/// Set model to a solver ///
	if (!m_solver.SetModel(&m_model))
		RaiseError(m_solver.GetError());
}

void CSimpleGranulator::SaveState()
{
	m_solver.SaveState();
}

void CSimpleGranulator::LoadState()
{
	m_solver.LoadState();
}

void CSimpleGranulator::Simulate(double _startTime, double _endTime)
{
	if (!m_solver.Calculate(_startTime, _endTime))
		RaiseError(m_solver.GetError());
}

void CUnitDAEModel::ResultsHandler(double _time, double* _vars, double* _ders, void *_unit)
{
	auto* unit = static_cast<CSimpleGranulator*>(_unit);

	const double mSolut = unit->m_inSolutStream->GetPhaseMassFlow(_time, EPhase::SOLID);	// Mass flow of solid phase in solution for current time
	const double Kos = unit->GetTDParameterValue("Kos", _time);								// Overspray part in solution for current time
	const double me = mSolut * (1 - Kos);													// Effective mass stream of the injected solution for current time
	const double mInNucl = unit->m_inNuclStream->GetPhaseMassFlow(_time, EPhase::SOLID);	// Mass of input nuclei - solid part
	const double mOutTemp = mInNucl + me;

	const double dustMass = _vars[m_iMdust];

	unit->m_holdup->AddTimePoint(_time);

	std::vector<double> temp(unit->m_classesNum, 0);
	for (size_t i = 0; i < unit->m_classesNum; ++i)
		temp[i] = _vars[m_iq3 + i] < 0 ? 0 : _vars[m_iq3 + i];

	// calculate and apply transformation matrix
	const std::vector<double> inDistr = unit->m_holdup->GetDistribution(_time, DISTR_SIZE);
	const std::vector<double> outDistr = Convertq3ToMassFractions(unit->m_sizeGrid, temp);
	CTransformMatrix TM;
	CSimpleGranulator::CalculateTM(DISTR_SIZE, inDistr, outDistr, TM);
	unit->m_holdup->ApplyTM(_time, TM);

	const double holdupMass = unit->m_holdup->GetMass(_time);

	static_cast<CBaseStream*>(unit->m_holdup)->Add(_time, *static_cast<CBaseStream*>(unit->m_inSolutStream));	// HACK: to handle holdup as material stream
	static_cast<CBaseStream*>(unit->m_holdup)->Add(_time, *static_cast<CBaseStream*>(unit->m_inNuclStream));

	unit->m_holdup->RemoveTimePointsAfter(_time);

	unit->m_holdup->SetMass(_time, holdupMass);

	unit->m_outNuclStream->CopyFromHoldup(_time, unit->m_holdup, mOutTemp);
	unit->m_outNuclStream->SetPhaseFraction(_time, EPhase::SOLID, 1);
	unit->m_outNuclStream->SetPhaseFraction(_time, EPhase::LIQUID, 0);
	unit->m_outNuclStream->SetPhaseFraction(_time, EPhase::VAPOR, 0);

	unit->m_outDustStream->CopyFromHoldup(_time, unit->m_holdup, unit->m_holdup->GetMass(_time));
	unit->m_outDustStream->AddStream(_time, unit->m_inGasStream);
	unit->m_outDustStream->SetMassFlow(_time, dustMass);
	unit->m_outDustStream->SetPhaseFraction(_time, EPhase::SOLID, 0);
	unit->m_outDustStream->SetPhaseFraction(_time, EPhase::LIQUID, 0);
	unit->m_outDustStream->SetPhaseFraction(_time, EPhase::VAPOR, 1);

	unit->SetStateVariable("Atot" , _vars[m_iAtot] , _time);
	unit->SetStateVariable("Mtot" , _vars[m_iMtot] , _time);
	unit->SetStateVariable("Mout" , _vars[m_iMout] , _time);
	unit->SetStateVariable("Mdust", _vars[m_iMdust], _time);
	unit->SetStateVariable("G"    , _vars[m_iG]    , _time);
	for (size_t i = 0; i < unit->m_classesNum; ++i)
		unit->SetStateVariable("PSD" + std::to_string(i), _vars[m_iq3 + i], _time);
}

void CUnitDAEModel::CalculateResiduals(double _time, double* _vars, double* _ders, double* _res, void* _unit)
{
	const auto* unit = static_cast<CSimpleGranulator*>(_unit);

	const double ATot = _vars[m_iAtot];
	const double mTot = _vars[m_iMtot];
	const double mOut = _vars[m_iMout];
	const double mDust = _vars[m_iMdust];
	const double G = _vars[m_iG];

	const double totGasMass = unit->m_inGasStream->GetMassFlow(_time);
	const double mSusp = unit->m_inSolutStream->GetPhaseMassFlow(_time, EPhase::SOLID);					// Mass flow of solid phase in solution for current time
	const double mNotSol = unit->m_inSolutStream->GetMassFlow(_time) - mSusp;							// Mass flow of all phases except solid in solution for current time
	const double Kos = unit->GetTDParameterValue("Kos", _time);											// Overspray part in solution for current time
	const double me = mSusp * (1 - Kos);																// Effective mass stream of the injected solution for current time
	const double solutSolDens = unit->m_inSolutStream->GetPhaseProperty(_time, EPhase::SOLID, DENSITY);	// Density of the solid in the solution
	const double mInNucl = unit->m_inNuclStream->GetPhaseMassFlow(_time, EPhase::SOLID);				// Mass of input nuclei - solid part
	const double mInNuclNotSol = unit->m_inNuclStream->GetMassFlow(_time) - mInNucl;					// Mass of input nuclei - not solid part
	const std::vector<double> psd = unit->m_inNuclStream->GetPSD(_time, PSD_q3);						// PSD of input stream

	// Total surface of all particles (ATot)
	double temp = 0;
	for (size_t i = 0; i < unit->m_classesNum; ++i)
		temp += _vars[m_iq3 + i] * unit->m_classSize[i] / unit->m_averDiam[i];
	_res[m_iAtot] = ATot - mTot * 6 * temp / solutSolDens;

	// Total mass of all particles (MTot)
	_res[m_iMtot] = mTot - unit->m_initMass;

	// Output mass flow of nuclei (MOut)
	_res[m_iMout] = mOut - (mInNucl + me);

	// Output mass flow of dust (MDust)
	_res[m_iMdust] = mDust - (mInNuclNotSol + mSusp*Kos + mNotSol + totGasMass);

	// Growth rate (G)
	if (ATot != 0)
		_res[m_iG] = G - 2 * me / (solutSolDens * ATot);
	else
		_res[m_iG] = 0;

	// PSD (q3)
	_res[m_iq3 + 0] = _ders[m_iq3 + 0] - 0;
	for (size_t i = 1; i < unit->m_classesNum; ++i)
	{
		const double der = -G / unit->m_classSize[i] * (_vars[m_iq3 + i] - _vars[m_iq3 + i - 1] * unit->m_preCalc[i]) + 1 / mTot * (mInNucl * psd[i] - mOut * _vars[m_iq3 + i]);
		_res[m_iq3 + i] = _ders[m_iq3 + i] - der;
	}
}
