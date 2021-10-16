/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DDTable.h"
#include "ContainerFunctions.h"
#include <QHeaderView>

CDDTable::CDDTable( QWidget *parent, Qt::WindowFlags flags ) : QWidget(parent, flags)
{
	m_bNormalize = false;

	m_pTable = new CQtTable( this );
	layout = new QHBoxLayout;
	layout->addWidget(m_pTable);
	setLayout(layout);

	m_pTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	QObject::connect( m_pTable, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(ItemWasChanged(QTableWidgetItem*)) );
	connect(m_pTable, &CQtTable::DataPasted, this, &CDDTable::DataPasted);
}

CDDTable::~CDDTable()
{
	m_pData.clear();
}

void CDDTable::SetDistribution(const std::vector<CTimeDependentValue*>& _values)
{
	m_pData = _values;

	m_pTable->setColumnCount(static_cast<int>(m_pData.size()) + 1 );
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
	if (m_pData.empty()) return;

	m_pTable->setHorizontalHeaderItem( 0, new QTableWidgetItem("Time [s]") );
	for (int i = 0; i < static_cast<int>(m_pData.size()); ++i)
	{
		if( m_pTable->columnCount() < i+2 )
			m_pTable->insertColumn( i+2 );
		m_pTable->setHorizontalHeaderItem( i+1, new QTableWidgetItem( QString::fromUtf8( (m_pData[i]->GetName() + " ["  + m_pData[i]->GetUnits() + "]").c_str()) ) );
	}
}

void CDDTable::CheckNormalization()
{
	if (m_pData.empty()) return;

	std::vector<std::vector<std::vector<double>>> data;
	for (auto& value : m_pData)
		data.push_back(value->GetRawData());

	for (size_t i = 0; i < m_pData.front()->GetTimePointsNumber(); ++i)
	{
		std::vector<double> vTemp;
		for (size_t j = 0; j < m_pData.size(); ++j)
			vTemp.push_back(data[j][1][i]);

		double dSum = 0;
		for( unsigned j=0; j<vTemp.size(); ++j )
			dSum += vTemp[j];
		if( dSum != 1 )
			for( int j=0; j<m_pTable->columnCount(); ++j )
				m_pTable->item( static_cast<int>(i), j )->setBackground( Qt::lightGray );
		else
			for( int j=0; j<m_pTable->columnCount(); ++j )
				m_pTable->item( static_cast<int>(i), j )->setBackground( Qt::white );
	}
}

void CDDTable::UpdateWholeView()
{
	if (m_pData.empty()) return;

	m_bAvoidSignal = true;

	std::vector<std::vector<std::vector<double>>> data;
	for (auto& value : m_pData)
		data.push_back(value->GetRawData());

	for (int iRow = 0; iRow < static_cast<int>(data.front()[0].size()); ++iRow)
	{
		if( iRow >= m_pTable->rowCount() )
			m_pTable->insertRow(iRow);

		std::vector<double> vTemp;
		for (size_t j = 0; j < m_pData.size(); ++j)
			vTemp.push_back(data[j][1][iRow]);
		m_pTable->SetItemNotEditable(iRow, 0, data.front()[0][iRow]);
		for (unsigned i = 0; i < vTemp.size(); ++i)
			m_pTable->SetItemEditable(iRow, i + 1, vTemp[i]);
	}
	while( m_pTable->rowCount() > (int)m_pData.front()->GetTimePointsNumber())
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
	const double time = m_pTable->item(nTimeIndex, 0)->text().toDouble();
	unsigned nDimIndex = _pItem->column() - 1;
	double dVal = _pItem->text().toDouble();
	if( dVal < 0 ) dVal = 0;
	m_pData[nDimIndex]->SetValue(time, dVal);

	UpdateWholeView();

	emit DataChanged();
}

void CDDTable::DataPasted()
{
	if (m_bAvoidSignal) return;

	for (int i = 0; i < m_pTable->rowCount(); ++i)
		for (int j = 1; j < m_pTable->columnCount(); ++j)
		{
			const double time = m_pTable->item(i, 0)->text().toDouble();
			const int iDim = j - 1;
			const double val = m_pTable->item(i, j)->text().toDouble();
			m_pData[iDim]->SetValue(time, val);
		}

	UpdateWholeView();
	emit DataChanged();
}