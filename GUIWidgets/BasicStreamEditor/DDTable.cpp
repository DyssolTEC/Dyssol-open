/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DDTable.h"
#include <QHeaderView>

CDDTable::CDDTable( QWidget *parent, Qt::WindowFlags flags ) : QWidget(parent, flags)
{
	m_pData = NULL;
	m_bNormalize = false;

	m_pTable = new CQtTable( this );
	layout = new QHBoxLayout;
	layout->addWidget(m_pTable);
	setLayout(layout);

	m_pTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	QObject::connect( m_pTable, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(ItemWasChanged(QTableWidgetItem*)) );
}

CDDTable::~CDDTable()
{
	m_pData = NULL;
}

void CDDTable::SetDistribution(CTimeDependentValue* _pDistribution )
{
	m_pData = _pDistribution;

	m_pTable->setColumnCount( 1 + 1 );
	SetHeaders();

	UpdateWholeView();
}

void CDDTable::SetNormalizationCheck( bool _bAnalyse )
{
	m_bNormalize = _bAnalyse;
	UpdateWholeView();
}

void CDDTable::SetEditable(bool _bEditable)
{
	m_pTable->SetEditable(_bEditable);
}

void CDDTable::SetHeaders()
{
	if( m_pData == NULL ) return;

	m_pTable->setHorizontalHeaderItem( 0, new QTableWidgetItem("Time [s]") );
	for( int i=0; i<1; ++i )
	{
		if( m_pTable->columnCount() < i+2 )
			m_pTable->insertColumn( i+2 );
		m_pTable->setHorizontalHeaderItem( i+1, new QTableWidgetItem( QString::fromUtf8( (m_pData->GetName() + " ["  + m_pData->GetUnits() + "]").c_str()) ) );
	}
}

void CDDTable::CheckNormalization()
{
	const auto data = m_pData->GetRawData();
	for( unsigned i=0; i< data.front().size(); ++i )
	{
		std::vector<double> vTemp = std::vector<double>(1, data[1][i]);
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

void CDDTable::UpdateWholeView()
{
	if( m_pData == NULL ) return;

	m_bAvoidSignal = true;

	const auto data = m_pData->GetRawData();
	int iRow;
	for( iRow=0; iRow< data.front().size(); ++iRow )
	{
		if( iRow >= m_pTable->rowCount() )
			m_pTable->insertRow(iRow);
		std::vector<double> vTemp = std::vector<double>(1, data[1][iRow]);
		m_pTable->setItem( iRow, 0, new QTableWidgetItem( QString::number(data[0][iRow]) ));
		m_pTable->item( iRow, 0 )->setFlags( m_pTable->item( iRow, 0 )->flags() & ~Qt::ItemIsEditable );
		for( unsigned i=0; i<vTemp.size(); ++i )
			m_pTable->setItem( iRow, i+1, new QTableWidgetItem( QString::number( vTemp[i] ) ));
	}
	while( m_pTable->rowCount() > (int)m_pData->GetTimePointsNumber())
		m_pTable->removeRow( m_pTable->rowCount()-1 );

	if( m_bNormalize )
		CheckNormalization();

	m_bAvoidSignal = false;
}

//void CDDTable::setVisible( bool _bVisible )
//{
//	if ( _bVisible )
//		UpdateWholeView();
//	QWidget::setVisible( _bVisible );
//}

void CDDTable::ItemWasChanged( QTableWidgetItem* _pItem )
{
	if( m_bAvoidSignal ) return;

	unsigned nTimeIndex = _pItem->row();
	double time = m_pTable->item(nTimeIndex, 0)->text().toDouble();
	unsigned nDimIndex = _pItem->column() - 1;
	double dVal = _pItem->text().toDouble();
	if( dVal < 0 ) dVal = 0;
	m_pData->SetValue(time, dVal );

	UpdateWholeView();

	emit DataChanged();
}
