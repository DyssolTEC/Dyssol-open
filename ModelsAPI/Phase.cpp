/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Phase.h"
#include "DistributionsGrid.h"
#include "ContainerFunctions.h"
#include "DyssolUtilities.h"
#include "DyssolStringConstants.h"
#include <utility>

// TODO: remove all reinterpret_cast and static_cast for MDMatrix

CPhase::CPhase(EPhase _state, std::string _name, const CDistributionsGrid& _grid, const SCacheSettings& _cache) :
	m_name{ std::move(_name) },
	m_state{ _state },
	m_grid{ _grid }
{
	SetCacheSettings(_cache);
	m_fractions.SetName(m_name);
	m_fractions.SetUnits("-");

	if (_state != EPhase::SOLID)
		// TODO: fix this when CMDMatrix uses size_t.
		m_distribution.SetDimension(DISTR_COMPOUNDS, m_grid.GetClassesByDistr(DISTR_COMPOUNDS));
	else
	{
		// TODO: fix this when CMDMatrix uses EDistrTypes.
		std::vector<EDistrTypes> types = m_grid.GetDistrTypes();
		m_distribution.SetDimensions(vector_cast<unsigned>(types), m_grid.GetClasses());
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

void CPhase::RemoveTimePoints(double _timeBeg, double _timeEnd, bool _inclusive/* = true*/)
{
	if (_timeBeg > _timeEnd) return;
	m_fractions.RemoveTimePoints(_timeBeg, _timeEnd, _inclusive);
	m_distribution.RemoveTimePoints(_timeBeg, _timeEnd, _inclusive);
}

void CPhase::RemoveAllTimePoints()
{
	m_fractions.RemoveAllTimePoints();
	m_distribution.RemoveAllTimePoints();
}

void CPhase::AddCompound(const std::string& _compoundKey)
{
	if (HasCompound(_compoundKey)) return;
	m_distribution.AddClass(DISTR_COMPOUNDS);
}

void CPhase::RemoveCompound(const std::string& _compoundKey)
{
	if (!HasCompound(_compoundKey)) return;
	m_distribution.RemoveClass(DISTR_COMPOUNDS, CompoundIndex(_compoundKey));
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
	return m_distribution.GetValue(_time, DISTR_COMPOUNDS, static_cast<unsigned>(_iCompound));
}

void CPhase::SetCompoundFraction(double _time, size_t _iCompound, double _value)
{
	m_distribution.SetValue(_time, DISTR_COMPOUNDS, static_cast<unsigned>(_iCompound), _value);
}

std::vector<double> CPhase::GetCompoundsDistribution(double _time) const
{
	return m_distribution.GetDistribution(_time, DISTR_COMPOUNDS);
}

void CPhase::SetCompoundsDistribution(double _time, const std::vector<double>& _value)
{
	m_distribution.SetDistribution(_time, DISTR_COMPOUNDS, _value);
}

void CPhase::CopyFrom(double _time, const CPhase& _source)
{
	m_fractions.CopyFrom(_time, _source.m_fractions);
	m_distribution.CopyFrom(_source.m_distribution, _time);
}

void CPhase::CopyFrom(double _timeDst, const CPhase& _source, double _timeSrc)
{
	m_fractions.CopyFrom(_timeDst, _source.m_fractions, _timeSrc);
	m_distribution.CopyFromTimePoint(_source.m_distribution, _timeSrc, _timeDst);
}

void CPhase::CopyFrom(double _timeBeg, double _timeEnd, const CPhase& _source)
{
	m_fractions.CopyFrom(_timeBeg, _timeEnd, _source.m_fractions);
	m_distribution.CopyFrom(_source.m_distribution, _timeBeg, _timeEnd);
}

void CPhase::Extrapolate(double _timeExtra, double _time)
{
	m_fractions.Extrapolate(_timeExtra, _time);
	m_distribution.AddTimePoint(_timeExtra, _time);
}

void CPhase::Extrapolate(double _timeExtra, double _time1, double _time2)
{
	m_fractions.Extrapolate(_timeExtra, _time1, _time2);
	m_distribution.ExtrapolateToPoint(_time1, _time2, _timeExtra);
}

void CPhase::Extrapolate(double _timeExtra, double _time1, double _time2, double _time3)
{
	m_fractions.Extrapolate(_timeExtra, _time1, _time2, _time3);
	m_distribution.ExtrapolateToPoint(_time1, _time2, _time3, _timeExtra);
}

CMDMatrix* CPhase::MDDistr()
{
	return &m_distribution;
}

const CMDMatrix* CPhase::MDDistr() const
{
	return &m_distribution;
}

CTimeDependentValue* CPhase::Fractions()
{
	return &m_fractions;
}

const CTimeDependentValue* CPhase::Fractions() const
{
	return &m_fractions;
}

void CPhase::SetCacheSettings(const SCacheSettings& _cache)
{
	m_fractions.SetCacheSettings(_cache);
	m_distribution.SetCachePath(_cache.path);
	m_distribution.SetCacheParams(_cache.isEnabled, _cache.window);
}

void CPhase::UpdateDistributionsGrid()
{
	if (m_state != EPhase::SOLID) return;
	m_distribution.UpdateDimensions(E2I(m_grid.GetDistrTypes()), m_grid.GetClasses());
}

void CPhase::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	if (!_h5File.IsValid())	return;

	_h5File.WriteAttribute(_path, StrConst::H5AttrSaveVersion, m_saveVersion);
	_h5File.WriteData(_path, StrConst::Phase_H5Name, m_name);
	_h5File.WriteData(_path, StrConst::Phase_H5State, E2I(m_state));
	m_fractions.SaveToFile(_h5File, _h5File.CreateGroup(_path, StrConst::Phase_H5Fractions));
	m_distribution.SaveToFile(_h5File, _h5File.CreateGroup(_path, StrConst::Phase_H5Distribution));
}

void CPhase::LoadFromFile(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid())	return;

	//const int version = _h5File.ReadAttribute(_path, StrConst::m_saveVersion);
	_h5File.ReadData(_path, StrConst::Phase_H5Name, m_name);
	uint32_t state;
	_h5File.ReadData(_path, StrConst::Phase_H5State, state);
	m_state = static_cast<EPhase>(state);
	m_fractions.LoadFromFile(_h5File, _path + "/" + StrConst::Phase_H5Fractions);
	m_distribution.LoadFromFile(_h5File, _path + "/" + StrConst::Phase_H5Distribution);
}

bool CPhase::HasCompound(const std::string& _compoundKey) const
{
	return VectorContains(m_grid.GetSymbolicGridByDistr(DISTR_COMPOUNDS), _compoundKey);
}

size_t CPhase::CompoundIndex(const std::string& _compoundKey) const
{
	const auto& compounds = m_grid.GetSymbolicGridByDistr(DISTR_COMPOUNDS);
	for (size_t i = 0; i < compounds.size(); ++i)
		if (compounds[i] == _compoundKey)
			return i;
	return static_cast<size_t>(-1);
}
