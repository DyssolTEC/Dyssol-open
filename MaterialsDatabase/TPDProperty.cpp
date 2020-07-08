/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "TPDProperty.h"
#include <algorithm>
#include <cmath>

CTPDProperty::CTPDProperty(unsigned _nProperty, const std::string& _sName, const std::wstring& _sUnits, const CCorrelation& _defaultValue)
	: CBaseProperty(_nProperty, _sName, _sUnits),
	m_defaultValue{ _defaultValue },
	m_vCorrelations{ _defaultValue }
{
}

double CTPDProperty::GetValue(double _dT, double _dP) const
{
	// try to find the first correlation with T and P within
	for (const auto& corr : m_vCorrelations)
		if (corr.IsInInterval(_dT, _dP))
			return corr.GetValue(_dT, _dP);

	// try to find the first correlation with T within
	for (const auto& corr : m_vCorrelations)
		if (corr.IsTInInterval(_dT))
			return corr.GetValue(_dT, _dP);

	// TODO: rethink this search to take also P into account
	//// try to find the first correlation with P within
	//for (size_t i = 0; i < m_vCorrelations.size(); ++i)
	//	if (m_vCorrelations[i].IsPInInterval(_dP))
	//		return m_vCorrelations[i].GetValue(_dT, _dP);

	// find the nearest correlation, taking only T into account
	double deltaTMin = MDBDescriptors::TEMP_MAX;
	size_t iNearest = 0;
	for (size_t i = 0; i < m_vCorrelations.size(); ++i)
	{
		const double deltaLeft = std::fabs(m_vCorrelations[i].GetTInterval().min - _dT);
		const double deltaRight = std::fabs(m_vCorrelations[i].GetTInterval().max - _dT);

		if (deltaLeft < deltaTMin)
		{
			deltaTMin = deltaLeft;
			iNearest = i;
		}
		if (deltaRight < deltaTMin)
		{
			deltaTMin = deltaRight;
			iNearest = i;
		}
	}
	return m_vCorrelations[iNearest].GetValue(_dT, _dP);
}

SInterval CTPDProperty::GetTInterval() const
{
	if(m_vCorrelations.empty())
		return SInterval{ -1, -1 };

	SInterval overall = m_vCorrelations.front().GetTInterval();
	for (size_t i = 1; i < m_vCorrelations.size(); ++i)
	{
		const SInterval curr = m_vCorrelations[i].GetTInterval();
		overall.min = std::min(overall.min, curr.min);
		overall.max = std::max(overall.max, curr.max);
	}
	return overall;
}

SInterval CTPDProperty::GetPInterval() const
{
	if (m_vCorrelations.empty())
		return SInterval{ -1, -1 };

	SInterval overall = m_vCorrelations.front().GetPInterval();
	for (size_t i = 1; i < m_vCorrelations.size(); ++i)
	{
		const SInterval curr = m_vCorrelations[i].GetPInterval();
		overall.min = std::min(overall.min, curr.min);
		overall.max = std::max(overall.max, curr.max);
	}
	return overall;
}

size_t CTPDProperty::CorrelationsNumber() const
{
	return m_vCorrelations.size();
}

CCorrelation* CTPDProperty::GetCorrelation(size_t _index)
{
	return const_cast<CCorrelation*>(static_cast<const CTPDProperty&>(*this).GetCorrelation(_index));
}

const CCorrelation* CTPDProperty::GetCorrelation(size_t _index) const
{
	if (_index >= m_vCorrelations.size())
		return nullptr;
	return &m_vCorrelations[_index];
}

void CTPDProperty::AddCorrelation(ECorrelationTypes _nType, const std::vector<double>& _vParams, const SInterval& _TInterval /*= { TEMP_MIN , TEMP_MAX }*/, const SInterval& _PInterval /*= { PRES_MIN , PRES_MAX }*/)
{
	AddCorrelation(CCorrelation(_nType, _vParams, _TInterval, _PInterval));
}

void CTPDProperty::AddCorrelation(const CCorrelation& _correlation)
{
	m_vCorrelations.push_back(_correlation);
}

void CTPDProperty::SetCorrelation(size_t _index, ECorrelationTypes _nType, const std::vector<double>& _vParams, const SInterval& _TInterval /*= { TEMP_MIN , TEMP_MAX }*/, const SInterval& _PInterval /*= { PRES_MIN , PRES_MAX }*/)
{
	if (_index >= m_vCorrelations.size()) return;
	m_vCorrelations[_index] = { _nType, _vParams, _TInterval, _PInterval };
}

void CTPDProperty::RemoveCorrelation(size_t _index)
{
	if (_index >= m_vCorrelations.size()) return;
	m_vCorrelations.erase(m_vCorrelations.begin() + _index);
}

bool CTPDProperty::ShiftCorrelationUp(size_t _index)
{
	if (_index == 0 || _index >= m_vCorrelations.size()) return false;
	std::iter_swap(m_vCorrelations.begin() + _index, m_vCorrelations.begin() + _index - 1);
	return true;

}

bool CTPDProperty::ShiftCorrelationDown(size_t _index)
{
	if (_index == m_vCorrelations.size() - 1 || _index >= m_vCorrelations.size()) return false;
	std::iter_swap(m_vCorrelations.begin() + _index, m_vCorrelations.begin() + _index + 1);
	return true;
}

void CTPDProperty::RemoveAllCorrelations()
{
	m_vCorrelations.clear();
}

bool CTPDProperty::IsDefaultValue() const
{
	return m_vCorrelations.size() == 1 && m_vCorrelations.front() == m_defaultValue;
}
