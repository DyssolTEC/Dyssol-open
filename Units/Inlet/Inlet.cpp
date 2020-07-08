/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Inlet.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CInlet();
}

CInlet::CInlet()
{
	m_sUnitName = "InletFlow";
	m_sAuthorName = "SPE TUHH";
	m_sUniqueID = "C55E0B290D8944C0832689B391867977";

	AddPort("InletMaterial", OUTPUT_PORT);

	AddFeed("InputMaterial", "ECFA7F77A5DF46b9A4E8CC3344D9397E");
}

void CInlet::Simulate(double _dStartTime, double _dEndTime)
{
	CMaterialStream *pFeed = GetFeed("InputMaterial");
	CMaterialStream *pOutputStream = GetPortStream("InletMaterial");
	pOutputStream->CopyFromStream(pFeed, _dStartTime, _dEndTime);
	pOutputStream->CopyFromStream(pFeed, _dEndTime);
}
