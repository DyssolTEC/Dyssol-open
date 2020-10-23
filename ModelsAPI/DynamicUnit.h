/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BaseUnit.h"

class CDynamicUnit : public CBaseUnit
{
public:
	CDynamicUnit();
	virtual ~CDynamicUnit();

	/** Calculates unit on specified time interval (for dynamic units).
	 *	\param _dStartTime Start of the time interval
	 *	\param _dEndTime End of the time interval*/
	void Simulate(double _dStartTime, double _dEndTime) override = 0;
};
