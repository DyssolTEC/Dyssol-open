/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ParamsItem.h"
#include "MultidimensionalGrid.h"

CParamsItem::CParamsItem(const CMultidimensionalGrid *_pGrid, QComboBox *_pCombo, QSlider *_pSlider, QWidget *parent, Qt::WindowFlags flags )
	: QWidget( parent, flags )
{
	m_pGrid = _pGrid;
	m_pCombo = _pCombo;
	m_pSlider = _pSlider;
	m_pSlider->setMaximum(0);
	m_pSlider->setTickPosition( QSlider::TicksBelow );
	m_pLayout = new QHBoxLayout();
	m_pLabel = new QLabel("0");
	m_pLayout->addWidget( m_pSlider );
	m_pLayout->addWidget( m_pLabel );
	m_pLayout->addWidget( m_pCombo );
	setLayout(m_pLayout);

	QObject::connect( m_pCombo, SIGNAL( currentIndexChanged(int) ), this, SLOT( DistrTypeChanged() ));
	QObject::connect( m_pSlider, SIGNAL( valueChanged(int) ), this, SLOT( SliderValueChanged() ));
}

int CParamsItem::GetDistrIndex()
{
	return m_pCombo->itemData( m_pCombo->currentIndex() ).toInt();
}

void CParamsItem::SetupSlider()
{
	// get distribution index
	int nDistrIndex = GetDistrIndex();
	if( nDistrIndex == -1 ) // clear
	{
		m_pSlider->setMaximum( 0 );
		m_pSlider->setSliderPosition( -1 );
		return;
	}

	const int distr = m_pCombo->itemData(m_pCombo->currentIndex()).toInt();
	const auto* gridDim = m_pGrid->GetGridDimension(static_cast<EDistrTypes>(distr));
	const size_t nMax = gridDim ? gridDim->ClassesNumber() : 0;

	int nLastPos = m_pSlider->sliderPosition();
	if( nMax != 0 )
		m_pSlider->setMaximum( (int)nMax-1 );
	else
		m_pSlider->setMaximum( 0 );
	if( nMax > 100 )
		m_pSlider->setTickInterval( (int)(nMax/100) );
	else
		m_pSlider->setTickInterval( 1 );
	if( nLastPos < m_pSlider->maximum() )
		m_pSlider->setSliderPosition( nLastPos );
	else
		m_pSlider->setSliderPosition( 0 );
}

void CParamsItem::UpdateLabel()
{
	QString sLabelText;
	std::vector<double> vNumGrid;
	std::vector<std::string> vStrGrid;

	const int distr = m_pCombo->itemData(m_pCombo->currentIndex()).toInt();
	const auto* gridDim = m_pGrid->GetGridDimension(static_cast<EDistrTypes>(distr));

	if (!gridDim) // clear
	{
		m_pLabel->setText( "" );
		return;
	}

	int nCurrIndex = m_pSlider->sliderPosition();
	const EGridEntry gridEntry = gridDim->GridType();

	switch( gridEntry )
	{
	case EGridEntry::GRID_NUMERIC:
		vNumGrid = dynamic_cast<const CGridDimensionNumeric*>(gridDim)->Grid();
		sLabelText = "[" + QString::number( vNumGrid[nCurrIndex] ) + " : " + QString::number( vNumGrid[nCurrIndex+1] ) + "]";
		break;
	case EGridEntry::GRID_SYMBOLIC:
		vStrGrid = dynamic_cast<const CGridDimensionSymbolic*>(gridDim)->Grid();
		sLabelText = QString::fromStdString(vStrGrid[nCurrIndex]);
		break;
	default:
		break;
	}

	m_pLabel->setText( sLabelText );
}

void CParamsItem::UpdateWholeView()
{
	SetupSlider();
	UpdateLabel();
}

void CParamsItem::DistrTypeChanged()
{
	UpdateWholeView();
}

void CParamsItem::SliderValueChanged()
{
	UpdateLabel();
}
