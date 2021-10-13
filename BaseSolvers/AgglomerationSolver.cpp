/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "AgglomerationSolver.h"
#include <cmath>

CAgglomerationSolver::CAgglomerationSolver() : CBaseSolver()
{
	m_type = ESolverTypes::SOLVER_AGGLOMERATION_1;
}

void CAgglomerationSolver::Initialize(const d_vect_t& _grid, double _beta0, EKernels _kernel, const d_vect_t& _parameters)
{
	SetParameters(_grid, _beta0, _kernel, nullptr, _parameters);
	Initialize();
}

void CAgglomerationSolver::Initialize(const d_vect_t& _grid, double _beta0, const std::function<kernel_t>& _kernel, const d_vect_t& _parameters)
{
	SetParameters(_grid, _beta0, EKernels::CUSTOM, _kernel, _parameters);
	Initialize();
}

void CAgglomerationSolver::Initialize()
{
}

void CAgglomerationSolver::Calculate(const d_vect_t& _n, d_vect_t& _rateB, d_vect_t& _rateD)
{
}

std::pair<CAgglomerationSolver::d_vect_t, CAgglomerationSolver::d_vect_t> CAgglomerationSolver::Calculate(const d_vect_t& _n)
{
	d_vect_t rateB, rateD;
	Calculate(_n, rateB, rateD);
	return { std::move(rateB), std::move(rateD) };
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
	case EKernels::CUSTOM:
		return m_CutomKernel(_u, _v);
	}

	return {};
}

void CAgglomerationSolver::SetParameters(const d_vect_t& _grid, double _beta0, EKernels _kernel, const std::function<kernel_t>& _kernelFun, const d_vect_t& _parameters)
{
	m_grid        = _grid;
	m_beta0       = _beta0;
	m_kernel      = _kernel;
	m_CutomKernel = _kernelFun;
	m_parameters  = _parameters;

	// checks
	if (m_grid.empty())
		RaiseError("PSD grid is empty.");
	if (m_kernel < static_cast<EKernels>(0) || m_kernel > static_cast<EKernels>(10))
		RaiseError("Wrong agglomeration kernel. The value must be in the range [0; 10].");
	if (m_kernel == EKernels::CUSTOM && !m_CutomKernel)
		RaiseError("A custom kernel is selected, but no custom function was set.");
}
