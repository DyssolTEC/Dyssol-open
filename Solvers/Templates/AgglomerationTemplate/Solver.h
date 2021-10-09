/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CSolver : public CAgglomerationSolver
{
public:
	void CreateBasicInfo() override;
	void Initialize() override;
	void Calculate(const d_vect_t& _n, d_vect_t& _rateB, d_vect_t& _rateD) override;
	void Finalize() override;
};
