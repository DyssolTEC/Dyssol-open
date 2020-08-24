/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Phase.h"
#include "DistributionsGrid.h"
#include "ContainerFunctions.h"
#include <utility>

CPhase::CPhase(EPhase _state, const CDistributionsGrid& _grid, std::vector<std::string> _compounds, const SCacheSettings& _cache) :
	m_state{ _state },
	m_compounds{ std::move(_compounds) },
	m_grid{ _grid }
{
	SetCacheParameters(_cache);

	if (_state != EPhase::SOLID)
		// TODO: fix this when CMDMatrix uses size_t.
		m_distribution.SetDimension(DISTR_COMPOUNDS, static_cast<unsigned>(m_compounds.size()));
	else
	{
		// TODO: fix this when CMDMatrix uses EDistrTypes.
		std::vector<EDistrTypes> types = m_grid.GetDistrTypes();
		m_distribution.SetDimensions(reinterpret_cast<std::vector<unsigned>&>(types), m_grid.GetClasses());
	}
}

std::string CPhase::GetName() const
{
	return m_name;
}

void CPhase::SetName(const std::string& _name)
{
	m_name = _name;
	m_fractions.SetName(_name);
}

EPhase CPhase::GetState() const
{
	return m_state;
}

void CPhase::AddTimePoint(double _time)
{
	if (_time < 0) return;
	m_fractions.AddTimePoint(_time);
	m_distribution.AddTimePoint(_time);
}

void CPhase::CopyTimePoint(double _timeDst, double _timeSrc)
{
	if (_timeDst < 0) return;
	m_fractions.CopyTimePoint(_timeDst, _timeSrc);
	m_distribution.AddTimePoint(_timeDst, _timeSrc);
}

void CPhase::RemoveTimePoints(double _timeBeg, double _timeEnd)
{
	if (_timeBeg > _timeEnd) return;
	m_fractions.RemoveTimePoints(_timeBeg, _timeEnd);
	m_distribution.RemoveTimePoints(_timeBeg, _timeEnd);
}

void CPhase::AddCompound(const std::string& _compoundKey)
{
	if (HasCompound(_compoundKey)) return;
	m_compounds.push_back(_compoundKey);
	m_distribution.AddClass(DISTR_COMPOUNDS);
}

void CPhase::RemoveCompound(const std::string& _compoundKey)
{
	if (!HasCompound(_compoundKey)) return;
	m_distribution.RemoveClass(DISTR_COMPOUNDS, CompoundIndex(_compoundKey));
	VectorDelete(m_compounds, _compoundKey);
}

double CPhase::GetFraction(double _time) const
{
	return m_fractions.GetValue(_time);
}

void CPhase::SetFraction(double _time, double _value)
{
	m_fractions.SetValue(_time, _value);
}

double CPhase::GetCompoundFraction(double _time, size_t _iCompound) const
{
	return m_distribution.GetValue(_time, DISTR_COMPOUNDS, _iCompound);
}

void CPhase::SetCompoundFraction(double _time, size_t _iCompound, double _fraction)
{
	m_distribution.SetValue(_time, DISTR_COMPOUNDS, _iCompound, _fraction);
}

void CPhase::SetCacheParameters(const SCacheSettings& _cache)
{
	m_distribution.SetCachePath(_cache.path);
	m_distribution.SetCacheParams(_cache.isEnabled, _cache.window);
}

CMDMatrix* CPhase::MDDistr()
{
	return &m_distribution;
}

const CMDMatrix* CPhase::MDDistr() const
{
	return &m_distribution;
}

bool CPhase::HasCompound(const std::string& _compoundKey) const
{
	return VectorContains(m_compounds, _compoundKey);
}

size_t CPhase::CompoundIndex(const std::string& _compoundKey) const
{
	for (size_t i = 0; i < m_compounds.size(); ++i)
		if (m_compounds[i] == _compoundKey)
			return i;
	return static_cast<size_t>(-1);
}
