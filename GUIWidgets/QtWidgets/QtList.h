/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2024, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <QListWidget>

class CQtList : public QListWidget
{
	Q_OBJECT

public:
	CQtList(QWidget* _parent);

	void AddItemEditable(const std::string& _text, const QVariant& _userData = -1);

	void InsertItemCheckable(int _row, const QString& _text, bool _checked = false, const QVariant& _userData = -1);
	void InsertItemCheckable(int _row, const std::string& _text, bool _checked = false, const QVariant& _userData = -1);
	void AddItemCheckable(const QString& _text, bool _checked = false, const QVariant& _userData = -1);
	void SetItemChecked(int _row, bool _checked) const;
	bool GetItemChecked(int _row) const;

	std::string CurrentItemUserDataStr() const;
	size_t CurrentItemUserDataUInt() const;

	void RestoreCurrentRow(int _row);

	void focusInEvent(QFocusEvent* _e) override;
	void focusOutEvent(QFocusEvent* _e) override;

signals:
	void GotFocus();
	void LostFocus();
};
