/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DynamicUnit.h"

class CInlet : public CDynamicUnit
{
public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Simulate(double _timeBeg, double _timeEnd) override;
};
