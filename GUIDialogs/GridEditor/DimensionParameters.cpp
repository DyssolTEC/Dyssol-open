/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DimensionParameters.h"
#include "MultidimensionalGrid.h"
#include "DyssolStringConstants.h"
#include "ContainerFunctions.h"
#include "DyssolUtilities.h"
#include "MaterialsDatabase.h"
#include "SignalBlocker.h"
#include <QStandardItem>

CDimensionParameters::CDimensionParameters(const CGridDimension& _grid, const CMaterialsDatabase& _materialsDB, QWidget* _parent)
	: QWidget{ _parent }
	, m_materialsDB{ _materialsDB }
	, m_grid{ _grid.Clone() }
{
	ui.setupUi(this);
	SetupComboBoxDistribution();
	SetupComboBoxEntry();
	SetupComboBoxFunction();
	SetupComboBoxUnits();

	connect(ui.comboDistribution, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CDimensionParameters::DistributionChanged);
	connect(ui.comboEntry       , QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CDimensionParameters::EntryChanged);
	connect(ui.comboFun         , QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CDimensionParameters::FunctionChanged);
	connect(ui.comboUnits       , QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CDimensionParameters::UnitsChanged);
	connect(ui.spinClasses      , QOverload<int>::of(&QSpinBox::valueChanged)        , this, &CDimensionParameters::ClassesChanged);
	connect(ui.lineMin          , &QLineEdit::textEdited                             , this, &CDimensionParameters::LimitsChanged);
	connect(ui.lineMax          , &QLineEdit::textEdited                             , this, &CDimensionParameters::LimitsChanged);
	connect(ui.tableGrid        , &CQtTable::cellChanged                             , this, &CDimensionParameters::GridChanged);

	UpdateWholeView();
}

EDistrTypes CDimensionParameters::GetDistributionType() const
{
	return m_grid->DimensionType();
}

void CDimensionParameters::SetupComboBoxDistribution() const
{
	[[maybe_unused]] CSignalBlocker blocker{ ui.comboDistribution };
	const auto& types = E2I(std::vector<EDistrTypes>{ std::begin(DISTR_TYPES), std::end(DISTR_TYPES) });
	std::vector<std::string> names{ std::begin(DISTR_NAMES), std::end(DISTR_NAMES) };;
	ui.comboDistribution->addItem("", DISTR_UNDEFINED);
	for (size_t i = 0; i < names.size(); ++i)
	{
		ui.comboDistribution->addItem(QString::fromStdString(names[i]), types[i]);
		// disable compounds
		if (types[i] == DISTR_COMPOUNDS)
		{
			QStandardItem* item = qobject_cast<const QStandardItemModel*>(ui.comboDistribution->model())->item(ui.comboDistribution->count() - 1);
			item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
		}
	}
	// always show all entries
	ui.comboDistribution->setMaxVisibleItems(std::size(DISTR_TYPES) + 1);
}

void CDimensionParameters::SetupComboBoxEntry() const
{
	[[maybe_unused]] CSignalBlocker blocker{ ui.comboEntry };
	ui.comboEntry->addItem(StrConst::GE_GridEntryNumeric , E2I(EGridEntry::GRID_NUMERIC));
	ui.comboEntry->addItem(StrConst::GE_GridEntrySymbolic, E2I(EGridEntry::GRID_SYMBOLIC));
}

void CDimensionParameters::SetupComboBoxFunction() const
{
	[[maybe_unused]] CSignalBlocker blocker{ ui.comboFun };
	ui.comboFun->addItem(StrConst::GE_GridTypeManual        , E2I(EGridFunction::GRID_FUN_MANUAL));
	ui.comboFun->addItem(StrConst::GE_GridTypeEquidistant   , E2I(EGridFunction::GRID_FUN_EQUIDISTANT));
	ui.comboFun->addItem(StrConst::GE_GridTypeGeometricS2L  , E2I(EGridFunction::GRID_FUN_GEOMETRIC_S2L));
	ui.comboFun->addItem(StrConst::GE_GridTypeLogarithmicS2L, E2I(EGridFunction::GRID_FUN_LOGARITHMIC_S2L));
	ui.comboFun->addItem(StrConst::GE_GridTypeGeometricL2S  , E2I(EGridFunction::GRID_FUN_GEOMETRIC_L2S));
	ui.comboFun->addItem(StrConst::GE_GridTypeLogarithmicL2S, E2I(EGridFunction::GRID_FUN_LOGARITHMIC_L2S));
}

void CDimensionParameters::SetupComboBoxUnits() const
{
	[[maybe_unused]] CSignalBlocker blocker{ ui.comboUnits };
	ui.comboUnits->addItem("m"  , E2I(EGridUnit::UNIT_M));
	ui.comboUnits->addItem("mm" , E2I(EGridUnit::UNIT_MM));
	ui.comboUnits->addItem("um" , E2I(EGridUnit::UNIT_UM));
	ui.comboUnits->addItem("m3" , E2I(EGridUnit::UNIT_M3));
	ui.comboUnits->addItem("mm3", E2I(EGridUnit::UNIT_MM3));
	ui.comboUnits->addItem("um3", E2I(EGridUnit::UNIT_UM3));
}

void CDimensionParameters::UpdateWholeView() const
{
	// order is important: function -> units -> limits
	UpdateDistribution();
	UpdateClasses();
	UpdateEntry();
	UpdateFunction();
	UpdateUnits();
	UpdateLimits();
	UpdateGrid();
	UpdateWidgetsVisibility();
}

void CDimensionParameters::UpdateDistribution() const
{
	SelectComboboxValue(ui.comboDistribution, E2I(m_grid->DimensionType()));
}

void CDimensionParameters::UpdateClasses() const
{
	[[maybe_unused]] CSignalBlocker blocker{ ui.spinClasses };
	ui.spinClasses->setValue(static_cast<int>(m_grid->ClassesNumber()));
}

void CDimensionParameters::UpdateEntry() const
{
	SelectComboboxValue(ui.comboEntry, E2I(m_grid->GridType()));
}

void CDimensionParameters::UpdateFunction() const
{
	SelectComboboxValue(ui.comboFun, E2I(DetermineGridFunction()));
}

void CDimensionParameters::UpdateLimits() const
{
	[[maybe_unused]] CSignalBlocker blocker{ ui.lineMin, ui.lineMax };
	if (m_grid->GridType() != EGridEntry::GRID_NUMERIC) return;
	const auto& vals = dynamic_cast<CGridDimensionNumeric*>(m_grid.get())->Grid();
	ui.lineMin->setText(QString::number(FromM(vals.front())));
	ui.lineMax->setText(QString::number(FromM(vals.back ())));
}

void CDimensionParameters::UpdateUnits() const
{
	SelectComboboxValue(ui.comboUnits, E2I(DetermineUnits()));
}

void CDimensionParameters::UpdateGrid() const
{
	[[maybe_unused]] CSignalBlocker blocker{ ui.tableGrid };
	if (m_grid->GridType() == EGridEntry::GRID_NUMERIC)
	{
		const auto vals = dynamic_cast<CGridDimensionNumeric*>(m_grid.get())->Grid();
		ui.tableGrid->setRowCount(static_cast<int>(vals.size()));
		ui.tableGrid->SetItemsColEditable(0, 0, FromM(vals));
	}
	else if (m_grid->GridType() == EGridEntry::GRID_SYMBOLIC)
	{
		auto vals = dynamic_cast<CGridDimensionSymbolic*>(m_grid.get())->Grid();
		ui.tableGrid->setRowCount(static_cast<int>(vals.size()));
		if (m_grid->DimensionType() == DISTR_COMPOUNDS)
			for (auto& v : vals)
				if (const auto* compound = m_materialsDB.GetCompound(v))
					v = compound->GetName();
		ui.tableGrid->SetItemsColEditable(0, 0, vals);
	}
}

void CDimensionParameters::UpdateWidgetsVisibility() const
{
	const bool visibleFun = m_grid->GridType() == EGridEntry::GRID_NUMERIC;
	ui.labelFun->setVisible(visibleFun);
	ui.comboFun->setVisible(visibleFun);
	const bool visibleUnits = m_grid->DimensionType() == DISTR_SIZE && m_grid->GridType() == EGridEntry::GRID_NUMERIC;
	ui.labelUnits->setVisible(visibleUnits);
	ui.comboUnits->setVisible(visibleUnits);
	const bool visibleLimits = m_grid->GridType() == EGridEntry::GRID_NUMERIC && static_cast<EGridFunction>(ui.comboFun->currentData().toUInt()) != EGridFunction::GRID_FUN_MANUAL;
	ui.labelMin->setVisible(visibleLimits);
	ui.lineMin->setVisible(visibleLimits);
	ui.labelMax->setVisible(visibleLimits);
	ui.lineMax->setVisible(visibleLimits);
}

void CDimensionParameters::DistributionChanged() const
{
	m_grid->SetType(static_cast<EDistrTypes>(ui.comboDistribution->currentData().toUInt()));
	UpdateWidgetsVisibility();
}

void CDimensionParameters::ClassesChanged() const
{
	SetGrid();
	UpdateGrid();
}

void CDimensionParameters::EntryChanged()
{
	const auto entry = static_cast<EGridEntry>(ui.comboEntry->currentData().toUInt());
	if (entry == EGridEntry::GRID_NUMERIC)
		m_grid.reset(new CGridDimensionNumeric(m_grid->DimensionType(), CalculateGridNumeric()));
	else if (entry == EGridEntry::GRID_SYMBOLIC)
		m_grid.reset(new CGridDimensionSymbolic(m_grid->DimensionType(), CalculateGridSymbolic()));
	UpdateWidgetsVisibility();
	UpdateGrid();
}

void CDimensionParameters::FunctionChanged() const
{
	SetGrid();
	UpdateWidgetsVisibility();
	UpdateGrid();
}

void CDimensionParameters::LimitsChanged() const
{
	SetGrid();
	UpdateWidgetsVisibility();
	UpdateGrid();
}

void CDimensionParameters::UnitsChanged() const
{
	if (m_grid->DimensionType() != DISTR_SIZE || m_grid->GridType() != EGridEntry::GRID_NUMERIC) return;
	UpdateLimits();
	UpdateGrid();
}

void CDimensionParameters::GridChanged() const
{
	SetGrid();
	UpdateFunction();
	UpdateLimits();
	UpdateGrid();
}

void CDimensionParameters::SetGrid() const
{
	if (m_grid->GridType() == EGridEntry::GRID_NUMERIC)
		dynamic_cast<CGridDimensionNumeric*>(m_grid.get())->SetGrid(CalculateGridNumeric());
	else if (m_grid->GridType() == EGridEntry::GRID_SYMBOLIC)
		dynamic_cast<CGridDimensionSymbolic*>(m_grid.get())->SetGrid(CalculateGridSymbolic());
}

std::vector<double> CDimensionParameters::CalculateGridNumeric() const
{
	const auto fun = static_cast<EGridFunction>(ui.comboFun->currentData().toUInt());
	const size_t number = ui.spinClasses->value() + 1;
	std::vector<double> res(number);
	if (fun != EGridFunction::GRID_FUN_MANUAL)
	{
		const double val1 = ui.lineMin->text().toDouble();
		const double val2 = ui.lineMax->text().toDouble();
		res = CreateGrid(fun, ui.spinClasses->value(), std::min(val1, val2), std::max(val1, val2));
	}
	else
	{
		const auto values = ui.tableGrid->GetItemsTextCol(0, 0);
		for (size_t i = 0; i < res.size() && i < values.size(); ++i)
			res[i] = values[i].toDouble();
	}
	res.resize(number); // ensure it has proper size
	return ToM(res);
}

std::vector<std::string> CDimensionParameters::CalculateGridSymbolic() const
{
	const auto values = ui.tableGrid->GetItemsTextCol(0, 0);
	std::vector<std::string> res(ui.spinClasses->value());
	for (size_t i = 0; i < res.size() && i < values.size(); ++i)
		res[i] = values[i].toStdString();
	return res;
}

EGridFunction CDimensionParameters::DetermineGridFunction() const
{
	if (m_grid->GridType() != EGridEntry::GRID_NUMERIC) return EGridFunction::GRID_FUN_MANUAL;
	const auto& vals = dynamic_cast<CGridDimensionNumeric*>(m_grid.get())->Grid();
	if (vals.size() < 3 || vals.front() >= vals.back()) return EGridFunction::GRID_FUN_MANUAL;
	const auto allfuns = { EGridFunction::GRID_FUN_EQUIDISTANT, EGridFunction::GRID_FUN_GEOMETRIC_S2L, EGridFunction::GRID_FUN_GEOMETRIC_L2S,
		EGridFunction::GRID_FUN_LOGARITHMIC_S2L, EGridFunction::GRID_FUN_LOGARITHMIC_L2S };
	for (const auto& fun : allfuns)
	{
		if (IsOfFunction(vals, fun))
			return fun;
		// additionally check for volumes
		if (m_grid->DimensionType() == DISTR_SIZE && IsOfFunction(vals, fun, EGridUnit::UNIT_M3))
			return fun;
	}
	return EGridFunction::GRID_FUN_MANUAL;
}

CDimensionParameters::EGridUnit CDimensionParameters::DetermineUnits() const
{
	if (m_grid->GridType() != EGridEntry::GRID_NUMERIC) return EGridUnit::UNIT_DEFAULT;
	if (m_grid->DimensionType() != DISTR_SIZE) return EGridUnit::UNIT_DEFAULT;
	const auto& vals = dynamic_cast<CGridDimensionNumeric*>(m_grid.get())->Grid();
	if (vals.empty()) return EGridUnit::UNIT_DEFAULT;
	const auto fun = static_cast<EGridFunction>(ui.comboFun->currentData().toUInt());
	const bool diams = fun == EGridFunction::GRID_FUN_MANUAL || !IsOfFunction(vals, fun, EGridUnit::UNIT_M3); // diameters or volumes
	const double ref = diams ? vals.back() : DiameterToVolume(vals.back()); // value to be analyzed
	if (diams)
	{
		if (ref < 1000e-6) return EGridUnit::UNIT_UM;
		if (ref < 1000e-4) return EGridUnit::UNIT_MM;
		return EGridUnit::UNIT_M;
	}
	else
	{
		if (ref < 1000e-18) return EGridUnit::UNIT_UM3;
		if (ref < 1000e-9)  return EGridUnit::UNIT_MM3;
		return EGridUnit::UNIT_M3;
	}
}

bool CDimensionParameters::IsOfFunction(const std::vector<double>& _v, EGridFunction _fun, EGridUnit _units) const
{
	const double val1 = FromM(_v.front(), _units);
	const double val2 = FromM(_v.back() , _units);
	const auto ref = ToM(CreateGrid(_fun, _v.size() - 1, std::min(val1, val2), std::max(val1, val2)), _units);
	if (ref.empty()) return false;
	const size_t iSmallestNonZero = VectorFind(_v, [](double v) { return v > 0; });
	if (iSmallestNonZero == static_cast<size_t>(-1)) return false;
	const double eps = _v[iSmallestNonZero] * 1e-5;
	const auto Cmp = [&](double d1, double d2) { return std::fabs(d2 - d1) <= eps; };
	return std::equal(_v.begin(), _v.end(), ref.begin(), Cmp);
}

double CDimensionParameters::ToM(double _val, EGridUnit _units) const
{
	return ToM(std::vector(1, _val), _units).front();
}

double CDimensionParameters::FromM(double _val, EGridUnit _units) const
{
	return FromM(std::vector(1, _val), _units).front();
}

std::vector<double> CDimensionParameters::ToM(const std::vector<double>& _vals, EGridUnit _units) const
{
	auto res = _vals;
	if (m_grid->DimensionType() == DISTR_SIZE && m_grid->GridType() == EGridEntry::GRID_NUMERIC)
	{
		const EGridUnit units = _units == EGridUnit::UNIT_DEFAULT ? static_cast<EGridUnit>(ui.comboUnits->currentData().toUInt()) : _units;
		for (double& val : res)
			val = SizeToM(val, units);
	}
	return res;
}

std::vector<double> CDimensionParameters::FromM(const std::vector<double>& _vals, EGridUnit _units) const
{
	auto res = _vals;
	if (m_grid->DimensionType() == DISTR_SIZE && m_grid->GridType() == EGridEntry::GRID_NUMERIC)
	{
		const EGridUnit units = _units == EGridUnit::UNIT_DEFAULT ? static_cast<EGridUnit>(ui.comboUnits->currentData().toUInt()) : _units;
		for (double& val : res)
			val = SizeFromM(val, units);
	}
	return res;
}

double CDimensionParameters::SizeToM(double _val, EGridUnit _units) const
{
	switch (_units)
	{
	case EGridUnit::UNIT_M:		  return _val;
	case EGridUnit::UNIT_MM:	  return _val / 1e+3;
	case EGridUnit::UNIT_UM:	  return _val / 1e+6;
	case EGridUnit::UNIT_M3:	  return VolumeToDiameter(_val);
	case EGridUnit::UNIT_MM3:	  return VolumeToDiameter(_val) / 1e+3;
	case EGridUnit::UNIT_UM3:	  return VolumeToDiameter(_val) / 1e+6;
	case EGridUnit::UNIT_DEFAULT: return _val;
	}
	return _val;
}

double CDimensionParameters::SizeFromM(double _val, EGridUnit _units) const
{
	switch (_units)
	{
	case EGridUnit::UNIT_M:		  return _val;
	case EGridUnit::UNIT_MM:	  return _val * 1e+3;
	case EGridUnit::UNIT_UM:	  return _val * 1e+6;
	case EGridUnit::UNIT_M3:	  return DiameterToVolume(_val);
	case EGridUnit::UNIT_MM3:	  return DiameterToVolume(_val) * 1e+9;
	case EGridUnit::UNIT_UM3:	  return DiameterToVolume(_val) * 1e+18;
	case EGridUnit::UNIT_DEFAULT: return _val;
	}
	return _val;
}

void CDimensionParameters::SelectComboboxValue(QComboBox* _combo, uint32_t _data)
{
	[[maybe_unused]] CSignalBlocker blocker{ _combo };
	for (int i = 0; i < _combo->count(); ++i)
		if (_combo->itemData(i).toUInt() == _data)
			_combo->setCurrentIndex(i);
}

bool CDimensionParameters::SetMessageAndReturn(const std::string& _message) const
{
	m_message = _message;
	return false;
}

bool CDimensionParameters::IsValid() const
{
	m_message.clear();
	if (m_grid->DimensionType() == DISTR_UNDEFINED)
		return SetMessageAndReturn(StrConst::GE_ErrorUndefined);
	const auto iName = GetDistributionTypeIndex(static_cast<EDistrTypes>(ui.comboDistribution->currentData().toUInt()));
	const auto distrName = iName != -1 ? DISTR_NAMES[iName] : "";
	if (m_grid->GridType() == EGridEntry::GRID_NUMERIC)
	{
		const auto& grid = dynamic_cast<CGridDimensionNumeric*>(m_grid.get())->Grid();
		if (grid.empty())
			return SetMessageAndReturn(StrConst::GE_ErrorEmpty(distrName));
		if (std::any_of(grid.begin(), grid.end(), [](auto v) { return v < 0; }))
			return SetMessageAndReturn(StrConst::GE_ErrorNegative(distrName));
		if (!std::is_sorted(grid.begin(), grid.end()))
			return SetMessageAndReturn(StrConst::GE_ErrorSequence(distrName));
	}
	else if (m_grid->GridType() == EGridEntry::GRID_SYMBOLIC)
	{
		const auto grid = dynamic_cast<CGridDimensionSymbolic*>(m_grid.get())->Grid();
		if (std::any_of(grid.begin(), grid.end(), [](auto v) { return v.empty(); }))
			return SetMessageAndReturn(StrConst::GE_ErrorGaps(distrName));
	}
	return true;
}

QString CDimensionParameters::LastMessage() const
{
	return QString::fromStdString(m_message);
}

CGridDimension& CDimensionParameters::GetGrid() const
{
	return *m_grid;
}