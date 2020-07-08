/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DistributionsGrid.h"
#include "DyssolStringConstants.h"
#include "DyssolUtilities.h"
#include <cmath>

const unsigned CDistributionsGrid::m_cnSaveVersion = 2;

void CDistributionsGrid::Clear()
{
	m_grids.clear();
}

void CDistributionsGrid::AddDimension(const SGridDimension& _grid)
{
	AddDimension(_grid.distrType, _grid.gridEntry, _grid.numGrid, _grid.strGrid, _grid.gridUnit, _grid.gridFun);
}

void CDistributionsGrid::AddDimension(EDistrTypes _distrType, EGridEntry _gridEntry, const std::vector<double>& _numGrid, const std::vector<std::string>& _strGrid, EGridUnit _unit, EGridFunction _gridFun)
{
	if (IsDistrTypePresent(_distrType)) return; // distribution with this type already exists

	size_t nClasses = 0;
	switch (_gridEntry)
	{
	case EGridEntry::GRID_NUMERIC:
		if (_numGrid.empty())	// wrong grid length
			return;
		for (size_t i = 0; i < _numGrid.size() - 1; ++i)
			if (_numGrid[i] >= _numGrid[i + 1])		// wrong grid sequence
				return;
		nClasses = _numGrid.size() - 1;
		break;
	case EGridEntry::GRID_SYMBOLIC:
		nClasses = _strGrid.size();
		break;
	case EGridEntry::GRID_UNDEFINED:
		break;
	}

	m_grids.emplace_back();
	m_grids.back().distrType = _distrType;
	m_grids.back().gridEntry = _gridEntry;
	m_grids.back().gridFun = _gridFun;
	m_grids.back().classes = static_cast<unsigned>(nClasses);
	m_grids.back().gridUnit = _unit;
	m_grids.back().numGrid = _numGrid;
	m_grids.back().strGrid = _strGrid;
}

SGridDimension CDistributionsGrid::GetDimension(size_t _index) const
{
	if (_index < m_grids.size())
		return m_grids[_index];
	return {};
}

void CDistributionsGrid::SetDimension(const SGridDimension& _dim)
{
	if (SGridDimension* pGrid = GetDimPtr(_dim.distrType))
		*pGrid = _dim;
	else
		AddDimension(_dim);
}

size_t CDistributionsGrid::GetDistributionsNumber() const
{
	return m_grids.size();
}

bool CDistributionsGrid::IsDistrTypePresent(EDistrTypes _distrType) const
{
	return GetDimPtr(_distrType) != nullptr;
}

void CDistributionsGrid::AddNamedClass(EDistrTypes _distrType, const std::string& _sName)
{
	SGridDimension* pGrid = GetDimPtr(_distrType);
	if (pGrid)
	{
		pGrid->classes++;
		pGrid->strGrid.push_back(_sName);
	}
}

void CDistributionsGrid::RemoveNamedClass(EDistrTypes _distrType, size_t _index)
{
	SGridDimension* pGrid = GetDimPtr(_distrType);
	if (pGrid && _index < pGrid->strGrid.size() && pGrid->classes > 0)
	{
		pGrid->classes--;
		pGrid->strGrid.erase(pGrid->strGrid.begin() + _index);
	}
}

EDistrTypes CDistributionsGrid::GetDistrType(size_t _index) const
{
	if (_index < m_grids.size())
		return m_grids[_index].distrType;
	return DISTR_UNDEFINED;
}

EGridEntry CDistributionsGrid::GetGridEntryByDistr(EDistrTypes _distrType) const
{
	if (const SGridDimension* pGrid = GetDimPtr(_distrType))
		return pGrid->gridEntry;
	return EGridEntry::GRID_UNDEFINED;
}

EGridEntry CDistributionsGrid::GetGridEntryByIndex(size_t _index) const
{
	if (_index < m_grids.size())
		return m_grids[_index].gridEntry;
	return EGridEntry::GRID_UNDEFINED;
}

EGridFunction CDistributionsGrid::GetGridFunctionByDistr(EDistrTypes _distrType) const
{
	if (const SGridDimension* pGrid = GetDimPtr(_distrType))
		return pGrid->gridFun;
	return EGridFunction::GRID_FUN_UNDEFINED;
}

EGridFunction CDistributionsGrid::GetGridFunctionByIndex(size_t _index) const
{
	if (_index < m_grids.size())
		return m_grids[_index].gridFun;
	return EGridFunction::GRID_FUN_UNDEFINED;
}

size_t CDistributionsGrid::GetClassesByDistr(EDistrTypes _distrType) const
{
	if (const SGridDimension* pGrid = GetDimPtr(_distrType))
		return pGrid->classes;
	return 0;
}

size_t CDistributionsGrid::GetClassesByIndex(size_t _index) const
{
	if (_index < m_grids.size())
		return m_grids[_index].classes;
	return 0;
}

EGridUnit CDistributionsGrid::GetGridUnitByDistr(EDistrTypes _distrType) const
{
	if (const SGridDimension* pGrid = GetDimPtr(_distrType))
		return pGrid->gridUnit;
	return EGridUnit::UNIT_DEFAULT;
}

EGridUnit CDistributionsGrid::GetGridUnitByIndex(size_t _index) const
{
	if (_index < m_grids.size())
		return m_grids[_index].gridUnit;
	return EGridUnit::UNIT_DEFAULT;
}

std::vector<double> CDistributionsGrid::GetNumericGridByDistr(EDistrTypes _distrType) const
{
	if(const SGridDimension* pGrid = GetDimPtr(_distrType))
		if (pGrid->gridEntry == EGridEntry::GRID_NUMERIC)
			return pGrid->numGrid;
	return {};
}

std::vector<double> CDistributionsGrid::GetNumericGridByIndex(size_t _index) const
{
	if (_index < m_grids.size())
		if (m_grids[_index].gridEntry == EGridEntry::GRID_NUMERIC)
			return m_grids[_index].numGrid;
	return {};
}

std::vector<std::string> CDistributionsGrid::GetSymbolicGridByDistr(EDistrTypes _distrType) const
{
	if(const SGridDimension* pGrid = GetDimPtr(_distrType))
		if (pGrid->gridEntry == EGridEntry::GRID_SYMBOLIC)
			return pGrid->strGrid;
	return {};
}

std::vector<std::string> CDistributionsGrid::GetSymbolicGridByIndex(size_t _index) const
{
	if (_index < m_grids.size())
		if (m_grids[_index].gridEntry == EGridEntry::GRID_SYMBOLIC)
			return m_grids[_index].strGrid;
	return {};
}

std::vector<unsigned> CDistributionsGrid::GetClasses() const
{
	std::vector<unsigned> res(m_grids.size());
	for (size_t i = 0; i < m_grids.size(); ++i)
		res[i] = m_grids[i].classes;
	return res;
}

std::vector<EDistrTypes> CDistributionsGrid::GetDistrTypes() const
{
	std::vector<EDistrTypes> res(m_grids.size());
	for (size_t i = 0; i < m_grids.size(); ++i)
		res[i] = m_grids[i].distrType;
	return res;
}

std::vector<double> CDistributionsGrid::GetClassSizesByDistr(EDistrTypes _distrType) const
{
	for (size_t i = 0; i < m_grids.size(); ++i)
		if (m_grids[i].distrType == _distrType)
			return GetClassSizesByIndex(i);
	return {};
}

std::vector<double> CDistributionsGrid::GetClassSizesByIndex(size_t _index) const
{
	std::vector<double> res;
	if (_index < m_grids.size() && m_grids[_index].gridEntry == EGridEntry::GRID_NUMERIC)
		for (size_t i = 0; i < m_grids[_index].numGrid.size() - 1; ++i)
			res.push_back(m_grids[_index].numGrid[i + 1] - m_grids[_index].numGrid[i]);
	return res;
}

std::vector<double> CDistributionsGrid::GetClassMeansByDistr(EDistrTypes _distrType) const
{
	for (size_t i = 0; i < m_grids.size(); ++i)
		if (m_grids[i].distrType == _distrType)
			return GetClassMeansByIndex(i);
	return {};
}

std::vector<double> CDistributionsGrid::GetClassMeansByIndex(size_t _index) const
{
	std::vector<double> res;
	if (_index < m_grids.size() && m_grids[_index].gridEntry == EGridEntry::GRID_NUMERIC)
		for (size_t i = 0; i < m_grids[_index].numGrid.size() - 1; ++i)
			res.push_back((m_grids[_index].numGrid[i] + m_grids[_index].numGrid[i + 1]) / 2);
	return res;
}

std::vector<double> CDistributionsGrid::GetPSDGrid(EPSDGridType _PSDGridType /*= EPSDGridType::DIAMETER*/) const
{
	const SGridDimension* pGrid = GetDimPtr(DISTR_SIZE);
	if (!pGrid) return {};

	switch (_PSDGridType)
	{
	case EPSDGridType::DIAMETER:	return pGrid->numGrid;
	case EPSDGridType::VOLUME:		return DiameterToVolume(pGrid->numGrid);
	}
	return {};
}

std::vector<double> CDistributionsGrid::GetPSDMeans(EPSDGridType _PSDGridType /*= EPSDGridType::DIAMETER*/) const
{
	const SGridDimension* pGrid = GetDimPtr(DISTR_SIZE);
	if (!pGrid) return {};

	std::vector<double> res;
	switch (_PSDGridType)
	{
	case EPSDGridType::DIAMETER:
		for (size_t i = 0; i < pGrid->numGrid.size() - 1; ++i)
			res.push_back((pGrid->numGrid[i] + pGrid->numGrid[i + 1]) / 2);
		break;
	case EPSDGridType::VOLUME:
		for (size_t i = 0; i < pGrid->numGrid.size() - 1; ++i)
			res.push_back((std::pow(pGrid->numGrid[i], 3) + std::pow(pGrid->numGrid[i + 1], 3)) * MATH_PI / 12);
		break;
	}
	return res;
}

void CDistributionsGrid::SaveToFile(CH5Handler& _h5File, const std::string& _sPath)
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_sPath, StrConst::DGrid_H5AttrSaveVersion, m_cnSaveVersion);

	_h5File.WriteAttribute(_sPath, StrConst::DGrid_H5AttrGridsNum, static_cast<int>(m_grids.size()));
	for (size_t i = 0; i < m_grids.size(); ++i)
	{
		const std::string sPath = _h5File.CreateGroup(_sPath, StrConst::DGrid_H5GroupName + std::to_string(i));
		_h5File.WriteData(sPath, StrConst::DGrid_H5DistrType,    E2I(m_grids[i].distrType));
		_h5File.WriteData(sPath, StrConst::DGrid_H5GridType,     E2I(m_grids[i].gridEntry));
		_h5File.WriteData(sPath, StrConst::DGrid_H5GridFun,      E2I(m_grids[i].gridFun));
		_h5File.WriteData(sPath, StrConst::DGrid_H5Units,        E2I(m_grids[i].gridUnit));
		_h5File.WriteData(sPath, StrConst::DGrid_H5Classes,      static_cast<unsigned>(m_grids[i].classes));
		_h5File.WriteData(sPath, StrConst::DGrid_H5NumGrid,      m_grids[i].numGrid);
		_h5File.WriteData(sPath, StrConst::DGrid_H5StrGrid,      m_grids[i].strGrid);
	}
}

void CDistributionsGrid::LoadFromFile(CH5Handler& _h5File, const std::string& _sPath)
{
	if (!_h5File.IsValid()) return;
	Clear();

	const int nVer = _h5File.ReadAttribute(_sPath, StrConst::DGrid_H5AttrSaveVersion);
	if (nVer == 1)
	{
		LoadFromFile_v1(_h5File, _sPath);
		return;
	}

	const int nGridsNum = _h5File.ReadAttribute(_sPath, StrConst::DGrid_H5AttrGridsNum);
	for (int i = 0; i < nGridsNum; ++i)
	{
		m_grids.emplace_back();

		const std::string sPath = _sPath + "/" + StrConst::DGrid_H5GroupName + std::to_string(i);
		_h5File.ReadData(sPath, StrConst::DGrid_H5DistrType,    reinterpret_cast<unsigned&>(m_grids[i].distrType));
		_h5File.ReadData(sPath, StrConst::DGrid_H5GridType,     reinterpret_cast<unsigned&>(m_grids[i].gridEntry));
		_h5File.ReadData(sPath, StrConst::DGrid_H5GridFun,      reinterpret_cast<unsigned&>(m_grids[i].gridFun));
		_h5File.ReadData(sPath, StrConst::DGrid_H5Units,        reinterpret_cast<unsigned&>(m_grids[i].gridUnit));
		_h5File.ReadData(sPath, StrConst::DGrid_H5Classes,      m_grids[i].classes);
		_h5File.ReadData(sPath, StrConst::DGrid_H5NumGrid,      m_grids[i].numGrid);
		_h5File.ReadData(sPath, StrConst::DGrid_H5StrGrid,      m_grids[i].strGrid);
	}
}

void CDistributionsGrid::LoadFromFile_v1(CH5Handler& _h5File, const std::string& _sPath)
{
	if (!_h5File.IsValid()) return;

	std::vector<unsigned> vDTypes, vGTypes, vClasses;
	_h5File.ReadData(_sPath, StrConst::DGrid_H5DistrType, vDTypes);
	_h5File.ReadData(_sPath, StrConst::DGrid_H5GridType, vGTypes);
	_h5File.ReadData(_sPath, StrConst::DGrid_H5Classes, vClasses);
	std::vector<std::vector<double>> vNumGrids(vDTypes.size());
	std::vector<std::vector<std::string>> vStrGrids(vDTypes.size());
	for (size_t i = 0; i < vDTypes.size(); ++i)
	{
		_h5File.ReadData(_sPath, StrConst::DGrid_H5NumGrid + std::to_string(i), vNumGrids[i]);
		_h5File.ReadData(_sPath, StrConst::DGrid_H5StrGrid + std::to_string(i), vStrGrids[i]);
	}

	for (size_t i = 0; i < vDTypes.size(); ++i)
	{
		m_grids.emplace_back();
		m_grids.back().distrType       = static_cast<EDistrTypes>(vDTypes[i]);
		m_grids.back().gridEntry       = static_cast<EGridEntry>(vGTypes[i]);
		m_grids.back().gridFun         = EGridFunction::GRID_FUN_MANUAL;
		m_grids.back().gridUnit        = EGridUnit::UNIT_DEFAULT;
		m_grids.back().classes         = vClasses[i];
		m_grids.back().numGrid         = vNumGrids[i];
		m_grids.back().strGrid         = vStrGrids[i];
	}
}

std::vector<double> CDistributionsGrid::CalculateGrid(EGridFunction _fun, size_t _classes, double _min, double _max)
{
	std::vector<double> vRes(_classes + 1);

	switch (_fun)
	{
	case EGridFunction::GRID_FUN_EQUIDISTANT:
		if (_max > _min)
		{
			const double dFactor = (_max - _min) / _classes;
			for (size_t i = 0; i < _classes + 1; ++i)
				vRes[i] = _min + dFactor * i;
		}
		break;
	case EGridFunction::GRID_FUN_GEOMETRIC_S2L:
		if (_max > _min && _min != 0)
		{
			const double dRatio = std::pow(_max / _min, 1. / _classes);
			double dSum = 0;
			for (size_t i = 0; i < _classes; ++i)
				dSum += std::pow(dRatio, i);
			const double dFactor = (_max - _min) / dSum;
			vRes[0] = _min;
			for (size_t i = 0; i < _classes; ++i)
				vRes[i + 1] = vRes[i] + std::pow(dRatio, i) * dFactor;
		}
		break;
	case EGridFunction::GRID_FUN_GEOMETRIC_L2S:
		if (_max > _min && _min != 0)
		{
			const double dRatio = 1. / std::pow(_max / _min, 1. / _classes);
			double dSum = 0;
			for (size_t i = 0; i < _classes; ++i)
				dSum += std::pow(dRatio, i);
			const double dFactor = (_max - _min) / dSum;
			vRes[0] = _min;
			for (size_t i = 0; i < _classes; ++i)
				vRes[i + 1] = vRes[i] + std::pow(dRatio, i) * dFactor;
		}
		break;
	case EGridFunction::GRID_FUN_LOGARITHMIC_S2L:
		if (_max > _min && _min != 0)
		{
			const double dL = 9. / (_classes);
			for (size_t i = 0; i < _classes + 1; ++i)
				vRes[_classes - i] = _max - std::log10(1 + i*dL) * (_max - _min);
		}
		break;
	case EGridFunction::GRID_FUN_LOGARITHMIC_L2S:
		if (_max > _min && _min != 0)
		{
			const double dL = 9. / (_classes);
			for (size_t i = 0; i < _classes + 1; ++i)
				vRes[i] = _min + std::log10(1 + i*dL) * (_max - _min);
		}
		break;
	case EGridFunction::GRID_FUN_MANUAL:
	case EGridFunction::GRID_FUN_UNDEFINED:
		break;
	}

	return vRes;
}

const SGridDimension* CDistributionsGrid::GetDimPtr(EDistrTypes _distrType) const
{
	for (const auto& dim : m_grids)
		if (dim.distrType == _distrType)
			return &dim;
	return nullptr;
}

SGridDimension* CDistributionsGrid::GetDimPtr(EDistrTypes _distrType)
{
	return const_cast<SGridDimension*>(static_cast<const CDistributionsGrid&>(*this).GetDimPtr(_distrType));
}
