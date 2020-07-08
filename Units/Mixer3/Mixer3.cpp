/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Mixer3.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CMixer3();
}

CMixer3::CMixer3()
{
	m_sUnitName = "Mixer3";
	m_sAuthorName = "SPE TUHH";
	m_sUniqueID = "DC8C81DF437B44FCB377305B1A3EE5CA";

	AddPort("In1", INPUT_PORT);
	AddPort("In2", INPUT_PORT);
	AddPort("In3", INPUT_PORT);
	AddPort("Out", OUTPUT_PORT);
}

void CMixer3::Simulate(double _dTime)
{
	CMaterialStream* pInStream1 = GetPortStream("In1");
	CMaterialStream* pInStream2 = GetPortStream("In2");
	CMaterialStream* pInStream3 = GetPortStream("In3");
	CMaterialStream* pOutStream = GetPortStream("Out");

	pOutStream->RemoveTimePointsAfter(_dTime, true);
	pOutStream->CopyFromStream(pInStream1, _dTime);
	pOutStream->AddStream(pInStream2, _dTime);
	pOutStream->AddStream(pInStream3, _dTime);
}
