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
	m_sUnitName = "DummyUnit4";
	m_sAuthorName = "Author";
	m_sUniqueID = "00000000000000000000000000000400";

	/// Add ports ///
	AddPort("InPort", INPUT_PORT);
	AddPort("OutPort", OUTPUT_PORT);

	/// Add unit parameters ///
	AddTDParameter("ParamTD", 0, 1e+6, 0, "kg", "Unit parameter description");
	AddConstParameter("ParamConst", 0, 1e+6, 0, "s", "Unit parameter description");
	AddStringParameter("ParamString", "Initial value", "Unit parameter description");

	/// Add holdups ///
	AddHoldup("HoldupName");

	/// Set this unit as user data of model ///
	m_Model.SetUserData(this);
}

CUnit::~CUnit()
{

}

void CUnit::Initialize(double _dTime)
{
	/// Add state variables of unit ///
	AddStateVariable("VarName", 0, true);

	/// Clear all state variables in model ///
	m_Model.ClearVariables();

	/// Add state variables to the model ///
	m_Model.m_nVariable0 = m_Model.AddDAEVariable(false, 1, 0, 1.0);

	/// Set tolerances to the model ///
	m_Model.SetTolerance(1e-3, 1e-5);

	/// Set model to the solver ///
	if (!m_Solver.SetModel(&m_Model))
		RaiseError(m_Solver.GetError());
}

void CUnit::Simulate(double _dStartTime, double _dEndTime)
{
	/// Run solver ///
	if (!m_Solver.Calculate(_dStartTime, _dEndTime))
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

void CMyDAEModel::ResultsHandler(double _dTime, double* _pVars, double* _pDerivs, void* _pUserData)
{
	auto unit = static_cast<CUnit*>(_pUserData);
	CMaterialStream* pInStream = static_cast<CMaterialStream*>(unit->GetPortStream("InPort"));
	CMaterialStream* pOutStream = static_cast<CMaterialStream*>(unit->GetPortStream("OutPort"));

	pOutStream->CopyFromStream(pInStream, _dTime);
	pOutStream->SetMassFlow(_dTime, _pVars[m_nVariable0]);


}

void CMyDAEModel::CalculateResiduals(double _dTime, double* _pVars, double* _pDers, double* _pRes, void* _pUserData)
{
	auto unit = static_cast<CUnit*>(_pUserData);

	/// Get value of variable ///
	double dValue = _pVars[m_nVariable0];

	/// Calculate residual ///
	_pRes[m_nVariable0] = dValue - (_dTime * _dTime);


}
