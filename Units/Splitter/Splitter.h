/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "SteadyStateUnit.h"
#include "MaterialStream.h"

class CSplitter : public CSteadyStateUnit
{
public:
	CSplitter();

	void Simulate(double _dTime) override;
};
