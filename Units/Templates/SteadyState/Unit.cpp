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
	m_sUnitName = "DummyUnit1";
	m_sAuthorName = "Author";
	m_sUniqueID = "00000000000000000000000000000100";

	/// Add ports ///
	AddPort("InPort", INPUT_PORT);
	AddPort("OutPort", OUTPUT_PORT);

	/// Add unit parameters ///
	AddTDParameter("ParamTD", 0, 1e+6, 0, "kg", "Unit parameter description");
	AddConstParameter("ParamConst", 0, 1e+6, 0, "s", "Unit parameter description");
	AddStringParameter("ParamString", "Initial value", "Unit parameter description");
}

CUnit::~CUnit()
{

}

void CUnit::Initialize(double _dTime)
{
	/// Add state variables ///
	AddStateVariable("VarName", 0, true);


}

void CUnit::Simulate(double _dTime)
{
	CMaterialStream* pInStream = GetPortStream("InPort");
	CMaterialStream* pOutStream = GetPortStream("OutPort");

	pOutStream->CopyFromStream(pInStream, _dTime);


}

void CUnit::Finalize()
{

}
