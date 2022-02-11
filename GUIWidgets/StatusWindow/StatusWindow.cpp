/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "StatusWindow.h"
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileInfo>

CStatusWindow::CStatusWindow(QWidget *parent)
	: QDialog(parent)
{
	InitClass( tr("Status"), tr("In progress..."), tr(""), tr("Terminate?"), true, parent );
}

CStatusWindow::CStatusWindow(const QString &_sWindowTitle, const QString &_sText, const QString &_sQuestion, bool _bStopEnabled, QWidget *parent /*= 0 */)
	: QDialog(parent)
{
	InitClass( _sWindowTitle, _sText, tr(""), _sQuestion, _bStopEnabled, parent );
}

CStatusWindow::~CStatusWindow()
{

}

void CStatusWindow::SetWindowTitle(const QString &_sTitle)
{
	m_sWindowTitle = _sTitle;
	this->setWindowTitle( _sTitle );
}

void CStatusWindow::SetText(const QString &_sText)
{
	m_sText = _sText;
	SetMessage();
}

void CStatusWindow::SetFileName(const QString &_sFileName)
{
	m_sFileName = _sFileName;
	SetMessage();
}

void CStatusWindow::SetConfirmQuestion(const QString &_sQuestion)
{
	m_sQuestion = _sQuestion;
}

void CStatusWindow::SetStopEnable(bool _bEnable)
{
	m_bStopEnabled = _bEnable;
	ui.buttonStop->setEnabled( _bEnable );
}

void CStatusWindow::InitClass(const QString &_sWindowTitle, const QString &_sText, const QString &_sFileName, const QString &_sQuestion, bool _bStopEnabled, QWidget *parent /*= 0 */)
{
	ui.setupUi(this);

	m_sText = _sText;
	m_sFileName = _sFileName;
	m_sQuestion = _sQuestion;

	SetWindowTitle( _sWindowTitle );

	SetMessage();

	SetStopEnable( _bStopEnabled );

	InitializeConnections();

	//move(parentWidget()->window()->frameGeometry().topLeft() + parentWidget()->window()->rect().center() - rect().center());
}

void CStatusWindow::InitializeConnections()
{
	connect( ui.buttonStop, SIGNAL(clicked()), this, SLOT(StopButtonPressed()) );
}

bool CStatusWindow::GetConfirmation()
{
	if( !m_bStopEnabled )
		return false;

	QMessageBox box( QMessageBox::Question, tr("Termination"), m_sQuestion, QMessageBox::Yes | QMessageBox::No, this );
	box.setDefaultButton( QMessageBox::No );
	int ans = box.exec();
	return ans == QMessageBox::Yes;
}

void CStatusWindow::SetMessage()
{
	QString sText;
	if( !m_sFileName.isEmpty() )
	{
		QFileInfo fi( m_sFileName );
		sText += "<b>" + fi.fileName() + "</b><br/>";
	}
	sText += m_sText;
	ui.label->setText( sText );
}

void CStatusWindow::closeEvent(QCloseEvent *event)
{
	if( GetConfirmation() )
		emit StopRequested();

	event->ignore();
}

void CStatusWindow::StopButtonPressed()
{
	close();
}
