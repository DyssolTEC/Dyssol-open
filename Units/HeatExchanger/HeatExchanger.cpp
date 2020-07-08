/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "HeatExchanger.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CHeatExchanger();
}

//////////////////////////////////////////////////////////////////////////
/// Unit

CHeatExchanger::CHeatExchanger()
{
	/// Basic unit's info ///
	m_sUnitName = "HeatExchanger";
	m_sAuthorName = "TUHH SPE";
	m_sUniqueID = "DB2C399331AA4F309DF63A819911251F";

	/// Add ports ///
	AddPort("Input1", INPUT_PORT);
	AddPort("Input2", INPUT_PORT);
	AddPort("Output1", OUTPUT_PORT);
	AddPort("Output2", OUTPUT_PORT);

	/// Add unit parameters ///
	AddConstParameter("Efficiency", 0, 1, 1, "-", "Efficiency of heat exchange");
}

void CHeatExchanger::Simulate(double _dTime)
{
	const double dEfficiency = GetConstParameterValue("Efficiency");

	// Get pointers to input streams
	CMaterialStream* pInStream1 = GetPortStream("Input1");
	CMaterialStream* pInStream2 = GetPortStream("Input2");

	// Get pointers to output streams
	CMaterialStream* pOutStream1 = GetPortStream("Output1");
	CMaterialStream* pOutStream2 = GetPortStream("Output2");

	// Copy information from input streams
	pOutStream1->CopyFromStream(pInStream1, _dTime);
	pOutStream2->CopyFromStream(pInStream2, _dTime);

	// Perform heat transfer with specified efficiency between output streams
	HeatExchange(pOutStream1, pOutStream2, _dTime, dEfficiency);
}
