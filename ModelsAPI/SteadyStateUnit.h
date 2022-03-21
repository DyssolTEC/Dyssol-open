/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BaseUnit.h"

class CSteadyStateUnit : public CBaseUnit
{
public:
	CSteadyStateUnit() = default;
	~CSteadyStateUnit() override = default;

	/**	Calculates unit on a time point (for steady-state units).
	 *	\param _time Time point to calculate*/
	void Simulate(double _time) override = 0;
};
