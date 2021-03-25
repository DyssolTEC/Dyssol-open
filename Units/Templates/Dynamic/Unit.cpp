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
	SetUnitName  ("DummyUnit2");
	SetAuthorName("Author");
	SetUniqueID  ("00000000000000000000000000000200");
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


}

void CUnit::Initialize(double _time)
{
	/// Add state variables ///
	AddStateVariable("VarName", 0.0);


}

void CUnit::Simulate(double _timeBeg, double _timeEnd)
{
	/// Get pointers to streams ///
	CMaterialStream* inStream  = GetPortStream("InPort");
	CMaterialStream* outStream = GetPortStream("OutPort");

	/// Get pointers to holdups ///
	CHoldup* holdup = GetHoldup("HoldupName");

	/// Unit body ///
	holdup->AddStream(_timeBeg, _timeEnd, inStream);
	outStream->CopyFromHoldup(_timeBeg, holdup, 1.0);
	outStream->CopyFromHoldup(_timeEnd, holdup, 1.0);


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
