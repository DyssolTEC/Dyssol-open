/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "UnitsViewer.h"

CUnitsViewer::CUnitsViewer( CFlowsheet* _pFlowsheet, QWidget *parent, Qt::WindowFlags flags )
	: QWidget(parent, flags)
{
	ui.setupUi(this);

	m_pFlowsheet = _pFlowsheet;

	m_bAvoidSignal = false;
	m_pSelectedModel = NULL;
	m_nSelectedVariable = -1;

	m_pStackedWidget = new QStackedWidget( this );
	ui.horizontalLayout->addWidget( m_pStackedWidget, 1 );

	m_pTableWidget = new CQtTable( this );
	m_pTableWidget->setEditTriggers( QAbstractItemView::NoEditTriggers );
	m_pTableWidget->setColumnCount(2);
	m_pTableWidget->setHorizontalHeaderItem( 0, new QTableWidgetItem( "Time [s]" ) );
	m_pTableWidget->setHorizontalHeaderItem( 1, new QTableWidgetItem( "Value" ) );

	m_pPlot = new QtPlot::CQtPlot( this );
	m_pPlot->AddCurve(new QtPlot::SCurve("", Qt::blue, 4, true, true, QtPlot::LABEL_TIME, QtPlot::LABEL_VALUE));

	m_pTabWidget = new QTabWidget( m_pStackedWidget );
	m_pTabWidget->addTab( m_pTableWidget, "Table view" );
	m_pTabWidget->addTab( m_pPlot, "Plot view" );

	m_pStreamsViewer = new CBasicStreamsViewer(m_pFlowsheet, m_pStackedWidget);

	m_pPlotsViewer = new CPlotsViewer( m_pStackedWidget );

	m_pStackedWidget->addWidget( m_pStreamsViewer );
	m_pStackedWidget->addWidget( m_pTabWidget );
	m_pStackedWidget->addWidget( m_pPlotsViewer );

	m_nLastTab = 0;
	m_nLastVariable = 0;
	m_nLastUnit = 0;
}

CUnitsViewer::~CUnitsViewer()
{

}

void CUnitsViewer::InitializeConnections()
{
	connect( m_pTabWidget,				SIGNAL( currentChanged ( int ) ),			this,	SLOT( TabChanged() ));
	connect( ui.listWidgetUnits,		SIGNAL( itemSelectionChanged() ),			this,	SLOT( UnitChanged() ));
	connect( ui.listWidgetVariables,	SIGNAL( itemSelectionChanged() ),			this,	SLOT( VariableChanged() ));
	connect( ui.listWidgetHoldups,		SIGNAL( itemSelectionChanged() ),			this,	SLOT( HoldupChanged() ) );
	connect( ui.listWidgetPlots,		SIGNAL( itemSelectionChanged() ),			this,	SLOT( PlotChanged() ));
	connect( ui.listWidgetCurves,		SIGNAL( itemSelectionChanged() ),			this,	SLOT( CurveChanged() ));
	connect( ui.toolBox,				SIGNAL( currentChanged ( int ) ),			this,	SLOT( FocusChanged(int) ));

	m_pStreamsViewer->InitializeConnections();
}

void CUnitsViewer::TabChanged()
{
	m_nSelectedVariable = ui.listWidgetVariables->currentRow();
	UpdateValuesView();
}

void CUnitsViewer::UpdateUnitsView()
{
	if (m_bAvoidSignal) return;
	m_bAvoidSignal = true;

	int nOldSelected = ui.listWidgetUnits->currentRow();
	ui.listWidgetUnits->clear();
	for (unsigned i = 0; i < m_pFlowsheet->GetModelsCount(); i++)
		ui.listWidgetUnits->insertItem(i, new QListWidgetItem(QString::fromStdString(m_pFlowsheet->GetModel(i)->GetModelName())));

	m_bAvoidSignal = false;

	if (nOldSelected < (int)m_pFlowsheet->GetModelsCount())
		ui.listWidgetUnits->setCurrentRow(nOldSelected);
}

void CUnitsViewer::UpdateVariablesView()
{
	if (m_bAvoidSignal) return;
	if (m_pSelectedModel == NULL) return;
	m_bAvoidSignal = true;

	int nOldSelected = ui.listWidgetVariables->currentRow();
	ui.listWidgetVariables->clear();
	for (unsigned i = 0; i < m_pSelectedModel->GetStoredStateVariablesNumber(); i++)
		ui.listWidgetVariables->insertItem(i, new QListWidgetItem(QString::fromStdString(m_pSelectedModel->GetStoredStateVariableName(i))));

	if ((nOldSelected != -1) && (nOldSelected < (int)m_pSelectedModel->GetStoredStateVariablesNumber()))
		ui.listWidgetVariables->setCurrentRow(nOldSelected);
	else if (m_pSelectedModel->GetStoredStateVariablesNumber() > 0)
		ui.listWidgetVariables->setCurrentRow(0);
	else
		ui.listWidgetVariables->setCurrentRow(-1);

	m_bAvoidSignal = false;
}

void CUnitsViewer::UpdateValuesView()
{
	if (m_bAvoidSignal) return;
	if(!m_pSelectedModel || m_nSelectedVariable == -1 || m_pSelectedModel->GetStoredStateVariablesNumber() == 0)
	{
		m_pTabWidget->setEnabled(false);
		if (m_pTabWidget->currentIndex() == 0)
			m_pTableWidget->setRowCount(0);
		else
			m_pPlot->ClearCurve(0);
		return;
	}
	m_pTabWidget->setEnabled(true);

	if (m_pTabWidget->currentIndex() == 0)
	{
		m_pTableWidget->setRowCount(0);
		std::vector<double> vTimes;
		std::vector<double> vValues;
		m_pSelectedModel->GetStoredStateVariableData(m_nSelectedVariable, &vTimes, &vValues);
		for (unsigned i = 0; i < vTimes.size(); ++i)
		{
			m_pTableWidget->insertRow(i);
			m_pTableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(vTimes[i])));
			m_pTableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(vValues[i])));
		}
	}
	else
	{
		m_pPlot->ClearCurve(0);
		m_pPlot->SetCurveName(0, QString::fromStdString(m_pSelectedModel->GetStoredStateVariableName(m_nSelectedVariable)));
		std::vector<double> vTimes;
		std::vector<double> vValues;
		m_pSelectedModel->GetStoredStateVariableData(m_nSelectedVariable, &vTimes, &vValues);
		m_pPlot->AddPoints(0, vTimes, vValues);
		if (vTimes.size() == 1)
			m_pPlot->SetCurveLinesVisibility(0, false);
	}
}

void CUnitsViewer::UpdateHoldupsView()
{
	if (m_bAvoidSignal) return;
	if (!m_pSelectedModel) return;
	m_bAvoidSignal = true;

	int nOldSelected = ui.listWidgetHoldups->currentRow();
	ui.listWidgetHoldups->clear();
	for (unsigned i = 0; i < m_pSelectedModel->GetHoldupsCount(); ++i)
		ui.listWidgetHoldups->insertItem(i, new QListWidgetItem(QString::fromStdString(m_pSelectedModel->GetHoldup(i)->GetStreamName())));

	if ((nOldSelected != -1) && (nOldSelected < (int)m_pSelectedModel->GetHoldupsCount()))
		ui.listWidgetHoldups->setCurrentRow(nOldSelected);
	else if (m_pSelectedModel->GetHoldupsCount() > 0)
		ui.listWidgetHoldups->setCurrentRow(0);
	else
		ui.listWidgetHoldups->setCurrentRow(-1);

	m_bAvoidSignal = false;
}

void CUnitsViewer::UpdatePlotsView()
{
	if (m_bAvoidSignal) return;
	if (!m_pSelectedModel)
	{
		ui.listWidgetPlots->clear();
		UpdateCurvesView();
		return;
	}
	m_bAvoidSignal = true;

	int nOldSelected = ui.listWidgetPlots->currentRow();
	ui.listWidgetPlots->clear();
	for (unsigned i = 0; i < m_pSelectedModel->GetPlotsNumber(); ++i)
		ui.listWidgetPlots->insertItem(i, new QListWidgetItem(QString::fromStdString(m_pSelectedModel->GetPlotName(i))));

	m_bAvoidSignal = false;

	if ((nOldSelected != -1) && (nOldSelected < (int)m_pSelectedModel->GetPlotsNumber()))
		ui.listWidgetPlots->setCurrentRow(nOldSelected);
	else if (ui.listWidgetPlots->count() > 0)
		ui.listWidgetPlots->setCurrentRow(0);
	else
		ui.listWidgetPlots->setCurrentRow(-1);

	UpdateCurvesView();
}

void CUnitsViewer::UpdateCurvesView()
{
	if( m_bAvoidSignal ) return;
	int iPlot = ui.listWidgetPlots->currentRow();
	if(( !m_pSelectedModel ) || (iPlot == -1))
	{
		ui.listWidgetCurves->clear();
		return;
	}
	m_bAvoidSignal = true;

	int nOldSelected = ui.listWidgetCurves->currentRow();
	ui.listWidgetCurves->clear();
	for( unsigned i=0; i<m_pSelectedModel->GetCurvesNumber(iPlot); ++i )
		ui.listWidgetCurves->insertItem( i, new QListWidgetItem( QString::fromStdString( m_pSelectedModel->GetCurveName( iPlot, i ) ) ) );

	m_bAvoidSignal = false;

	if(( nOldSelected == -1 ) && (ui.listWidgetCurves->count() > 0))
		ui.listWidgetCurves->setCurrentRow(0);
	else if ( nOldSelected < (int)m_pSelectedModel->GetCurvesNumber(iPlot) )
		ui.listWidgetCurves->setCurrentRow( nOldSelected );
	else if(ui.listWidgetCurves->count() > 0)
		ui.listWidgetCurves->setCurrentRow(0);
}

void CUnitsViewer::UnitChanged()
{
	m_pSelectedModel = m_pFlowsheet->GetModel( ui.listWidgetUnits->currentRow() );
	m_pPlotsViewer->SetSelectedModel( m_pSelectedModel );
	UpdateVariablesView();
	UpdateValuesView();
	UpdateHoldupsView();
	UpdatePlotsView();
}

void CUnitsViewer::VariableChanged()
{
	m_nSelectedVariable = ui.listWidgetVariables->currentRow();
	if ((m_nSelectedVariable == -1) && (m_pSelectedModel->GetStoredStateVariablesNumber() > 0))
		m_nSelectedVariable = 0;
	UpdateValuesView();
}

void CUnitsViewer::HoldupChanged()
{
	if( !m_pSelectedModel ) return;
	QModelIndexList indexes = ui.listWidgetHoldups->selectionModel()->selection().indexes();
	std::vector<const CStream*> vHoldups;
	for( int i=0; i<indexes.size(); ++i )
		if( ( indexes[i].row() >= 0 ) && ( indexes[i].row() < (int)m_pSelectedModel->GetHoldupsCount() ) )
			vHoldups.push_back(m_pSelectedModel->GetHoldup(indexes[i].row()));
	m_pStreamsViewer->SetStreams(vHoldups);
}

void CUnitsViewer::PlotChanged()
{
	if( m_bAvoidSignal ) return;
	m_pPlotsViewer->SetSelectedPlot( ui.listWidgetPlots->currentRow() );
	UpdateCurvesView();
}

void CUnitsViewer::CurveChanged()
{
	if( m_bAvoidSignal ) return;
	QModelIndexList indexes = ui.listWidgetCurves->selectionModel()->selection().indexes();
	std::vector<int> vOutIndexes;
	for( int i=0; i<indexes.size(); ++i )
		vOutIndexes.push_back(indexes[i].row());
	m_pPlotsViewer->SetSelectedCurve( vOutIndexes );
}

void CUnitsViewer::FocusChanged(int index)
{
	m_pStackedWidget->setCurrentIndex(index);
}

void CUnitsViewer::UpdateWholeView()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	UnitChanged();
	HoldupChanged();
	m_pStreamsViewer->UpdateWholeView();
	UpdateUnitsView();
	UpdateVariablesView();
	UpdateValuesView();
	UpdatePlotsView();

	QApplication::restoreOverrideCursor();
}

void CUnitsViewer::setVisible( bool _bVisible )
{
	if( ( _bVisible ) && ( !this->isVisible() ) )
	{
		UpdateWholeView();
		LoadViewState();
	}
	else
	{
		SaveViewState();
	}
	QWidget::setVisible( _bVisible );
}

void CUnitsViewer::SaveViewState()
{
	m_nLastTab = m_pTabWidget->currentIndex() == -1 ? 0 : m_pTabWidget->currentIndex();
	m_nLastUnit = ui.listWidgetUnits->currentRow() == -1 ? 0 : ui.listWidgetUnits->currentRow();
	m_nLastVariable = ui.listWidgetVariables->currentRow() == -1 ? 0 : ui.listWidgetVariables->currentRow();
	m_nLastPlot = ui.listWidgetPlots->currentRow() == -1 ? 0 : ui.listWidgetPlots->currentRow();
	m_nLastCurve = ui.listWidgetCurves->currentRow() == -1 ? 0 : ui.listWidgetCurves->currentRow();
}

void CUnitsViewer::LoadViewState()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	if( m_nLastTab < m_pTabWidget->count() )
		m_pTabWidget->setCurrentIndex( m_nLastTab );
	if( m_nLastUnit < ui.listWidgetUnits->count() )
		ui.listWidgetUnits->setCurrentRow( m_nLastUnit );
	if( m_nLastVariable < ui.listWidgetVariables->count() )
		ui.listWidgetVariables->setCurrentRow( m_nLastVariable );
	if( m_nLastPlot < ui.listWidgetPlots->count() )
		ui.listWidgetPlots->setCurrentRow( m_nLastPlot );
	if( m_nLastCurve < ui.listWidgetCurves->count() )
		ui.listWidgetCurves->setCurrentRow( m_nLastCurve );

	QApplication::restoreOverrideCursor();
}
