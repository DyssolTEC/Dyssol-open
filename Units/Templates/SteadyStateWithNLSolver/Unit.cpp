/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Unit.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CUnit();
}

//////////////////////////////////////////////////////////////////////////
/// Unit

CUnit::CUnit()
{
	/// Basic unit's info ///
	m_sUnitName = "DummyUnit5";
	m_sAuthorName = "Author";
	m_sUniqueID = "00000000000000000000000000000500";

	/// Add ports ///
	AddPort("InPort", INPUT_PORT);
	AddPort("OutPort", OUTPUT_PORT);

	/// Add unit parameters ///
	AddTDParameter("ParamTD", 0, 1e+6, 0, "kg", "Unit parameter description");
	AddConstParameter("ParamConst", 0, 1e+6, 0, "s", "Unit parameter description");
	AddStringParameter("ParamString", "Initial value", "Unit parameter description");

	/// Add user data to model ///
	m_NLModel.SetUserData(this);
}

CUnit::~CUnit()
{

}

void CUnit::Initialize(double _dTime)
{
	/// Add state variables of unit ///
	AddStateVariable("VarName", 0, true);

	/// Clear all state variables in model ///
	m_NLModel.ClearVariables();

	/// Add variable to the model of nonlinear equation system ///
	m_NLModel.m_nVariable0 = m_NLModel.AddNLVariable(0.0, 0.0);
	m_NLModel.m_nVariable1 = m_NLModel.AddNLVariable(0.0, 0.0);

	/// Set model to the solver ///
	if (!m_NLSolver.SetModel(&m_NLModel))
		RaiseError(m_NLSolver.GetError());


}

void CUnit::Simulate(double _dTime)
{
	CMaterialStream* pInStream = GetPortStream("InPort");
	CMaterialStream* pOutStream = GetPortStream("OutPort");

	pOutStream->CopyFromStream(pInStream, _dTime);

	/// Run solver ///
	if (!m_NLSolver.Calculate(_dTime))
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

void CMyNLModel::CalculateFunctions(double* _pVars, double* _pFunc, void* _pUserData)
{
	auto unit = static_cast<CUnit*>(_pUserData);

	CMaterialStream* pInStream = static_cast<CMaterialStream*>(unit->GetPortStream("InPort"));

	/// Get value of variable ///
	double dValue0 = _pVars[m_nVariable0];
	double dValue1 = _pVars[m_nVariable1];

	/// Calculate residual ///
	_pFunc[m_nVariable0] =  - dValue0 - dValue1 - 3;
	_pFunc[m_nVariable1] = std::pow(dValue0,2) + std::pow(dValue1, 2) - 17;
}

void CMyNLModel::ResultsHandler(double _dTime, double* _pVars, void* _pUserData)
{
	auto unit = static_cast<CUnit*>(_pUserData);
	CMaterialStream* pInStream = static_cast<CMaterialStream*>(unit->GetPortStream("InPort"));
	CMaterialStream* pOutStream = static_cast<CMaterialStream*>(unit->GetPortStream("OutPort"));

	/// Do something with results of nonlinear solver ///
	double dValue0 = _pVars[m_nVariable0];
	double dValue1 = _pVars[m_nVariable1];
	double dTemp = pInStream->GetTemperature(_dTime);
	pOutStream->SetTemperature(_dTime, dTemp + std::fabs(dValue0) + std::fabs(dValue1));
}
