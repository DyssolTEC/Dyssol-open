/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "AgglomerationSolver.h"
#include <cmath>

CAgglomerationSolver::CAgglomerationSolver() :
	m_kernel(EKernels::BROWNIAN),
	m_beta0(1)
{
	m_solverType = ESolverTypes::SOLVER_AGGLOMERATION_1;
}

void CAgglomerationSolver::Initialize(const std::vector<double>& _vGrid, double _beta0, EKernels _kernel, size_t _rank /*= 3*/, const std::vector<double>& vParams /*= {}*/)
{
}

bool CAgglomerationSolver::Calculate(const std::vector<double>& _vN, std::vector<double>& _vBRate, std::vector<double>& _vDRate)
{
	return false;
}

double CAgglomerationSolver::Kernel(double _u, double _v) const
{
	switch (m_kernel)
	{
	case EKernels::CONSTANT:
		return 1;
	case EKernels::SUM:
		return _u + _v;
	case EKernels::PRODUCT:
		return _u * _v;
	case EKernels::BROWNIAN:
		return (std::pow(_u, 1. / 3.) + std::pow(_v, 1. / 3.)) * (std::pow(_u, -1. / 3.) + std::pow(_v, -1. / 3.));
	case EKernels::SHEAR:
		return std::pow(std::pow(_u, 1. / 3.) + std::pow(_v, 1. / 3.), 7. / 3.);
	case EKernels::PEGLOW:
		return std::pow(_u + _v, 0.71053) / std::pow(_u * _v, 0.06211);
	case EKernels::COAGULATION:
		return std::pow(_u, 2. / 3.) + std::pow(_v, 2. / 3.);
	case EKernels::GRAVITATIONAL:
		return std::pow(std::pow(_u, 1. / 3.) + std::pow(_v, 1. / 3.), 2.) * std::fabs(std::pow(_u, 1. / 6.) - std::pow(_v, 1. / 6.));
	case EKernels::EKE:
		return std::pow(std::pow(_u, 1. / 3.) + std::pow(_v, 1. / 3.), 2.) * std::sqrt(1. / _u + 1. / _v);
	case EKernels::THOMPSON:
		return std::pow(_u - _v, 2.) / (_u + _v);
	}

	// catch wrongly specified kernel function
	RaiseError("Wrong agglomeration kernel. Must be in the range [0; 9].");
	return 0;
}
