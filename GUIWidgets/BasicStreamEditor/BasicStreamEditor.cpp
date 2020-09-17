/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "BasicStreamEditor.h"
#include "Phase.h"
#include <QMessageBox>

CBasicStreamEditor::CBasicStreamEditor(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	m_bAvoidSignal = false;
	m_pSelectedHoldup = nullptr;
	m_pSolidDistrEditor = new CSolidDistributionsEditor(ui.mainTabWidget);
	m_pFlowsheet = nullptr;

	m_pDDTableMTP = new CDDTable(ui.mainTabWidget);
	ui.mainTabWidget->insertTab(0, m_pDDTableMTP, "MTP");

	m_pDDTablePhase = new CDDTable(ui.mainTabWidget);
	m_pDDTablePhase->SetNormalizationCheck(true);
	ui.mainTabWidget->insertTab(1, m_pDDTablePhase, "Phase fractions");

	UpdateTabs();

	ui.mainTabWidget->setCurrentIndex(0);

	m_nLastTab = 0;
	m_nLastTime = 0;

	InitializeConnections();
}

void CBasicStreamEditor::InitializeConnections()
{
	connect(ui.addTimePoint, &QPushButton::clicked, this, &CBasicStreamEditor::AddTimePoint);
	connect(ui.removeTimePoint, &QPushButton::clicked, this, &CBasicStreamEditor::RemoveTimePoint);
	connect(ui.removeAllTimePoints, &QPushButton::clicked, this, &CBasicStreamEditor::RemoveAllTimePoints);
	connect(ui.timePointsTable, &QTableWidget::itemChanged, this, &CBasicStreamEditor::ChangeTimePoint);
	connect(ui.timePointsTable, &QTableWidget::currentCellChanged, this, &CBasicStreamEditor::TableTimeChanged);
	connect(ui.mainTabWidget, &QTabWidget::currentChanged, this, &CBasicStreamEditor::UpdateTabContent);
	connect(m_pDDTableMTP, &CDDTable::DataChanged, this, &CBasicStreamEditor::ChangeData);
	connect(m_pDDTablePhase, &CDDTable::DataChanged, this, &CBasicStreamEditor::ChangeData);
}

void CBasicStreamEditor::SetFlowsheet(CFlowsheet* _pFlowsheet)
{
	m_pFlowsheet = _pFlowsheet;
}

void CBasicStreamEditor::SetStream(CBaseStream* _pStream)
{
	m_pSelectedHoldup = _pStream;
	UpdateWholeView();
}

void CBasicStreamEditor::SetEditable(bool _bEditable)
{
	ui.timePointsTable->SetEditable(_bEditable);
	ui.addTimePoint->setEnabled(_bEditable);
	ui.removeTimePoint->setEnabled(_bEditable);
	ui.removeAllTimePoints->setEnabled(_bEditable);
	m_pDDTableMTP->SetEditable(_bEditable);
	m_pDDTablePhase->SetEditable(_bEditable);
	for (auto mdTable : m_vMDMTablePhases)
		mdTable->SetEditable(_bEditable);
	//if(m_pFlowsheet->GetPhaseIndex(SOA_SOLID) != -1)
		m_pSolidDistrEditor->SetEditable(_bEditable);
}

void CBasicStreamEditor::setVisible(bool _bVisible)
{
	if (_bVisible && !this->isVisible())
	{
		UpdateWholeView();
		LoadViewState();
	}
	else
		SaveViewState();
	QWidget::setVisible(_bVisible);
}

void CBasicStreamEditor::UpdateWholeView()
{
	UpdateTabs();
	UpdateTabContent();
}

void CBasicStreamEditor::UpdateTabs()
{
	if (!m_pFlowsheet) return;
	m_bAvoidSignal = true;

	const int iOldTab = ui.mainTabWidget->currentIndex();

	if (ui.mainTabWidget->count() > static_cast<int>(m_vMDMTablePhases.size()) + 2)
		ui.mainTabWidget->removeTab(static_cast<int>(m_vMDMTablePhases.size()) + 2);

	if (m_pFlowsheet->GetPhaseIndex(SOA_SOLID) == -1)
	{
		m_pSolidDistrEditor->GetViewState(m_vLastCombos, m_vLastSliders);
		m_pSolidDistrEditor->setVisible(false);
	}
	for (unsigned i = 0; i<m_vMDMTablePhases.size(); ++i)
		ui.mainTabWidget->removeTab(2);
	m_vMDMTablePhases.clear();

	unsigned nSolidIndex = 0;
	for (unsigned i = 0; i<m_pFlowsheet->GetPhasesNumber(); ++i)
	{
		if (m_pFlowsheet->GetPhaseAggregationState(i) == SOA_SOLID)
		{
			//m_pSolidDistrEditor = new CSolidDistributionsEditor(ui.mainTabWidget);
			m_pSolidDistrEditor->SetFlowsheet(m_pFlowsheet);
			if(m_pSelectedHoldup)
				m_pSolidDistrEditor->SetDistribution(m_pSelectedHoldup->GetPhase(CFlowsheet::PhaseSOA2EPhase(m_pFlowsheet->GetPhaseAggregationState(i)))->MDDistr(), m_pSelectedHoldup);
			else
				m_pSolidDistrEditor->SetDistribution(nullptr, nullptr);
			m_pSolidDistrEditor->SetViewState(m_vLastCombos, m_vLastSliders);
			connect(m_pSolidDistrEditor, SIGNAL(DataChanged()), this, SLOT(ChangeData()));
			nSolidIndex = i;
		}
		else
		{
			CMDMTable *pMDMTable = new CMDMTable(ui.mainTabWidget);
			m_vMDMTablePhases.push_back(pMDMTable);
			ui.mainTabWidget->insertTab(i + 2, pMDMTable, QString::fromStdString(m_pFlowsheet->GetPhaseName(i)));
			connect(pMDMTable, SIGNAL(DataChanged()), this, SLOT(ChangeData()));
		}
	}

	if (m_pFlowsheet->GetPhaseIndex(SOA_SOLID) != -1)
		ui.mainTabWidget->insertTab(m_pFlowsheet->GetPhasesNumber() + 2, m_pSolidDistrEditor, QString::fromStdString(m_pFlowsheet->GetPhaseName(nSolidIndex)));

	if (iOldTab < ui.mainTabWidget->count())
		ui.mainTabWidget->setCurrentIndex(iOldTab);

	m_bAvoidSignal = false;
}

void CBasicStreamEditor::UpdateTabContent()
{
	m_bAvoidSignal = true;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	SetControlsActivity(m_pSelectedHoldup != nullptr);

	int nTabIndex = ui.mainTabWidget->currentIndex();
	if (nTabIndex == -1)
		return;

	if (m_pFlowsheet->GetPhaseIndex(SOA_SOLID) != -1)
		m_pSolidDistrEditor->GetViewState(m_vLastCombos, m_vLastSliders);

	if (nTabIndex == 0)
		UpdateStreamMTPTab();
	else if (nTabIndex == 1)
		UpdatePhaseFractionsTab();
	else if (nTabIndex < (int)m_vMDMTablePhases.size() + 2)
		UpdatePhaseTab(nTabIndex - 2);
	else
	{
		if (m_pFlowsheet->GetPhaseIndex(SOA_SOLID) != -1)
			m_pSolidDistrEditor->SetViewState(m_vLastCombos, m_vLastSliders);
		UpdateDistributionTab();
		int nTimeRow = ui.timePointsTable->currentRow();
		if (ui.timePointsTable->rowCount() > 0)
		{
			if (nTimeRow == -1)
				TableTimeChanged(0, 0, 0, 0);
			else
				TableTimeChanged(nTimeRow, 0, 0, 0);
		}
	}

	UpdateTimePointsTable();

	QApplication::restoreOverrideCursor();

	m_bAvoidSignal = false;
}

void CBasicStreamEditor::UpdateStreamMTPTab()
{
	if (m_pSelectedHoldup == NULL)
		m_pDDTableMTP->setVisible(false);
	else
	{
		m_bAvoidSignal = true;
		m_pDDTableMTP->SetDistribution(m_pSelectedHoldup->GetOverallProperty(EOverall::OVERALL_MASS));
		m_bAvoidSignal = false;
		m_pDDTableMTP->setVisible(true);
	}
}

void CBasicStreamEditor::UpdatePhaseFractionsTab()
{
	if (m_pSelectedHoldup == NULL)
		m_pDDTablePhase->setVisible(false);
	else
	{
		m_bAvoidSignal = true;
		m_pDDTablePhase->SetDistribution(m_pSelectedHoldup->GetPhase(EPhase::SOLID)->Fractions());
		m_bAvoidSignal = false;
		m_pDDTablePhase->setVisible(true);
	}
}

void CBasicStreamEditor::UpdatePhaseTab(unsigned _nIndex)
{
	if (!m_pFlowsheet) return;

	if (_nIndex >= m_vMDMTablePhases.size()) return;

	if (m_pSelectedHoldup == NULL)
		m_vMDMTablePhases[_nIndex]->setVisible(false);
	else
	{
		unsigned iPhaseIndex = _nIndex;
		for (unsigned i = 0; i <= _nIndex; ++i)
		{
			if ((i<m_pFlowsheet->GetPhasesNumber()) && (m_pFlowsheet->GetPhasesAggregationStates()->at(i) == SOA_SOLID))
			{
				iPhaseIndex += 1;
				break;
			}
		}

		m_bAvoidSignal = true;
		m_vMDMTablePhases[_nIndex]->SetDistribution(m_pSelectedHoldup->GetPhase(CFlowsheet::PhaseSOA2EPhase(m_pFlowsheet->GetPhaseAggregationState(iPhaseIndex)))->MDDistr(), m_pFlowsheet->GetCompoundsNames());
		m_bAvoidSignal = false;
		m_vMDMTablePhases[_nIndex]->setVisible(true);
	}
}

void CBasicStreamEditor::UpdateDistributionTab()
{
	if (!m_pFlowsheet) return;

	if (m_pFlowsheet->GetPhaseIndex(SOA_SOLID) == -1)
		return;

	if (m_pSelectedHoldup == NULL)
		m_pSolidDistrEditor->setVisible(false);
	else
	{
		m_bAvoidSignal = true;
		for (unsigned i = 0; i<m_pFlowsheet->GetPhasesNumber(); ++i)
			if (m_pFlowsheet->GetPhaseAggregationState(i) == SOA_SOLID)
			{
				m_pSolidDistrEditor->SetDistribution(m_pSelectedHoldup->GetPhase(CFlowsheet::PhaseSOA2EPhase(m_pFlowsheet->GetPhaseAggregationState(i)))->MDDistr(), m_pSelectedHoldup);
				break;
			}
		m_bAvoidSignal = false;
		m_pSolidDistrEditor->setVisible(true);
	}
}

void CBasicStreamEditor::UpdateTimePointsTable()
{
	if (m_pSelectedHoldup == NULL)
	{
		while (ui.timePointsTable->rowCount() > 0)
			ui.timePointsTable->removeRow(ui.timePointsTable->rowCount() - 1);
		return;
	}

	m_bAvoidSignal = true;

	std::vector<double> vTimePoints = m_pSelectedHoldup->GetAllTimePoints();
	for (int i = 0; i<(int)vTimePoints.size(); ++i)
	{
		if (i >= ui.timePointsTable->rowCount())
			ui.timePointsTable->insertRow(i);
		ui.timePointsTable->setItem(i, 0, new QTableWidgetItem(QString::number(vTimePoints[i])));
	}
	while (ui.timePointsTable->rowCount() >(int)vTimePoints.size())
		ui.timePointsTable->removeRow(ui.timePointsTable->rowCount() - 1);

	m_bAvoidSignal = false;
}

void CBasicStreamEditor::AddTimePoint()
{
	if (m_pSelectedHoldup == NULL) return;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	double dLastTP = m_pSelectedHoldup->GetLastTimePoint();
	m_pSelectedHoldup->AddTimePoint(dLastTP + 1);
	if (dLastTP == -1)
	{
		m_pSelectedHoldup->SetOverallProperty(dLastTP + 1, EOverall::OVERALL_TEMPERATURE, 300);
		m_pSelectedHoldup->SetOverallProperty(dLastTP + 1, EOverall::OVERALL_PRESSURE, 100000);
	}
	UpdateTabContent();

	QApplication::restoreOverrideCursor();

	emit DataChanged();
}

void CBasicStreamEditor::RemoveTimePoint()
{
	if (m_pSelectedHoldup == NULL) return;
	if (ui.timePointsTable->rowCount() == 0) return; // no data has been defined

	QApplication::setOverrideCursor(Qt::WaitCursor);

	double dTime;
	if (ui.timePointsTable->currentRow() == -1)
		dTime = (ui.timePointsTable->item(ui.timePointsTable->rowCount() - 1, 0))->text().toDouble();
	else
		dTime = (ui.timePointsTable->item(ui.timePointsTable->currentRow(), 0))->text().toDouble();
	m_pSelectedHoldup->RemoveTimePoint(dTime);
	UpdateTabContent();

	QApplication::restoreOverrideCursor();

	emit DataChanged();
}

void CBasicStreamEditor::RemoveAllTimePoints()
{
	if (m_pSelectedHoldup == NULL) return;
	if (ui.timePointsTable->rowCount() == 0) return; // no data has been defined

	if (QMessageBox::question(this, tr("Removal"), tr("Do you really want to remove all time points?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		QApplication::setOverrideCursor(Qt::WaitCursor);

		m_pSelectedHoldup->RemoveAllTimePoints();
		UpdateTabContent();

		QApplication::restoreOverrideCursor();

		emit DataChanged();
	}
}

void CBasicStreamEditor::ChangeTimePoint()
{
	if (m_bAvoidSignal) return;
	if (m_pSelectedHoldup == NULL) return;
	double dNewValue = (ui.timePointsTable->item(ui.timePointsTable->currentRow(), 0))->text().toDouble();
	//m_pSelectedHoldup->ChangeTimePoint(ui.timePointsTable->currentRow(), dNewValue);
	UpdateTabContent();
	emit DataChanged();
}

void CBasicStreamEditor::TableTimeChanged(int _nRow, int _nCol, int _nPrevRow, int _nPrevCol)
{
	if (ui.mainTabWidget->currentIndex() == m_vMDMTablePhases.size() + 2) // solid distribution tab
	{
		QApplication::setOverrideCursor(Qt::WaitCursor);
		if (m_pFlowsheet->GetPhaseIndex(SOA_SOLID) != -1)
		{
			m_pSolidDistrEditor->GetViewState(m_vLastCombos, m_vLastSliders);
			m_pSolidDistrEditor->SetViewState(m_vLastCombos, m_vLastSliders);
		}
		m_pSolidDistrEditor->SetTimePoint(_nRow);
		QApplication::restoreOverrideCursor();
	}
}

void CBasicStreamEditor::SetControlsActivity(bool _bActive)
{
	ui.frameHoldupControls->setEnabled(_bActive);
}

void CBasicStreamEditor::SaveViewState()
{
	m_nLastTab = ui.mainTabWidget->currentIndex() == -1 ? 0 : ui.mainTabWidget->currentIndex();
	m_nLastTime = ui.timePointsTable->currentRow() == -1 ? 0 : ui.timePointsTable->currentRow();
	if (m_pFlowsheet->GetPhaseIndex(SOA_SOLID) != -1)
		m_pSolidDistrEditor->GetViewState(m_vLastCombos, m_vLastSliders);
}

void CBasicStreamEditor::LoadViewState()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	if (m_nLastTab < ui.mainTabWidget->count())
		ui.mainTabWidget->setCurrentIndex(m_nLastTab);
	if (m_nLastTime < ui.timePointsTable->rowCount())
		ui.timePointsTable->selectRow(m_nLastTime);
	if (m_pFlowsheet->GetPhaseIndex(SOA_SOLID) != -1)
		m_pSolidDistrEditor->SetViewState(m_vLastCombos, m_vLastSliders);

	QApplication::restoreOverrideCursor();
}

void CBasicStreamEditor::ChangeData()
{
	emit DataChanged();
}
