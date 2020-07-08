/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DustFormationTesterTab.h"
#include "MaterialStream.h"

CDustFormationTesterTab::CDustFormationTesterTab(const CFlowsheet* _pFlowsheet, QWidget *parent)
	: QDialog(parent),
	m_pFlowsheet(_pFlowsheet)
{
	ui.setupUi(this);
	ui.tableWidgetData->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);
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
	for (size_t i = 0; i < m_pFlowsheet->GetStreamsCount(); ++i)
	{
		const CMaterialStream* pStream = m_pFlowsheet->GetStream(i);
		AddItemToList(ui.listWidgetStreams, pStream->GetStreamName(), pStream->GetStreamKey());
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
	for (size_t i = 0; i < m_pFlowsheet->GetModelsCount(); ++i)
	{
		const CBaseModel* pModel = m_pFlowsheet->GetModel(i);
		AddItemToList(ui.listWidgetUnits, pModel->GetModelName(), pModel->GetModelKey());
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
	const CBaseModel* pModel = GetSelectedModel();
	if(!pModel)
	{
		ui.listWidgetHoldups->blockSignals(bOldBlock);
		return;
	}

	// update table
	for (size_t i = 0; i < pModel->GetHoldupsCount(); ++i)
	{
		const CHoldup* pHoldup = pModel->GetHoldup(i);
		AddItemToList(ui.listWidgetHoldups, pHoldup->GetStreamName(), pHoldup->GetStreamKey());
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
	for (size_t i = 0; i < m_pFlowsheet->GetCompoundsNumber(); ++i)
		ui.comboBoxCompound->addItem(QString::fromStdString(m_pFlowsheet->GetCompoundName(i)), QString::fromStdString(m_pFlowsheet->GetCompoundKey(i)));
}

void CDustFormationTesterTab::UpdateDataTable()
{
	QSignalBlocker blocker(ui.tableWidgetData);

	// remove old data
	ui.tableWidgetData->setRowCount(0);

	// prepare stuff
	const CStream* pStream = GetSelectedStream();
	if (!pStream)
		return;

	const std::vector<double> vTPs = pStream->GetAllTimePoints();
	const std::string sCompound = GetSelectedCompound();
	std::vector<std::string> vCompounds;
	if (!sCompound.empty())
		vCompounds.push_back(sCompound);
	m_tester.SetGrid(m_pFlowsheet->GetDistributionsGrid()->GetNumericGridByDistr(DISTR_SIZE));
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

const CStream* CDustFormationTesterTab::GetSelectedStream() const
{
	switch (m_focusType)
	{
	case EType::STREAMS:	return GetSelectedMaterialStream();
	case EType::UNITS:		return GetSelectedHoldup();
	default:				return nullptr;
	}
}

const CBaseModel* CDustFormationTesterTab::GetSelectedModel() const
{
	const QListWidgetItem* pItem = ui.listWidgetUnits->currentItem();
	if (!pItem) return nullptr;
	const std::string sKey = pItem->data(Qt::UserRole).toString().toStdString();
	return m_pFlowsheet->GetModel(sKey);
}

const CHoldup* CDustFormationTesterTab::GetSelectedHoldup() const
{
	const CBaseModel* pModel = GetSelectedModel();
	if (!pModel) return nullptr;
	const QListWidgetItem* pItem = ui.listWidgetHoldups->currentItem();
	if (!pItem) return nullptr;
	const std::string sKey = pItem->data(Qt::UserRole).toString().toStdString();
	return pModel->GetHoldup(sKey);
}

const CMaterialStream* CDustFormationTesterTab::GetSelectedMaterialStream() const
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
