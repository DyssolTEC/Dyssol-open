/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BaseUnit.h"

class CDynamicUnit : public CBaseUnit
{
public:
	CDynamicUnit() = default;
	~CDynamicUnit() override = default;

	/** Calculates unit on specified time interval (for dynamic units).
	 *	\param _timeBeg Start of the time interval
	 *	\param _timeEnd End of the time interval*/
	void Simulate(double _timeBeg, double _timeEnd) override = 0;
};
