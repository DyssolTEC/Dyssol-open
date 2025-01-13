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
	SetUnitName  ("Template unit steady-state");
	SetAuthorName("Author");
	SetUniqueID  ("00000000000000000000000000000100");
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


}

void CUnit::Initialize(double _time)
{
	/// Add state variables ///
	AddStateVariable("VarName", 0.0);


}

void CUnit::Simulate(double _time)
{
	CStream* inStream  = GetPortStream("InPort");
	CStream* outStream = GetPortStream("OutPort");

	outStream->CopyFromStream(_time, inStream);


}

void CUnit::Finalize()
{

}
