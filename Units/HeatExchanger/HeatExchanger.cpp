/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "HeatExchanger.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CHeatExchanger();
}

//////////////////////////////////////////////////////////////////////////
/// Unit

void CHeatExchanger::CreateBasicInfo()
{
	/// Basic unit's info ///
	SetUnitName("Heat exchanger");
	SetAuthorName("TUHH SPE");
	SetUniqueID("DB2C399331AA4F309DF63A819911251F");
}

void CHeatExchanger::CreateStructure()
{
	/// Add ports ///
	AddPort("Input1", EUnitPort::INPUT);
	AddPort("Input2", EUnitPort::INPUT);
	AddPort("Output1", EUnitPort::OUTPUT);
	AddPort("Output2", EUnitPort::OUTPUT);

	/// Add unit parameters ///
	AddConstRealParameter("Efficiency", 1, "-", "Efficiency of heat exchange", 0, 1);
}

void CHeatExchanger::Simulate(double _time)
{
	const double efficiency = GetConstRealParameterValue("Efficiency");

	// Get pointers to input streams
	CMaterialStream* inStream1 = GetPortStream("Input1");
	CMaterialStream* inStream2 = GetPortStream("Input2");

	// Get pointers to output streams
	CMaterialStream* outStream1 = GetPortStream("Output1");
	CMaterialStream* outStream2 = GetPortStream("Output2");

	// Copy information from input streams
	outStream1->CopyFromStream(_time, inStream1);
	outStream2->CopyFromStream(_time, inStream2);

	// Perform heat transfer with specified efficiency between output streams
	HeatExchange(_time, outStream1, outStream2, efficiency);
}
