/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "GridLimitsDialog.h"

CGridLimitsDialog::CGridLimitsDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	m_dXMin = m_dXMax = m_dYMin = m_dYMax = NULL;
	m_bLogX = m_bLogY = false;

	QObject::connect( ui.buttonOk, SIGNAL(clicked()), this, SLOT(accept()) );
	QObject::connect( ui.buttonCancel, SIGNAL(clicked()), this, SLOT(close()) );
	QObject::connect( ui.buttonApply, SIGNAL(clicked()), this, SLOT(ApplySlot()) );
}

CGridLimitsDialog::~CGridLimitsDialog()
{

}

void CGridLimitsDialog::SetLabels( QString _sXLabel, QString _sYLabel )
{
	ui.labelXDescr->setText( _sXLabel );
	ui.labelYDescr->setText( _sYLabel );
}

void CGridLimitsDialog::SetLimits( double& _dXMin, double& _dXMax, double& _dYMin, double& _dYmax )
{
	m_dXMin = _dXMin;
	m_dXMax = _dXMax;
	m_dYMin = _dYMin;
	m_dYMax = _dYmax;

	UpdateWholeView();
}

void CGridLimitsDialog::UpdateLogScaling(void)
{
	if ((m_bLogX == NULL) || (m_bLogY == NULL))
		return;
	m_bLogX = ui.checkLogX->isChecked();
	m_bLogY = ui.checkLogY->isChecked();
}

void CGridLimitsDialog::SetLogScaling(bool &_bLogX, bool &_bLogY)
{
	m_bLogX = _bLogX;
	m_bLogY = _bLogY;
	UpdateLogScaling();
}

void CGridLimitsDialog::UpdateWholeView()
{
	if( ( m_dXMin == NULL ) || ( m_dXMax == NULL ) || ( m_dYMin == NULL ) || ( m_dYMax == NULL ) )
		return;

	UpdateLogScaling();

	ui.lineEditXMin->setText( QString::number( m_dXMin ) );
	ui.lineEditXMax->setText( QString::number( m_dXMax ) );
	ui.lineEditYMin->setText( QString::number( m_dYMin ) );
	ui.lineEditYMax->setText( QString::number( m_dYMax ) );
}

void CGridLimitsDialog::ApplyChanges()
{
	m_dXMin = ui.lineEditXMin->text().toDouble();
	m_dXMax = ui.lineEditXMax->text().toDouble();

	m_dYMin = ui.lineEditYMin->text().toDouble();
	m_dYMax = ui.lineEditYMax->text().toDouble();

	UpdateLogScaling();
}

void CGridLimitsDialog::ApplySlot()
{
	ApplyChanges();
	UpdateWholeView();
	emit ApplyPressed();
}

void CGridLimitsDialog::accept()
{
	ApplyChanges();
	emit OkPressed();
	QDialog::accept();
}
