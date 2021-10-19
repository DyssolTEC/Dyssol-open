/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Solver.h"

extern "C" DECLDIR CAgglomerationSolver* CREATE_SOLVER_FUN_AGG1()
{
	return new CSolver();
}

void CSolver::CreateBasicInfo()
{
	SetName(" Dummy Agglomeration Solver");
	SetAuthorName("Author");
	SetUniqueID("00000000000000000000000000010000");
}

void CSolver::Initialize()
{

}

void CSolver::Calculate(const d_vect_t& _n, d_vect_t& _rateB, d_vect_t& _rateD)
{
	_rateB.assign(_n.size(), 0.0);
	_rateD.assign(_n.size(), 0.0);

}

void CSolver::Finalize()
{

}
