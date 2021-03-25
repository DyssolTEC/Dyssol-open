/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Solver.h"

extern "C" DECLDIR CAgglomerationSolver* CREATE_SOLVER_FUN_AGG1()
{
	return new CSolver();
}

CSolver::~CSolver()
{

}

void CSolver::CreateBasicInfo()
{
	m_solverName = "Dummy Agglomeration Solver";
	m_authorName = "Author";
	m_solverUniqueKey = "00000000000000000000000000010000";
}

void CSolver::Initialize(const std::vector<double>& _vGrid, double _beta0, EKernels _kernel, size_t _rank, const std::vector<double>& vParams)
{

}

bool CSolver::Calculate(const std::vector<double>& _vN, std::vector<double>& _vBRate, std::vector<double>& _vDRate)
{
	_vBRate.assign(_vN.size(), 0);
	_vDRate.assign(_vN.size(), 0);
	return true;
}

void CSolver::Finalize()
{

}
