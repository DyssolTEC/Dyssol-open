/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Unit.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CUnit();
}

//////////////////////////////////////////////////////////////////////////
/// Unit

void CUnit::CreateBasicInfo()
{
	/// Basic unit's info ///
	SetUnitName  ("DummyUnit4");
	SetAuthorName("Author");
	SetUniqueID  ("00000000000000000000000000000400");
}

void CUnit::CreateStructure()
{
	/// Add ports ///
	AddPort("InPort" , EUnitPort::INPUT);
	AddPort("OutPort", EUnitPort::OUTPUT);

	/// Add unit parameters ///
	AddTDParameter       ("ParamTD"    , 0, "kg"        , "Unit parameter description");
	AddConstRealParameter("ParamConst" , 0, "s"         , "Unit parameter description");
	AddStringParameter   ("ParamString", "Initial value", "Unit parameter description");

	/// Add holdups ///
	AddHoldup("HoldupName");

	/// Set this unit as user data of model ///
	m_Model.SetUserData(this);


}

void CUnit::Initialize(double _time)
{
	/// Add state variables of unit ///
	AddStateVariable("VarName", 0.0);

	/// Clear all state variables in model ///
	m_Model.ClearVariables();

	/// Add state variables to the model ///
	m_Model.m_iVariable0 = m_Model.AddDAEVariable(false, 1, 0, 1.0);

	/// Set tolerances to the model ///
	m_Model.SetTolerance(1e-3, 1e-5);

	/// Set model to the solver ///
	if (!m_Solver.SetModel(&m_Model))
		RaiseError(m_Solver.GetError());


}

void CUnit::Simulate(double _timeBeg, double _timeEnd)
{
	/// Run solver ///
	if (!m_Solver.Calculate(_timeBeg, _timeEnd))
		RaiseError(m_Solver.GetError());
}

void CUnit::SaveState()
{
	/// Save solver's state ///
	m_Solver.SaveState();
}

void CUnit::LoadState()
{
	/// Load solver's state ///
	m_Solver.LoadState();
}

void CUnit::Finalize()
{

}

//////////////////////////////////////////////////////////////////////////
/// Solver

void CMyDAEModel::CalculateResiduals(double _time, double* _vars, double* _derivs, double* _res, void* _unit)
{
	const auto* unit = static_cast<CUnit*>(_unit);
	const CStream* inStream = unit->GetPortStream("InPort");
	const CHoldup* holdup   = unit->GetHoldup("HoldupName");

	/// Get value of variable ///
	const double value = _vars[m_iVariable0];

	/// Calculate residual ///
	const double inMassFlow = inStream->GetMassFlow(_time);
	const double holdupTemperature = holdup->GetTemperature(_time);
	_res[m_iVariable0] = value - (inMassFlow + holdupTemperature * _time);


}

void CMyDAEModel::ResultsHandler(double _time, double* _vars, double* _derivs, void* _unit)
{
	auto* unit = static_cast<CUnit*>(_unit);
	CStream* inStream = unit->GetPortStream("InPort");
	CStream* outStream = unit->GetPortStream("OutPort");
	CHoldup* holdup = unit->GetHoldup("HoldupName");

	/// Apply calculated variables ///
	holdup->AddStream(holdup->GetPreviousTimePoint(_time), _time, inStream);
	outStream->CopyFromHoldup(_time, holdup, _vars[m_iVariable0]);


}
