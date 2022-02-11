/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CUnit : public CDynamicUnit
{
public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _time) override;
	void Simulate(double _timeBeg, double _timeEnd) override;
	void SaveState() override;
	void LoadState() override;
	void Finalize() override;
};
