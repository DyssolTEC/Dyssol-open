/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "PhasesEditor.h"
#include "Flowsheet.h"
#include "ContainerFunctions.h"
#include "DyssolUtilities.h"
#include <QMessageBox>

CPhasesEditor::CPhasesEditor(CFlowsheet* _pFlowsheet, QWidget* _parent, Qt::WindowFlags flags)
	: CQtDialog{ _parent, flags }
{
	ui.setupUi(this);

	m_pFlowsheet = _pFlowsheet;

	SetHelpLink("001_ui/gui.html#sec-gui-menu-setup-phases");
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

void CPhasesEditor::UpdateFromFlowsheet()
{
	if (isVisible())
		UpdateWholeView();
}

void CPhasesEditor::UpdateWholeView()
{
	ui.tableWidget->setRowCount(0);
	for( unsigned i=0; i<m_vCombos.size(); ++i )
		delete m_vCombos[i];
	m_vCombos.clear();

	size_t index = 0;
	for (const auto& phase : m_pFlowsheet->GetPhases())
	{
		AddPhase();
		ui.tableWidget->item( ui.tableWidget->rowCount()-1, 0 )->setText( QString::fromStdString(phase.name) );
		m_vCombos[index++]->setCurrentIndex(E2I(phase.state) - 1);
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
	pCombo->insertItem(E2I(EPhase::SOLID), "Solid" );
	pCombo->insertItem(E2I(EPhase::LIQUID), "Liquid" );
	pCombo->insertItem(E2I(EPhase::VAPOR), "Vapor" );
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

	std::vector<EPhase> vNewPhases;
	// add or rename phases
	for( int i=0; i<ui.tableWidget->rowCount(); ++i )
	{
		std::string sPhaseName = ui.tableWidget->item( i, 0 )->text().toStdString();
		auto state = static_cast<EPhase>(m_vCombos[i]->currentIndex() + 1);
		vNewPhases.push_back(state);
		if (m_pFlowsheet->HasPhase(state))
		{
			const auto& phases = m_pFlowsheet->GetPhases();
			const size_t index = VectorFind(phases, [&](const auto& p) { return p.state == state; });
			if (phases[index].name != sPhaseName)
			{
				m_pFlowsheet->RemovePhase(state);
				m_pFlowsheet->AddPhase(state, sPhaseName);
			}
		}
		else
			m_pFlowsheet->AddPhase(state, sPhaseName);
	}

	// remove phases
	for (const auto& phase : m_pFlowsheet->GetPhases())
		if (!VectorContains(vNewPhases, phase.state))
			m_pFlowsheet->RemovePhase(phase.state);

	//m_pFlowsheet->ClearPhases();

	//for( int i=0; i<ui.tableWidget->rowCount(); ++i )
		//m_pFlowsheet->AddPhase( qs2ss(ui.tableWidget->item( i, 0 )->text()), m_vCombos[i]->currentIndex() );

	emit DataChanged();

	return true;
}
