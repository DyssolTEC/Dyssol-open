/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "MDMTable.h"
#include <QHeaderView>

CMDMTable::CMDMTable(QWidget *parent)
	: QWidget(parent)
{
	m_pData = NULL;

	m_pTable = new CQtTable( this );
	layout = new QHBoxLayout;
	layout->addWidget(m_pTable);
	setLayout(layout);

	m_pTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	QObject::connect( m_pTable, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(ItemWasChanged(QTableWidgetItem*)) );
}

CMDMTable::~CMDMTable()
{
	m_pData = NULL;
}

void CMDMTable::SetDistribution( CMDMatrix* _pDistribution, const std::vector<std::string>& _sNames )
{
	m_pData = _pDistribution;

	if( m_pData == NULL )
		return;

	m_pTable->setColumnCount( m_pData->GetDimensionSizeByType( DISTR_COMPOUNDS ) + 1 );
	SetHeaders( _sNames );

	UpdateWholeView();
}

void CMDMTable::SetEditable(bool _bEditable)
{
	m_pTable->SetEditable(_bEditable);
}

void CMDMTable::SetHeaders(const std::vector<std::string>& _sNames)
{
	if( m_pData == NULL ) return;

	auto names = _sNames;

	m_pTable->setHorizontalHeaderItem( 0, new QTableWidgetItem( "Time [s]" ) );
	int nCompoundsNumber = (int)m_pData->GetDimensionSizeByType( DISTR_COMPOUNDS );
	if(names.size() != nCompoundsNumber )
		names.resize( nCompoundsNumber );
	for( int i=0; i<nCompoundsNumber; ++i )
	{
		if( m_pTable->columnCount() < i+2 )
			m_pTable->insertColumn( i+2 );
		m_pTable->setHorizontalHeaderItem( i+1, new QTableWidgetItem( QString::fromUtf8(names[i].c_str() ) ) );
	}
}

void CMDMTable::CheckNormalization()
{
	for( unsigned i=0; i<m_pData->GetTimePointsNumber(); ++i )
	{
		std::vector<double> vTemp;
		m_pData->GetVectorValue( i, DISTR_COMPOUNDS, vTemp );
		double dSum = 0;
		for( unsigned j=0; j<vTemp.size(); ++j )
			dSum += vTemp[j];
		if( dSum != 1 )
			for( int j=0; j<m_pTable->columnCount(); ++j )
				m_pTable->item( i, j )->setBackground( Qt::lightGray );
		else
			for( int j=0; j<m_pTable->columnCount(); ++j )
				m_pTable->item( i, j )->setBackground( Qt::white );
	}
}

void CMDMTable::UpdateWholeView()
{
	if( m_pData == NULL ) return;

	m_bAvoidSignal = true;

	std::vector<double> vTimes = m_pData->GetAllTimePoints();
	for( int iRow=0; iRow<(int)vTimes.size(); ++iRow )
	{
		if( iRow >= m_pTable->rowCount() )
			m_pTable->insertRow(iRow);
		std::vector<double> vTemp;
		m_pData->GetVectorValue( (unsigned)iRow, DISTR_COMPOUNDS, vTemp );
		m_pTable->setItem( iRow, 0, new QTableWidgetItem( QString::number( vTimes[iRow] ) ) );
		m_pTable->item( iRow, 0 )->setFlags( m_pTable->item( iRow, 0 )->flags() & ~Qt::ItemIsEditable );
		for( unsigned i=0; i<vTemp.size(); ++i )
			m_pTable->setItem( iRow, i+1, new QTableWidgetItem( QString::number( vTemp[i] ) ));
	}
	while( m_pTable->rowCount() > (int)vTimes.size() )
		m_pTable->removeRow( m_pTable->rowCount()-1 );

	CheckNormalization();

	m_bAvoidSignal = false;
}

//void CMDMTable::setVisible( bool _bVisible )
//{
//	if ( _bVisible )
//		UpdateWholeView();
//	QWidget::setVisible( _bVisible );
//}

void CMDMTable::ItemWasChanged( QTableWidgetItem* _pItem )
{
	if( m_bAvoidSignal ) return;

	unsigned nTimeIndex = _pItem->row();
	unsigned nCoord = _pItem->column() - 1;
	double dVal = _pItem->text().toDouble();
	if( dVal < 0 ) dVal = 0;
	m_pData->SetValue( nTimeIndex, DISTR_COMPOUNDS, nCoord, dVal );

	UpdateWholeView();

	emit DataChanged();
}
