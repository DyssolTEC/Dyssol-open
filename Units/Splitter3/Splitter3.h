/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "SteadyStateUnit.h"
#include "MaterialStream.h"

class CSplitter3 : public CSteadyStateUnit
{
public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Simulate(double _dTime) override;
};
