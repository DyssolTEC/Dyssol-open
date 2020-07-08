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
	m_sUnitName = "DummyUnit2";
	m_sAuthorName = "Author";
	m_sUniqueID = "00000000000000000000000000000200";

	/// Add ports ///
	AddPort("InPort", INPUT_PORT);
	AddPort("OutPort", OUTPUT_PORT);

	/// Add unit parameters ///
	AddTDParameter("ParamTD", 0, 1e+6, 0, "kg", "Unit parameter description");
	AddConstParameter("ParamConst", 0, 1e+6, 0, "s", "Unit parameter description");
	AddStringParameter("ParamString", "Initial value", "Unit parameter description");

	/// Add holdups ///
	AddHoldup("HoldupName");
}

CUnit::~CUnit()
{

}

void CUnit::Initialize(double _dTime)
{
	/// Add state variables ///
	AddStateVariable("VarName", 0, true);


}

void CUnit::Simulate(double _dStartTime, double _dEndTime)
{
	/// Get pointers to streams ///
	CMaterialStream* pInStream = GetPortStream("InPort");
	CMaterialStream* pOutStream = GetPortStream("OutPort");

	/// Get pointers to holdups ///
	CHoldup* pHoldup = GetHoldup("HoldupName");

	pOutStream->CopyFromStream(pInStream, _dStartTime, _dEndTime);


}

void CUnit::SaveState()
{

}

void CUnit::LoadState()
{

}

void CUnit::Finalize()
{

}
