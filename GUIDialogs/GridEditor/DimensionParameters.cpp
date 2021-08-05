/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DimensionParameters.h"
#include "DyssolStringConstants.h"
#include "DyssolUtilities.h"
#include "MaterialsDatabase.h"

CDimensionParameters::CDimensionParameters(const CMaterialsDatabase& _materialsDB, QWidget* parent)
	: QWidget(parent)
	, m_materialsDB{ _materialsDB }
{
	ui.setupUi(this);

	m_bAvoidSignals = false;
	m_grid = std::make_unique<CGridDimensionNumeric>();
	SetupComboBoxGridFunction();
	SetupComboBoxGridEntry();
	SetupComboBoxUnits();

	connect(ui.comboBoxGridEntry,	QOverload<int>::of(&QComboBox::currentIndexChanged),	this, &CDimensionParameters::GridEntryChanged);
	connect(ui.comboBoxGridFun,		QOverload<int>::of(&QComboBox::currentIndexChanged), 	this, &CDimensionParameters::GridFunChanged);
	connect(ui.spinBoxClasses,		QOverload<int>::of(&QSpinBox::valueChanged),			this, &CDimensionParameters::ClassesChanged);
	connect(ui.comboBoxUnits,		QOverload<int>::of(&QComboBox::currentIndexChanged),	this, &CDimensionParameters::UnitsChanged);
	connect(ui.comboBoxDispUnits,	QOverload<int>::of(&QComboBox::currentIndexChanged),	this, &CDimensionParameters::UnitsChanged);
	connect(ui.lineEditLimitMin,	&QLineEdit::textEdited,									this, &CDimensionParameters::LimitsChanged);
	connect(ui.lineEditLimitMax,	&QLineEdit::textEdited,									this, &CDimensionParameters::LimitsChanged);
	connect(ui.tableWidgetGrid,		&CQtTable::cellChanged,									this, &CDimensionParameters::GridDataChanged);

	UpdateWholeView();
}

void CDimensionParameters::SetDistributionType(EDistrTypes _type)
{
	if (!m_grid) return;
	m_grid->SetType(_type);
	UpdateTableName();
	UpdateUnitsVisibility();
}

void CDimensionParameters::SetGrid(const CGridDimension& _grid)
{
	if (_grid.GridType() == EGridEntry::GRID_NUMERIC)
		m_grid.reset(new CGridDimensionNumeric(dynamic_cast<const CGridDimensionNumeric&>(_grid)));
	else
		m_grid.reset(new CGridDimensionSymbolic(dynamic_cast<const CGridDimensionSymbolic&>(_grid)));

	UpdateTableName();
	UpdateLimitsVisibility();
	UpdateUnitsVisibility();
	UpdateControls();
	UpdateGridData(m_grid->DimensionType(),
		m_grid->GridType() == EGridEntry::GRID_NUMERIC ? dynamic_cast<CGridDimensionNumeric*>(m_grid.get())->Grid() : std::vector<double>{},
		m_grid->GridType() == EGridEntry::GRID_SYMBOLIC ? dynamic_cast<CGridDimensionSymbolic*>(m_grid.get())->Grid() : std::vector<std::string>{});
	UpdateGrid();
}

CGridDimension* CDimensionParameters::GetGrid() const
{
	return m_grid.get();
}

CDimensionParameters::EDPErrors CDimensionParameters::IsValid() const
{
	if (!m_grid) return { EDPErrors::DP_ERROR_EMPTY };
	switch (m_grid->GridType())
	{
	case EGridEntry::GRID_NUMERIC:
	{
		const auto& grid = dynamic_cast<CGridDimensionNumeric*>(m_grid.get())->Grid();
		for (size_t i = 0; i < grid.size(); ++i)
		{
			if (grid[i] < 0)
				return EDPErrors::DP_ERROR_NEGATIVE;
			if (i > 0 && grid[i] <= grid[i - 1])
				return EDPErrors::DP_ERROR_SEQUENCE;
		}
		break;
	}
	case EGridEntry::GRID_SYMBOLIC:
		for (const auto& str : dynamic_cast<CGridDimensionSymbolic*>(m_grid.get())->Grid())
			if (str.empty())
				return EDPErrors::DP_ERROR_EMPTY;
		break;
	}

	return EDPErrors::DP_ERROR_NO_ERRORS;
}

void CDimensionParameters::SetupComboBoxGridEntry() const
{
	ui.comboBoxGridEntry->clear();
	ui.comboBoxGridEntry->insertItem(E2I(EGridEntry::GRID_NUMERIC),  StrConst::GE_GridEntryNumeric,  E2I(EGridEntry::GRID_NUMERIC));
	ui.comboBoxGridEntry->insertItem(E2I(EGridEntry::GRID_SYMBOLIC), StrConst::GE_GridEntrySymbolic, E2I(EGridEntry::GRID_SYMBOLIC));
	SelectGridEntry(EGridEntry::GRID_NUMERIC);
}

void CDimensionParameters::SetupComboBoxGridFunction() const
{
	ui.comboBoxGridFun->clear();
	ui.comboBoxGridFun->insertItem(E2I(EGridFunction::GRID_FUN_MANUAL), StrConst::GE_GridTypeManual, E2I(EGridFunction::GRID_FUN_MANUAL));
	ui.comboBoxGridFun->insertItem(E2I(EGridFunction::GRID_FUN_EQUIDISTANT), StrConst::GE_GridTypeEquidistant, E2I(EGridFunction::GRID_FUN_EQUIDISTANT));
	ui.comboBoxGridFun->insertItem(E2I(EGridFunction::GRID_FUN_GEOMETRIC_S2L), StrConst::GE_GridTypeGeometricS2L, E2I(EGridFunction::GRID_FUN_GEOMETRIC_S2L));
	ui.comboBoxGridFun->insertItem(E2I(EGridFunction::GRID_FUN_LOGARITHMIC_S2L), StrConst::GE_GridTypeLogarithmicS2L, E2I(EGridFunction::GRID_FUN_LOGARITHMIC_S2L));
	ui.comboBoxGridFun->insertItem(E2I(EGridFunction::GRID_FUN_GEOMETRIC_L2S), StrConst::GE_GridTypeGeometricL2S, E2I(EGridFunction::GRID_FUN_GEOMETRIC_L2S));
	ui.comboBoxGridFun->insertItem(E2I(EGridFunction::GRID_FUN_LOGARITHMIC_L2S), StrConst::GE_GridTypeLogarithmicL2S, E2I(EGridFunction::GRID_FUN_LOGARITHMIC_L2S));
	SelectGridFunction(EGridFunction::GRID_FUN_MANUAL);
}

void CDimensionParameters::SetupComboBoxUnits() const
{
	ui.comboBoxUnits->clear();
	ui.comboBoxUnits->insertItem(static_cast<int>(EGridUnit::UNIT_M),	"m");
	ui.comboBoxUnits->insertItem(static_cast<int>(EGridUnit::UNIT_MM),	"mm");
	ui.comboBoxUnits->insertItem(static_cast<int>(EGridUnit::UNIT_UM),	"um");
	ui.comboBoxUnits->insertItem(static_cast<int>(EGridUnit::UNIT_M3),	"m3");
	ui.comboBoxUnits->insertItem(static_cast<int>(EGridUnit::UNIT_MM3),	"mm3");
	ui.comboBoxUnits->insertItem(static_cast<int>(EGridUnit::UNIT_UM3),	"um3");

	ui.comboBoxDispUnits->clear();
	ui.comboBoxDispUnits->insertItem(static_cast<int>(EGridUnit::UNIT_M),	"m");
	ui.comboBoxDispUnits->insertItem(static_cast<int>(EGridUnit::UNIT_MM),	"mm");
	ui.comboBoxDispUnits->insertItem(static_cast<int>(EGridUnit::UNIT_UM),	"um");
	ui.comboBoxDispUnits->insertItem(static_cast<int>(EGridUnit::UNIT_M3),	"m3");
	ui.comboBoxDispUnits->insertItem(static_cast<int>(EGridUnit::UNIT_MM3),	"mm3");
	ui.comboBoxDispUnits->insertItem(static_cast<int>(EGridUnit::UNIT_UM3),	"um3");
}

void CDimensionParameters::SelectGridEntry(EGridEntry _entry) const
{
	for (int i = 0; i < ui.comboBoxGridEntry->count(); ++i)
		if (ui.comboBoxGridEntry->itemData(i).toInt() == E2I(_entry))
			ui.comboBoxGridEntry->setCurrentIndex(i);
}

EGridEntry CDimensionParameters::GetSelectedGridEntry() const
{
	return static_cast<EGridEntry>(ui.comboBoxGridEntry->currentData().toInt());
}

void CDimensionParameters::SelectGridFunction(EGridFunction _function) const
{
	for (int i = 0; i < ui.comboBoxGridFun->count(); ++i)
		if (ui.comboBoxGridFun->itemData(i).toInt() == E2I(_function))
			ui.comboBoxGridFun->setCurrentIndex(i);
}

EGridFunction CDimensionParameters::GetSelectedGridFunction() const
{
	return static_cast<EGridFunction>(ui.comboBoxGridFun->currentData().toInt());
}

void CDimensionParameters::SelectGridUnit(EGridUnit _unit) const
{
	for (int i = 0; i < ui.comboBoxUnits->count(); ++i)
		if (i == E2I(_unit))
			ui.comboBoxUnits->setCurrentIndex(i);
}

EGridUnit CDimensionParameters::GetSelectedGridUnit() const
{
	return static_cast<EGridUnit>(ui.comboBoxUnits->currentIndex());
}

void CDimensionParameters::SelectDisplayGridUnit(EGridUnit _unit) const
{
	for (int i = 0; i < ui.comboBoxDispUnits->count(); ++i)
		if (i == E2I(_unit))
			ui.comboBoxDispUnits->setCurrentIndex(i);
}

EGridUnit CDimensionParameters::GetSelectedDisplayGridUnit() const
{
	return static_cast<EGridUnit>(ui.comboBoxDispUnits->currentIndex());
}

void CDimensionParameters::UpdateWholeView()
{
	UpdateLimitsVisibility();
	UpdateUnitsVisibility();
	UpdateGrid();
}

void CDimensionParameters::UpdateTableName() const
{
	if (!m_grid) return;
	const unsigned iDistr = GetDistributionTypeIndex(m_grid->DimensionType());
	const QString sName = QStringList{ DISTR_NAMES }[iDistr];
	ui.tableWidgetGrid->setHorizontalHeaderItem(0, new QTableWidgetItem(sName));
}

void CDimensionParameters::UpdateControls()
{
	if (!m_grid) return;
	m_bAvoidSignals = true;

	const auto& gridNum = m_grid->GridType() == EGridEntry::GRID_NUMERIC ? dynamic_cast<CGridDimensionNumeric*>(m_grid.get())->Grid() : std::vector<double>{};
	const auto& gridStr = m_grid->GridType() == EGridEntry::GRID_SYMBOLIC ? dynamic_cast<CGridDimensionSymbolic*>(m_grid.get())->Grid() : std::vector<std::string>{};
	SelectGridEntry(m_grid->GridType());
	SelectGridFunction(m_grid->GridType() == EGridEntry::GRID_NUMERIC ? dynamic_cast<CGridDimensionNumeric*>(m_grid.get())->Function() : EGridFunction::GRID_FUN_MANUAL);
	SelectGridUnit(m_gridUnit);
	SelectDisplayGridUnit(m_gridUnit);
	if (!gridNum.empty())
	{
		ui.lineEditLimitMin->setText(QString::number(SizeFromSI(gridNum.front(), static_cast<EGridUnit>(ui.comboBoxUnits->currentIndex()))));
		ui.lineEditLimitMax->setText(QString::number(SizeFromSI(gridNum.back(),  static_cast<EGridUnit>(ui.comboBoxUnits->currentIndex()))));
	}
	switch (m_grid->GridType())
	{
	case EGridEntry::GRID_NUMERIC:
		ui.spinBoxClasses->setValue((int)gridNum.size() - 1);
		break;
	case EGridEntry::GRID_SYMBOLIC:
		ui.spinBoxClasses->setValue((int)gridStr.size());
		break;
	}

	m_bAvoidSignals = false;
}

void CDimensionParameters::UpdateUnitsVisibility() const
{
	if (!m_grid) return;
	const bool bVisible1 = m_grid->DimensionType() == DISTR_SIZE && m_grid->GridType() == EGridEntry::GRID_NUMERIC;
	ui.labelUnits->setVisible(bVisible1);
	ui.comboBoxUnits->setVisible(bVisible1);
	const bool bVisible2 = bVisible1 && (m_grid->GridType() == EGridEntry::GRID_NUMERIC && dynamic_cast<CGridDimensionNumeric*>(m_grid.get())->Function() != EGridFunction::GRID_FUN_MANUAL);
	ui.labelDispUnits->setVisible(bVisible2);
	ui.comboBoxDispUnits->setVisible(bVisible2);
}

void CDimensionParameters::UpdateLimitsVisibility() const
{
	if (!m_grid) return;
	const bool bVisible1 = m_grid->GridType() == EGridEntry::GRID_NUMERIC;
	ui.labelDistrFunction->setVisible(bVisible1);
	ui.comboBoxGridFun->setVisible(bVisible1);
	const bool bVisible2 = m_grid->GridType() == EGridEntry::GRID_NUMERIC && (m_grid->GridType() == EGridEntry::GRID_NUMERIC && dynamic_cast<CGridDimensionNumeric*>(m_grid.get())->Function() != EGridFunction::GRID_FUN_MANUAL);
	ui.labelMin->setVisible(bVisible2);
	ui.lineEditLimitMin->setVisible(bVisible2);
	ui.labelMax->setVisible(bVisible2);
	ui.lineEditLimitMax->setVisible(bVisible2);
}

void CDimensionParameters::UpdateGrid()
{
	if (!m_grid) return;
	switch (m_grid->GridType())
	{
	case EGridEntry::GRID_NUMERIC:
	{
		auto* numeric = dynamic_cast<CGridDimensionNumeric*>(m_grid.get());
		switch (numeric->Function())
		{
		case EGridFunction::GRID_FUN_MANUAL:
		{
			auto grid = numeric->Grid();
			grid.resize(ui.spinBoxClasses->value() + 1);
			numeric->SetGrid(grid);
			break;
		}
		case EGridFunction::GRID_FUN_EQUIDISTANT:
		case EGridFunction::GRID_FUN_GEOMETRIC_S2L:
		case EGridFunction::GRID_FUN_LOGARITHMIC_S2L:
		case EGridFunction::GRID_FUN_GEOMETRIC_L2S:
		case EGridFunction::GRID_FUN_LOGARITHMIC_L2S:
			numeric->SetGrid(CalculateGrid());
			break;
		case EGridFunction::GRID_FUN_UNDEFINED:
			break;
		}
		break;
	}
	case EGridEntry::GRID_SYMBOLIC:	// just use current values
	{
		auto grid = dynamic_cast<CGridDimensionSymbolic*>(m_grid.get())->Grid();
		grid.resize(ui.spinBoxClasses->value());
		dynamic_cast<CGridDimensionSymbolic*>(m_grid.get())->SetGrid(grid);
		break;
	}
	}

	auto gridNum = m_grid->GridType() == EGridEntry::GRID_NUMERIC ? dynamic_cast<CGridDimensionNumeric*>(m_grid.get())->Grid() : std::vector<double>{};
	const auto& gridStr = m_grid->GridType() == EGridEntry::GRID_SYMBOLIC ? dynamic_cast<CGridDimensionSymbolic*>(m_grid.get())->Grid() : std::vector<std::string>{};
	FromSI(gridNum);

	UpdateGridData(m_grid->DimensionType(), gridNum, gridStr);
}

void CDimensionParameters::UpdateGridData(EDistrTypes _dim, const std::vector<double>& _vNumGrid, const std::vector<std::string>& _vStrGrid) const
{
	if (!m_grid) return;
	QSignalBlocker blocker(ui.tableWidgetGrid);

	switch (m_grid->GridType())
	{
	case EGridEntry::GRID_NUMERIC:
		ui.tableWidgetGrid->setRowCount((int)_vNumGrid.size());
		ui.tableWidgetGrid->SetItemsColEditable(0, 0, _vNumGrid);
		break;
	case EGridEntry::GRID_SYMBOLIC:
		ui.tableWidgetGrid->setRowCount((int)_vStrGrid.size());
		if (_dim != DISTR_COMPOUNDS)
			ui.tableWidgetGrid->SetItemsColEditable(0, 0, _vStrGrid);
		else
		{
			std::vector<std::string> names;
			for (const auto& key : _vStrGrid)
				if (const auto* compound = m_materialsDB.GetCompound(key))
					names.push_back(compound->GetName());
				else
					names.push_back(key);
			ui.tableWidgetGrid->SetItemsColEditable(0, 0, names);
		}
		break;
	}
}

std::vector<double> CDimensionParameters::CalculateGrid() const
{
	const double dMin = ui.lineEditLimitMin->text().toDouble();
	const double dMax = ui.lineEditLimitMax->text().toDouble();

	std::vector<double> vRes = CreateGrid(dynamic_cast<CGridDimensionNumeric*>(m_grid.get())->Function(), ui.spinBoxClasses->value(), dMin, dMax);
	ToSI(vRes);
	return vRes;
}

double CDimensionParameters::ToSI(double _dVal) const
{
	if (m_grid->DimensionType() == DISTR_SIZE && m_grid->GridType() == EGridEntry::GRID_NUMERIC)
		return SizeToSI(_dVal, static_cast<EGridUnit>(ui.comboBoxUnits->currentIndex()));
	return _dVal;
}

void CDimensionParameters::ToSI(std::vector<double>& _vVal) const
{
	if (m_grid->DimensionType() == DISTR_SIZE && m_grid->GridType() == EGridEntry::GRID_NUMERIC)
		for (double& val : _vVal)
			val = ToSI(val);
}

double CDimensionParameters::SizeToSI(double _dVal, EGridUnit _srcUnits) const
{
	switch (_srcUnits)
	{
	case EGridUnit::UNIT_M:		  return _dVal;
	case EGridUnit::UNIT_MM:	  return _dVal / 1000;
	case EGridUnit::UNIT_UM:	  return _dVal / 1000000;
	case EGridUnit::UNIT_M3:	  return std::pow(6 * _dVal / MATH_PI, 1. / 3.);
	case EGridUnit::UNIT_MM3:	  return std::pow(6 * _dVal / 1e+9 / MATH_PI, 1. / 3.);
	case EGridUnit::UNIT_UM3:	  return std::pow(6 * _dVal / 1e+18 / MATH_PI, 1. / 3.);
	case EGridUnit::UNIT_DEFAULT: return _dVal;
	}
	return _dVal;
}

double CDimensionParameters::FromSI(double _dVal) const
{
	if (m_grid->DimensionType() == DISTR_SIZE && m_grid->GridType() == EGridEntry::GRID_NUMERIC)
		return SizeFromSI(_dVal, static_cast<EGridUnit>(ui.comboBoxDispUnits->currentIndex()));
	return _dVal;
}

void CDimensionParameters::FromSI(std::vector<double>& _vVal) const
{
	if (m_grid->DimensionType() == DISTR_SIZE && m_grid->GridType() == EGridEntry::GRID_NUMERIC)
		for (double& val : _vVal)
			val = FromSI(val);
}

double CDimensionParameters::SizeFromSI(double _dVal, EGridUnit _dstUnits) const
{
	switch (_dstUnits)
	{
	case EGridUnit::UNIT_M:		  return _dVal;
	case EGridUnit::UNIT_MM:	  return _dVal * 1000;
	case EGridUnit::UNIT_UM:	  return _dVal * 1000000;
	case EGridUnit::UNIT_M3:	  return MATH_PI / 6 * std::pow(_dVal, 3.);
	case EGridUnit::UNIT_MM3:	  return MATH_PI / 6 * std::pow(_dVal, 3.) * 1e+9;
	case EGridUnit::UNIT_UM3:	  return MATH_PI / 6 * std::pow(_dVal, 3.) * 1e+18;
	case EGridUnit::UNIT_DEFAULT: return _dVal;
	}
	return _dVal;
}


void CDimensionParameters::GridEntryChanged()
{
	if (!m_grid) return;
	if (m_bAvoidSignals) return;
	if (m_grid->GridType() == GetSelectedGridEntry()) return;
	if (GetSelectedGridEntry() == EGridEntry::GRID_NUMERIC)
		m_grid.reset(new CGridDimensionNumeric(m_grid->DimensionType()));
	else
		m_grid.reset(new CGridDimensionSymbolic(m_grid->DimensionType()));
	UpdateUnitsVisibility();
	UpdateLimitsVisibility();
	UpdateGrid();
}

void CDimensionParameters::ClassesChanged()
{
	if (m_bAvoidSignals) return;
	UpdateGrid();
}

void CDimensionParameters::GridFunChanged()
{
	if (!m_grid) return;
	if (m_bAvoidSignals) return;
	m_bAvoidSignals = true;
	if (auto* numeric = dynamic_cast<CGridDimensionNumeric*>(m_grid.get()))
	{
		const EGridFunction gridFunOld = numeric->Function();
		numeric->SetFunction(GetSelectedGridFunction());
		if (gridFunOld == EGridFunction::GRID_FUN_MANUAL && numeric->Function() != EGridFunction::GRID_FUN_MANUAL && !numeric->Grid().empty())
		{
			const double dV1 = FromSI(numeric->Grid().front());
			const double dV2 = FromSI(numeric->Grid().back());
			ui.lineEditLimitMin->setText(QString::number(dV1));
			ui.lineEditLimitMax->setText(QString::number(dV2));
		}
	}
	m_bAvoidSignals = false;
	UpdateLimitsVisibility();
	UpdateUnitsVisibility();
	UpdateGrid();
}

void CDimensionParameters::LimitsChanged()
{
	if (m_bAvoidSignals) return;
	UpdateLimitsVisibility();
	UpdateGrid();
}

void CDimensionParameters::GridDataChanged()
{
	if (!m_grid) return;
	if (m_bAvoidSignals) return;

	switch (m_grid->GridType())
	{
	case EGridEntry::GRID_NUMERIC:
	{
		auto* numeric = dynamic_cast<CGridDimensionNumeric*>(m_grid.get());
		std::vector<double> data(ui.tableWidgetGrid->rowCount());
		for (int iRow = 0; iRow < ui.tableWidgetGrid->rowCount(); ++iRow)
		{
			const double dVal = ui.tableWidgetGrid->item(iRow, 0)->text().toDouble();
			data[iRow] = ToSI(dVal);
			const double dDispVal = FromSI(data[iRow]);
			ui.tableWidgetGrid->item(iRow, 0)->setText(QString::number(dDispVal));
		}
		numeric->SetGrid(data);
		break;
	}
	case EGridEntry::GRID_SYMBOLIC:
	{
		auto* symbolic = dynamic_cast<CGridDimensionSymbolic*>(m_grid.get());
		std::vector<std::string> data(ui.tableWidgetGrid->rowCount());
		for (int iRow = 0; iRow < ui.tableWidgetGrid->rowCount(); ++iRow)
		{
			data[iRow] = ui.tableWidgetGrid->item(iRow, 0)->text().toStdString();
			ui.tableWidgetGrid->item(iRow, 0)->setText(QString::fromStdString(data[iRow]));
		}
		symbolic->SetGrid(data);
		break;
	}
	}
}

void CDimensionParameters::UnitsChanged(int _index)
{
	if (!m_grid) return;
	if (m_bAvoidSignals) return;
	if (m_grid->DimensionType() == DISTR_SIZE && (m_grid->GridType() == EGridEntry::GRID_SYMBOLIC || dynamic_cast<CGridDimensionNumeric*>(m_grid.get())->Function() == EGridFunction::GRID_FUN_MANUAL))
	{
		m_bAvoidSignals = true;
		ui.comboBoxUnits->setCurrentIndex(_index);
		ui.comboBoxDispUnits->setCurrentIndex(_index);
		m_bAvoidSignals = false;
	}
	m_gridUnit = GetSelectedGridUnit();
	UpdateGrid();
}
