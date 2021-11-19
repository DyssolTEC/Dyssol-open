/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "PBMSolver.h"

CPBMSolver::CPBMSolver() : CBaseSolver()
{
	m_type = ESolverTypes::SOLVER_PBM_1;
}

void CPBMSolver::SetInitialHoldup(const CHoldup* _holdup)
{

}

void CPBMSolver::SetInputStream(const CStream* _stream)
{

}

void CPBMSolver::SetOutputStream(double _massFlow)
{

}

void CPBMSolver::Calculate(double _timeBeg, double _timeEnd)
{

}

CHoldup* CPBMSolver::GetResult()
{
	return nullptr;
}
