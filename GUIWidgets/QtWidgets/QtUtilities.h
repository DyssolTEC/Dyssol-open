/* Copyright (c) 2023, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include <QApplication>
#include <QClipboard>
#include <vector>

/**
 * Parses pasted text as a table of doubles.
 */
inline std::vector<std::vector<double>> ParseClipboardAsDoubles()
{
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
		// split by columns
		QStringList data = rows[irow].split(QRegExp("[\t ]"));
		// remove trailing whitespaces
		while (!data.empty() && data.back().size() == 0)
			data.pop_back();
		// parse data
		for (int icol = 0; icol < data.length(); ++icol)
			res[irow].push_back(data[icol].toDouble());
	}
	return res;
}
