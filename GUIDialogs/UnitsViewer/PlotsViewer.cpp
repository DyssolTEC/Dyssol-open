/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "PlotsViewer.h"
#include "UnitContainer.h"
#include "BaseUnit.h"
#include "DyssolStringConstants.h"

CPlotsViewer::CPlotsViewer(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	m_pTableWidget = new CQtTable( this );
	m_pTableWidget->setEditTriggers( QAbstractItemView::NoEditTriggers );
	ui.tabWidget->addTab( m_pTableWidget, StrConst::BSV_TabNameTable );

	m_pPlot = new QtPlot::CQtPlot( this );
	m_pPlot->SetLegendPosition(POSITION_TOP_LEFT);
	ui.tabWidget->addTab( m_pPlot, StrConst::BSV_TabNamePlot );

	m_pModel = nullptr;
	m_iPlot = -1;
	m_bAvoidSignal = false;

	InitializeConnections();
}

CPlotsViewer::~CPlotsViewer()
{

}

void CPlotsViewer::InitializeConnections()
{
	connect( ui.horizontalSlider,	SIGNAL(valueChanged(int)),		this, SLOT(SliderPositionChanged(int)) );
	connect( ui.tabWidget,			SIGNAL(currentChanged(int)),	this, SLOT(TabChanged(int)) );
}

void CPlotsViewer::SetSelectedModel(CUnitContainer* _pModel)
{
	m_pModel = _pModel;
	setEnabled(m_pModel && m_pModel->GetModel() && m_pModel->GetModel()->GetPlotsManager().GetPlotsNumber() != 0);
}

void CPlotsViewer::SetSelectedPlot(int _nIndex)
{
	if (!m_pModel || !m_pModel->GetModel()) return;
	const auto plots = m_pModel->GetModel()->GetPlotsManager().GetAllPlots();
	if( ( _nIndex >= 0 ) && ( _nIndex < (int)plots.size() ) )
	{
		m_iPlot = _nIndex;
		if (plots[_nIndex]->Is2D())
		{
			ui.horizontalSlider->setVisible(false);
			ui.labelZValue->setVisible(false);
			ui.labelZName->setVisible(false);
		}
		else
		{
			ui.horizontalSlider->setVisible(true);
			ui.labelZValue->setVisible(true);
			ui.labelZName->setVisible(true);
		}
	}
	else
		m_iPlot = -1;

	m_iCurves.clear();
	UpdateWholeView();
}

void CPlotsViewer::SetSelectedCurve(int _nIndex)
{
	std::vector<int> ind(1, _nIndex);
	SetSelectedCurve(ind);
}

void CPlotsViewer::SetSelectedCurve(const std::vector<int>& _vIndexes)
{
	m_iCurves = _vIndexes;
	UpdateSliderPosFromSelectedIndex();
	UpdateTabWidget();
}

void CPlotsViewer::UpdateSlider()
{
	if (!m_pModel || !m_pModel->GetModel()) return;
	if(m_iPlot == -1) return;
	const auto plots = m_pModel->GetModel()->GetPlotsManager().GetAllPlots();
	if (m_iPlot >= static_cast<int>(plots.size()) || plots[m_iPlot]->Is2D()) return;

	ui.horizontalSlider->setMaximum(static_cast<int>(plots[m_iPlot]->GetCurvesNumber()) - 1);
	ui.horizontalSlider->setTickInterval( 1 );
}

void CPlotsViewer::UpdateSliderLabel()
{
	ui.labelZValue->setText( "" );

	if (!m_pModel || !m_pModel->GetModel()) return;
	if(m_iPlot == -1) return;
	const auto plots = m_pModel->GetModel()->GetPlotsManager().GetAllPlots();
	if (plots[m_iPlot]->Is2D())	return;
	if(m_iCurves.empty()) return;

	//int index = ui.horizontalSlider->sliderPosition();
	ui.labelZValue->setText(QString::number(plots[m_iPlot]->GetAllCurves()[m_iCurves.back()]->GetZValue()));
}

void CPlotsViewer::UpdateSliderLabelName()
{
	ui.labelZName->setText( "" );

	if (!m_pModel || !m_pModel->GetModel()) return;
	if(m_iPlot == -1) return;
	const auto plots = m_pModel->GetModel()->GetPlotsManager().GetAllPlots();
	if (m_iPlot >= static_cast<int>(plots.size()) || plots[m_iPlot]->Is2D())	return;

	ui.labelZName->setText( QString::fromStdString(plots[m_iPlot]->GetLabelZ()) + ":" );
}

void CPlotsViewer::UpdateSliderPosFromSelectedIndex()
{
	if(!m_pModel) return;
	//if(m_iPlot == -1) return;
	//if(m_pModel->IsPlot2D(m_iPlot))	return;

	//m_bAvoidSignal = true;
	//if(m_iCurves.empty())
	//	ui.horizontalSlider->setSliderPosition(0);
	//else
	//	ui.horizontalSlider->setSliderPosition(m_iCurves.back());
	//m_bAvoidSignal = false;

	m_bAvoidSignal = true;
	const auto plots = m_pModel->GetModel()->GetPlotsManager().GetAllPlots();
	if( m_pModel && m_pModel->GetModel() && m_iPlot != -1 && m_iPlot < static_cast<int>(plots.size()) && !plots[m_iPlot]->Is2D() && !m_iCurves.empty() )
		ui.horizontalSlider->setSliderPosition(m_iCurves.back());
	else
		ui.horizontalSlider->setSliderPosition(0);
	m_bAvoidSignal = false;
}

void CPlotsViewer::UpdateTabWidget()
{
	switch( ui.tabWidget->currentIndex() )
	{
	case 0:
		UpdateTable();
		break;
	case 1:
		UpdatePlot();
		break;
	default: break;
	}
}

void CPlotsViewer::UpdateTable()
{
	if((!m_pModel) || !m_pModel->GetModel() || (m_iPlot == -1) || (m_iCurves.empty()) )
	{
		m_pTableWidget->setRowCount(0);
		m_pTableWidget->setColumnCount(0);
		return;
	}

	m_pTableWidget->setRowCount(0);
	m_pTableWidget->setColumnCount((int)m_iCurves.size()*3-1 );
	const auto plots = m_pModel->GetModel()->GetPlotsManager().GetAllPlots();
	const auto curves = plots[m_iPlot]->GetAllCurves();
	for( unsigned i=0; i<m_iCurves.size(); ++i )
	{
		m_pTableWidget->setHorizontalHeaderItem( i*3, new QTableWidgetItem( QString::fromStdString(plots[m_iPlot]->GetLabelX()) ) );
		m_pTableWidget->setHorizontalHeaderItem( i*3+1, new QTableWidgetItem( QString::fromStdString(plots[m_iPlot]->GetLabelY()) ) );
		if(i != m_iCurves.size()-1)
			m_pTableWidget->setHorizontalHeaderItem( i*3+2, new QTableWidgetItem("") );
		std::vector<double> vX = curves[m_iCurves[i]]->GetXValues();
		std::vector<double> vY = curves[m_iCurves[i]]->GetYValues();
		for( int j=0; j<(int)vX.size(); ++j )
		{
			if( m_pTableWidget->rowCount() <= j )
				m_pTableWidget->insertRow(j);
			m_pTableWidget->setItem( j, i*3, new QTableWidgetItem( QString::number( vX[j] ) ));
			m_pTableWidget->setItem( j, i*3+1, new QTableWidgetItem( QString::number( vY[j] ) ));
		}
	}
	m_pTableWidget->resizeColumnsToContents();
}

void CPlotsViewer::UpdatePlot()
{
	if((!m_pModel) || !m_pModel->GetModel() || (m_iPlot == -1) || (m_iCurves.empty()) )
	{
		m_pPlot->ClearPlot();
		return;
	}

	const auto plots = m_pModel->GetModel()->GetPlotsManager().GetAllPlots();
	const auto curves = plots[m_iPlot]->GetAllCurves();
	m_pPlot->ClearPlot();
	m_pPlot->SetManualLabelsNames(QString::fromStdString(plots[m_iPlot]->GetLabelX()), QString::fromStdString(plots[m_iPlot]->GetLabelY()));
	std::vector<double> vdTemp;
	std::vector<unsigned> vCurves;
	for( unsigned i=0; i<m_iCurves.size(); ++i )
	{
		QColor color = Qt::GlobalColor(Qt::red + i%(Qt::transparent - Qt::red));
		QtPlot::SCurve *pCurve = new QtPlot::SCurve( QString::fromStdString(curves[m_iCurves[i]]->GetName()), color, PLOT_LINE_WIDTH, true, true, QtPlot::LABEL_MANUAL, QtPlot::LABEL_MANUAL );
		vCurves.push_back( m_pPlot->AddCurve(pCurve) );
	}
	for( unsigned i=0; i<m_iCurves.size(); ++i )
	{
		std::vector<double> vX = curves[m_iCurves[i]]->GetXValues();
		std::vector<double> vY = curves[m_iCurves[i]]->GetYValues();
		m_pPlot->AddPoints( vCurves[i], vX, vY );
	}
}

void CPlotsViewer::UpdateWholeView()
{
	UpdateSlider();
	UpdateSliderLabelName();
	UpdateTabWidget();
}

void CPlotsViewer::setVisible(bool _bVisible)
{
	if( ( _bVisible ) && ( !this->isVisible() ) )
		UpdateWholeView();
	QWidget::setVisible( _bVisible );
}

void CPlotsViewer::SliderPositionChanged(int _nIndex)
{
	if(m_bAvoidSignal) return;
	int index = ui.horizontalSlider->sliderPosition();
	m_iCurves.clear();
	m_iCurves.push_back(index);
	UpdateSliderLabel();
	UpdateTabWidget();
}

void CPlotsViewer::TabChanged(int _nIndex)
{
	UpdateTabWidget();
}
