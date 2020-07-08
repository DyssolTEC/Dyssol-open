/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Mixer.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CMixer();
}

CMixer::CMixer()
{
	m_sUnitName = "Mixer";
	m_sAuthorName = "SPE TUHH";
	m_sUniqueID = "3872940337B3492CBAA5D7E9595EBD99";

	AddPort("In1", INPUT_PORT);
	AddPort("In2", INPUT_PORT);
	AddPort("Out", OUTPUT_PORT);
}

void CMixer::Simulate(double _dTime)
{
	CMaterialStream* pInStream1 = GetPortStream("In1");
	CMaterialStream* pInStream2 = GetPortStream("In2");
	CMaterialStream* pOutStream = GetPortStream("Out");

	pOutStream->RemoveTimePointsAfter(_dTime, true);
	pOutStream->CopyFromStream(pInStream1, _dTime);
	pOutStream->AddStream(pInStream2, _dTime);
}
