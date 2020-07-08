/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "PhasesEditor.h"
#include <QMessageBox>

CPhasesEditor::CPhasesEditor( CFlowsheet* _pFlowsheet, QWidget *parent, Qt::WindowFlags flags )
	: QDialog(parent, flags)
{
	ui.setupUi(this);

	m_pFlowsheet = _pFlowsheet;
}

CPhasesEditor::~CPhasesEditor()
{

}

void CPhasesEditor::InitializeConnections()
{
	QObject::connect( ui.pushButtonAdd, SIGNAL( clicked() ), this, SLOT( AddPhase() ));
	QObject::connect( ui.pushButtonRemove, SIGNAL( clicked() ), this, SLOT( RemovePhase() ));
	QObject::connect( ui.pushButtonCancel, SIGNAL( clicked() ), this, SLOT( reject() ));
	QObject::connect( ui.pushButtonOk, SIGNAL( clicked() ), this, SLOT( accept() ));
}

bool CPhasesEditor::ValidateInput()
{
	for( int i=0; i<ui.tableWidget->rowCount()-1; ++i )
	{
		for( int j=i+1; j<ui.tableWidget->rowCount(); ++j )
		{
			if( ui.tableWidget->item( i, 0 )->text() == ui.tableWidget->item( j, 0 )->text() )
			{
				QMessageBox::warning( this, tr("Error"), tr("Phases with the same names have been defined!") );
				return false;
			}
		}
	}
	unsigned nSolidCounter = 0;
	unsigned nVaporCounter = 0;
	unsigned nLiquid1Counter = 0;
	unsigned nLiquid2Counter = 0;
	for( unsigned i=0; i<m_vCombos.size(); ++i )
	{
		switch( m_vCombos[i]->currentIndex() )
		{
		case SOA_SOLID:
			nSolidCounter++;
			break;
		case SOA_VAPOR:
			nVaporCounter++;
			break;
		case SOA_LIQUID:
			nLiquid1Counter++;
			break;
		case SOA_LIQUID2:
			nLiquid2Counter++;
			break;
		}
	}

	if( nSolidCounter > 1 )
	{
		QMessageBox::warning( this, tr("Error"), tr("More than one solid phase has been defined!") );
		return false;
	}
	if( nVaporCounter > 1 )
	{
		QMessageBox::warning( this, tr("Error"), tr("More than one vapor phase has been defined!") );
		return false;
	}
	if( nLiquid1Counter > 1 )
	{
		QMessageBox::warning( this, tr("Error"), tr("More than one liquid phase has been defined!") );
		return false;
	}
	if( nLiquid2Counter > 1 )
	{
		QMessageBox::warning( this, tr("Error"), tr("More than one liquid2 phase has been defined!") );
		return false;
	}

	return true;
}

void CPhasesEditor::UpdateWholeView()
{
	ui.tableWidget->setRowCount(0);
	for( unsigned i=0; i<m_vCombos.size(); ++i )
		delete m_vCombos[i];
	m_vCombos.clear();

	for( unsigned i=0; i<m_pFlowsheet->GetPhasesNumber(); ++i )
	{
		AddPhase();
		ui.tableWidget->item( ui.tableWidget->rowCount()-1, 0 )->setText( QString::fromStdString(m_pFlowsheet->GetPhaseName(i)) );
		m_vCombos[i]->setCurrentIndex( m_pFlowsheet->GetPhaseAggregationState(i) );
	}
}

void CPhasesEditor::setVisible( bool _bVisible )
{
	if( ( _bVisible ) && ( !this->isVisible() ) )
		UpdateWholeView();
	QDialog::setVisible( _bVisible );
}

void CPhasesEditor::accept()
{
	if( ApplyChanges() )
		QDialog::accept();
}

void CPhasesEditor::AddPhase()
{
	ui.tableWidget->setRowCount( ui.tableWidget->rowCount() + 1 );
	int nCurrRow = ui.tableWidget->rowCount() - 1;
	ui.tableWidget->setItem( nCurrRow, 0, new QTableWidgetItem( "Phase" + QString::number(nCurrRow+1) ) );
	QComboBox *pCombo = new QComboBox();
	m_vCombos.push_back( pCombo );
	pCombo->insertItem( SOA_SOLID, "Solid" );
	pCombo->insertItem( SOA_LIQUID, "Liquid" );
	pCombo->insertItem( SOA_LIQUID2, "Liquid2" );
	pCombo->insertItem( SOA_VAPOR, "Vapor" );
	ui.tableWidget->setCellWidget( nCurrRow, 1, pCombo );
}

void CPhasesEditor::RemovePhase()
{
	//QMessageBox::StandardButton reply = QMessageBox::question( this, "Phase removal", "Do you really want to remove this phase?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
	//if( ( reply == QMessageBox::No ) || ( reply == QMessageBox::Cancel ) )
	//	return;

	//unsigned nIndex;
	//if( ui.tableWidget->currentRow() == -1 )
	//	nIndex = ui.tableWidget->rowCount()-1;
	//else
	//	nIndex = ui.tableWidget->currentRow();
	//ui.tableWidget->removeRow( nIndex );
	//delete m_vCombos[nIndex];
	//m_vCombos.erase( m_vCombos.begin() + nIndex );

	int nRow = ui.tableWidget->currentRow();
	if( nRow == -1 )
		return;

	QString sQuestion = QString( "Do you really want to remove phase '%1'?" ).arg( ui.tableWidget->item( nRow, 0 )->text() );
	QMessageBox::StandardButton reply = QMessageBox::question( this, "Phase removal", sQuestion, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
	if( ( reply == QMessageBox::No ) || ( reply == QMessageBox::Cancel ) )
		return;

	ui.tableWidget->removeRow( nRow );
	delete m_vCombos[nRow];
	m_vCombos.erase( m_vCombos.begin() + nRow );
}

bool CPhasesEditor::ApplyChanges()
{
	if( !ValidateInput() )
		return false;

	std::vector<int> vNewPhases;
	// add or rename phases
	for( int i=0; i<ui.tableWidget->rowCount(); ++i )
	{
		std::string sPhaseName = ui.tableWidget->item( i, 0 )->text().toStdString();
		int nPhaseSOA = m_vCombos[i]->currentIndex();
		vNewPhases.push_back( nPhaseSOA );
		if( m_pFlowsheet->IsPhaseDefined( nPhaseSOA ) )
		{
			if( m_pFlowsheet->GetPhaseName( m_pFlowsheet->GetPhaseIndex( nPhaseSOA ) ) != sPhaseName )
				m_pFlowsheet->ChangePhase( m_pFlowsheet->GetPhaseIndex( nPhaseSOA ), sPhaseName, nPhaseSOA );
		}
		else
			m_pFlowsheet->AddPhase( sPhaseName, nPhaseSOA );
	}

	// remove phases
	for( unsigned i=0; i<m_pFlowsheet->GetPhasesNumber(); ++i )
		if( std::find( vNewPhases.begin(), vNewPhases.end(), m_pFlowsheet->GetPhaseAggregationState(i) ) == vNewPhases.end() )
			m_pFlowsheet->RemovePhase(i);

	//m_pFlowsheet->ClearPhases();

	//for( int i=0; i<ui.tableWidget->rowCount(); ++i )
		//m_pFlowsheet->AddPhase( qs2ss(ui.tableWidget->item( i, 0 )->text()), m_vCombos[i]->currentIndex() );

	emit DataChanged();

	return true;
}
