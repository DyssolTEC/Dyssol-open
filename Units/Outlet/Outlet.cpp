/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Outlet.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new COutlet();
}

void COutlet::CreateBasicInfo()
{
	SetUnitName("OutletFlow");
	SetAuthorName("SPE TUHH");
	SetUniqueID("40A70301168D4F9F84DF08918DA6F5E2");
	SetHelpLink("003_models/unit_outletflow.html");
}

void COutlet::CreateStructure()
{
	AddPort("In", EUnitPort::INPUT);
}

void COutlet::Simulate(double _timeBeg, double _timeEnd)
{
}
