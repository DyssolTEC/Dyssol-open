/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "TwoWayMap.h"
#include "ContainerFunctions.h"
#include <numeric>

void CTwoWayMap::Set(double _left, double _right)
{
	if (m_direct.HasParam(_left) || m_revert.HasParam(_right)) return;
	m_direct.SetValue(_left, _right);
	m_revert.SetValue(_right, _left);
}

void CTwoWayMap::SetLeftToRight(const CDependentValues& _table)
{
	if (VectorUnique(_table.GetParamsList()) && VectorUnique(_table.GetValuesList()))
		m_direct = _table;
	else
		m_direct = Unique(_table);
	m_revert = Reverted(_table);
}

void CTwoWayMap::SetRightToLeft(const CDependentValues& _table)
{
	if (VectorUnique(_table.GetParamsList()) && VectorUnique(_table.GetValuesList()))
		m_revert = _table;
	else
		m_revert = Unique(_table);
	m_direct = Reverted(_table);
}

void CTwoWayMap::Remove(double _left, double _right)
{
	if (!m_direct.HasParam(_left) || m_direct.GetValue(_left) != _right) return;
	m_direct.RemoveValue(_left);
	m_revert.RemoveValue(_right);
}

void CTwoWayMap::RemoveLeft(double _left)
{
	if (!m_direct.HasParam(_left)) return;
	m_revert.RemoveValue(m_direct.GetValue(_left));
	m_direct.RemoveValue(_left);
}

void CTwoWayMap::RemoveRight(double _right)
{
	if (!m_revert.HasParam(_right)) return;
	m_direct.RemoveValue(m_revert.GetValue(_right));
	m_revert.RemoveValue(_right);
}

double CTwoWayMap::GetLeft(double _right) const
{
	return m_revert.GetValue(_right);
}

double CTwoWayMap::GetRight(double _left) const
{
	return m_direct.GetValue(_left);
}

bool CTwoWayMap::HasLeft(double _left) const
{
	return m_direct.HasParam(_left);
}

bool CTwoWayMap::HasRight(double _right) const
{
	return m_revert.HasParam(_right);
}

const CDependentValues& CTwoWayMap::GetLeftToRightTable() const
{
	return m_direct;
}

const CDependentValues& CTwoWayMap::GetRightToLeftTable() const
{
	return m_revert;
}

size_t CTwoWayMap::Size() const
{
	return m_direct.Size();
}

bool CTwoWayMap::IsEmpty() const
{
	return m_direct.IsEmpty();
}

void CTwoWayMap::Clear()
{
	m_direct.Clear();
	m_revert.Clear();
}

CDependentValues CTwoWayMap::Reverted(const CDependentValues& _table)
{
	CDependentValues res;
	for (const auto& [l, r] : _table)
		res.SetValue(r, l);
	return res;
}

CDependentValues CTwoWayMap::Unique(const CDependentValues& _table)
{
	auto params = _table.GetParamsList();
	auto values = _table.GetValuesList();
	// initial index locations
	std::vector<size_t> iparams(params.size());
	std::vector<size_t> ivalues(values.size());
	std::iota(iparams.begin(), iparams.end(), 0);
	std::iota(ivalues.begin(), ivalues.end(), 0);
	// sort indices based on comparing values in vectors
	std::stable_sort(iparams.begin(), iparams.end(), [&params](size_t i1, size_t i2) { return params[i1] < params[i2]; });
	std::stable_sort(ivalues.begin(), ivalues.end(), [&values](size_t i1, size_t i2) { return values[i1] < values[i2]; });
	// indices of elements to be deleted
	std::set<size_t> todel;
	// find all but first repeating params
	for (size_t i = 1; i < iparams.size(); ++i)
		if (params[iparams[i]] == params[iparams[i - 1]])
			todel.insert(iparams[i]);
	// find all but first repeating values
	for (size_t i = 1; i < ivalues.size(); ++i)
		if (values[ivalues[i]] == values[ivalues[i - 1]])
			todel.insert(ivalues[i]);
	// create a copy of the table
	CDependentValues res = _table;
	// remove not unique
	for (const auto& i : todel)
		res.RemoveValue(params[i]);
	return res;
}
