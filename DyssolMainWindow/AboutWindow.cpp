/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "AboutWindow.h"
#include "DyssolStringConstants.h"
#include "DyssolSystemDefines.h"
#include <QDesktopServices>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QScrollBar>

CAboutWindow::CAboutWindow(QWidget* parent)	: QDialog(parent)
{
	ui.setupUi(this);

	SetTitle();
	SetDescription();
	SetLicense();
	InitializeConnections();
}

void CAboutWindow::InitializeConnections() const
{
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &CAboutWindow::accept);
}

void CAboutWindow::SetTitle()
{
	setWindowTitle(StrConst::Dyssol_AboutTitle);
}

void CAboutWindow::SetDescription() const
{
	const QString description("<b>" + QString(StrConst::Dyssol_AboutDyssolDescr) + "</b><br/>"
		"Version " + CURRENT_VERSION_STR + "<br/>"
		"Build " + CURRENT_BUILD_VERSION + "<br/>"
		"<br/>"
		"Copyright " + QString::number(QDate::currentDate().year()) + ", Dyssol Development Team<br/>"
		"<br/>"
		"Models Creator: " + VISUAL_STUDIO_VERSION + " (" + ARCHITECTURE + ") <br/>"
		"<br/>"
		"Check for updates at <a href=\"" + QString(StrConst::Dyssol_AboutUpdateLink) + "\">" + QString(StrConst::Dyssol_AboutUpdateLinkView) + "</a><br/>");

	ui.labelText->setText(description);
}

void CAboutWindow::SetLicense() const
{
	QFile file(":/Resources/License");
	file.open(QIODevice::ReadOnly);
	ui.textBrowserLicense->append(QTextStream{ &file }.readAll());
	QTextCursor textCursor = ui.textBrowserLicense->textCursor();
	textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
	ui.textBrowserLicense->setTextCursor(textCursor);
	file.close();
}
