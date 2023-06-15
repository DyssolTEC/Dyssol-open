/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Inlet.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CInlet();
}

void CInlet::CreateBasicInfo()
{
	SetUnitName("InletFlow");
	SetAuthorName("SPE TUHH");
	SetUniqueID("C55E0B290D8944C0832689B391867977");
	SetHelpLink("003_models/unit_inletflow.html");
}

void CInlet::CreateStructure()
{
	AddPort("InletMaterial", EUnitPort::OUTPUT);
	AddFeed("InputMaterial");
}

void CInlet::Simulate(double _timeBeg, double _timeEnd)
{
	CMaterialStream* feed = GetFeed("InputMaterial");
	CMaterialStream* outputStream = GetPortStream("InletMaterial");
	outputStream->CopyFromStream(_timeBeg, _timeEnd, feed);
	outputStream->CopyFromStream(_timeEnd, feed);
}
