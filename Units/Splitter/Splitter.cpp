/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Splitter.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CSplitter();
}

void CSplitter::CreateBasicInfo()
{
	SetUnitName("Splitter");
	SetAuthorName("SPE TUHH");
	SetUniqueID("85C77D52955242DCA863D43336A90B51");
	SetHelpLink("003_models/unit_splitter.html");
}

void CSplitter::CreateStructure()
{
	AddPort("In", EUnitPort::INPUT);
	AddPort("Out1", EUnitPort::OUTPUT);
	AddPort("Out2", EUnitPort::OUTPUT);

	AddTDParameter("KSplitt", 0.5, "-", "Fraction of inlet flow going to outlet flow 1", 0, 1);
}

void CSplitter::Simulate(double _time)
{
	CMaterialStream* inStream   = GetPortStream("In");
	CMaterialStream* outStream1 = GetPortStream("Out1");
	CMaterialStream* outStream2 = GetPortStream("Out2");

	outStream1->CopyFromStream(_time, inStream);
	outStream2->CopyFromStream(_time, inStream);

	const double massFlowIn = inStream->GetMassFlow(_time);
	const double splitFactor = GetTDParameterValue("KSplitt", _time);
	if (splitFactor < 0 || splitFactor > 1)
		RaiseError("Parameter 'KSplitt' has to be between 0 and 1.");

	outStream1->SetMassFlow(_time, massFlowIn * splitFactor);
	outStream2->SetMassFlow(_time, massFlowIn * (1 - splitFactor));
}
