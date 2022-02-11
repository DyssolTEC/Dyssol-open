/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Mixer.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CMixer();
}

void CMixer::CreateBasicInfo()
{
	SetUnitName("Mixer");
	SetAuthorName("SPE TUHH");
	SetUniqueID("3872940337B3492CBAA5D7E9595EBD99");
}

void CMixer::CreateStructure()
{
	AddPort("In1", EUnitPort::INPUT);
	AddPort("In2", EUnitPort::INPUT);
	AddPort("Out", EUnitPort::OUTPUT);
}

void CMixer::Simulate(double _time)
{
	CMaterialStream* inStream1 = GetPortStream("In1");
	CMaterialStream* inStream2 = GetPortStream("In2");
	CMaterialStream* outStream = GetPortStream("Out");

	outStream->RemoveTimePointsAfter(_time, true);
	outStream->CopyFromStream(_time, inStream1);
	outStream->AddStream(_time, inStream2);
}
