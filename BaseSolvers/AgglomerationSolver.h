/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BaseSolver.h"
#include <vector>

class CAgglomerationSolver : public CExternalSolver
{
public:
	enum EKernels : size_t
	{
		CONSTANT      = 0,
		SUM           = 1,
		PRODUCT       = 2,
		BROWNIAN      = 3,
		SHEAR         = 4,
		PEGLOW        = 5,
		COAGULATION   = 6,
		GRAVITATIONAL = 7,
		EKE           = 8,
		THOMPSON      = 9
	};

protected:
	EKernels m_kernel;		// Selected kernel function.
	double m_beta0;			// Pre-factor for kernel (set zero for no agglomeration).

public:
	CAgglomerationSolver();
	virtual ~CAgglomerationSolver() = default;

	/// _vGrid - diameter related grid.
	virtual void Initialize(const std::vector<double>& _vGrid, double _beta0, EKernels _kernel, size_t _rank = 3, const std::vector<double>& vParams = {});
	virtual bool Calculate(const std::vector<double>& _vN, std::vector<double>& _vBRate, std::vector<double>& _vDRate);

protected:
	/// Calculates the chosen kernel function for particles with volumes _u and _v.
	double Kernel(double _u, double _v) const;
};

typedef DECLDIR CAgglomerationSolver* (*CreateAgglomerationSolver)();
