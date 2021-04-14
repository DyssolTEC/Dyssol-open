/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Mixer3.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CMixer3();
}

void CMixer3::CreateBasicInfo()
{
	SetUnitName("Mixer3");
	SetAuthorName("SPE TUHH");
	SetUniqueID("DC8C81DF437B44FCB377305B1A3EE5CA");
}

void CMixer3::CreateStructure()
{
	AddPort("In1", EUnitPort::INPUT);
	AddPort("In2", EUnitPort::INPUT);
	AddPort("In3", EUnitPort::INPUT);
	AddPort("Out", EUnitPort::OUTPUT);
}

void CMixer3::Simulate(double _time)
{
	CMaterialStream* inStream1 = GetPortStream("In1");
	CMaterialStream* inStream2 = GetPortStream("In2");
	CMaterialStream* inStream3 = GetPortStream("In3");
	CMaterialStream* outStream = GetPortStream("Out");

	outStream->RemoveTimePointsAfter(_time, true);
	outStream->CopyFromStream(_time, inStream1);
	outStream->AddStream(_time, inStream2);
	outStream->AddStream(_time, inStream3);
}
