/* Copyright (c) 2023, DyssolTEC.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "QtDialog.h"
#include "DyssolStringConstants.h"
#include <QKeyEvent>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDesktopServices>

void CDyssolBaseWidget::SetChildWidgets(std::initializer_list<CDyssolBaseWidget*> _widgets)
{
	m_childWidgets = _widgets;
}

void CDyssolBaseWidget::AddChildWidget(CDyssolBaseWidget* _widget)
{
	m_childWidgets.push_back(_widget);
}

void CDyssolBaseWidget::SetPointers(CFlowsheet* _flowsheet, CModelsManager* _modelsManager, QSettings* _settings)
{
	m_flowsheet     = _flowsheet;
	m_modelsManager = _modelsManager;
	m_settings = _settings;
	m_settings      = _settings;
	for (auto* w : m_childWidgets)
		w->SetPointers(_flowsheet, _modelsManager, _settings);
	OnPointersSet();
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

void CQtDialog::OnNewFlowsheet()
{
	if (isVisible())
		UpdateWholeView();
}

void CQtDialog::setVisible(bool _flag)
{
	if (_flag)
		UpdateWholeView();
	QDialog::setVisible(_flag);
}

void CQtDialog::keyPressEvent(QKeyEvent* _event)
{
	if (_event->key() == Qt::Key_F1)
		OpenHelp(m_helpLink);
	else
		QDialog::keyPressEvent(_event);
}
