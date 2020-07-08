/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CSolver : public CAgglomerationSolver
{
public:
	CSolver();
	~CSolver();

	void Initialize(const std::vector<double>& _vGrid, double _beta0, EKernels _kernel, size_t _rank, const std::vector<double>& vParams) override;
	bool Calculate(const std::vector<double>& _vN, std::vector<double>& _vBRate, std::vector<double>& _vDRate) override;
	void Finalize() override;
};
