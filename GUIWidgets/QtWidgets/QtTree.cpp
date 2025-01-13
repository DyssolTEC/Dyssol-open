/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "QtTree.h"
#include <QEvent>

#include "SignalBlocker.h"


CQtTree::CQtTree(QWidget* _parent)
	: QTreeWidget{ _parent }
{
}

QTreeWidgetItem* CQtTree::CreateItem(EFlags _flags)
{
	return CreateItem<QTreeWidget>(this, -1, "", _flags, -1);
}

QTreeWidgetItem* CQtTree::CreateItem(QTreeWidgetItem* _parent, EFlags _flags)
{
	return CreateItem<QTreeWidgetItem>(_parent, -1, "", _flags, -1);
}

QTreeWidgetItem* CQtTree::CreateItem(int _col, const std::string& _text, EFlags _flags, const QVariant& _data)
{
	return CreateItem<QTreeWidget>(this, _col, _text, _flags, _data);
}

QTreeWidgetItem* CQtTree::CreateItem(QTreeWidget* _parent, int _col, const std::string& _text, EFlags _flags, const QVariant& _data)
{
	return CreateItem<QTreeWidget>(_parent, _col, _text, _flags, _data);
}

QTreeWidgetItem* CQtTree::CreateItem(QTreeWidgetItem* _parent, int _col, const std::string& _text, EFlags _flags, const QVariant& _data)
{
	return CreateItem<QTreeWidgetItem>(_parent, _col, _text, _flags, _data);
}

QTreeWidgetItem* CQtTree::CreateItem(QTreeWidgetItem* _parent, int _col, const std::string& _text, EFlags _flags, const std::string& _data)
{
	return CreateItem<QTreeWidgetItem>(_parent, _col, _text, _flags, QString::fromStdString(_data));
}

void CQtTree::SetText(QTreeWidgetItem* _item, int _col, const std::string& _text, const QVariant& _data)
{
	if (_col < 0) return;
	_item->setText(_col, QString::fromStdString(_text));
	if (_data != -1)
		_item->setData(_col, Qt::UserRole, _data);
}

QComboBox* CQtTree::SetComboBox(QTreeWidgetItem* _item, int _col, const std::vector<QString>& _names, const std::vector<QVariant>& _data, const QVariant& _selected)
{
	auto* combo = new QComboBox{ this };
	int selected = -1;
	for (size_t i = 0; i < _names.size(); ++i)
	{
		combo->addItem(_names[i], i < _data.size() ? _data[i] : QVariant{});
		if (_data[i] == _selected)
			selected = static_cast<int>(i);
	}
	combo->setCurrentIndex(selected);
	combo->installEventFilter(this);
	_item->setSizeHint(_col, { combo->sizeHint().width() + 10, combo->sizeHint().height() });
	connect(combo, QOverload<int>::of(&QComboBox::highlighted), this, [this, _item] { setCurrentItem(_item); });
	connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, combo, _item] { ComboBoxIndexChanged(combo, _item); });
	setItemWidget(_item, _col, combo);
	return combo;
}

void CQtTree::SetupComboBox(QTreeWidgetItem* _item, int _col, const std::vector<QString>& _names, const std::vector<QVariant>& _data, const QVariant& _selected) const
{
	auto* combo = dynamic_cast<QComboBox*>(itemWidget(_item, _col));
	if (!combo) return;
	QSignalBlocker blocker{ combo };
	combo->clear();
	int selected = -1;
	for (size_t i = 0; i < _names.size(); ++i)
	{
		combo->addItem(_names[i], i < _data.size() ? _data[i] : QVariant{});
		if (_data[i] == _selected)
			selected = static_cast<int>(i);
	}
	combo->setCurrentIndex(selected);
}

void CQtTree::SetupComboBox(QTreeWidgetItem* _item, int _col, const std::vector<std::string>& _names, const std::vector<std::string>& _data, const std::string& _selected) const
{
	if (_names.size() != _data.size()) return;
	std::vector<QString> names;
	std::vector<QVariant> data;
	for (int i = 0; i < static_cast<int>(_names.size()); ++i)
	{
		names.emplace_back(QString::fromStdString(_names[i]));
		data.emplace_back(QString::fromStdString(_data[i]));
	}
	SetupComboBox(_item, _col, names, data, QString::fromStdString(_selected));
}

void CQtTree::SetComboBoxValue(QTreeWidgetItem* _item, int _col, const QVariant& _value) const
{
	auto* combo = dynamic_cast<QComboBox*>(itemWidget(_item, _col));
	if (!combo) return;
	QSignalBlocker blocker{ combo };
	for (int i = 0; i < combo->count(); ++i)
		if (combo->itemData(i) == _value)
		{
			combo->setCurrentIndex(i);
			return;
		}
	combo->setCurrentIndex(-1);
}

QVariant CQtTree::GetComboBoxValue(QTreeWidgetItem* _item, int _col) const
{
	const auto* combo = dynamic_cast<QComboBox*>(itemWidget(_item, _col));
	if (!combo) return{};
	return combo->itemData(combo->currentIndex());
}

QCheckBox* CQtTree::SetCheckBox(QTreeWidgetItem* _item, int _col, const QString& _text, bool _value)
{
	auto* check = new QCheckBox{ _text, this };
	QSignalBlocker blocker{ check };
	check->setChecked(_value);
	connect(check, &QCheckBox::stateChanged, this, [this, check, _item] { CheckBoxStateChanged(check, _item); });
	setItemWidget(_item, _col, check);
	return check;
}

void CQtTree::SetCheckBoxValue(QTreeWidgetItem* _item, int _col, bool _value) const
{
	auto* check = dynamic_cast<QCheckBox*>(itemWidget(_item, _col));
	if (!check) return;
	QSignalBlocker blocker{ check };
	check->setChecked(_value);
}

bool CQtTree::GetCheckBoxValue(QTreeWidgetItem* _item, int _col) const
{
	const auto* check = dynamic_cast<QCheckBox*>(itemWidget(_item, _col));
	if (!check) return{};
	return check->isChecked();
}

void CQtTree::SetItemFlags(QTreeWidgetItem* _item, EFlags _flags)
{
	if (QtTreeUtils::Contains(_flags, EFlags::EDIT))		_item->setFlags(_item->flags() | Qt::ItemIsEditable);
	if (QtTreeUtils::Contains(_flags, EFlags::NO_EDIT))		_item->setFlags(_item->flags() & ~Qt::ItemIsEditable);
	if (QtTreeUtils::Contains(_flags, EFlags::SELECT))		_item->setFlags(_item->flags() | Qt::ItemIsSelectable);
	if (QtTreeUtils::Contains(_flags, EFlags::NO_SELECT))	_item->setFlags(_item->flags() & ~Qt::ItemIsSelectable);
	if (QtTreeUtils::Contains(_flags, EFlags::ENABLED))		_item->setFlags(_item->flags() | Qt::ItemIsEnabled);
	if (QtTreeUtils::Contains(_flags, EFlags::DISABLED))	_item->setFlags(_item->flags() & ~Qt::ItemIsEnabled);
}

void CQtTree::SetCurrentItem(const std::vector<size_t>& _indices)
{
	if (_indices.empty()) return;
	QTreeWidgetItem* item = topLevelItem(static_cast<int>(_indices[0]) < topLevelItemCount() ? static_cast<int>(_indices[0]) : topLevelItemCount() - 1);
	for (size_t i = 1; i < _indices.size(); ++i)
		item = item->child(static_cast<int>(_indices[i]) < item->childCount() ? static_cast<int>(_indices[i]) : item->childCount() - 1);
	setCurrentItem(item);
}

void CQtTree::SetCurrentItem(const QVariant& _data)
{
	setCurrentItem(GetItem(_data));
}

QTreeWidgetItem* CQtTree::GetItem(const QVariant& _data) const
{
	for (auto* item : findItems(QString("*"), Qt::MatchWrap | Qt::MatchWildcard | Qt::MatchRecursive))
		for (int i = 0; i < item->columnCount(); ++i)
			if (item->data(i, Qt::UserRole) == _data)
				return item;
	return nullptr;
}

QVariant CQtTree::GetData(const QTreeWidgetItem* _item, int _col)
{
	if (!_item || _col >= _item->columnCount()) return {};
	return _item->data(_col, Qt::UserRole);
}

QString CQtTree::GetDataQStr(const QTreeWidgetItem* _item, int _col)
{
	return GetData(_item, _col).toString();
}

std::string CQtTree::GetDataStr(const QTreeWidgetItem* _item, int _col)
{
	return GetDataQStr(_item, _col).toStdString();
}

QString CQtTree::GetCurrentDataQStr(int _col) const
{
	return GetDataQStr(currentItem(), _col);
}

bool CQtTree::IsSuccessor(const QTreeWidgetItem* _parent, const QTreeWidgetItem* _child)
{
	if (_parent == _child)
		return true;

	for (int i = 0; i < _parent->childCount(); ++i)
	{
		const QTreeWidgetItem* currChild = _parent->child(i);
		if (currChild == _child)
			return true;
		if (CQtTree::IsSuccessor(currChild, _child))
			return true;
	}

	return false;
}

bool CQtTree::blockSignals(bool _flag)
{
	for (auto* w : findChildren<QComboBox*>(QString(), Qt::FindChildrenRecursively)) w->blockSignals(_flag);
	for (auto* w : findChildren<QCheckBox*>(QString(), Qt::FindChildrenRecursively)) w->blockSignals(_flag);
	return QTreeWidget::blockSignals(_flag);
}

bool CQtTree::eventFilter(QObject* _object, QEvent* _event)
{
	if (_event->type() == QEvent::Wheel && dynamic_cast<QComboBox*>(_object))
		return true;
	return QTreeWidget::eventFilter(_object, _event);
}

template <typename T>
QTreeWidgetItem* CQtTree::CreateItem(T* _parent, int _col, const std::string& _text, EFlags _flags, const QVariant& _data)
{
	auto* item = new QTreeWidgetItem{ _parent };
	if (_col != -1 && _data != -1)
		item->setData(_col, Qt::UserRole, _data);
	if (_col != -1 && !_text.empty())
		item->setText(_col, QString::fromStdString(_text));
	if (_flags != EFlags::DEFAULT)
		SetItemFlags(item, _flags);
	return item;
}
