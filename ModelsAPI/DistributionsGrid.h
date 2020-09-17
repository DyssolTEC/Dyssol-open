/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolTypes.h"
#include "H5Handler.h"

/// Types of grid units for size.
enum class EGridUnit : uint32_t
{
	UNIT_M       = 0,
	UNIT_MM      = 1,
	UNIT_UM      = 2,
	UNIT_M3      = 3,
	UNIT_MM3     = 4,
	UNIT_UM3     = 5,
	UNIT_DEFAULT = 256
};

struct SGridDimension
{
	EDistrTypes distrType;				// Types of dimension ( e.g. DISTRIBUTION_SIZE )
	EGridEntry gridEntry;				// Grid entry type (GRID_NUMERIC | GRID_SYMBOLIC)
	EGridFunction gridFun;				// Function, that was use to define grid (GRID_FUN_MANUAL | GRID_FUN_EQUIDISTANT | GRID_FUN_GEOMETRIC | GRID_FUN_LOGARITHMIC)
	EGridUnit gridUnit;					// Unit which was selected by user for processing (UNIT_M | UNIT_MM | UNIT_UM | UNIT_M3 | UNIT_MM3 | UNIT_UM3 | UNIT_DEFAULT)
	uint32_t classes;					// Number of classes
	std::vector<double> numGrid;		// Grid for GRID_NUMERIC
	std::vector<std::string> strGrid;	// Grid for DISCRETE_SYMBOLIC
	SGridDimension() : distrType(DISTR_COMPOUNDS), gridEntry(EGridEntry::GRID_NUMERIC), gridFun(EGridFunction::GRID_FUN_MANUAL), gridUnit(EGridUnit::UNIT_DEFAULT), classes(1) {}
};

class CDistributionsGrid
{
	static const unsigned m_cnSaveVersion;

	std::vector<SGridDimension> m_grids;

public:
	/** Clear all data. */
	void Clear();

	void AddDimension(const SGridDimension& _grid);
	void AddDimension(EDistrTypes _distrType, EGridEntry _gridEntry, const std::vector<double>& _numGrid, const std::vector<std::string>& _strGrid, EGridUnit _unit = EGridUnit::UNIT_DEFAULT,  EGridFunction _gridFun = EGridFunction::GRID_FUN_MANUAL);
	SGridDimension GetDimension(size_t _index) const;
	void SetDimension(const SGridDimension& _dim);
	size_t GetDistributionsNumber() const;

	bool IsDistrTypePresent(EDistrTypes _distrType) const;

	void AddNamedClass(EDistrTypes _distrType, const std::string& _sName);
	void RemoveNamedClass(EDistrTypes _distrType, size_t _index);

	EDistrTypes GetDistrType(size_t _index) const;
	EGridEntry GetGridEntryByDistr(EDistrTypes _distrType) const;
	EGridEntry GetGridEntryByIndex(size_t _index) const;
	EGridFunction GetGridFunctionByDistr(EDistrTypes _distrType) const;
	EGridFunction GetGridFunctionByIndex(size_t _index) const;
	size_t GetClassesByDistr(EDistrTypes _distrType) const;
	size_t GetClassesByIndex(size_t _index) const;
	EGridUnit GetGridUnitByDistr(EDistrTypes _distrType) const;
	EGridUnit GetGridUnitByIndex(size_t _index) const;

	std::vector<double> GetNumericGridByDistr(EDistrTypes _distrType) const;
	std::vector<double> GetNumericGridByIndex(size_t _index) const;
	std::vector<std::string> GetSymbolicGridByDistr(EDistrTypes _distrType) const;
	std::vector<std::string> GetSymbolicGridByIndex(size_t _index) const;

	std::vector<unsigned> GetClasses() const;
	std::vector<EDistrTypes> GetDistrTypes() const;

	std::vector<double> GetClassSizesByDistr(EDistrTypes _distrType) const;
	std::vector<double> GetClassSizesByIndex(size_t _index) const;
	std::vector<double> GetClassMeansByDistr(EDistrTypes _distrType) const;
	std::vector<double> GetClassMeansByIndex(size_t _index) const;

	std::vector<double> GetPSDGrid(EPSDGridType _PSDGridType = EPSDGridType::DIAMETER) const;
	std::vector<double> GetPSDMeans(EPSDGridType _PSDGridType = EPSDGridType::DIAMETER) const;

	// ========== Functions to save/load

	/** Save grid to file.*/
	void SaveToFile(CH5Handler& _h5File, const std::string& _sPath);
	/** Load grid from file*/
	void LoadFromFile(CH5Handler& _h5File, const std::string& _sPath);
	void LoadFromFile_v1(CH5Handler& _h5File, const std::string& _sPath);

	static std::vector<double> CalculateGrid(EGridFunction _fun, size_t _classes, double _min, double _max);

private:
	const SGridDimension* GetDimPtr(EDistrTypes _distrType) const;
	SGridDimension* GetDimPtr(EDistrTypes _distrType);
};
