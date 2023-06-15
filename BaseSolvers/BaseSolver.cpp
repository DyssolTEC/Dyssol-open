/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "BaseSolver.h"
#include <stdexcept>

ESolverTypes CBaseSolver::GetType() const
{
	return m_type;
}

std::string CBaseSolver::GetName() const
{
	return m_name;
}

std::string CBaseSolver::GetAuthorName() const
{
	return m_authorName;
}

size_t CBaseSolver::GetVersion() const
{
	return m_version;
}

std::string CBaseSolver::GetUniqueID() const
{
	return m_uniqueID;
}

std::string CBaseSolver::GetHelpLink() const
{
	return m_helpLink;
}

void CBaseSolver::SetName(const std::string& _name)
{
	m_name = _name;
}

void CBaseSolver::SetAuthorName(const std::string& _author)
{
	m_authorName = _author;
}

void CBaseSolver::SetVersion(size_t _version)
{
	m_version = _version;
}

void CBaseSolver::SetUniqueID(const std::string& _id)
{
	m_uniqueID = _id;
}

void CBaseSolver::SetHelpLink(const std::string& _helpLink)
{
	m_helpLink = _helpLink;
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

void CBaseSolver::RaiseError(const std::string& _message) const
{
	throw std::logic_error(m_name + ": " + _message);
}
