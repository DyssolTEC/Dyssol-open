/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "GridEditor.h"
#include "Flowsheet.h"
#include "BaseUnit.h"
#include "ContainerFunctions.h"
#include "DyssolUtilities.h"
#include "DyssolStringConstants.h"
#include "SignalBlocker.h"
#include <QMessageBox>
#include <QStandardItem>


CGridEditor::CGridEditor(CFlowsheet* _flowsheet, const CMaterialsDatabase& _materialsDB, QWidget* _parent, Qt::WindowFlags _flags)
	: QDialog{ _parent, _flags }
	, m_materialsDB{ _materialsDB }
	, m_flowsheet{ _flowsheet }
	, m_grid{ _flowsheet->GetGrid() }
{
	ui.setupUi(this);
	ui.splitter->setSizes(QList{ 140, 400 });
	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);
}

void CGridEditor::InitializeConnections() const
{
	connect(ui.treeGrids                                  , &CQtTree::currentItemChanged  , this, &CGridEditor::GridSelected);
	connect(ui.treeGrids                                  , &CQtTree::CheckBoxStateChanged, this, &CGridEditor::GridActivityChanged);
	connect(ui.buttonPlus                                 , &QPushButton::clicked         , this, &CGridEditor::AddDistributionClicked);
	connect(ui.buttonMinus                                , &QPushButton::clicked         , this, &CGridEditor::RemDistributionClicked);
	connect(ui.buttonBox->button(QDialogButtonBox::Apply) , &QPushButton::clicked         , this, &CGridEditor::ApplyChanges);
	connect(ui.buttonBox->button(QDialogButtonBox::Ok)    , &QPushButton::clicked         , this, &CGridEditor::accept);
	connect(ui.buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked         , this, &CGridEditor::reject);
}

void CGridEditor::setVisible(bool _visible)
{
	if (_visible && !isVisible())
		UpdateWholeView();
	QDialog::setVisible(_visible);
}

void CGridEditor::accept()
{
	if (ApplyChanges())
		QDialog::accept();
}

void CGridEditor::UpdateWholeView()
{
	UpdateGridsList();
	UpdateDimensionsList();
}

void CGridEditor::UpdateGridsList() const
{
	[[maybe_unused]] CSignalBlocker blocker{ ui.treeGrids };
	const auto oldData = ui.treeGrids->GetCurrentData(1); // key of currently selected entity
	ui.treeGrids->clear();
	ui.treeGrids->setColumnCount(2);
	const auto& mainGrid = m_flowsheet->GetGrid();
	// add global grid
	auto* global = ui.treeGrids->CreateItem();
	ui.treeGrids->SetCheckBox(global, 0, "", true)->setEnabled(false);
	ui.treeGrids->SetText(global, 1, "Global", "global");
	// add unit grids
	bool allSame = true;
	auto* units = ui.treeGrids->CreateItem(0, "Unit-specific", CQtTree::EFlags::NoEdit | CQtTree::EFlags::NoSelect, "unit-specific");
	for (const auto& u : m_flowsheet->GetAllUnits())
	{
		const bool exist = u->GetModel() != nullptr;
		const bool activ = exist && mainGrid != u->GetModel()->GetGrid();
		allSame &= !activ;
		auto* item = ui.treeGrids->CreateItem(units);
		ui.treeGrids->SetCheckBox(item, 0, "", activ)->setEnabled(exist);
		ui.treeGrids->SetText(item, 1, u->GetName(), QString::fromStdString(u->GetKey()));
		ui.treeGrids->SetItemFlags(item, exist ? CQtTree::EFlags::Enabled : CQtTree::EFlags::Diasabled);
	}
	// resize columns and expand/contract rows
	for (int i = 0; i < ui.treeGrids->columnCount(); ++i)
		ui.treeGrids->resizeColumnToContents(i);
	units->setExpanded(!allSame);
	// restore/set current item
	blocker.Unblock();
	ui.treeGrids->SetCurrentItem(oldData);
	if (!ui.treeGrids->currentItem())
		ui.treeGrids->SetCurrentItem("global");
}

void CGridEditor::UpdateDimensionsList()
{
	[[maybe_unused]] CSignalBlocker blocker{ ui.listDims };
	ui.listDims->clear();
	for (const auto& dim : m_grid.GetGridDimensions())
	{
		auto* widget = AddGridDimension(*dim);
		// disable the item with compounds
		if (dim->DimensionType() == DISTR_COMPOUNDS)
			widget->setEnabled(false);
	}
}

void CGridEditor::GridSelected()
{
	const auto key = ui.treeGrids->GetCurrentData(1).toStdString();
	const auto* unit = m_flowsheet->GetUnit(key);
	const auto* model = unit ? unit->GetModel() : nullptr;
	if (key != "global" && model)
		m_grid = model->GetGrid();
	else
		m_grid = m_flowsheet->GetGrid();
	UpdateDimensionsList();
}

void CGridEditor::GridActivityChanged(const QCheckBox* _checkbox, const QTreeWidgetItem* _item)
{
	if (_checkbox->isChecked()) return;
	const auto key = ui.treeGrids->GetData(_item, 1).toStdString();
	auto* model = m_flowsheet->GetUnit(key)->GetModel();
	if (!model) return UpdateGridsList();
	const auto name = QString::fromStdString(m_flowsheet->GetUnit(key)->GetName());
	const auto reply = QMessageBox::question(this, "Confirm removal", tr("Set global grid to unit '%1'?").arg(name), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	if (reply != QMessageBox::Yes) return UpdateGridsList();
	model->SetGrid(m_flowsheet->GetGrid());
	m_flowsheet->UpdateGrids(); // notify the flowsheet
	GridSelected();				// update selection and view
}

void CGridEditor::AddDistributionClicked()
{
	[[maybe_unused]] CSignalBlocker blocker{ ui.listDims };
	AddGridDimension(CGridDimensionNumeric{ DISTR_UNDEFINED });
}

void CGridEditor::RemDistributionClicked()
{
	const int row = ui.listDims->currentRow();
	if (row <= 0 || row >= ui.listDims->count()) return;
	const auto* widget = dynamic_cast<CDimensionParameters*>(ui.listDims->itemWidget(ui.listDims->currentItem()));
	const auto name = std::vector<QString>(DISTR_NAMES)[GetDistributionTypeIndex(widget->GetDistributionType())];
	const auto reply = QMessageBox::question(this, "Confirm removal", tr("Remove distribution '%1'?").arg(name), QMessageBox::Yes | QMessageBox::No);
	if (reply != QMessageBox::Yes) return;
	delete ui.listDims->takeItem(row);
}

CDimensionParameters* CGridEditor::AddGridDimension(const CGridDimension& _dim)
{
	[[maybe_unused]] CSignalBlocker blocker{ ui.listDims };
	const auto widget = new CDimensionParameters{ _dim, m_materialsDB, this };
	auto* item = new QListWidgetItem();
	item->setData(Qt::UserRole, E2I(_dim.DimensionType()));
	item->setSizeHint(widget->minimumSizeHint() * 1.1);
	ui.listDims->addItem(item);
	ui.listDims->setItemWidget(item, widget);
	return widget;
}

bool CGridEditor::ApplyChanges()
{
	if (!IsValid()) return false;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	// build the grid
	CMultidimensionalGrid newGrid;
	for (int i = 0; i < ui.listDims->count(); ++i)
		newGrid.AddDimension(dynamic_cast<CDimensionParameters*>(ui.listDims->itemWidget(ui.listDims->item(i)))->GetGrid());

	// set the grid
	const auto key = ui.treeGrids->GetCurrentData(1).toStdString();
	if (key.empty()) return false;
	if (key == "global")
		m_flowsheet->SetMainGrid(newGrid);
	else if (auto* model = m_flowsheet->GetUnit(key)->GetModel())
	{
		model->SetGrid(newGrid);
		m_flowsheet->UpdateGrids(); // notify the flowsheet
	}

	// update widgets
	UpdateGridsList();
	emit DataChanged();

	QApplication::restoreOverrideCursor();
	return true;
}

bool CGridEditor::IsValid()
{
	std::vector<EDistrTypes> allTypes(ui.listDims->count());
	for (int i = 0; i < ui.listDims->count(); ++i)
		allTypes[i] = dynamic_cast<CDimensionParameters*>(ui.listDims->itemWidget(ui.listDims->item(i)))->GetDistributionType();
	if (!VectorUnique(allTypes))
	{
		QMessageBox::warning(this, "Warning", StrConst::GE_ErrorDuplicates);
		return false;
	}

	for (int i = 0; i < ui.listDims->count(); ++i)
	{
		const auto widget = dynamic_cast<CDimensionParameters*>(ui.listDims->itemWidget(ui.listDims->item(i)));
		if (!widget->IsValid())
		{
			QMessageBox::warning(this, "Warning", widget->LastMessage());
			return false;
		}
	}
	return true;
}
