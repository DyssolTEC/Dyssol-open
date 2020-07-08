/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <QTreeWidget>
#include <QComboBox>

class CQtTree : public QTreeWidget
{
	Q_OBJECT

public:
	CQtTree(QWidget *parent = nullptr);

	QTreeWidgetItem* AddItem(int _col, const std::string& _text, const QVariant& _userData = -1);
	static QTreeWidgetItem* AddChildItem(QTreeWidgetItem* _parent, int _col, const std::string& _text, const QVariant& _userData = -1);

	QComboBox* AddChildItemComboBox(QTreeWidgetItem* _parent, int _col, const std::vector<QString>& _names, const std::vector<QVariant>& _userData, int _iSelected);

	void SetCurrentItem(const std::vector<size_t>& _indices);

	bool blockSignals(bool _flag);
	bool eventFilter(QObject* _object, QEvent* _event) override;

signals:
	void ComboBoxIndexChanged(QComboBox* _combo, QTreeWidgetItem* _item);
};
