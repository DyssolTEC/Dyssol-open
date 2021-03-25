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
		m_direct = CDependentValues::Unique(_table);
	m_revert = Reverted(_table);
}

void CTwoWayMap::SetRightToLeft(const CDependentValues& _table)
{
	if (VectorUnique(_table.GetParamsList()) && VectorUnique(_table.GetValuesList()))
		m_revert = _table;
	else
		m_revert = CDependentValues::Unique(_table);
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
