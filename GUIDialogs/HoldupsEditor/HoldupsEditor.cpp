/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "HoldupsEditor.h"
#include "Flowsheet.h"
#include "BaseUnit.h"
#include "BaseStream.h"

CHoldupsEditor::CHoldupsEditor(CFlowsheet* _pFlowsheet, CMaterialsDatabase* _materialsDB, QWidget* _parent)
	: CQtDialog{ _parent }
	, m_pFlowsheet{ _pFlowsheet }
	, m_pSelectedModel{ nullptr }
	, m_nLastModel{ 0 }
	, m_nLastHoldup{ 0 }
{
	ui.setupUi(this);
	ui.widgetHoldupsEditor->SetFlowsheet(_pFlowsheet, _materialsDB);
	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);

	SetHelpLink("001_ui/gui.html#sec-gui-menu-setup-holdups");
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
	const auto oldPos = ui.modelsList->GetCurrentCellPos();

	ui.holdupsList->setColumnCount(1);
	ui.holdupsList->setRowCount(0);

	m_pSelectedModel = ui.modelsList->currentRow() != -1 ? m_pFlowsheet->GetUnit(ui.modelsList->GetItemUserData(ui.modelsList->currentRow(), 0).toStdString()) : nullptr;
	if (m_pSelectedModel && m_pSelectedModel->GetModel())
	{
		for (const auto& stream : m_pSelectedModel->GetModel()->GetStreamsManager().GetAllInit())
		{
			ui.holdupsList->insertRow(ui.holdupsList->rowCount());
			ui.holdupsList->SetItemNotEditable(ui.holdupsList->rowCount() - 1, 0, stream->GetName());
		}
	}

	ui.holdupsList->SetCurrentCellPos(oldPos);
	NewHoldupSelected();
}

void CHoldupsEditor::UpdateUnitsList() const
{
	QSignalBlocker blocker(ui.modelsList);
	const auto oldPos = ui.modelsList->GetCurrentCellPos();

	ui.modelsList->setColumnCount(1);
	ui.modelsList->setRowCount(0);

	for (const auto& unit : m_pFlowsheet->GetAllUnits())
	{
		if (unit && unit->GetModel() && !unit->GetModel()->GetStreamsManager().GetAllInit().empty())
		{
			ui.modelsList->insertRow(ui.modelsList->rowCount());
			ui.modelsList->SetItemNotEditable(ui.modelsList->rowCount() - 1, 0, unit->GetName(), QString::fromStdString(unit->GetKey()));
		}
	}
	ui.modelsList->SetCurrentCellPos(oldPos);
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

void CHoldupsEditor::UpdateFromFlowsheet()
{
	if (isVisible())
		UpdateWholeView();
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
		if (ui.holdupsList->currentRow() >= 0 && m_pSelectedModel->GetModel() && int(m_pSelectedModel->GetModel()->GetStreamsManager().GetAllInit().size()) > ui.holdupsList->currentRow())
			pSelectedHoldup = m_pSelectedModel->GetModel()->GetStreamsManager().GetAllInit()[ui.holdupsList->currentRow()];

	ui.widgetHoldupsEditor->SetStream(pSelectedHoldup);
}

void CHoldupsEditor::ChangeData()
{
	emit DataChanged();
}
