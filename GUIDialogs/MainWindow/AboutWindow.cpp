/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

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
	m_headerTeamName = MakeLink("DyssolTEC GmbH", "https://www.dyssoltec.com/") + " and contributors";
	m_headerUpdatesLink = "https://github.com/DyssolTEC/Dyssol-open/releases";

	m_mainDevelopers = { "Vasyl Skorych" };
	m_otherDevelopers = {
		"Anton Gladky",
		"Christian Eichler",
		"Illia Bereza",
		"Lea Rieck",
		"Lusine Shahmuradyan",
		"Maksym Dosta",
		"Moritz Buchholz",
		"Robin Ahrens",
		"Sarra Daknou",
		"Xiye Zhou",
		"Yuliia Tykhonova"
	};

	m_libraries = {
		{ "Breathe"         , "https://www.breathe-doc.org/"                , "Copyright 2009, Michael Jones"                                                         , "Modified BSD license", "https://github.com/breathe-doc/breathe/blob/main/LICENSE"           },
		{ "Doxygen"         , "https://www.doxygen.nl"                      , "Copyright 1997, Dimitri van Heesch"                                                    , "GPL license"         , "https://github.com/doxygen/doxygen/blob/master/LICENSE"             },
		{ "Graphviz"        , "https://graphviz.org"                        , "Copyright 2021, Graphviz contributors"                                                 , "CPL v1.0 license"    , "https://graphviz.org/license/"                                      },
		{ "HDF5"            , "https://www.hdfgroup.org/solutions/hdf5/"    , "Copyright 2018, The HDF Group"                                                         , "BSD-like license"    , "https://github.com/HDFGroup/hdf5/?tab=License-1-ov-file#readme"     },
		{ "Inno Setup"      , "http://www.jrsoftware.org/isinfo.php"        , "Copyright 2020, Jordan Russell"                                                        , "Modified BSD license", "https://jrsoftware.org/files/is/license.txt"                        },
		{ "KISS FFT"        , "https://github.com/mborgerding/kissfft"      , "Copyright 2010, Mark Borgerding"                                                       , "BSD-3-Clause license", "https://github.com/mborgerding/kissfft/blob/master/COPYING"         },
		{ "Qt"              , "http://www.qt.io/"                           , "Copyright 2020, The Qt Company"                                                        , "LGPL v3 license"     , "https://doc.qt.io/qt-5/lgpl.html"                                   },
		{ "Sphinx"          , "https://www.sphinx-doc.org/en/master/"       , "Copyright 2007, Sphinx team"                                                           , "BSD-2-Clause license", "https://github.com/sphinx-doc/sphinx/blob/master/LICENSE"           },
		{ "sphinx_rtd_theme", "https://sphinx-rtd-theme.readthedocs.io"     , "Copyright 2013, Dave Snider, Read the Docs, Inc. & contributors"                       , "MIT license"         , "https://github.com/readthedocs/sphinx_rtd_theme/blob/master/LICENSE"},
		{ "SUNDIALS"        , "https://computing.llnl.gov/projects/sundials", "Copyright 2019, Lawrence Livermore National Security and Southern Methodist University", "BSD-3-Clause license", "https://computing.llnl.gov/projects/sundials/license"               },
		{ "zlib"            , "http://www.zlib.net/"                        , "Copyright 1995, Jean-loup Gailly and Mark Adler"                                       , "zlib license"        , "https://www.zlib.net/zlib_license.html"                             },
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
		"Check for updates " + MakeLink("here", m_headerUpdatesLink) + "<br/>"
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
	text.append(StyledString("Main developer:"));
	for (const auto& name : m_mainDevelopers)
		text.append(StyledString("- " + name));
	text.append(StyledString("<br />"));
	text.append(StyledString("Other developers and contributors:"));
	for (const auto& name : m_otherDevelopers)
		text.append(StyledString("- " + name));
	text.append(StyledString("and others"));

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
			+ MakeLink(lib.name, lib.link) + "<br />"
			+ lib.text + "<br />"
			+ MakeLink(lib.licenseName, lib.licenseLink) + "</p>");

	ui.textBrowserLibraries->setHtml(text);
	QTextCursor textCursor = ui.textBrowserLibraries->textCursor();
	textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
	ui.textBrowserLibraries->setTextCursor(textCursor);
}

QString CAboutWindow::MakeLink(const QString& _text, const QString& _url)
{
	return "<a href=\"" + _url + "\"><span style=\" text-decoration: underline; color:#0000ff;\">" + _text + "</span></a>";
}
