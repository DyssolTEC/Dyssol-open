/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_DimensionParameters.h"
#include "MultidimensionalGrid.h"

class CMaterialsDatabase;

/// Types of grid units for size.
enum class EGridUnit : uint32_t
{
	UNIT_M = 0,
	UNIT_MM = 1,
	UNIT_UM = 2,
	UNIT_M3 = 3,
	UNIT_MM3 = 4,
	UNIT_UM3 = 5,
	UNIT_DEFAULT = 256
};

class CDimensionParameters : public QWidget
{
	Q_OBJECT
public:
	enum class EDPErrors : unsigned { DP_ERROR_NO_ERRORS, DP_ERROR_EMPTY, DP_ERROR_NEGATIVE, DP_ERROR_SEQUENCE };

private:
	Ui::CDimensionParameters ui;
	const CMaterialsDatabase& m_materialsDB;	// Reference to a global database of materials.
	std::unique_ptr<CGridDimension> m_grid;
	EGridUnit m_gridUnit{ EGridUnit::UNIT_DEFAULT };
	bool m_bAvoidSignals;

public:
	CDimensionParameters(const CMaterialsDatabase& _materialsDB, QWidget* parent = 0);

	void SetDistributionType(EDistrTypes _type);
	void SetGrid(const CGridDimension& _grid);
	CGridDimension* GetGrid() const;
	EDPErrors IsValid() const;

private:
	void SetupComboBoxGridEntry() const;
	void SetupComboBoxGridFunction() const;
	void SetupComboBoxUnits() const;

	void SelectGridEntry(EGridEntry _entry) const;
	EGridEntry GetSelectedGridEntry() const;
	void SelectGridFunction(EGridFunction _function) const;
	EGridFunction GetSelectedGridFunction() const;
	void SelectGridUnit(EGridUnit _unit) const;
	EGridUnit GetSelectedGridUnit() const;
	void SelectDisplayGridUnit(EGridUnit _unit) const;
	EGridUnit GetSelectedDisplayGridUnit() const;

	void UpdateWholeView();
	void UpdateTableName() const;
	void UpdateControls();
	void UpdateUnitsVisibility() const;
	void UpdateLimitsVisibility() const;
	void UpdateGrid();
	void UpdateGridData(EDistrTypes _dim, const std::vector<double>& _vNumGrid, const std::vector<std::string>& _vStrGrid) const;

	std::vector<double> CalculateGrid() const;
	double ToSI(double _dVal) const;
	void ToSI(std::vector<double>& _vVal) const;
	double SizeToSI(double _dVal, EGridUnit _srcUnits) const;
	double FromSI(double _dVal) const;
	void FromSI(std::vector<double>& _vVal) const;
	double SizeFromSI(double _dVal, EGridUnit _dstUnits) const;

private slots:
	void GridEntryChanged();
	void ClassesChanged();
	void GridFunChanged();
	void LimitsChanged();
	void GridDataChanged();
	void UnitsChanged(int _index);
};
