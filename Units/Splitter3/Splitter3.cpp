/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Splitter3.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CSplitter3();
}

void CSplitter3::CreateBasicInfo()
{
	SetUnitName("Splitter3");
	SetAuthorName("SPE TUHH");
	SetUniqueID("DF90D2471D2E4600800A3546B4BED43E");
}

void CSplitter3::CreateStructure()
{
	AddPort("In", EUnitPort::INPUT);
	AddPort("Out1", EUnitPort::OUTPUT);
	AddPort("Out2", EUnitPort::OUTPUT);
	AddPort("Out3", EUnitPort::OUTPUT);

	AddTDParameter("KSplitt1", 0.5, "-", "Fraction of inlet flow going to stream Out1", 0, 1);
	AddTDParameter("KSplitt2", 0.5, "-", "Fraction of inlet flow going to stream Out2", 0, 1);
}

void CSplitter3::Simulate(double _time)
{
	CMaterialStream* inStream   = GetPortStream("In");
	CMaterialStream* outStream1 = GetPortStream("Out1");
	CMaterialStream* outStream2 = GetPortStream("Out2");
	CMaterialStream* outStream3 = GetPortStream("Out3");

	outStream1->CopyFromStream(_time, inStream);
	outStream2->CopyFromStream(_time, inStream);
	outStream3->CopyFromStream(_time, inStream);

	const double massFlowIn = inStream->GetMassFlow(_time);
	const double splitFactor1 = GetTDParameterValue("KSplitt1", _time);
	const double splitFactor2 = GetTDParameterValue("KSplitt2", _time);
	if (splitFactor1 < 0 || splitFactor1 > 1)
		RaiseError("Parameter 'KSplitt1' has to be between 0 and 1.");
	if (splitFactor2 < 0 || splitFactor2 > 1)
		RaiseError("Parameter 'KSplitt2' has to be between 0 and 1.");
	if (splitFactor1 + splitFactor2 > 1)
		RaiseError("(KSplitt1 + KSplitt2) has to be between 0 and 1.");

	outStream1->SetMassFlow(_time, massFlowIn * splitFactor1);
	outStream2->SetMassFlow(_time, massFlowIn * splitFactor2);
	outStream3->SetMassFlow(_time, massFlowIn * (1 - splitFactor1 - splitFactor2));
}
