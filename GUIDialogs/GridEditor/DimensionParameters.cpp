/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DimensionParameters.h"
#include "DyssolStringConstants.h"
#include "DyssolUtilities.h"

CDimensionParameters::CDimensionParameters(QWidget* parent) : QWidget(parent)
{
	ui.setupUi(this);

	m_bAvoidSignals = false;
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
	m_grid.distrType = _type;
	UpdateTableName();
	UpdateUnitsVisibility();
}

void CDimensionParameters::SetGrid(const SGridDimension& _grid)
{
	m_grid = _grid;

	UpdateTableName();
	UpdateLimitsVisibility();
	UpdateUnitsVisibility();
	UpdateControls();
	UpdateGridData(m_grid.numGrid, m_grid.strGrid);
	UpdateGrid();
}

SGridDimension CDimensionParameters::GetGrid() const
{
	return m_grid;
}

CDimensionParameters::EDPErrors CDimensionParameters::IsValid() const
{
	switch (m_grid.gridEntry)
	{
	case EGridEntry::GRID_NUMERIC:
		for (size_t i = 0; i < m_grid.numGrid.size(); ++i)
		{
			if (m_grid.numGrid[i] < 0)
				return EDPErrors::DP_ERROR_NEGATIVE;
			if (i > 0 && m_grid.numGrid[i] <= m_grid.numGrid[i - 1])
				return EDPErrors::DP_ERROR_SEQUENCE;
		}
		break;
	case EGridEntry::GRID_SYMBOLIC:
		for (const auto& str : m_grid.strGrid)
			if (str.empty())
				return EDPErrors::DP_ERROR_EMPTY;
		break;
	case EGridEntry::GRID_UNDEFINED:
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
	const unsigned iDistr = GetDistributionTypeIndex(m_grid.distrType);
	const QString sName = QStringList{ DISTR_NAMES }[iDistr];
	ui.tableWidgetGrid->setHorizontalHeaderItem(0, new QTableWidgetItem(sName));
}

void CDimensionParameters::UpdateControls()
{
	m_bAvoidSignals = true;

	SelectGridEntry(m_grid.gridEntry);
	SelectGridFunction(m_grid.gridFun);
	SelectGridUnit(m_grid.gridUnit);
	SelectDisplayGridUnit(m_grid.gridUnit);
	if (!m_grid.numGrid.empty())
	{
		ui.lineEditLimitMin->setText(QString::number(SizeFromSI(m_grid.numGrid.front(), static_cast<EGridUnit>(ui.comboBoxUnits->currentIndex()))));
		ui.lineEditLimitMax->setText(QString::number(SizeFromSI(m_grid.numGrid.back(),  static_cast<EGridUnit>(ui.comboBoxUnits->currentIndex()))));
	}
	switch (m_grid.gridEntry)
	{
	case EGridEntry::GRID_NUMERIC:
		ui.spinBoxClasses->setValue((int)m_grid.numGrid.size() - 1);
		break;
	case EGridEntry::GRID_SYMBOLIC:
		ui.spinBoxClasses->setValue((int)m_grid.strGrid.size());
		break;
	case EGridEntry::GRID_UNDEFINED:
		break;
	}

	m_bAvoidSignals = false;
}

void CDimensionParameters::UpdateUnitsVisibility() const
{
	const bool bVisible1 = m_grid.distrType == DISTR_SIZE && m_grid.gridEntry == EGridEntry::GRID_NUMERIC;
	ui.labelUnits->setVisible(bVisible1);
	ui.comboBoxUnits->setVisible(bVisible1);
	const bool bVisible2 = bVisible1 && (m_grid.gridFun != EGridFunction::GRID_FUN_MANUAL);
	ui.labelDispUnits->setVisible(bVisible2);
	ui.comboBoxDispUnits->setVisible(bVisible2);
}

void CDimensionParameters::UpdateLimitsVisibility() const
{
	const bool bVisible1 = m_grid.gridEntry == EGridEntry::GRID_NUMERIC;
	ui.labelDistrFunction->setVisible(bVisible1);
	ui.comboBoxGridFun->setVisible(bVisible1);
	const bool bVisible2 = m_grid.gridEntry == EGridEntry::GRID_NUMERIC && m_grid.gridFun != EGridFunction::GRID_FUN_MANUAL;
	ui.labelMin->setVisible(bVisible2);
	ui.lineEditLimitMin->setVisible(bVisible2);
	ui.labelMax->setVisible(bVisible2);
	ui.lineEditLimitMax->setVisible(bVisible2);
}

void CDimensionParameters::UpdateGrid()
{
	switch (m_grid.gridEntry)
	{
	case EGridEntry::GRID_NUMERIC:
		switch (m_grid.gridFun)
		{
		case EGridFunction::GRID_FUN_MANUAL:
			m_grid.numGrid.resize(m_grid.classes + 1);
			break;
		case EGridFunction::GRID_FUN_EQUIDISTANT:
		case EGridFunction::GRID_FUN_GEOMETRIC_S2L:
		case EGridFunction::GRID_FUN_LOGARITHMIC_S2L:
		case EGridFunction::GRID_FUN_GEOMETRIC_L2S:
		case EGridFunction::GRID_FUN_LOGARITHMIC_L2S:
			m_grid.numGrid = CalculateGrid();
			break;
		case EGridFunction::GRID_FUN_UNDEFINED:
			break;
		}
		break;
	case EGridEntry::GRID_SYMBOLIC:	// just use current values
		m_grid.strGrid.resize(m_grid.classes);
		break;
	case EGridEntry::GRID_UNDEFINED:
		break;
	}

	std::vector<double> vTempNumGrid = m_grid.numGrid;
	FromSI(vTempNumGrid);

	UpdateGridData(vTempNumGrid, m_grid.strGrid);
}

void CDimensionParameters::UpdateGridData(const std::vector<double>& _vNumGrid, const std::vector<std::string>& _vStrGrid) const
{
	QSignalBlocker blocker(ui.tableWidgetGrid);

	switch (m_grid.gridEntry)
	{
	case EGridEntry::GRID_NUMERIC:
		ui.tableWidgetGrid->setRowCount((int)_vNumGrid.size());
		ui.tableWidgetGrid->SetItemsColEditable(0, 0, _vNumGrid);
		break;
	case EGridEntry::GRID_SYMBOLIC:
		ui.tableWidgetGrid->setRowCount((int)_vStrGrid.size());
		ui.tableWidgetGrid->SetItemsColEditable(0, 0, _vStrGrid);
		break;
	case EGridEntry::GRID_UNDEFINED:
		break;
	}
}

std::vector<double> CDimensionParameters::CalculateGrid() const
{
	const double dMin = ui.lineEditLimitMin->text().toDouble();
	const double dMax = ui.lineEditLimitMax->text().toDouble();

	std::vector<double> vRes = CDistributionsGrid::CalculateGrid(m_grid.gridFun, m_grid.classes, dMin, dMax);
	ToSI(vRes);
	return vRes;
}

double CDimensionParameters::ToSI(double _dVal) const
{
	if (m_grid.distrType == DISTR_SIZE && m_grid.gridEntry == EGridEntry::GRID_NUMERIC)
		return SizeToSI(_dVal, static_cast<EGridUnit>(ui.comboBoxUnits->currentIndex()));
	return _dVal;
}

void CDimensionParameters::ToSI(std::vector<double>& _vVal) const
{
	if (m_grid.distrType == DISTR_SIZE && m_grid.gridEntry == EGridEntry::GRID_NUMERIC)
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
	if (m_grid.distrType == DISTR_SIZE && m_grid.gridEntry == EGridEntry::GRID_NUMERIC)
		return SizeFromSI(_dVal, static_cast<EGridUnit>(ui.comboBoxDispUnits->currentIndex()));
	return _dVal;
}

void CDimensionParameters::FromSI(std::vector<double>& _vVal) const
{
	if (m_grid.distrType == DISTR_SIZE && m_grid.gridEntry == EGridEntry::GRID_NUMERIC)
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
	if (m_bAvoidSignals) return;
	m_grid.gridEntry = GetSelectedGridEntry();
	UpdateUnitsVisibility();
	UpdateLimitsVisibility();
	UpdateGrid();
}

void CDimensionParameters::ClassesChanged()
{
	if (m_bAvoidSignals) return;
	m_grid.classes = ui.spinBoxClasses->value();
	UpdateGrid();
}

void CDimensionParameters::GridFunChanged()
{
	if (m_bAvoidSignals) return;
	m_bAvoidSignals = true;
	const EGridFunction gridFunOld = m_grid.gridFun;
	m_grid.gridFun = GetSelectedGridFunction();
	if (gridFunOld == EGridFunction::GRID_FUN_MANUAL && m_grid.gridFun != EGridFunction::GRID_FUN_MANUAL && !m_grid.numGrid.empty())
	{
		const double dV1 = FromSI(m_grid.numGrid.front());
		const double dV2 = FromSI(m_grid.numGrid.back());
		ui.lineEditLimitMin->setText(QString::number(dV1));
		ui.lineEditLimitMax->setText(QString::number(dV2));
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
	if (m_bAvoidSignals) return;

	switch (m_grid.gridEntry)
	{
	case EGridEntry::GRID_NUMERIC:
		for (int iRow = 0; iRow < ui.tableWidgetGrid->rowCount(); ++iRow)
		{
			const double dVal = ui.tableWidgetGrid->item(iRow, 0)->text().toDouble();
			m_grid.numGrid[iRow] = ToSI(dVal);
			const double dDispVal = FromSI(m_grid.numGrid[iRow]);
			ui.tableWidgetGrid->item(iRow, 0)->setText(QString::number(dDispVal));
		}
		break;
	case EGridEntry::GRID_SYMBOLIC:
		for (int iRow = 0; iRow < ui.tableWidgetGrid->rowCount(); ++iRow)
		{
			m_grid.strGrid[iRow] = ui.tableWidgetGrid->item(iRow, 0)->text().toStdString();
			ui.tableWidgetGrid->item(iRow, 0)->setText(QString::fromStdString(m_grid.strGrid[iRow]));
		}
		break;
	case EGridEntry::GRID_UNDEFINED:
		break;
	}
}

void CDimensionParameters::UnitsChanged(int _index)
{
	if (m_bAvoidSignals) return;
	if (m_grid.distrType == DISTR_SIZE && m_grid.gridFun == EGridFunction::GRID_FUN_MANUAL)
	{
		m_bAvoidSignals = true;
		ui.comboBoxUnits->setCurrentIndex(_index);
		ui.comboBoxDispUnits->setCurrentIndex(_index);
		m_bAvoidSignals = false;
	}
	m_grid.gridUnit = GetSelectedGridUnit();
	UpdateGrid();
}
