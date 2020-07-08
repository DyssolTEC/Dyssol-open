/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "PBMSolver.h"

CPBMSolver::CPBMSolver()
{
	m_solverType = ESolverTypes::SOLVER_PBM_1;
}

void CPBMSolver::SetInitialHoldup(const CHoldup* _pHoldup)
{

}

void CPBMSolver::SetInputStream(const CMaterialStream* _pStream)
{

}

void CPBMSolver::SetOutputStream(double _dMassFlow)
{

}

void CPBMSolver::Calculate(double _dTStart, double _dTEnd)
{

}

CHoldup* CPBMSolver::GetResult()
{
	return nullptr;
}
