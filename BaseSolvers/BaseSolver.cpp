/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "BaseSolver.h"
#include <stdexcept>

CBaseSolver::CBaseSolver():
	m_nCompilerVer(COMPILER_VERSION),
	m_solverType(ESolverTypes::SOLVER_NONE),
	m_solverName(""),
	m_authorName(""),
	m_solverUniqueKey(""),
	m_solverVersion(1)
{
}

void CBaseSolver::Initialize()
{
}

void CBaseSolver::Finalize()
{
}

void CBaseSolver::SaveState()
{
}

void CBaseSolver::LoadState()
{
}

ESolverTypes CBaseSolver::GetType() const
{
	return m_solverType;
}

std::string CBaseSolver::GetName() const
{
	return m_solverName;
}

std::string CBaseSolver::GetUniqueID() const
{
	return m_solverUniqueKey;
}

std::string CBaseSolver::GetAuthorName() const
{
	return m_authorName;
}

unsigned CBaseSolver::GetVersion() const
{
	return m_solverVersion;
}

void CBaseSolver::RaiseError(const std::string& _sDescription) const
{
	throw std::logic_error(m_solverName + ": " + _sDescription);
}
