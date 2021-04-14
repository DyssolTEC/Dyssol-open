/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CMixer : public CSteadyStateUnit
{
public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Simulate(double _time) override;
};
