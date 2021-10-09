/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "AgglomerationSolver.h"
#include "ThreadPool.h"

class CAgglomerationFixedPivot : public CAgglomerationSolver
{
	size_t n{};				// Number of size-intervals.
	d_matr_t beta;			// Precalculated kernel.
	u_matr_t target;
	d_vect_t pivotPoints;

public:
	void CreateBasicInfo() override;
	void Initialize() override;
	void Calculate(const d_vect_t& _n, d_vect_t& _rateB, d_vect_t& _rateD) override;

private:
	void ApplyFixedPivot(const d_vect_t& _f, d_vect_t& _rateB, d_vect_t& _rateD);
};
