/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Splitter3.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CSplitter3();
}

CSplitter3::CSplitter3()
{
	m_sUnitName = "Splitter3";
	m_sAuthorName = "SPE TUHH";
	m_sUniqueID = "DF90D2471D2E4600800A3546B4BED43E";

	AddPort("In", INPUT_PORT);
	AddPort("Out1", OUTPUT_PORT);
	AddPort("Out2", OUTPUT_PORT);
	AddPort("Out3", OUTPUT_PORT);

	AddTDParameter("KSplitt1", 0, 1, 0.5, "-", "Fraction of inlet flow going to stream Out1");
	AddTDParameter("KSplitt2", 0, 1, 0.5, "-", "Fraction of inlet flow going to stream Out2");
}

void CSplitter3::Simulate(double _dTime)
{
	CMaterialStream* pInStream = GetPortStream("In");
	CMaterialStream* pOutStream1 = GetPortStream("Out1");
	CMaterialStream* pOutStream2 = GetPortStream("Out2");
	CMaterialStream* pOutStream3 = GetPortStream("Out3");

	pOutStream1->CopyFromStream(pInStream, _dTime);
	pOutStream2->CopyFromStream(pInStream, _dTime);
	pOutStream3->CopyFromStream(pInStream, _dTime);

	const double dMassFlowIn = pInStream->GetMassFlow(_dTime);
	const double dSplitFactor1 = GetTDParameterValue("KSplitt1", _dTime);
	const double dSplitFactor2 = GetTDParameterValue("KSplitt2", _dTime);
	if (dSplitFactor1 < 0 || dSplitFactor1 > 1)
		RaiseError("Parameter 'KSplitt1' has to be between 0 and 1.");
	if (dSplitFactor2 < 0 || dSplitFactor2 > 1)
		RaiseError("Parameter 'KSplitt2' has to be between 0 and 1.");
	if (dSplitFactor1 + dSplitFactor2 > 1)
		RaiseError("(KSplitt1 + KSplitt2) has to be between 0 and 1.");

	pOutStream1->SetMassFlow(_dTime, dMassFlowIn * dSplitFactor1);
	pOutStream2->SetMassFlow(_dTime, dMassFlowIn * dSplitFactor2);
	pOutStream3->SetMassFlow(_dTime, dMassFlowIn * (1 - dSplitFactor1 - dSplitFactor2));
}
