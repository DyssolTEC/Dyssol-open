/* Copyright (c) 2023, DyssolTEC.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "QtDialog.h"
#include "DyssolStringConstants.h"
#include <QKeyEvent>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDesktopServices>

void CDyssolBaseWidget::SetPointers(CModelsManager* _modelsManager, QSettings* _settings)
{
	m_modelsManager = _modelsManager;
	m_settings = _settings;
}

CModelsManager* CDyssolBaseWidget::GetModelsManager() const
{
	return m_modelsManager;
}

QSettings* CDyssolBaseWidget::GetSettings() const
{
	return m_settings;
}


CQtDialog::CQtDialog(QWidget* _parent)
	: CQtDialog{ _parent, Qt::WindowFlags() }
{
}

CQtDialog::CQtDialog(QWidget* _parent, Qt::WindowFlags _flags)
	: QDialog{ _parent, _flags }
{
	// disable context menu
	setContextMenuPolicy(Qt::NoContextMenu);
}

void CQtDialog::SetHelpLink(const QString& _helpLink)
{
	m_helpLink = _helpLink;
}

QString CQtDialog::GetHelpLink()
{
	return m_helpLink;
}

void CQtDialog::OpenHelp(const QString& _link)
{
	if (_link.isEmpty())
	{
		QMessageBox::information(this, StrConst::Dyssol_MainWindowName, "No help file available.");
		return;
	}

	// try to open like remote url from web
	QDesktopServices::openUrl(QUrl(StrConst::Dyssol_HelpURL + _link));
}