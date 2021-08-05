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

	int nDistr = m_pCombo->itemData( m_pCombo->currentIndex() ).toInt();
	size_t nMax;
	if (nDistr != -1)
		nMax = m_pGrid->GetGridDimension(static_cast<EDistrTypes>(nDistrIndex))->ClassesNumber();
	else
		nMax = 0;
	//unsigned nMax = m_pGrid->GetClassesNumberByIndex( static_cast<unsigned>(nDistrIndex) );

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

	//int nDistrIndex = GetDistrIndex();
	int nDistr = m_pCombo->itemData( m_pCombo->currentIndex() ).toInt();

	if( nDistr == -1 ) // clear
	{
		m_pLabel->setText( "" );
		return;
	}

	int nCurrIndex = m_pSlider->sliderPosition();
	EGridEntry nState = m_pGrid->GetGridDimension(static_cast<EDistrTypes>(nDistr))->GridType();

	switch( nState )
	{
	case EGridEntry::GRID_NUMERIC:
		vNumGrid = m_pGrid->GetGridDimensionNumeric(static_cast<EDistrTypes>(nDistr))->Grid();
		sLabelText = "[" + QString::number( vNumGrid[nCurrIndex] ) + " : " + QString::number( vNumGrid[nCurrIndex+1] ) + "]";
		break;
	case EGridEntry::GRID_SYMBOLIC:
		vStrGrid = m_pGrid->GetGridDimensionSymbolic(static_cast<EDistrTypes>(nDistr))->Grid();
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
