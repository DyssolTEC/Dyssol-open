/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "CompoundsManager.h"
#include "MaterialsDatabase.h"
#include "Flowsheet.h"

CCompoundsManager::CCompoundsManager(CFlowsheet* _pFlowsheet, CMaterialsDatabase* _pDatabase, QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
	m_pFlowsheet = _pFlowsheet;
	m_pMaterialsDatabase = _pDatabase;
}

void CCompoundsManager::InitializeConnections()
{
	// signals from buttons
	connect(ui.pushButtonOk,			&QPushButton::clicked,				this, &CCompoundsManager::accept);
	connect(ui.pushButtonCancel,		&QPushButton::clicked,				this, &CCompoundsManager::close);
	connect(ui.addSelectedCompound,		&QPushButton::clicked,				this, &CCompoundsManager::AddSelectedCompound);
	connect(ui.removeSelectedCompound,	&QPushButton::clicked,				this, &CCompoundsManager::RemoveSelectedCompound);
	connect(ui.availableCompounds,		&QTableWidget::itemDoubleClicked,	this, &CCompoundsManager::AddSelectedCompound);
	connect(ui.usedCompounds,			&QTableWidget::itemDoubleClicked,	this, &CCompoundsManager::RemoveSelectedCompound);
}

void CCompoundsManager::UpdateWholeView()
{
	m_vAddedCompKeys = m_pFlowsheet->GetCompounds();
	UpdateAvailableCompounds();
	UpdateUsedCompounds();
}

void CCompoundsManager::UpdateAvailableCompounds()
{
	ui.availableCompounds->clear();
	ui.availableCompounds->setColumnCount(1);
	ui.availableCompounds->setRowCount((int)m_pMaterialsDatabase->CompoundsNumber());
	for (int i = 0; i < (int)m_pMaterialsDatabase->CompoundsNumber(); ++i)
		ui.availableCompounds->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(m_pMaterialsDatabase->GetCompound(i)->GetName())));
}

void CCompoundsManager::UpdateUsedCompounds()
{
	ui.usedCompounds->clear();
	ui.usedCompounds->setColumnCount(1);
	ui.usedCompounds->setRowCount((int)m_vAddedCompKeys.size());
	for (int i = 0; i < (int)m_vAddedCompKeys.size(); ++i)
	{
		const CCompound* pCompound = m_pMaterialsDatabase->GetCompound( m_vAddedCompKeys[i] );
		if (pCompound)
			ui.usedCompounds->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(pCompound->GetName())));
		else
			ui.usedCompounds->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(m_vAddedCompKeys[i])));
	}
}

void CCompoundsManager::setVisible( bool _bVisible )
{
	if ( _bVisible )
		UpdateWholeView();
	QDialog::setVisible( _bVisible );
}

void CCompoundsManager::AddSelectedCompound()
{
	if ( ui.availableCompounds->currentRow() < 0 ) return;

	const CCompound* pCompound = m_pMaterialsDatabase->GetCompound(ui.availableCompounds->currentRow());
	if (!pCompound) return;

	int nIndex = GetAddedCompIndex( pCompound->GetKey() );
	if( nIndex != -1 ) return; // already exists

	m_vAddedCompKeys.push_back( pCompound->GetKey() );
	UpdateUsedCompounds();
}

void CCompoundsManager::RemoveSelectedCompound()
{
	if ((ui.usedCompounds->currentRow() < 0) || (ui.usedCompounds->currentRow() >= (int)m_vAddedCompKeys.size())) return;

	m_vAddedCompKeys.erase( m_vAddedCompKeys.begin() + ui.usedCompounds->currentRow() );
	UpdateUsedCompounds();
}

void CCompoundsManager::accept()
{
	ApplyChanges();
	QDialog::accept();
}

void CCompoundsManager::close()
{
	m_vAddedCompKeys.clear();
	QDialog::close();
}

void CCompoundsManager::ApplyChanges()
{
	size_t nOldNumber = m_pFlowsheet->GetCompoundsNumber();
	size_t nNewNumber = m_vAddedCompKeys.size();

	std::vector<std::string> vOldCompounds = m_pFlowsheet->GetCompounds();
	std::vector<std::string> vNewCompounds = m_vAddedCompKeys;

	// remove compounds
	for (size_t i = 0; i < nOldNumber; ++i)
		if (GetAddedCompIndex(vOldCompounds[i]) == -1)
			m_pFlowsheet->RemoveCompound(vOldCompounds[i]);

	// add compounds
	for (size_t i = 0; i < nNewNumber; ++i)
		m_pFlowsheet->AddCompound(vNewCompounds[i]);

	emit DataChanged();
}

int CCompoundsManager::GetAddedCompIndex( const std::string& _sKey ) const
{
	int nRes = -1;
	for( unsigned i=0; i<m_vAddedCompKeys.size(); ++i )
	{
		if( _sKey == m_vAddedCompKeys[i] )
		{
			nRes = i;
			break;
		}
	}
	return nRes;
}
