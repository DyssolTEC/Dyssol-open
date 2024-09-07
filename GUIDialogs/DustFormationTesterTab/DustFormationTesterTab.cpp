/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DustFormationTesterTab.h"
#include "Flowsheet.h"
#include "BaseUnit.h"
#include "Stream.h"
#include "MaterialsDatabase.h"

CDustFormationTesterTab::CDustFormationTesterTab(const CFlowsheet* _pFlowsheet, const CMaterialsDatabase* _matrialsDB, QWidget *_parent)
	: CQtDialog{ _parent }
	, m_pFlowsheet{ _pFlowsheet }
	, m_matrialsDB{ _matrialsDB }
{
	ui.setupUi(this);
	ui.tableWidgetData->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);

	SetHelpLink("");
}

void CDustFormationTesterTab::InitializeConnections()
{
	connect(ui.toolBox,								&QToolBox::currentChanged,			this, &CDustFormationTesterTab::StreamsUnitsFocusChanged);
	connect(ui.listWidgetStreams,					&QListWidget::currentRowChanged,	this, &CDustFormationTesterTab::SelectedStreamChanged);
	connect(ui.listWidgetUnits,						&QListWidget::currentRowChanged,	this, &CDustFormationTesterTab::SelectedUnitChanged);
	connect(ui.listWidgetHoldups,					&QListWidget::currentRowChanged,	this, &CDustFormationTesterTab::SelectedHoldupChanged);
	connect(ui.lineEditPorosity,					&QLineEdit::editingFinished,		this, &CDustFormationTesterTab::PorosityChanged);
	connect(ui.lineEditMoisture,					&QLineEdit::editingFinished,		this, &CDustFormationTesterTab::MoistureChanged);
	connect(ui.lineEditMoisture90,					&QLineEdit::editingFinished,		this, &CDustFormationTesterTab::Moisture90Changed);
	connect(ui.comboBoxCompound, QOverload<int>::of(&QComboBox::currentIndexChanged),	this, &CDustFormationTesterTab::CompoundChanged);
}

void CDustFormationTesterTab::NewFlowsheetDataSet()
{
	if (isVisible())
		UpdateWholeView();
}

void CDustFormationTesterTab::UpdateWholeView()
{
	if (!isVisible()) return;
	UpdateStreams();
	UpdateUnits();
	UpdateHoldups();
	UpdatePorosity();
	UpdateMoisture();
	UpdateMoisture90();
	UpdateCompounds();
	UpdateDataTable();
}

void CDustFormationTesterTab::setVisible(bool _bVisible)
{
	QDialog::setVisible(_bVisible);
	if (_bVisible)
		UpdateWholeView();
}

void CDustFormationTesterTab::UpdateStreams() const
{
	// block signals from the table
	const bool bOldBlock = ui.listWidgetStreams->blockSignals(true);

	// save selected row
	const int iOldRow = ui.listWidgetStreams->currentRow();

	// update table
	ui.listWidgetStreams->clear();
	for (const auto& stream : m_pFlowsheet->GetAllStreams())
	{
		AddItemToList(ui.listWidgetStreams, stream->GetName(), stream->GetKey());
	}

	// if streams block is selected, unblock the table here
	if (m_focusType == EType::STREAMS)
		ui.listWidgetStreams->blockSignals(bOldBlock);

	// restore selection
	RestoreSelectedRow(ui.listWidgetStreams, iOldRow);

	// if units block is selected, unblock the table here
	if (m_focusType == EType::UNITS)
		ui.listWidgetStreams->blockSignals(bOldBlock);
}

void CDustFormationTesterTab::UpdateUnits() const
{
	// block signals from the table
	const bool bOldBlock = ui.listWidgetUnits->blockSignals(true);

	// save selected row
	const int iOldRow = ui.listWidgetUnits->currentRow();

	// update table
	ui.listWidgetUnits->clear();
	for (const auto& unit : m_pFlowsheet->GetAllUnits())
	{
		AddItemToList(ui.listWidgetUnits, unit->GetName(), unit->GetKey());
	}

	// if units block is selected, unblock the table here
	if (m_focusType == EType::UNITS)
		ui.listWidgetUnits->blockSignals(bOldBlock);

	// restore selection
	RestoreSelectedRow(ui.listWidgetUnits, iOldRow);

	// if streams block is selected, unblock the table here
	if (m_focusType == EType::STREAMS)
		ui.listWidgetUnits->blockSignals(bOldBlock);
}

void CDustFormationTesterTab::UpdateHoldups() const
{
	// block signals from the table
	const bool bOldBlock = ui.listWidgetHoldups->blockSignals(true);

	// save selected row
	const int iOldRow = ui.listWidgetHoldups->currentRow();

	// clear old data
	ui.listWidgetHoldups->clear();

	// prepare stuff
	const CUnitContainer* pModel = GetSelectedModel();
	if(!pModel || !pModel->GetModel())
	{
		ui.listWidgetHoldups->blockSignals(bOldBlock);
		return;
	}

	// update table
	for (const auto& holdup : pModel->GetModel()->GetStreamsManager().GetHoldups())
	{
		AddItemToList(ui.listWidgetHoldups, holdup->GetName(), holdup->GetKey());
	}

	// if units block is selected, unblock the table here
	if (m_focusType == EType::UNITS)
		ui.listWidgetHoldups->blockSignals(bOldBlock);

	// restore selection
	RestoreSelectedRow(ui.listWidgetHoldups, iOldRow);

	// if streams block is selected, unblock the table here
	if (m_focusType == EType::STREAMS)
		ui.listWidgetHoldups->blockSignals(bOldBlock);
}

void CDustFormationTesterTab::UpdatePorosity() const
{
	QSignalBlocker blocker(ui.lineEditPorosity);
	ui.lineEditPorosity->setText(QString::number(m_tester.GetBulkPorosity()));
}

void CDustFormationTesterTab::UpdateMoisture() const
{
	QSignalBlocker blocker(ui.lineEditMoisture);
	ui.lineEditMoisture->setText(QString::number(m_tester.GetMoistureContent()));
}

void CDustFormationTesterTab::UpdateMoisture90() const
{
	QSignalBlocker blocker(ui.lineEditMoisture90);
	ui.lineEditMoisture90->setText(QString::number(m_tester.GetMoistureContent90()));
}

void CDustFormationTesterTab::UpdateCompounds() const
{
	QSignalBlocker blocker(ui.comboBoxCompound);

	// clear old
	ui.comboBoxCompound->clear();
	// create entry for mixture
	ui.comboBoxCompound->addItem("Total mixture", "");
	// add compounds
	for (const auto& key : m_pFlowsheet->GetCompounds())
		ui.comboBoxCompound->addItem(QString::fromStdString(m_matrialsDB->GetCompound(key) ? m_matrialsDB->GetCompound(key)->GetName() : ""), QString::fromStdString(key));
}

void CDustFormationTesterTab::UpdateDataTable()
{
	QSignalBlocker blocker(ui.tableWidgetData);

	// remove old data
	ui.tableWidgetData->setRowCount(0);

	// prepare stuff
	const CBaseStream* pStream = GetSelectedStream();
	if (!pStream)
		return;

	const std::vector<double> vTPs = pStream->GetAllTimePoints();
	const std::string sCompound = GetSelectedCompound();
	std::vector<std::string> vCompounds;
	if (!sCompound.empty())
		vCompounds.push_back(sCompound);
	m_tester.SetGrid(m_pFlowsheet->GetGrid().GetPSDGrid());
	ui.tableWidgetData->setRowCount(static_cast<int>(vTPs.size()));

	for (int i = 0; i < static_cast<int>(vTPs.size()); ++i)
	{
		// set PSD to tester
		m_tester.SetPSD(pStream->GetPSD(vTPs[i], PSD_MassFrac, vCompounds));
		// obtain new values
		std::vector<double> values = m_tester.GetAll();
		// set time point to table
		ui.tableWidgetData->SetItemNotEditable(i, 0, vTPs[i]);
		// set data to table
		for (int j = 0; j < static_cast<int>(values.size()); ++j)
			ui.tableWidgetData->SetItemNotEditable(i, j + 1, values[j] >= 0 ? QString::number(values[j]) : "N/A");
	}
}

void CDustFormationTesterTab::AddItemToList(QListWidget* _pList, const std::string& _sName, const std::string& _sKey)
{
	QListWidgetItem* pItem = new QListWidgetItem(QString::fromStdString(_sName));
	pItem->setData(Qt::UserRole, QString::fromStdString(_sKey));
	_pList->addItem(pItem);
}

void CDustFormationTesterTab::RestoreSelectedRow(QListWidget* _pList, int iRow)
{
	if (iRow == -1 && _pList->count() != 0)
		_pList->setCurrentRow(0, QItemSelectionModel::SelectCurrent);
	else if (iRow < _pList->count())
		_pList->setCurrentRow(iRow, QItemSelectionModel::SelectCurrent);
	else
		_pList->setCurrentRow(_pList->count() - 1, QItemSelectionModel::SelectCurrent);
}

const CBaseStream* CDustFormationTesterTab::GetSelectedStream() const
{
	switch (m_focusType)
	{
	case EType::STREAMS:	return GetSelectedMaterialStream();
	case EType::UNITS:		return GetSelectedHoldup();
	default:				return nullptr;
	}
}

const CUnitContainer* CDustFormationTesterTab::GetSelectedModel() const
{
	const QListWidgetItem* pItem = ui.listWidgetUnits->currentItem();
	if (!pItem) return nullptr;
	const std::string sKey = pItem->data(Qt::UserRole).toString().toStdString();
	return m_pFlowsheet->GetUnit(sKey);
}

const CHoldup* CDustFormationTesterTab::GetSelectedHoldup() const
{
	const CUnitContainer* pModel = GetSelectedModel();
	if (!pModel) return nullptr;
	const QListWidgetItem* pItem = ui.listWidgetHoldups->currentItem();
	if (!pItem) return nullptr;
	const std::string sKey = pItem->data(Qt::UserRole).toString().toStdString();
	const CBaseUnit* model = pModel->GetModel();
	if (!model) return nullptr;
	return model->GetStreamsManager().GetHoldup(sKey);
}

const CStream* CDustFormationTesterTab::GetSelectedMaterialStream() const
{
	const QListWidgetItem* pItem = ui.listWidgetStreams->currentItem();
	if (!pItem) return nullptr;
	const std::string sKey = pItem->data(Qt::UserRole).toString().toStdString();
	return m_pFlowsheet->GetStream(sKey);
}

std::string CDustFormationTesterTab::GetSelectedCompound() const
{
	return ui.comboBoxCompound->currentData().toString().toStdString();
}

void CDustFormationTesterTab::StreamsUnitsFocusChanged(int _index)
{
	m_focusType = static_cast<EType>(_index);
	UpdateDataTable();
}

void CDustFormationTesterTab::SelectedStreamChanged()
{
	UpdateDataTable();
}

void CDustFormationTesterTab::SelectedUnitChanged()
{
	UpdateHoldups();
	UpdateDataTable();
}

void CDustFormationTesterTab::SelectedHoldupChanged()
{
	UpdateDataTable();
}

void CDustFormationTesterTab::PorosityChanged()
{
	m_tester.SetBulkPorosity(ui.lineEditPorosity->text().toDouble());
	UpdatePorosity();
	UpdateDataTable();
}

void CDustFormationTesterTab::MoistureChanged()
{
	m_tester.SetMoistureContent(ui.lineEditMoisture->text().toDouble());
	UpdateMoisture();
	UpdateDataTable();
}

void CDustFormationTesterTab::Moisture90Changed()
{
	m_tester.SetMoistureContent90(ui.lineEditMoisture90->text().toDouble());
	UpdateMoisture90();
	UpdateDataTable();
}

void CDustFormationTesterTab::CompoundChanged()
{
	UpdateDataTable();
}
