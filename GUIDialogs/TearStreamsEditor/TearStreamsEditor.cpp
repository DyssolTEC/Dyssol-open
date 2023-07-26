/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "TearStreamsEditor.h"
#include "Flowsheet.h"
#include "Stream.h"
#include "ParametersHolder.h"
#include "DyssolStringConstants.h"
#include "SignalBlocker.h"
#include <QMessageBox>

CTearStreamsEditor::CTearStreamsEditor(CFlowsheet* _pFlowsheet, CMaterialsDatabase* _materialsDB, CModelsManager* _modelsManager, QWidget* _parent)
	: CQtDialog{ _modelsManager, _parent }
{
	ui.setupUi(this);
	ui.widgetStreamsEditor->SetFlowsheet(_pFlowsheet, _materialsDB);
	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);

	m_pFlowsheet = _pFlowsheet;
	m_pSequence = _pFlowsheet->GetCalculationSequence();

	SetHelpLink("001_ui/gui.html#sec-gui-menu-setup-recycles");
}

void CTearStreamsEditor::InitializeConnections()
{
	connect(ui.radioButtonAuto,		&QRadioButton::toggled,					this, &CTearStreamsEditor::ModeChanged);
	connect(ui.radioButtonUser,		&QRadioButton::toggled,					this, &CTearStreamsEditor::ModeChanged);
	connect(ui.pushButtonClearAll,	&QRadioButton::clicked,					this, &CTearStreamsEditor::ClearAllStreams);
	connect(ui.tablePartitions,		&QTableWidget::itemSelectionChanged,	this, &CTearStreamsEditor::UpdateStreamsList);
	connect(ui.tableStreams,		&QTableWidget::itemSelectionChanged,	this, &CTearStreamsEditor::NewStreamSelected);
	connect(ui.widgetStreamsEditor, &CBasicStreamEditor::DataChanged,		this, &CTearStreamsEditor::DataChanged);
}

void CTearStreamsEditor::setVisible(bool _bVisible)
{
	if (_bVisible && !this->isVisible())
		UpdateWholeView();
	CQtDialog::setVisible(_bVisible);
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

void CTearStreamsEditor::UpdateMode() const
{
	[[maybe_unused]] CSignalBlocker blocker{ ui.radioButtonAuto, ui.radioButtonUser };
	if (m_pFlowsheet->GetParameters()->initializeTearStreamsAutoFlag)
		ui.radioButtonAuto->setChecked(true);
	else
		ui.radioButtonUser->setChecked(true);
	SetEditable();
}

void CTearStreamsEditor::NewStreamSelected() const
{
	CBaseStream* pSelectedStream = nullptr;
	const int iPartition = ui.tablePartitions->currentRow();
	if (iPartition >= 0 && iPartition < static_cast<int>(m_pSequence->PartitionsNumber()))
	{
		const auto initStreams = m_pFlowsheet->GetCalculationSequence()->GetAllInitialStreams();
		const int iStream = ui.tableStreams->currentRow();
		if (iStream >= 0 && iStream < static_cast<int>(initStreams[iPartition].size()))
			pSelectedStream = initStreams[iPartition][iStream];

	}
	ui.widgetStreamsEditor->SetStream(pSelectedStream);
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

		m_pFlowsheet->GetCalculationSequence()->ClearInitialStreamsData();
		NewStreamSelected();
		QApplication::restoreOverrideCursor();

		emit DataChanged();
	}
}

void CTearStreamsEditor::ModeChanged() const
{
	const bool manual = ui.radioButtonUser->isChecked();
	m_pFlowsheet->GetParameters()->InitializeTearStreamsAutoFlag(!manual);
	SetEditable();
}

void CTearStreamsEditor::SetEditable() const
{
	const bool enabled = !m_pFlowsheet->GetParameters()->initializeTearStreamsAutoFlag;
	ui.pushButtonClearAll->setEnabled(enabled);
	ui.widgetStreamsEditor->SetEditable(enabled);
}
