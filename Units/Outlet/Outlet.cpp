/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Outlet.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new COutlet();
}

void COutlet::CreateBasicInfo()
{
	m_sUnitName = "OutletFlow";
	m_sAuthorName = "SPE TUHH";
	m_sUniqueID = "40A70301168D4F9F84DF08918DA6F5E2";
}

void COutlet::CreateStructure()
{
	AddPort("In", INPUT_PORT);
}

void COutlet::Simulate(double _timeBeg, double _timeEnd)
{
}
