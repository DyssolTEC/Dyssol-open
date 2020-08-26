/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "TearStreamsEditor.h"
#include "Stream.h"
#include "FlowsheetParameters.h"
#include "DyssolStringConstants.h"
#include <QMessageBox>

CTearStreamsEditor::CTearStreamsEditor(CFlowsheet* _pFlowsheet, QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
	ui.widgetStreamsEditor->SetFlowsheet(_pFlowsheet);
	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);

	m_pFlowsheet = _pFlowsheet;
	m_pSequence = _pFlowsheet->GetCalculationSequence();
}

void CTearStreamsEditor::InitializeConnections()
{
	connect(ui.radioButtonAuto,		&QRadioButton::toggled,					this, &CTearStreamsEditor::SetEditable);
	connect(ui.radioButtonUser,		&QRadioButton::toggled,					this, &CTearStreamsEditor::SetEditable);
	connect(ui.pushButtonClearAll,	&QRadioButton::clicked,					this, &CTearStreamsEditor::ClearAllStreams);
	connect(ui.tablePartitions,		&QTableWidget::itemSelectionChanged,	this, &CTearStreamsEditor::UpdateStreamsList);
	connect(ui.tableStreams,		&QTableWidget::itemSelectionChanged,	this, &CTearStreamsEditor::NewStreamSelected);
	connect(ui.widgetStreamsEditor, &CBasicStreamEditor::DataChanged,		this, &CTearStreamsEditor::DataChanged);
}

void CTearStreamsEditor::setVisible(bool _bVisible)
{
	if (_bVisible && !this->isVisible())
		UpdateWholeView();
	QWidget::setVisible(_bVisible);
}

void CTearStreamsEditor::UpdateWholeView()
{
	UpdatePartitionsList();
	UpdateStreamsList();
	UpdateMode();
}

void CTearStreamsEditor::UpdatePartitionsList()
{
	const bool bBlocked = ui.tablePartitions->blockSignals(true);

	const int iOldRow = ui.tablePartitions->currentRow();

	ui.tablePartitions->clear();
	ui.tablePartitions->setColumnCount(1);

	ui.tablePartitions->setRowCount((int)m_pSequence->PartitionsNumber());
	for (int i = 0; i < (int)m_pSequence->PartitionsNumber(); ++i)
		ui.tablePartitions->setItem(i, 0, new QTableWidgetItem(QString(StrConst::TSE_PartitionName) + " " + QString::number(i)));

	ui.tablePartitions->RestoreSelectedCell(iOldRow, 0);

	ui.tablePartitions->blockSignals(bBlocked);
}

void CTearStreamsEditor::UpdateStreamsList()
{
	const bool bBlocked = ui.tableStreams->blockSignals(true);

	const int iOldRow = ui.tableStreams->currentRow();
	const int iPartition = ui.tablePartitions->currentRow();

	ui.tableStreams->clear();
	ui.tableStreams->setColumnCount(1);

	if(iPartition >= 0 && iPartition < static_cast<int>(m_pSequence->PartitionsNumber()))
	{
		ui.tableStreams->setRowCount((int)m_pSequence->TearStreamsNumber(iPartition));
		for (int i = 0; i < (int)m_pSequence->TearStreamsNumber(iPartition); ++i)
			ui.tableStreams->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(m_pSequence->PartitionTearStreams(iPartition)[i]->GetName())));
	}

	ui.tableStreams->RestoreSelectedCell(iOldRow, 0);

	ui.tableStreams->blockSignals(bBlocked);

	NewStreamSelected();
}

void CTearStreamsEditor::UpdateMode()
{
	if (m_pFlowsheet->m_pParams->initializeTearStreamsAutoFlag)
		ui.radioButtonAuto->setChecked(true);
	else
		ui.radioButtonUser->setChecked(true);
}

void CTearStreamsEditor::NewStreamSelected()
{
	CBaseStream* pSelectedStream = nullptr;
	const int iPartition = ui.tablePartitions->currentRow();
	if (iPartition >= 0 && iPartition < static_cast<int>(m_pSequence->PartitionsNumber()))
	{
		const int iStream = ui.tableStreams->currentRow();
		if (iStream >= 0 && iStream < static_cast<int>(m_pFlowsheet->m_vvInitTearStreams[iPartition].size()))
			pSelectedStream = &m_pFlowsheet->m_vvInitTearStreams[iPartition][iStream];

	}
	ui.widgetStreamsEditor->SetStream(pSelectedStream);

	// TODO: remove if CBasicStreamEditor::m_pSolidDistrEditor will always present
	SetEditable();
}

void CTearStreamsEditor::ClearAllStreams()
{
	if(ui.radioButtonAuto->isChecked())
	{
		QMessageBox::information(this, StrConst::TSE_WindowName, StrConst::TSE_InfoWrongMode);
		return;
	}

	if(QMessageBox::question(this, StrConst::TSE_WindowName, StrConst::TSE_QuestionRemoveAll, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QApplication::setOverrideCursor(Qt::WaitCursor);

		for (auto& part : m_pFlowsheet->m_vvInitTearStreams)
			for (auto& str : part)
				str.RemoveTimePointsAfter(0, true);
		NewStreamSelected();
		QApplication::restoreOverrideCursor();

		emit DataChanged();
	}
}

void CTearStreamsEditor::SetEditable()
{
	const bool bEnable = ui.radioButtonUser->isChecked();
	// TODO: put back if CBasicStreamEditor::m_pSolidDistrEditor will always present
	//if (bEnable != m_pFlowsheet->m_pParams->initializeTearStreamsAutoFlag)
	//	return;

	m_pFlowsheet->m_pParams->InitializeTearStreamsAutoFlag(!bEnable);

	ui.pushButtonClearAll->setEnabled(bEnable);
	ui.widgetStreamsEditor->SetEditable(bEnable);
}
