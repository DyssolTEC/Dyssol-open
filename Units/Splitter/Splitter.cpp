/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Splitter.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CSplitter();
}

void CSplitter::CreateBasicInfo()
{
	m_sUnitName = "Splitter";
	m_sAuthorName = "SPE TUHH";
	m_sUniqueID = "85C77D52955242DCA863D43336A90B51";
}

void CSplitter::CreateStructure()
{
	AddPort("In", INPUT_PORT);
	AddPort("Out1", OUTPUT_PORT);
	AddPort("Out2", OUTPUT_PORT);

	AddTDParameter("KSplitt", 0, 1, 0.5, "-", "Fraction of inlet flow going to outlet flow 1");
}

void CSplitter::Simulate(double _dTime)
{
	CMaterialStream* pInStream = GetPortStream("In");
	CMaterialStream* pOutStream1 = GetPortStream("Out1");
	CMaterialStream* pOutStream2 = GetPortStream("Out2");

	pOutStream1->CopyFromStream(pInStream, _dTime);
	pOutStream2->CopyFromStream(pInStream, _dTime);

	const double dMassFlowIn = pInStream->GetMassFlow(_dTime);
	const double dSplitFactor = GetTDParameterValue("KSplitt", _dTime);
	if (dSplitFactor < 0 || dSplitFactor > 1)
		RaiseError("Parameter 'KSplitt' has to be between 0 and 1.");

	pOutStream1->SetMassFlow(_dTime, dMassFlowIn * dSplitFactor);
	pOutStream2->SetMassFlow(_dTime, dMassFlowIn * (1 - dSplitFactor));
}
