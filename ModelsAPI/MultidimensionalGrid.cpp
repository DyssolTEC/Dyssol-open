/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "MultidimensionalGrid.h"
#include "H5Handler.h"
#include "ContainerFunctions.h"
#include "DyssolUtilities.h"
#include "DyssolStringConstants.h"
#include <algorithm>
#include <utility>

struct FindByType
{
	explicit FindByType(EDistrTypes _type) : m_type{ _type } {}
	bool operator()(const std::unique_ptr<CGridDimension>& _d) const
	{
		return _d->DimensionType() == m_type;
	}
private:
	EDistrTypes m_type;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
///////////// CGridDimension

CGridDimension::CGridDimension(EGridEntry _entry)
	: m_entry{ _entry }
{
}

CGridDimension::CGridDimension(EDistrTypes _type, EGridEntry _entry)
	: m_type{ _type }
	, m_entry{ _entry }
{
}

CGridDimension* CGridDimension::Clone() const
{
	return new CGridDimension(*this);
}

bool CGridDimension::Equal(const CGridDimension& _other) const
{
	return m_type == _other.m_type && m_entry == _other.m_entry;
}

bool operator==(const CGridDimension& _lhs, const CGridDimension& _rhs)
{
	if (typeid(_lhs) != typeid(_rhs)) return false;
	return _lhs.Equal(_rhs);
}

bool operator!=(const CGridDimension& _lhs, const CGridDimension& _rhs)
{
	return !(_lhs == _rhs);
}

EDistrTypes CGridDimension::DimensionType() const
{
	return m_type;
}

EGridEntry CGridDimension::GridType() const
{
	return m_entry;
}

size_t CGridDimension::ClassesNumber() const
{
	return 0;
}

void CGridDimension::SetType(EDistrTypes _type)
{
	m_type = _type;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///////////// CGridDimensionNumeric

CGridDimensionNumeric::CGridDimensionNumeric()
	: CGridDimension{ EGridEntry::GRID_NUMERIC }
{
}

CGridDimensionNumeric::CGridDimensionNumeric(EDistrTypes _type)
	: CGridDimension{ _type, EGridEntry::GRID_NUMERIC }
{
}

CGridDimensionNumeric::CGridDimensionNumeric(EDistrTypes _type, std::vector<double> _grid)
	: CGridDimension{ _type, EGridEntry::GRID_NUMERIC }
	, m_grid{ std::move(_grid) }
{
}

std::vector<double> CGridDimensionNumeric::GetClassesMeans() const
{
	if (m_grid.size() < 2) return {};
	auto res = ReservedVector<double>(m_grid.size() - 1);
	for (size_t i = 0; i < m_grid.size() - 1; ++i)
		res.push_back((m_grid[i] + m_grid[i + 1]) / 2);
	return res;
}

std::vector<double> CGridDimensionNumeric::GetClassesSizes() const
{
	if (m_grid.size() < 2) return {};
	auto res = ReservedVector<double>(m_grid.size() - 1);
	for (size_t i = 0; i < m_grid.size() - 1; ++i)
		res.push_back(m_grid[i + 1] - m_grid[i]);
	return res;
}

void CGridDimensionNumeric::SaveToFile(const CH5Handler& _h5File, const std::string& _path) const
{
	_h5File.WriteData(_path, StrConst::DGrid_H5DistrType, E2I(DimensionType()));
	_h5File.WriteData(_path, StrConst::DGrid_H5GridType, E2I(GridType()));
	_h5File.WriteData(_path, StrConst::DGrid_H5NumGrid, m_grid);
}

void CGridDimensionNumeric::LoadFromFile(const CH5Handler& _h5File, const std::string& _path)
{
	EDistrTypes type;
	_h5File.ReadData(_path, StrConst::DGrid_H5DistrType, reinterpret_cast<uint32_t&>(type));
	SetType(type);
	_h5File.ReadData(_path, StrConst::DGrid_H5NumGrid, m_grid);
}

CGridDimensionNumeric* CGridDimensionNumeric::Clone() const
{
	return new CGridDimensionNumeric(*this);
}

bool CGridDimensionNumeric::Equal(const CGridDimension& _other) const
{
	const auto* p = dynamic_cast<const CGridDimensionNumeric*>(&_other);
	return p && CGridDimension::Equal(_other) && m_grid == p->m_grid;
}

size_t CGridDimensionNumeric::ClassesNumber() const
{
	return m_grid.size() - 1;
}

std::vector<double> CGridDimensionNumeric::Grid() const
{
	return m_grid;
}

void CGridDimensionNumeric::SetGrid(const std::vector<double>& _grid)
{
	m_grid = _grid;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///////////// CGridDimensionSymbolic

CGridDimensionSymbolic::CGridDimensionSymbolic()
	: CGridDimension{ EGridEntry::GRID_SYMBOLIC }
{
}

CGridDimensionSymbolic::CGridDimensionSymbolic(EDistrTypes _type)
	: CGridDimension{ _type, EGridEntry::GRID_SYMBOLIC }
{
}

CGridDimensionSymbolic::CGridDimensionSymbolic(EDistrTypes _type, std::vector<std::string> _grid)
	: CGridDimension{ _type, EGridEntry::GRID_SYMBOLIC }
	, m_grid{ std::move(_grid) }
{
}

void CGridDimensionSymbolic::AddClass(const std::string& _entry)
{
	m_grid.push_back(_entry);
}

void CGridDimensionSymbolic::RemoveClass(const std::string& _entry)
{
	if (!VectorContains(m_grid, _entry)) return;
	VectorDelete(m_grid, _entry);
}

void CGridDimensionSymbolic::SaveToFile(const CH5Handler& _h5File, const std::string& _path) const
{
	_h5File.WriteData(_path, StrConst::DGrid_H5DistrType, E2I(DimensionType()));
	_h5File.WriteData(_path, StrConst::DGrid_H5GridType, E2I(GridType()));
	_h5File.WriteData(_path, StrConst::DGrid_H5StrGrid, m_grid);
}

void CGridDimensionSymbolic::LoadFromFile(const CH5Handler& _h5File, const std::string& _path)
{
	EDistrTypes type;
	_h5File.ReadData(_path, StrConst::DGrid_H5DistrType, reinterpret_cast<uint32_t&>(type));
	SetType(type);
	_h5File.ReadData(_path, StrConst::DGrid_H5StrGrid, m_grid);
}

CGridDimensionSymbolic* CGridDimensionSymbolic::Clone() const
{
	return new CGridDimensionSymbolic(*this);
}

bool CGridDimensionSymbolic::Equal(const CGridDimension& _other) const
{
	const auto* p = dynamic_cast<const CGridDimensionSymbolic*>(&_other);
	return p && CGridDimension::Equal(_other) && m_grid == p->m_grid;
}

size_t CGridDimensionSymbolic::ClassesNumber() const
{
	return m_grid.size();
}

std::vector<std::string> CGridDimensionSymbolic::Grid() const
{
	return m_grid;
}

void CGridDimensionSymbolic::SetGrid(const std::vector<std::string>& _grid)
{
	m_grid = _grid;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///////////// CMultidimensionalGrid

CMultidimensionalGrid::CMultidimensionalGrid(const CMultidimensionalGrid& _other)
{
	for (const auto& d : _other.m_grids)
		AddDimension(*d);
}

CMultidimensionalGrid& CMultidimensionalGrid::operator=(const CMultidimensionalGrid& _other)
{
	if (this != &_other)
	{
		Clear();
		for (const auto& d : _other.m_grids)
			AddDimension(*d);
	}
	return *this;
}

void CMultidimensionalGrid::Clear()
{
	m_grids.clear();
}

std::vector<size_t> CMultidimensionalGrid::GetClassesNumbers() const
{
	std::vector<size_t> res(m_grids.size());
	for (size_t i = 0; i < m_grids.size(); ++i)
		res[i] = m_grids[i]->ClassesNumber();
	return res;
}

std::vector<EDistrTypes> CMultidimensionalGrid::GetDimensionsTypes() const
{
	std::vector<EDistrTypes> res(m_grids.size());
	for (size_t i = 0; i < m_grids.size(); ++i)
		res[i] = m_grids[i]->DimensionType();
	return res;
}

std::vector<EGridEntry> CMultidimensionalGrid::GetGridsTypes() const
{
	std::vector<EGridEntry> res(m_grids.size());
	for (size_t i = 0; i < m_grids.size(); ++i)
		res[i] = m_grids[i]->GridType();
	return res;
}

size_t CMultidimensionalGrid::GetDimensionsNumber() const
{
	return m_grids.size();
}

void CMultidimensionalGrid::AddDimension(const CGridDimension& _gridDimension)
{
	if (HasDimension(_gridDimension.DimensionType())) return;
	if (_gridDimension.GridType() == EGridEntry::GRID_NUMERIC)
		AddNumericDimension( dynamic_cast<const CGridDimensionNumeric&>(_gridDimension));
	else
		AddSymbolicDimension(dynamic_cast<const CGridDimensionSymbolic&>(_gridDimension));
}

CGridDimensionNumeric* CMultidimensionalGrid::AddNumericDimension(EDistrTypes _type)
{
	return AddNumericDimension(CGridDimensionNumeric{ _type });
}

CGridDimensionNumeric* CMultidimensionalGrid::AddNumericDimension(EDistrTypes _type, const std::vector<double>& _grid)
{
	return AddNumericDimension({ _type, _grid });
}

CGridDimensionNumeric* CMultidimensionalGrid::AddNumericDimension(const CGridDimensionNumeric& _gridDimension)
{
	if (HasDimension(_gridDimension.DimensionType())) return {};
	const auto& grid = _gridDimension.Grid();
	if (!grid.empty() && !std::is_sorted(grid.begin(), grid.end())) return {};
	return dynamic_cast<CGridDimensionNumeric*>(m_grids.emplace_back(std::make_unique<CGridDimensionNumeric>(_gridDimension)).get());
}

CGridDimensionSymbolic* CMultidimensionalGrid::AddSymbolicDimension(EDistrTypes _type)
{
	return AddSymbolicDimension(CGridDimensionSymbolic{ _type });
}

CGridDimensionSymbolic* CMultidimensionalGrid::AddSymbolicDimension(EDistrTypes _type, const std::vector<std::string>& _grid)
{
	return AddSymbolicDimension({ _type, _grid });
}

CGridDimensionSymbolic* CMultidimensionalGrid::AddSymbolicDimension(const CGridDimensionSymbolic& _gridDimension)
{
	if (HasDimension(_gridDimension.DimensionType())) return {};
	if (!VectorUnique(_gridDimension.Grid())) return {};
	return dynamic_cast<CGridDimensionSymbolic*>(m_grids.emplace_back(std::make_unique<CGridDimensionSymbolic>(_gridDimension)).get());
}

const CGridDimension* CMultidimensionalGrid::GetGridDimension(EDistrTypes _type) const
{
	const auto it = std::find_if(m_grids.begin(), m_grids.end(), FindByType{ _type });
	if (it != m_grids.end()) return it->get();
	return nullptr;
}

CGridDimension* CMultidimensionalGrid::GetGridDimension(EDistrTypes _type)
{
	return const_cast<CGridDimension*>(static_cast<const CMultidimensionalGrid&>(*this).GetGridDimension(_type));
}

const CGridDimensionNumeric* CMultidimensionalGrid::GetGridDimensionNumeric(EDistrTypes _type) const
{
	return dynamic_cast<const CGridDimensionNumeric*>(GetGridDimension(_type));
}

CGridDimensionNumeric* CMultidimensionalGrid::GetGridDimensionNumeric(EDistrTypes _type)
{
	return const_cast<CGridDimensionNumeric*>(static_cast<const CMultidimensionalGrid&>(*this).GetGridDimensionNumeric(_type));
}

const CGridDimensionSymbolic* CMultidimensionalGrid::GetGridDimensionSymbolic(EDistrTypes _type) const
{
	return dynamic_cast<const CGridDimensionSymbolic*>(GetGridDimension(_type));
}

CGridDimensionSymbolic* CMultidimensionalGrid::GetGridDimensionSymbolic(EDistrTypes _type)
{
	return const_cast<CGridDimensionSymbolic*>(static_cast<const CMultidimensionalGrid&>(*this).GetGridDimensionSymbolic(_type));
}

void CMultidimensionalGrid::RemoveDimension(EDistrTypes _type)
{
	if (!HasDimension(_type)) return;
	VectorDelete(m_grids, FindByType{ _type });
}

std::vector<const CGridDimension*> CMultidimensionalGrid::GetGridDimensions() const
{
	std::vector<const CGridDimension*> res(m_grids.size());
	for (size_t i = 0; i < m_grids.size(); ++i)
		res[i] = m_grids[i].get();
	return res;
}

std::vector<CGridDimension*> CMultidimensionalGrid::GetGridDimensions()
{
	std::vector<CGridDimension*> res(m_grids.size());
	for (size_t i = 0; i < m_grids.size(); ++i)
		res[i] = m_grids[i].get();
	return res;
}

std::vector<double> CMultidimensionalGrid::GetNumericGrid(EDistrTypes _type) const
{
	if (const auto* dim = GetGridDimensionNumeric(_type))
		return dim->Grid();
	return {};
}

std::vector<std::string> CMultidimensionalGrid::GetSymbolicGrid(EDistrTypes _type) const
{
	if (const auto* dim = GetGridDimensionSymbolic(_type))
		return dim->Grid();
	return {};
}

std::vector<double> CMultidimensionalGrid::GetPSDGrid(EPSDGridType _type) const
{
	const auto* dim = GetGridDimensionNumeric(DISTR_SIZE);
	if (!dim) return {};
	switch (_type)
	{
	case EPSDGridType::DIAMETER:	return dim->Grid();
	case EPSDGridType::VOLUME:		return DiameterToVolume(dim->Grid());
	}
	return {};
}

std::vector<double> CMultidimensionalGrid::GetPSDMeans(EPSDGridType _type) const
{
	const auto* dim = GetGridDimensionNumeric(DISTR_SIZE);
	if (!dim) return {};
	switch (_type)
	{
	case EPSDGridType::DIAMETER:	return dim->GetClassesMeans();
	case EPSDGridType::VOLUME:
	{
		const auto& volumes = GetPSDGrid(EPSDGridType::VOLUME);
		std::vector<double> res;
		for (size_t i = 0; i < dim->ClassesNumber(); ++i)
			res.push_back((volumes[i] + volumes[i + 1]) / 2);
		return res;
	}
	}
	return {};
}

bool CMultidimensionalGrid::HasDimension(EDistrTypes _type) const
{
	return std::any_of(m_grids.begin(), m_grids.end(), FindByType{ _type });
}

bool CMultidimensionalGrid::operator==(const CMultidimensionalGrid& _other) const
{
	if (m_grids.size() != _other.m_grids.size()) return false;
	for (size_t i = 0; i < m_grids.size(); ++i)
		if (*m_grids[i] != *_other.m_grids[i])
			return false;
	return true;
}

bool CMultidimensionalGrid::operator!=(const CMultidimensionalGrid& _other) const
{
	return !(*this == _other);
}

void CMultidimensionalGrid::SaveToFile(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::DGrid_H5AttrSaveVersion, m_saveVersion);
	// number of grid dimensions
	_h5File.WriteAttribute(_path, StrConst::DGrid_H5AttrGridsNum, static_cast<int>(m_grids.size()));
	// grid types for all dimensions
	_h5File.WriteData(_path, StrConst::DGrid_H5GridType, E2I(GetGridsTypes()));
	// save each grid dimension
	for (size_t i = 0; i < m_grids.size(); ++i)
		switch (m_grids[i]->GridType())
		{
		case EGridEntry::GRID_NUMERIC:	dynamic_cast<CGridDimensionNumeric*> (m_grids[i].get())->SaveToFile(_h5File, _h5File.CreateGroup(_path, StrConst::DGrid_H5GroupName + std::to_string(i)));  break;
		case EGridEntry::GRID_SYMBOLIC: dynamic_cast<CGridDimensionSymbolic*>(m_grids[i].get())->SaveToFile(_h5File, _h5File.CreateGroup(_path, StrConst::DGrid_H5GroupName + std::to_string(i)));  break;
		case EGridEntry::GRID_UNDEFINED: break;
		}
}

void CMultidimensionalGrid::LoadFromFile(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;
	Clear();

	const int version = _h5File.ReadAttribute(_path, StrConst::DGrid_H5AttrSaveVersion);
	if (version == 1)
	{
		LoadFromFile_v1(_h5File, _path);
		return;
	}
	if (version == 2)
	{
		LoadFromFile_v2(_h5File, _path);
		return;
	}

	// number of grid dimensions
	const int number = _h5File.ReadAttribute(_path, StrConst::DGrid_H5AttrGridsNum);
	// grid types for all dimensions
	std::vector<uint32_t> gridTypes;
	_h5File.ReadData(_path, StrConst::DGrid_H5GridType, gridTypes);
	// load each grid dimension
	for (int i = 0; i < number; ++i)
		switch (static_cast<EGridEntry>(gridTypes[i]))
		{
		case EGridEntry::GRID_NUMERIC:	AddNumericDimension ()->LoadFromFile(_h5File, _path + "/" + StrConst::DGrid_H5GroupName + std::to_string(i)); break;
		case EGridEntry::GRID_SYMBOLIC:	AddSymbolicDimension()->LoadFromFile(_h5File, _path + "/" + StrConst::DGrid_H5GroupName + std::to_string(i)); break;
		case EGridEntry::GRID_UNDEFINED: break;
		}
}

void CMultidimensionalGrid::LoadFromFile_v2(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;
	Clear();

	const int gridsNum = _h5File.ReadAttribute(_path, StrConst::DGrid_H5AttrGridsNum);
	for (int i = 0; i < gridsNum; ++i)
	{
		EDistrTypes distrType{};
		std::vector<double> numGrid;
		std::vector<std::string> strGrid;
		const std::string path = _path + "/" + StrConst::DGrid_H5GroupName + std::to_string(i);
		_h5File.ReadData(path, StrConst::DGrid_H5DistrType, reinterpret_cast<uint32_t&>(distrType));
		_h5File.ReadData(path, StrConst::DGrid_H5NumGrid, numGrid);
		_h5File.ReadData(path, StrConst::DGrid_H5StrGrid, strGrid);

		if (!numGrid.empty())
			AddNumericDimension(distrType, numGrid);
		else
			AddSymbolicDimension(distrType, strGrid);
	}
}

void CMultidimensionalGrid::LoadFromFile_v1(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;
	Clear();

	std::vector<unsigned> types;
	_h5File.ReadData(_path, StrConst::DGrid_H5DistrType, types);
	std::vector<std::vector<double>> numGrids(types.size());
	std::vector<std::vector<std::string>> strGrids(types.size());
	for (size_t i = 0; i < types.size(); ++i)
	{
		_h5File.ReadData(_path, StrConst::DGrid_H5NumGrid + std::to_string(i), numGrids[i]);
		_h5File.ReadData(_path, StrConst::DGrid_H5StrGrid + std::to_string(i), strGrids[i]);
	}

	for (size_t i = 0; i < types.size(); ++i)
		if (!numGrids.empty())
			AddNumericDimension(static_cast<EDistrTypes>(types[i]), numGrids[i]);
		else
			AddSymbolicDimension(static_cast<EDistrTypes>(types[i]), strGrids[i]);
}


