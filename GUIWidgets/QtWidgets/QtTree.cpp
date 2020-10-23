/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "QtTree.h"
#include <QEvent>

CQtTree::CQtTree(QWidget *parent) : QTreeWidget(parent)
{
}

QTreeWidgetItem* CQtTree::AddItem(int _col, const std::string& _text, const QVariant& _userData)
{
	auto* item = new QTreeWidgetItem(this);
	item->setData(_col, Qt::UserRole, _userData);
	item->setText(_col, QString::fromStdString(_text));
	return item;
}

QTreeWidgetItem* CQtTree::AddChildItem(QTreeWidgetItem* _parent, int _col, const std::string& _text, const QVariant& _userData)
{
	auto* item = new QTreeWidgetItem(_parent);
	item->setData(_col, Qt::UserRole, _userData);
	item->setText(_col, QString::fromStdString(_text));
	return item;
}

QComboBox* CQtTree::AddChildItemComboBox(QTreeWidgetItem* _parent, int _col, const std::vector<QString>& _names, const std::vector<QVariant>& _userData, const QVariant& _selected)
{
	auto* item = new QTreeWidgetItem(_parent);
	auto* combo = new QComboBox();
	setItemWidget(item, _col, combo);
	int iSelected = -1;
	for (size_t i = 0; i < _names.size(); ++i)
	{
		combo->insertItem(combo->count(), _names[i], i < _userData.size() ? _userData[i] : QVariant());
		if (_userData[i] == _selected)
			iSelected = static_cast<int>(i);
	}
	combo->setCurrentIndex(iSelected);
	combo->installEventFilter(this);
	item->setSizeHint(_col, { combo->sizeHint().width() + 10, combo->sizeHint().height() });
	connect(combo, QOverload<int>::of(&QComboBox::highlighted), this, [=] { setCurrentItem(item); });
	connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=] { ComboBoxIndexChanged(combo, item); });
	return combo;
}

void CQtTree::SetCurrentItem(const std::vector<size_t>& _indices)
{
	if (_indices.empty()) return;
	QTreeWidgetItem* item = topLevelItem(static_cast<int>(_indices[0]) < topLevelItemCount() ? static_cast<int>(_indices[0]) : topLevelItemCount() - 1);
	for (size_t i = 1; i < _indices.size(); ++i)
		item = item->child(static_cast<int>(_indices[i]) < item->childCount() ? static_cast<int>(_indices[i]) : item->childCount() - 1);
	setCurrentItem(item);
}

bool CQtTree::blockSignals(bool _flag)
{
	QList<QComboBox*> listComboBox = findChildren<QComboBox*>(QString(), Qt::FindChildrenRecursively);
	for (auto& cb : listComboBox)
		cb->blockSignals(_flag);
	return QTreeWidget::blockSignals(_flag);
}

bool CQtTree::eventFilter(QObject* _object, QEvent* _event)
{
	if (_event->type() == QEvent::Wheel && dynamic_cast<QComboBox*>(_object))
		return true;
	return QTreeWidget::eventFilter(_object, _event);
}
