/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "SimulatorTab.h"
#include "DyssolStringConstants.h"
#include "Stream.h"
#include <QMessageBox>
#include <QDateTime>

CSimulatorTab::CSimulatorTab(CFlowsheet* _pFlowsheet, CSimulator* _pSimulator, QWidget* _parent /*= 0*/) :
	QWidget(_parent),
	m_pFlowsheet{ _pFlowsheet },
	m_pSimulator{ _pSimulator },
	m_pProgressThread{ new CProgressThread(m_pSimulator) }
{
	ui.setupUi(this);
}

CSimulatorTab::~CSimulatorTab()
{
	m_pProgressThread->Stop();
	m_pProgressThread->deleteLater();
}

void CSimulatorTab::InitializeConnections() const
{
	connect(ui.lineEditTime,		          &QLineEdit::editingFinished, this, &CSimulatorTab::SetSimulationTime);
	connect(ui.buttonRun,			          &QPushButton::clicked,	   this, &CSimulatorTab::StartSimulation);
	connect(ui.buttonClearResults,	          &QPushButton::clicked,	   this, &CSimulatorTab::ClearSimulationResults);
	connect(ui.buttonClearRecycles,	          &QPushButton::clicked,	   this, &CSimulatorTab::ClearInitialRecycleStreams);
	connect(ui.buttonClearResultsAndRecycles, &QPushButton::clicked,	   this, &CSimulatorTab::ClearAll);

	connect(m_pProgressThread,	&CProgressThread::Finished,		this, &CSimulatorTab::SimulationFinished);
	connect(&m_logTimer,	    &QTimer::timeout,				this, &CSimulatorTab::UpdateLog);
}

void CSimulatorTab::setVisible(bool _visible)
{
	QWidget::setVisible(_visible);
	if (_visible)
		UpdateWholeView();
}

void CSimulatorTab::UpdateWholeView() const
{
	UpdateSimulationTime();
}

void CSimulatorTab::OnNewFlowsheet() const
{
	ClearLog();
}

void CSimulatorTab::SetSimulationTime()
{
	m_pFlowsheet->SetSimulationTime(ui.lineEditTime->text().toDouble());
	UpdateSimulationTime();
	emit DataChanged();
}

void CSimulatorTab::StartSimulation()
{
	if (m_pSimulator->GetCurrentStatus() == ESimulatorStatus::SIMULATOR_RUNNED)
		AbortSimulation();
	else
	{
		// run timer
		const QDateTime now = QDateTime::currentDateTime();
		m_simulationTimer.start();

		// initialize GUI elements
		ui.buttonRun->setText(StrConst::ST_ButtonRunTextStop);
		ui.tableLog->SetItemNotEditable(EStatTable::STARTED_TIME, 0, now.toString("hh:mm:ss"));
		ui.textBrowserLog->clear();
		ui.textBrowserLog->append(QString::fromStdString(StrConst::ST_LogSimStart(now.toString("hh:mm:ss").toStdString(), now.toString("dd.MM.yyyy").toStdString())));
		ui.textBrowserLog->append(StrConst::ST_LogInitStart);

		// initialize flowsheet
		const std::string error = m_pFlowsheet->Initialize();
		if (!error.empty())
		{
			ui.textBrowserLog->setTextColor(QColor(Qt::red));
			ui.textBrowserLog->append(QString::fromStdString(error));
			ui.textBrowserLog->setTextColor(QColor(Qt::black));
			ui.buttonRun->setText(StrConst::ST_ButtonRunTextRun);
			return;
		}

		ui.textBrowserLog->append(StrConst::ST_LogInitFinish);

		emit DataChanged();

		// run simulation
		BlockUI(true);
		emit SimulatorStateToggled(true);
		m_pProgressThread->Run();
		m_logTimer.start(100);
	}
}

void CSimulatorTab::AbortSimulation() const
{
	// stop simulation
	m_pProgressThread->StopTask();

	// set the message to the log
	UpdateLog(); // set the rest messages from the queue
	ui.textBrowserLog->setTextColor(QColor(Qt::red));
	ui.textBrowserLog->append(StrConst::ST_LogSimUserStop);
	ui.textBrowserLog->setTextColor(QColor(Qt::black));
}

void CSimulatorTab::SimulationFinished()
{
	// stop simulation threads and timers
	const QDateTime now = QDateTime::currentDateTime();
	m_logTimer.stop();
	m_pProgressThread->Stop();

	// update simulation log
	UpdateLog();

	// setup GUI elements
	ui.textBrowserLog->append(QString::fromStdString(StrConst::ST_LogSimFinishedTime(now.toString("hh:mm:ss").toStdString(), now.toString("dd.MM.yyyy").toStdString(), QString::number(m_simulationTimer.elapsed()/1000.).toStdString())));
	ui.buttonRun->setText(StrConst::ST_ButtonRunTextRun);

	BlockUI(false);
	emit SimulatorStateToggled(false);
}

void CSimulatorTab::ClearSimulationResults()
{
	if (QMessageBox::question(this, StrConst::ST_TitleClearResults, StrConst::ST_QuestionClearResults, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel) != QMessageBox::Yes)
		return;

	m_pFlowsheet->ClearSimulationResults();
	ClearLog();

	emit DataChanged();
}

void CSimulatorTab::ClearInitialRecycleStreams()
{
	if (QMessageBox::question(this, StrConst::ST_TitleClearRecycles, StrConst::ST_QuestionClearRecycles, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel) != QMessageBox::Yes)
		return;

	for (auto& partition : m_pFlowsheet->m_vvInitTearStreams)
		for (auto& stream : partition)
			stream.RemoveTimePointsAfter(0, true);

	emit DataChanged();
}

void CSimulatorTab::ClearAll()
{
	if (QMessageBox::question(this, StrConst::ST_TitleClearAll, StrConst::ST_QuestionClearAll, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel) != QMessageBox::Yes)
		return;

	m_pFlowsheet->ClearSimulationResults();
	for (auto& partition : m_pFlowsheet->m_vvInitTearStreams)
		for (auto& stream : partition)
			stream.RemoveTimePointsAfter(0, true);
	ClearLog();

	emit DataChanged();
}

void CSimulatorTab::ClearLog() const
{
	ui.textBrowserLog->clear();
	ui.tableLog->clearContents();
}

void CSimulatorTab::UpdateLog() const
{
	while (!m_pSimulator->m_log.EndOfLog())
	{
		switch (m_pSimulator->m_log.GetReadColor())
		{
		case CSimulatorLog::ELogColor::DEFAULT: ui.textBrowserLog->setTextColor(QColor(Qt::black));		break;
		case CSimulatorLog::ELogColor::RED:		ui.textBrowserLog->setTextColor(QColor(Qt::red));		break;
		case CSimulatorLog::ELogColor::ORANGE:	ui.textBrowserLog->setTextColor(QColor(255, 128, 0));	break;
		}
		ui.textBrowserLog->append(QString::fromStdString(m_pSimulator->m_log.Read()));
	}
	ui.textBrowserLog->setTextColor(QColor(Qt::black));

	ui.tableLog->SetItemNotEditable(EStatTable::TIME_WIN_START,   0, m_pSimulator->m_dTWStart);
	ui.tableLog->SetItemNotEditable(EStatTable::TIME_WIN_END,     0, m_pSimulator->m_dTWEnd);
	ui.tableLog->SetItemNotEditable(EStatTable::TIME_WIN_LENGTH,  0, m_pSimulator->m_dTWLength);
	ui.tableLog->SetItemNotEditable(EStatTable::ITERATION_NUMBER, 0, m_pSimulator->m_iTWIterationFull);
	ui.tableLog->SetItemNotEditable(EStatTable::WINDOW_NUMBER,    0, m_pSimulator->m_iWindowNumber);
	ui.tableLog->SetItemNotEditable(EStatTable::UNIT_NAME,        0, m_pSimulator->m_sUnitName);
	ui.tableLog->SetItemNotEditable(EStatTable::ELAPSED_TIME,     0, QDateTime::fromTime_t(m_simulationTimer.elapsed() / 1000).toUTC().toString("hh:mm:ss"));
}

void CSimulatorTab::UpdateSimulationTime() const
{
	ui.lineEditTime->setText(QString::number(m_pFlowsheet->GetSimulationTime()));
}

void CSimulatorTab::BlockUI(bool _block) const
{
	ui.buttonClearRecycles->setEnabled(!_block);
	ui.buttonClearResults->setEnabled(!_block);
	ui.buttonClearResultsAndRecycles->setEnabled(!_block);
	ui.lineEditTime->setEnabled(!_block);
}
