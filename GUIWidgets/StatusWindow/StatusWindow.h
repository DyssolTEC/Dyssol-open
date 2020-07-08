/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_StatusWindow.h"

class CStatusWindow : public QDialog
{
	Q_OBJECT

private:
	bool m_bStopEnabled;
	QString m_sWindowTitle;
	QString m_sText;
	QString m_sFileName;
	QString m_sQuestion;

public:
	CStatusWindow( QWidget *parent = 0 );
	CStatusWindow( const QString &_sWindowTitle, const QString &_sText, const QString &_sQuestion, bool _bStopEnabled, QWidget *parent = 0 );
	~CStatusWindow();

	void SetWindowTitle( const QString &_sTitle );
	void SetText( const QString &_sText );
	void SetFileName( const QString &_sFileName );
	void SetConfirmQuestion( const QString &_sQuestion );
	void SetStopEnable( bool _bEnable );

private:
	Ui::CStatusWindow ui;

	void InitClass( const QString &_sWindowTitle, const QString &_sText, const QString &_sFileName, const QString &_sQuestion, bool _bStopEnabled, QWidget *parent = 0 );
	void InitializeConnections();
	bool GetConfirmation();
	void SetMessage();

public slots:
	void closeEvent(QCloseEvent *event);

private slots:
	void StopButtonPressed();

signals:
	void StopRequested();
};
