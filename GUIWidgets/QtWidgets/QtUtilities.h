/* Copyright (c) 2023, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <vector>
#include <locale>

/**
 * Parses pasted text as a table of doubles.
 * Each row contains at least one element.
 */
inline std::vector<std::vector<double>> ParseClipboardAsDoubles()
{
	QString regExp = "[\t; ]";

	// parse pasted text
	const QString text = QApplication::clipboard()->text();
	// split by rows
	QStringList rows = text.split(QRegExp(QLatin1String("\n")));
	// remove trailing whitespaces
	while (!rows.empty() && rows.back().size() == 0)
		rows.pop_back();
	std::vector<std::vector<double>> res(rows.length());
	for (int irow = 0; irow < rows.length(); ++irow)
	{
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 9)
		constexpr auto skipEmptyParts = Qt::SkipEmptyParts;
#else
		constexpr auto skipEmptyParts = QString::SkipEmptyParts;
#endif
		// split by columns
		QStringList data = rows[irow].split(QRegExp(regExp), skipEmptyParts);
		// remove trailing whitespaces
		while (!data.empty() && data.back().size() == 0)
			data.pop_back();
		// parse data
		for (int icol = 0; icol < data.length(); ++icol)
			res[irow].push_back(data[icol].toDouble());
	}
	return res;
}

inline QMessageBox::StandardButton AskYesNoCancel(QWidget* _parent, const std::string& _title, const std::string& _text)
{
	return QMessageBox::question(_parent, _title.c_str(), _text.c_str(), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
}

inline QMessageBox::StandardButton AskYesAllNoAllCancel(QWidget* _parent, const std::string& _title, const std::string& _text)
{
	return QMessageBox::question(_parent, _title.c_str(), _text.c_str(), QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel);
}

inline QMessageBox::StandardButton Notify(QWidget* _parent, const std::string& _title, const std::string& _text)
{
	return QMessageBox::information(_parent, _title.c_str(), _text.c_str(), QMessageBox::Ok);
}