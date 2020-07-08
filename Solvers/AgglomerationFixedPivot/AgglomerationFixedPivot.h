/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "AgglomerationSolver.h"
#include "DyssolDefines.h"
#include "ThreadPool.h"

#include <cmath>
#include <iostream>
#include <fstream>

class CAgglomerationFixedPivot : public CAgglomerationSolver
{
	size_t n{};	// number of intervals
	std::vector<std::vector<double>> m_kern;
	std::vector<std::vector<size_t>> m_target;
	std::vector<double> m_pivotPoints;

public:
	CAgglomerationFixedPivot();

	void Initialize(const std::vector<double>& _vGrid, double _beta0, EKernels _kernel, size_t _rank, const std::vector<double>& vParams) override;
	bool Calculate(const std::vector<double>& _vN, std::vector<double>& _vBRate, std::vector<double>& _vDRate) override;

private:
	void ApplyFixedPivot(const std::vector<double>& _f, std::vector<double>& _BRate, std::vector<double>& _DRate);
};
