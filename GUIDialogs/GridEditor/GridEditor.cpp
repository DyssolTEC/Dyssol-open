/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "GridEditor.h"
#include "Flowsheet.h"
#include <QMessageBox>
#include <QStandardItem>

CGridEditor::CGridEditor(CFlowsheet* _pFlowsheet, const CMaterialsDatabase& _materialsDB, QWidget* parent, Qt::WindowFlags flags)
	: QDialog(parent, flags)
	, m_materialsDB{ _materialsDB }
	, m_pFlowsheet{ _pFlowsheet }
	, m_pGrid{ _pFlowsheet->GetGrid() }
{
	ui.setupUi(this);
	this->setWindowFlags(this->windowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);

}

CGridEditor::~CGridEditor()
{

}

void CGridEditor::InitializeConnections()
{
	connect(ui.pushButtonApply,		&QPushButton::clicked,				this, &CGridEditor::ApplyChanges);
	connect(ui.pushButtonOk,		&QPushButton::clicked,				this, &CGridEditor::accept);
	connect(ui.pushButtonCancel,	&QPushButton::clicked,				this, &CGridEditor::reject);
	connect(ui.pushButtonPlus,		&QPushButton::clicked,				this, &CGridEditor::AddDimensionClicked);
	connect(ui.pushButtonMinus,		&QPushButton::clicked,				this, &CGridEditor::RemoveDimensionClicked);
	connect(ui.tableWidgetDims,		&CQtTable::ComboBoxIndexChanged,	this, &CGridEditor::DimensionTypeChanged);
}

QComboBox* CGridEditor::AddComboBoxDimensions(unsigned _nRow)
{
	QComboBox* pCombo = ui.tableWidgetDims->SetComboBox(_nRow, 0, std::vector<QString>(DISTR_NAMES), std::vector<QVariant>(DISTR_TYPES), DISTR_SIZE);
	pCombo->setMaxVisibleItems(DISTRIBUTIONS_NUMBER);
	// disable 'compounds'
	QStandardItem* item = qobject_cast<const QStandardItemModel*>(pCombo->model())->item(0);
	item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
	item->setData(pCombo->palette().color(QPalette::Disabled, QPalette::Text), Qt::ForegroundRole);
	return pCombo;
}

CDimensionParameters* CGridEditor::AddDimensionParam()
{
	CDimensionParameters* pDimPar = new CDimensionParameters(m_materialsDB, this);
	QListWidgetItem* pItem = new QListWidgetItem();
	pItem->setSizeHint(pDimPar->minimumSizeHint()*1.1);
	ui.listWidgetParams->addItem(pItem);
	ui.listWidgetParams->setItemWidget(pItem, pDimPar);
	return pDimPar;
}

void CGridEditor::UpdateWholeView()
{
	m_bAvoidSignal = true;
	ui.tableWidgetDims->setRowCount(0);
	ui.listWidgetParams->clear();
	if (!m_pFlowsheet)
	{
		m_bAvoidSignal = false;
		return;
	}

	ui.tableWidgetDims->setRowCount((int)m_pGrid.GetDimensionsNumber());
	int i = 0;
	for (const auto& gridDim : m_pGrid.GetGridDimensions())
	{
		QComboBox* pCombo = AddComboBoxDimensions(i);

		int nTypeIndex = GetDistributionTypeIndex(gridDim->DimensionType());
		pCombo->setCurrentIndex(nTypeIndex);

		CDimensionParameters* pDimPar = AddDimensionParam();
		pDimPar->SetGrid(*gridDim);

		// disable first combobox with compounds and connected widget
		if (gridDim->DimensionType() == DISTR_COMPOUNDS)
		{
			pCombo->setEnabled(false);
			pDimPar->setEnabled(false);
		}
		i++;
	}
	m_bAvoidSignal = false;
}

bool CGridEditor::Validate()
{
	for (int i = 0; i < ui.tableWidgetDims->rowCount() - 1; ++i)
		for (int j = 1; j < ui.tableWidgetDims->rowCount(); ++j)
		{
			if (i == j) continue;

			int nDim1 = ((QComboBox*)ui.tableWidgetDims->cellWidget(i, 0))->currentIndex();
			int nDim2 = ((QComboBox*)ui.tableWidgetDims->cellWidget(j, 0))->currentIndex();
			if (nDim1 == nDim2)
			{
				QMessageBox::warning(this, tr("Warning"), tr("Unable to apply. Dimension with numbers %1 and %2 have the same types").arg(i + 1).arg(j + 1));
				return false;
			}
		}

	for (int i = 0; i < ui.listWidgetParams->count(); ++i)
	{
		CDimensionParameters* item = static_cast<CDimensionParameters*>(ui.listWidgetParams->itemWidget(ui.listWidgetParams->item(i)));
		CDimensionParameters::EDPErrors nError = item->IsValid();
		if (nError != CDimensionParameters::EDPErrors::DP_ERROR_NO_ERRORS)
		{
			switch (nError)
			{
			case CDimensionParameters::EDPErrors::DP_ERROR_EMPTY:
				QMessageBox::warning(this, tr("Warning"), tr("Unable to apply. Dimension with number %1 has empty classes").arg(i + 1));
				break;
			case CDimensionParameters::EDPErrors::DP_ERROR_NEGATIVE:
				QMessageBox::warning(this, tr("Warning"), tr("Unable to apply. Dimension with number %1 has negative classes").arg(i + 1));
				break;
			case CDimensionParameters::EDPErrors::DP_ERROR_SEQUENCE:
				QMessageBox::warning(this, tr("Warning"), tr("Unable to apply. Classes in dimension with number %1 have wrong sequence").arg(i + 1));
				break;
			case CDimensionParameters::EDPErrors::DP_ERROR_NO_ERRORS:
				break;
			}
			return false;
		}
	}
	return true;
}

void CGridEditor::setVisible(bool _bVisible)
{
	if ((_bVisible) && (!this->isVisible()))
		UpdateWholeView();
	QDialog::setVisible(_bVisible);
}

void CGridEditor::accept()
{
	if (ApplyChanges())
		QDialog::accept();
}

void CGridEditor::AddDimensionClicked()
{
	if (m_bAvoidSignal) return;
	m_bAvoidSignal = true;
	ui.tableWidgetDims->setRowCount(ui.tableWidgetDims->rowCount() + 1);
	int nCurrRow = ui.tableWidgetDims->rowCount() - 1;
	QComboBox* combo = AddComboBoxDimensions(nCurrRow);
	CDimensionParameters* pDimPar = AddDimensionParam();
	pDimPar->SetDistributionType((EDistrTypes)combo->currentIndex());
	m_bAvoidSignal = false;
}

void CGridEditor::RemoveDimensionClicked()
{
	if (m_bAvoidSignal) return;
	int _nRow = ui.tableWidgetDims->currentRow();
	if (_nRow <= 0) return;
	int reply = QMessageBox::question(this, tr("Removal"), tr("Do you really want to remove dimension with number %1?").arg(_nRow + 1), QMessageBox::Yes | QMessageBox::No);
	if (reply != QMessageBox::Yes) return;

	m_bAvoidSignal = true;
	ui.tableWidgetDims->removeRow(_nRow);
	delete ui.listWidgetParams->takeItem(_nRow);
	m_bAvoidSignal = false;
}

void CGridEditor::DimensionTypeChanged(int _nRow, int _nCol, QComboBox* _pComboBox) const
{
	if (m_bAvoidSignal)	return;
	if (_nRow < 0 || _nRow >= ui.tableWidgetDims->rowCount()) return;
	const int nValue = _pComboBox->currentIndex();
	if (nValue == -1) return;
	auto* item = dynamic_cast<CDimensionParameters*>(ui.listWidgetParams->itemWidget(ui.listWidgetParams->item(_nRow)));
	item->SetDistributionType(static_cast<EDistrTypes>(std::vector<unsigned>(DISTR_TYPES)[nValue]));
}

bool CGridEditor::ApplyChanges()
{
	if (!Validate()) return false;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	CMultidimensionalGrid newGrid;
	for (int i = 0; i < ui.tableWidgetDims->rowCount(); ++i)
	{
		QComboBox* pCombo = ((QComboBox*)ui.tableWidgetDims->cellWidget(i, 0));
		if (pCombo->currentIndex() == -1) continue;
		CDimensionParameters* item = static_cast<CDimensionParameters*>(ui.listWidgetParams->itemWidget(ui.listWidgetParams->item(i)));
		newGrid.AddDimension(*item->GetGrid());
	}
	m_pFlowsheet->SetMainGrid(newGrid);

	emit DataChanged();

	QApplication::restoreOverrideCursor();

	return true;
}
