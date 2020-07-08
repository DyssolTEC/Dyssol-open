/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "AgglomerationSolver.h"
#include "DyssolDefines.h"
#include "ThreadPool.h"
#include <cmath>

class CAgglomerationCellAverage : public CAgglomerationSolver
{
	size_t n;	// number of intervals
	std::vector<std::vector<double>> kern;

public:
	CAgglomerationCellAverage();

	void Initialize(const std::vector<double>& _vGrid, double _beta0, EKernels _kernel, size_t _rank, const std::vector<double>& vParams) override;
	bool Calculate(const std::vector<double>& _vN, std::vector<double>& _vBRate, std::vector<double>& _vDRate) override;

private:
	void ApplyCellAverage(const std::vector<double>& _f, std::vector<double>& _BRate, std::vector<double>& _DRate);
	static double Heavyside(double v);
};
