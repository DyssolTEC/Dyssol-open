/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "StreamsViewer.h"
#include "MaterialStream.h"

CStreamsViewer::CStreamsViewer(CFlowsheet* _pFlowsheet, QWidget* _parent /*= nullptr*/, Qt::WindowFlags flags /*= 0 */) :
	QWidget(_parent, flags),
	m_pFlowsheet{ _pFlowsheet },
	m_pViewer{ new CBasicStreamsViewer(_pFlowsheet, this) }
{
	ui.setupUi(this);
	ui.horizontalLayout->addWidget(m_pViewer);
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

	for (size_t i = 0; i < m_pFlowsheet->GetStreamsCount(); ++i)
	{
		QListWidgetItem *pItem = new QListWidgetItem(QString::fromStdString(m_pFlowsheet->GetStream(i)->GetStreamName()));
		pItem->setData(Qt::UserRole, QString::fromStdString(m_pFlowsheet->GetStream(i)->GetStreamKey()));
		ui.streamsList->insertItem(int(i), pItem);
	}

	// restore old selection
	ui.streamsList->setCurrentRow(oldCurrent < ui.streamsList->count() ? oldCurrent : -1);
	for (const auto& index : oldSelection)
		if (index.row() < ui.streamsList->count())
			ui.streamsList->setItemSelected(ui.streamsList->item(index.row()), true);
	StreamChanged();
}

void CStreamsViewer::StreamChanged() const
{
	std::vector<const CStream*> vStreams;
	for (auto& index : ui.streamsList->selectionModel()->selection().indexes())
		if (index.row() >= 0 && index.row() < ui.streamsList->count())
			vStreams.push_back(m_pFlowsheet->GetStream(ui.streamsList->item(index.row())->data(Qt::UserRole).toString().toStdString()));
	m_pViewer->SetStreams(vStreams);
}
