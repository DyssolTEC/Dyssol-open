/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "PlotsViewer.h"
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

void CPlotsViewer::SetSelectedModel( CBaseModel *_pModel )
{
	m_pModel = _pModel;
	setEnabled(m_pModel && m_pModel->GetPlotsNumber() != 0);
}

void CPlotsViewer::SetSelectedPlot(int _nIndex)
{
	if(!m_pModel) return;
	if( ( _nIndex >= 0 ) && ( _nIndex < (int)m_pModel->GetPlotsNumber() ) )
	{
		m_iPlot = _nIndex;
		if( m_pModel->IsPlot2D(_nIndex) )
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
	if(!m_pModel) return;
	if(m_iPlot == -1) return;
	if(m_pModel->IsPlot2D(m_iPlot))	return;

	ui.horizontalSlider->setMaximum( m_pModel->GetCurvesNumber(m_iPlot) - 1 );
	ui.horizontalSlider->setTickInterval( 1 );
}

void CPlotsViewer::UpdateSliderLabel()
{
	ui.labelZValue->setText( "" );

	if(!m_pModel) return;
	if(m_iPlot == -1) return;
	if(m_pModel->IsPlot2D(m_iPlot))	return;
	if(m_iCurves.empty()) return;

	//int index = ui.horizontalSlider->sliderPosition();
	ui.labelZValue->setText( QString::number( m_pModel->GetCurveZ(m_iPlot, m_iCurves.back()) ) );
}

void CPlotsViewer::UpdateSliderLabelName()
{
	ui.labelZName->setText( "" );

	if(!m_pModel) return;
	if(m_iPlot == -1) return;
	if(m_pModel->IsPlot2D(m_iPlot))	return;

	ui.labelZName->setText( QString::fromStdString(m_pModel->GetPlotZAxisName(m_iPlot)) + ":" );
}

void CPlotsViewer::UpdateSliderPosFromSelectedIndex()
{
	//if(!m_pModel) return;
	//if(m_iPlot == -1) return;
	//if(m_pModel->IsPlot2D(m_iPlot))	return;

	//m_bAvoidSignal = true;
	//if(m_iCurves.empty())
	//	ui.horizontalSlider->setSliderPosition(0);
	//else
	//	ui.horizontalSlider->setSliderPosition(m_iCurves.back());
	//m_bAvoidSignal = false;

	m_bAvoidSignal = true;
	if( (m_pModel) && (m_iPlot != -1) && !m_pModel->IsPlot2D(m_iPlot) && !m_iCurves.empty() )
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
	if((!m_pModel) || (m_iPlot == -1) || (m_iCurves.empty()) )
	{
		m_pTableWidget->setRowCount(0);
		m_pTableWidget->setColumnCount(0);
		return;
	}

	m_pTableWidget->setRowCount(0);
	m_pTableWidget->setColumnCount((int)m_iCurves.size()*3-1 );
	for( unsigned i=0; i<m_iCurves.size(); ++i )
	{
		m_pTableWidget->setHorizontalHeaderItem( i*3, new QTableWidgetItem( QString::fromStdString(m_pModel->GetPlotXAxisName(m_iPlot)) ) );
		m_pTableWidget->setHorizontalHeaderItem( i*3+1, new QTableWidgetItem( QString::fromStdString(m_pModel->GetPlotYAxisName(m_iPlot)) ) );
		if(i != m_iCurves.size()-1)
			m_pTableWidget->setHorizontalHeaderItem( i*3+2, new QTableWidgetItem("") );
		std::vector<double> vX = m_pModel->GetCurveX( m_iPlot, m_iCurves[i] );
		std::vector<double> vY = m_pModel->GetCurveY( m_iPlot, m_iCurves[i] );
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
	if((!m_pModel) || (m_iPlot == -1) || (m_iCurves.empty()) )
	{
		m_pPlot->ClearPlot();
		return;
	}

	m_pPlot->ClearPlot();
	m_pPlot->SetManualLabelsNames(QString::fromStdString(m_pModel->GetPlotXAxisName(m_iPlot)), QString::fromStdString(m_pModel->GetPlotYAxisName(m_iPlot)));
	std::vector<double> vdTemp;
	std::vector<unsigned> vCurves;
	for( unsigned i=0; i<m_iCurves.size(); ++i )
	{
		QColor color = Qt::GlobalColor(Qt::red + i%(Qt::transparent - Qt::red));
		QtPlot::SCurve *pCurve = new QtPlot::SCurve( QString::fromStdString(m_pModel->GetCurveName(m_iPlot, m_iCurves[i])), color, PLOT_LINE_WIDTH, true, true, QtPlot::LABEL_MANUAL, QtPlot::LABEL_MANUAL );
		vCurves.push_back( m_pPlot->AddCurve(pCurve) );
	}
	for( unsigned i=0; i<m_iCurves.size(); ++i )
	{
		std::vector<double> vX = m_pModel->GetCurveX( m_iPlot, m_iCurves[i] );
		std::vector<double> vY = m_pModel->GetCurveY( m_iPlot, m_iCurves[i] );
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
