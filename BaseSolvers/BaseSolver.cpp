/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "BaseSolver.h"
#include <stdexcept>

CExternalSolver::CExternalSolver():
	m_nCompilerVer(COMPILER_VERSION),
	m_solverType(ESolverTypes::SOLVER_NONE),
	m_solverName(""),
	m_authorName(""),
	m_solverUniqueKey(""),
	m_solverVersion(1)
{
}

void CExternalSolver::Initialize()
{
}

void CExternalSolver::Finalize()
{
}

void CExternalSolver::SaveState()
{
}

void CExternalSolver::LoadState()
{
}

ESolverTypes CExternalSolver::GetType() const
{
	return m_solverType;
}

std::string CExternalSolver::GetName() const
{
	return m_solverName;
}

std::string CExternalSolver::GetUniqueID() const
{
	return m_solverUniqueKey;
}

std::string CExternalSolver::GetAuthorName() const
{
	return m_authorName;
}

unsigned CExternalSolver::GetVersion() const
{
	return m_solverVersion;
}

void CExternalSolver::RaiseError(const std::string& _sDescription) const
{
	throw std::logic_error(m_solverName + ": " + _sDescription);
}
