/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "HoldupsEditor.h"
#include "BaseStream.h"

CHoldupsEditor::CHoldupsEditor(CFlowsheet* _pFlowsheet, QWidget *parent) :
	QDialog(parent),
	m_pFlowsheet{ _pFlowsheet },
	m_pSelectedModel{ nullptr },
	m_nLastModel{ 0 },
	m_nLastHoldup{ 0 }

{
	ui.setupUi(this);
	ui.widgetHoldupsEditor->SetFlowsheet(_pFlowsheet);
	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);
}

void CHoldupsEditor::InitializeConnections() const
{
	connect(ui.modelsList,			&QTableWidget::itemSelectionChanged,	this, &CHoldupsEditor::UpdateHoldupsList);
	connect(ui.holdupsList,			&QTableWidget::itemSelectionChanged,	this, &CHoldupsEditor::NewHoldupSelected);
	connect(ui.widgetHoldupsEditor, &CBasicStreamEditor::DataChanged,		this, &CHoldupsEditor::ChangeData);
}

void CHoldupsEditor::UpdateWholeView()
{
	UpdateUnitsList();
	UpdateHoldupsList();
}

void CHoldupsEditor::UpdateHoldupsList()
{
	QSignalBlocker blocker(ui.holdupsList);
	const auto oldPos = ui.modelsList->CurrentCellPos();

	ui.holdupsList->setColumnCount(1);
	ui.holdupsList->setRowCount(0);

	m_pSelectedModel = ui.modelsList->currentRow() != -1 ? m_pFlowsheet->GetModel(ui.modelsList->GetItemUserData(ui.modelsList->currentRow(), 0).toStdString()) : nullptr;
	if (m_pSelectedModel)
	{
		for (size_t i = 0; i < m_pSelectedModel->GetHoldupsCount(); ++i)
		{
			ui.holdupsList->insertRow(int(i));
			const CBaseStream* pStream = m_pSelectedModel->GetHoldupInit(i);
			if (pStream)
				ui.holdupsList->SetItemNotEditable(int(i), 0, pStream->GetName());
		}
	}

	ui.holdupsList->RestoreSelectedCell(oldPos);
	NewHoldupSelected();
}

void CHoldupsEditor::UpdateUnitsList() const
{
	QSignalBlocker blocker(ui.modelsList);
	const auto oldPos = ui.modelsList->CurrentCellPos();

	ui.modelsList->setColumnCount(1);
	ui.modelsList->setRowCount(0);

	for (size_t i = 0; i < m_pFlowsheet->GetModelsCount(); ++i)
	{
		const CBaseModel* m_pModel = m_pFlowsheet->GetModel(i);
		if (m_pModel && m_pModel->GetHoldupsCount() != 0)
		{
			ui.modelsList->insertRow(ui.modelsList->rowCount());
			ui.modelsList->SetItemNotEditable(ui.modelsList->rowCount() - 1, 0, m_pModel->GetModelName(), QString::fromStdString(m_pModel->GetModelKey()));
		}
	}
	ui.modelsList->RestoreSelectedCell(oldPos);
}

void CHoldupsEditor::setVisible( bool _bVisible )
{
	QDialog::setVisible(_bVisible);
	if (_bVisible)
	{
		UpdateWholeView();
		LoadViewState();
	}
	else
		SaveViewState();
}

void CHoldupsEditor::SaveViewState()
{
	m_nLastModel = ui.modelsList->currentRow() == -1 ? 0 : ui.modelsList->currentRow();
	m_nLastHoldup = ui.holdupsList->currentRow() == -1 ? 0 : ui.holdupsList->currentRow();
}

void CHoldupsEditor::LoadViewState() const
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	if (m_nLastModel < ui.modelsList->rowCount())
		ui.modelsList->setCurrentCell(m_nLastModel, 0);
	if (m_nLastHoldup < ui.holdupsList->rowCount())
		ui.holdupsList->setCurrentCell(m_nLastHoldup, 0);

	QApplication::restoreOverrideCursor();
}

void CHoldupsEditor::NewHoldupSelected() const
{
	CBaseStream* pSelectedHoldup = nullptr;
	if (m_pSelectedModel != nullptr)
		if (ui.holdupsList->currentRow() >= 0 && int(m_pSelectedModel->GetHoldupsCount()) > ui.holdupsList->currentRow())
			pSelectedHoldup = m_pSelectedModel->GetHoldupInit(ui.holdupsList->currentRow());

	ui.widgetHoldupsEditor->SetStream(pSelectedHoldup);
}

void CHoldupsEditor::ChangeData()
{
	emit DataChanged();
}
