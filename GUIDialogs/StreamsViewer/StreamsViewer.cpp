/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "StreamsViewer.h"
#include "BasicStreamsViewer.h"
#include "Flowsheet.h"
#include "Stream.h"

CStreamsViewer::CStreamsViewer(CFlowsheet* _pFlowsheet, CMaterialsDatabase* _materialsDB, QWidget* _parent, Qt::WindowFlags _flags)
	: CQtDialog{ _parent, _flags }
	, m_pFlowsheet{ _pFlowsheet }
	, m_pViewer{ new CBasicStreamsViewer(_pFlowsheet, _materialsDB, this) }
{
	ui.setupUi(this);
	ui.horizontalLayout->addWidget(m_pViewer);

	SetHelpLink("001_ui/gui.html#sec-gui-tabs-streams");
}

void CStreamsViewer::SetPointers(CModelsManager* _modelsManager, QSettings* _settings)
{
	CQtDialog::SetPointers(_modelsManager, _settings);
	m_pViewer->SetPointers(_modelsManager, _settings);
}

void CStreamsViewer::InitializeConnections() const
{
	connect(ui.streamsList, &QListWidget::itemSelectionChanged, this, &CStreamsViewer::StreamChanged);
	m_pViewer->InitializeConnections();
}

void CStreamsViewer::UpdateWholeView() const
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	UpdateStreamsView();
	StreamChanged();
	m_pViewer->UpdateWholeView();

	QApplication::restoreOverrideCursor();
}

void CStreamsViewer::setVisible(bool _bVisible)
{
	QWidget::setVisible(_bVisible);
	if (_bVisible)
	{
		const bool bAvoidSignal = m_pViewer->blockSignals(true);
		UpdateWholeView();
		m_pViewer->blockSignals(bAvoidSignal);
		QWidget::setVisible(_bVisible);
	}
}

void CStreamsViewer::UpdateStreamsView() const
{
	QSignalBlocker blocker(ui.streamsList);

	const auto oldSelection = ui.streamsList->selectionModel()->selection().indexes();
	const int oldCurrent = ui.streamsList->currentRow();

	ui.streamsList->clear();

	for (const auto& stream : m_pFlowsheet->GetAllStreams())
	{
		QListWidgetItem *pItem = new QListWidgetItem(QString::fromStdString(stream->GetName()));
		pItem->setData(Qt::UserRole, QString::fromStdString(stream->GetKey()));
		ui.streamsList->insertItem(ui.streamsList->count(), pItem);
	}

	// restore old selection
	ui.streamsList->setCurrentRow(oldCurrent < ui.streamsList->count() ? oldCurrent : -1);
	for (const auto& index : oldSelection)
		if (index.row() < ui.streamsList->count())
			(*ui.streamsList->item(index.row())).setSelected(true);
	StreamChanged();
}

void CStreamsViewer::StreamChanged() const
{
	std::vector<const CBaseStream*> vStreams;
	for (auto& index : ui.streamsList->selectionModel()->selection().indexes())
		if (index.row() >= 0 && index.row() < ui.streamsList->count())
			vStreams.push_back(m_pFlowsheet->GetStream(ui.streamsList->item(index.row())->data(Qt::UserRole).toString().toStdString()));
	m_pViewer->SetStreams(vStreams);
}
