/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CUnit : public CDynamicUnit
{
public:
	CUnit();
	~CUnit();

	void Initialize(double _dTime) override;
	void Simulate(double _dStartTime, double _dEndTime) override;
	void SaveState() override;
	void LoadState() override;
	void Finalize() override;
};
