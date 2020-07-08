/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CUnit : public CSteadyStateUnit
{
public:
	CUnit();
	~CUnit();

	void Initialize(double _dTime) override;
	void Simulate(double _dTime) override;
	void Finalize() override;
};
