/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "AboutWindow.h"
#include "DyssolSystemDefines.h"
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QScrollBar>

CAboutWindow::CAboutWindow(QWidget* parent)	: QDialog(parent)
{
	ui.setupUi(this);

	m_headerProgramName = "Dyssol: Dynamic Simulation of Solids Processes";
	m_headerTeamName = "Dyssol Development Team";
	m_headerUpdatesLink = "https://github.com/DyssolTEC/Dyssol-open/releases";

	m_mainDevelopers = { "Vasyl Skorych", "Maksym Dosta", "Moritz Buchholz" };
	m_otherDevelopers = { "Anton Gladky", "Illia Bereza", "Lusine Shahmuradyan", "Robin Ahrens", "Xiye Zhou" };

	m_libraries = {
		{ "Qt"        , "http://www.qt.io/"                            , "Copyright 2020, The Qt Company"                                                        , "LGPL v3 license"     , "https://doc.qt.io/qt-5/lgpl.html"                           },
		{ "zlib"      ,	"http://www.zlib.net/"                         , "Copyright 2017, Jean-loup Gailly and Mark Adler"                                       , "zlib license"        , "https://www.zlib.net/zlib_license.html"                     },
		{ "HDF5"      ,	"https://www.hdfgroup.org/solutions/hdf5/"     , "Copyright 2018, The HDF Group"                                                         , "BSD-like license"    , "https://support.hdfgroup.org/ftp/HDF5/releases/COPYING"     },
		{ "SUNDIALS"  ,	"https://computing.llnl.gov/projects/sundials/", "Copyright 2019, Lawrence Livermore National Security and Southern Methodist University", "BSD-3-Clause license", "https://computing.llnl.gov/projects/sundials/license"       },
		{ "KISS FFT"  ,	"https://github.com/mborgerding/kissfft"       , "Copyright 2010, Mark Borgerding"                                                       , "BSD-3-Clause license", "https://github.com/mborgerding/kissfft/blob/master/COPYING" },
		{ "Inno Setup",	"http://www.jrsoftware.org/isinfo.php"         , "Copyright 2020, Jordan Russell"                                                        , "Modified BSD license", "https://jrsoftware.org/files/is/license.txt"                },
		{ "Graphviz"  ,	"https://graphviz.org/"                        , "Copyright 2021, Graphviz contributors"                                                 , "CPL v1.0 license"    , "https://graphviz.org/license/"                              },
	};

	SetHeaderText();
	SetLicense();
	SetContributors();
	SetThirdParties();
	InitializeConnections();
}

void CAboutWindow::InitializeConnections() const
{
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &CAboutWindow::accept);
}

void CAboutWindow::SetHeaderText() const
{
	const QString description(
		"<b>" + m_headerProgramName + "</b><br/>"
		"Version " + CURRENT_VERSION_STR + "<br/>"
		"Build " + CURRENT_BUILD_VERSION + "<br/>"
		"<br/>"
		"Copyright " + QString::number(QDate::currentDate().year()) + ", " + m_headerTeamName + "<br/>"
		"<br/>"
#ifdef _MSC_VER
		"Models Creator: " + VISUAL_STUDIO_VERSION + " (" + ARCHITECTURE + ") <br/>"
		"<br/>"
#endif
		"Check for updates <a href=\"" + m_headerUpdatesLink + "\"><span style=\"text-decoration: underline; color:#0000ff;\">here</span></a><br/>"
	);

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

void CAboutWindow::SetContributors() const
{
	const auto StyledString = [&](const QString& _s)
	{
		return R"(<p style="margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;"><span style="font-size:9pt;">)" + _s + "</span></p>";
	};

	QString text;
	text.append(StyledString("Development Team:"));
	for (const auto& name : m_mainDevelopers)
		text.append(StyledString("- " + name));
	text.append(StyledString("<br />"));
	text.append(StyledString("Other contributors:"));
	for (const auto& name : m_otherDevelopers)
		text.append(StyledString("- " + name));

	ui.textBrowserDevelopers->setHtml(text);
	QTextCursor textCursor = ui.textBrowserDevelopers->textCursor();
	textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
	ui.textBrowserDevelopers->setTextCursor(textCursor);
}

void CAboutWindow::SetThirdParties() const
{
	QString text;
	for (const auto& lib : m_libraries)
		text.append(
			"<p style=\"margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
			"<a href=\"" + lib.link + "\">"
			"<span style=\"text-decoration: underline; color:#0000ff;\">" + lib.name + "</span></a><br />"
			+ lib.text + "<br />"
			"<a href=\"" + lib.licenseLink + "\">"
			"<span style=\" text-decoration: underline; color:#0000ff;\">" + lib.licenseName + "</span></a></p>");

	ui.textBrowserLibraries->setHtml(text);
	QTextCursor textCursor = ui.textBrowserLibraries->textCursor();
	textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
	ui.textBrowserLibraries->setTextCursor(textCursor);
}
