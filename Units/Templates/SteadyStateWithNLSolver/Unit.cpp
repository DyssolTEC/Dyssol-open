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
	SetUnitName  ("Template unit steady-state with solver");
	SetAuthorName("Author");
	SetUniqueID  ("00000000000000000000000000000500");
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

	/// Add user data to model ///
	m_NLModel.SetUserData(this);
}

void CUnit::Initialize(double _time)
{
	/// Add state variables of unit ///
	AddStateVariable("VarName", 0.0);

	/// Clear all state variables in model ///
	m_NLModel.ClearVariables();

	/// Add variable to the model of nonlinear equation system ///
	m_NLModel.m_iVariable0 = m_NLModel.AddNLVariable(0.0, 0.0);
	m_NLModel.m_iVariable1 = m_NLModel.AddNLVariable(0.0, 0.0);

	/// Set model to the solver ///
	if (!m_NLSolver.SetModel(&m_NLModel))
		RaiseError(m_NLSolver.GetError());


}

void CUnit::Simulate(double _time)
{
	CStream* inStream  = GetPortStream("InPort");
	CStream* outStream = GetPortStream("OutPort");

	outStream->CopyFromStream(_time, inStream);

	/// Run solver ///
	if (!m_NLSolver.Calculate(_time))
		RaiseError(m_NLSolver.GetError());


}

void CUnit::Finalize()
{

}

void CUnit::SaveState()
{
	/// Save solver's state ///
	m_NLSolver.SaveState();
}

void CUnit::LoadState()
{
	/// Load solver's state ///
	m_NLSolver.LoadState();
}

//////////////////////////////////////////////////////////////////////////
/// Solver

void CMyNLModel::CalculateFunctions(double* _vars, double* _func, void* _unit)
{
	const auto* unit = static_cast<CUnit*>(_unit);

	/// Get value of variable ///
	const double value0 = _vars[m_iVariable0];
	const double value1 = _vars[m_iVariable1];

	/// Calculate residual ///
	_func[m_iVariable0] =  - value0 - value1 - unit->GetCompoundProperty(unit->GetCompoundKey(0), MOLAR_MASS);
	_func[m_iVariable1] = std::pow(value0, 2) + std::pow(value1, 2) - 17;


}

void CMyNLModel::ResultsHandler(double _time, double* _vars, void* _unit)
{
	auto* unit = static_cast<CUnit*>(_unit);
	CStream* inStream  = unit->GetPortStream("InPort");
	CStream* outStream = unit->GetPortStream("OutPort");

	/// Do something with results of nonlinear solver ///
	const double value0 = _vars[m_iVariable0];
	const double value1 = _vars[m_iVariable1];
	const double temperature = inStream->GetTemperature(_time);
	outStream->SetTemperature(_time, temperature + std::fabs(value0) + std::fabs(value1));


}
