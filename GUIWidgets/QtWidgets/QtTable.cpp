/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "QtTable.h"
#include "QtUtilities.h"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>

CQtTable::CQtTable(QWidget* parent)	: QTableWidget(parent)
{
}

CQtTable::CQtTable(const int rows, const int columns, QWidget* parent) : QTableWidget(rows, columns, parent)
{
}

std::pair<int, int> CQtTable::GetGeometry() const
{
	return { rowCount(), columnCount() };
}

void CQtTable::SetGeometry(int _rows, int _cols)
{
	setRowCount(_rows);
	setColumnCount(_cols);
}

void CQtTable::EnablePasting(bool _flag)
{
	m_pasteEnabled = _flag;
}

QString CQtTable::GetColHeaderItem(int _col) const
{
	if (const auto* itm = horizontalHeaderItem(_col))
		return itm->text();
	return {};
}

QString CQtTable::GetRowHeaderItem(int _row) const
{
	if (const auto* itm = verticalHeaderItem(_row))
		return itm->text();
	return {};
}

std::vector<QString> CQtTable::GetColHeaderItems(int _startcol) const
{
	std::vector<QString> res;
	for (int i = _startcol; i < columnCount(); ++i)
		res.push_back(GetColHeaderItem(i));
	return res;
}

std::vector<QString> CQtTable::GetRowHeaderItems(int _startrow) const
{
	std::vector<QString> res;
	for (int i = _startrow; i < rowCount(); ++i)
		res.push_back(GetRowHeaderItem(i));
	return res;
}

void CQtTable::SetColHeaderItem(int _col, const std::string& _text)
{
	if(horizontalHeaderItem(_col))
		horizontalHeaderItem(_col)->setText(QString::fromStdString(_text));
	else
		setHorizontalHeaderItem(_col, new QTableWidgetItem(QString::fromStdString(_text)));
}

void CQtTable::SetRowHeaderItem(int _row, const std::string& _text)
{
	if (verticalHeaderItem(_row))
		verticalHeaderItem(_row)->setText(QString::fromStdString(_text));
	else
		setVerticalHeaderItem(_row, new QTableWidgetItem(QString::fromStdString(_text)));
}

void CQtTable::SetColHeaderItems(int _startcol, const std::vector<std::string>& _text)
{
	for(int i = 0; i < static_cast<int>(_text.size()); ++i)
		if(_startcol + i < columnCount())
			SetColHeaderItem(_startcol + i, _text[i]);
}

void CQtTable::SetRowHeaderItems(int _startrow, const std::vector<std::string>& _text)
{
	for (int i = 0; i < static_cast<int>(_text.size()); ++i)
		if (_startrow + i < rowCount())
			SetRowHeaderItem(_startrow + i, _text[i]);
}

QString CQtTable::GetItemText(int _row, int _col) const
{
	if (const auto* i = item(_row, _col))
		return i->text();
	return {};
}

std::vector<QString> CQtTable::GetItemsTextCol(int _startrow, int _col) const
{
	std::vector<QString> res;
	for (int i = _startrow; i < rowCount(); ++i)
		res.push_back(GetItemText(i, _col));
	return res;
}

std::vector<QString> CQtTable::GetItemsTextRow(int _row, int _startcol) const
{
	std::vector<QString> res;
	for (int i = _startcol; i < columnCount(); ++i)
		res.push_back(GetItemText(_row, i));
	return res;
}

void CQtTable::SetItemEditable(const int _row, const int _col, const QString& _text, const QVariant& _userData /*= -1*/)
{
	SetItemNotEditable(_row, _col, _text, _userData);
	item(_row, _col)->setFlags(item(_row, _col)->flags() | Qt::ItemIsEditable);
}

void CQtTable::SetItemEditable(int _row, int _col, const std::string& _text, const QVariant& _userData)
{
	SetItemEditable(_row, _col, QString::fromStdString(_text), _userData);
}

void CQtTable::SetItemEditable(int _row, int _col, double _value, const QVariant& _userData)
{
	SetItemEditable(_row, _col, QString::number(_value), _userData);
}

void CQtTable::SetItemEditablePrecise(int _row, int _col, double _value, int _precision, const QVariant& _userData)
{
	SetItemEditable(_row, _col, QString::number(_value, 'g', _precision), _userData);
}

void CQtTable::SetItemEditable(int _row, int _col)
{
	SetItemEditable(_row, _col, QString{});
}

void CQtTable::SetItemsColEditable(int _startrow, int _col, const std::vector<double>& _val)
{
	for (int i = 0; i < static_cast<int>(_val.size()); ++i)
		if (_startrow + i < rowCount())
			SetItemEditable(_startrow + i, _col, _val[i]);
}

void CQtTable::SetItemsColEditable(int _startrow, int _col, const std::vector<int64_t>& _val)
{
	for (int i = 0; i < static_cast<int>(_val.size()); ++i)
		if (_startrow + i < rowCount())
			SetItemEditable(_startrow + i, _col, _val[i]);
}

void CQtTable::SetItemsColEditable(int _startrow, int _col, const std::vector<uint64_t>& _val)
{
	for (int i = 0; i < static_cast<int>(_val.size()); ++i)
		if (_startrow + i < rowCount())
			SetItemEditable(_startrow + i, _col, _val[i]);
}

void CQtTable::SetItemsColEditable(int _startrow, int _col, const std::vector<std::string>& _val)
{
	for (int i = 0; i < static_cast<int>(_val.size()); ++i)
		if (_startrow + i < rowCount())
			SetItemEditable(_startrow + i, _col, _val[i]);
}

void CQtTable::SetItemsRowEditable(int _row, int _startcol, const std::vector<double>& _val)
{
	for (int i = 0; i < static_cast<int>(_val.size()); ++i)
		if (_startcol + i < columnCount())
			SetItemEditable(_row, _startcol + i, _val[i]);
}

void CQtTable::SetItemsRowEditable(int _row, int _startcol, const std::vector<std::string>& _val)
{
	for (int i = 0; i < static_cast<int>(_val.size()); ++i)
		if (_startcol + i < columnCount())
			SetItemEditable(_row, _startcol + i, _val[i]);
}

void CQtTable::SetItemNotEditable(const int _row, const int _col, const QString& _text, const QVariant& _userData /*= -1*/)
{
	const bool bExist = item(_row, _col) != nullptr;
	QTableWidgetItem* pItem = bExist ? item(_row, _col) : new QTableWidgetItem(_text);
	if (bExist)
		pItem->setText(_text);
	pItem->setData(Qt::DisplayRole, _text);
	if (_userData != -1)
		pItem->setData(Qt::UserRole, _userData);
	pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
	if (!bExist)
		setItem(_row, _col, pItem);
}

void CQtTable::SetItemNotEditable(int _row, int _col, const std::string& _text, const QVariant& _userData)
{
	SetItemNotEditable(_row, _col, QString::fromStdString(_text), _userData);
}

void CQtTable::SetItemNotEditable(int _row, int _col, const std::string& _text, const std::string& _userData)
{
	SetItemNotEditable(_row, _col, QString::fromStdString(_text), QString::fromStdString(_userData));
}

void CQtTable::SetItemNotEditable(int _row, int _col, const std::wstring& _text, const QVariant& _userData)
{
	SetItemNotEditable(_row, _col, QString::fromStdWString(_text), _userData);
}

void CQtTable::SetItemNotEditable(int _row, int _col, double _value, const QVariant& _userData)
{
	SetItemNotEditable(_row, _col, QString::number(_value), _userData);
}

void CQtTable::SetItemNotEditable(int _row, int _col)
{
	SetItemNotEditable(_row, _col, QString{});
}

void CQtTable::SetItemsColNotEditable(int _startrow, int _col, const std::vector<double>& _val)
{
	for (int i = 0; i < static_cast<int>(_val.size()); ++i)
		if(_startrow + i < rowCount())
			SetItemNotEditable(_startrow + i, _col, _val[i]);
}

void CQtTable::SetItemsColNotEditable(int _startrow, int _col, const std::string& _val)
{
	for (int i = 0; i < rowCount(); ++i)
		if (_startrow + i < rowCount())
			SetItemNotEditable(_startrow + i, _col, _val);
}

void CQtTable::SetItemsColNotEditable(int _startrow, int _col, const std::vector<std::string>& _val)
{
	for (int i = 0; i < static_cast<int>(_val.size()); ++i)
		if (_startrow + i < rowCount())
			SetItemNotEditable(_startrow + i, _col, _val[i]);
}

void CQtTable::SetItemsRowNotEditable(int _row, int _startcol, const std::vector<double>& _val)
{
	for (int i = 0; i < static_cast<int>(_val.size()); ++i)
		if (_startcol + i < columnCount())
			SetItemNotEditable(_row, _startcol + i, _val[i]);
}

QTableWidgetItem* CQtTable::FindItem(const std::string& _userData) const
{
	if (!model()->index(0, 0).isValid()) return nullptr;
	const QModelIndexList matches = model()->match(model()->index(0, 0), Qt::UserRole, QString::fromStdString(_userData));
	if (matches.isEmpty()) return nullptr;
	const auto index = matches.first();
	return item(index.row(), index.column());
}

QCheckBox* CQtTable::SetCheckBox(const int _row, const int _col, bool _checked /*= true*/)
{
	delete item(_row, _col);
	auto *pWidget = new QWidget(this);
	auto *pCheckBox = new QCheckBox(pWidget);
	auto *pLayout = new QHBoxLayout(pWidget);
	pLayout->addWidget(pCheckBox);
	pLayout->setAlignment(Qt::AlignCenter);
	pLayout->setContentsMargins(0, 0, 0, 0);
	pWidget->setLayout(pLayout);
	pCheckBox->setChecked(_checked);
	pCheckBox->setObjectName("CheckBox");
	connect(pCheckBox, &QCheckBox::stateChanged, this, [=] { CheckBoxStateChanged(_row, _col, pCheckBox); });
	setCellWidget(_row, _col, pWidget);
	return pCheckBox;
}

QCheckBox* CQtTable::GetCheckBox(int _row, int _col) const
{
	return cellWidget(_row, _col)->findChild<QCheckBox*>("CheckBox");
}

void CQtTable::SetCheckBoxChecked(int _row, int _col, bool _checked) const
{
	auto *pCheckBox = cellWidget(_row, _col)->findChild<QCheckBox*>("CheckBox");
	if (!pCheckBox) return;
	QSignalBlocker blocker(pCheckBox);
	pCheckBox->setChecked(_checked);
}

bool CQtTable::GetCheckBoxChecked(int _row, int _col) const
{
	auto *pCheckBox = cellWidget(_row, _col)->findChild<QCheckBox*>("CheckBox");
	if (!pCheckBox) return false;
	return pCheckBox->isChecked();
}

QRadioButton* CQtTable::SetRadioButton(int _row, int _col, bool _checked)
{
	delete item(_row, _col);
	auto* widget = new QWidget{ this };
	auto* radio = new QRadioButton{ widget };
	auto* layout = new QHBoxLayout{ widget };
	layout->addWidget(radio);
	layout->setAlignment(Qt::AlignCenter);
	layout->setContentsMargins(0, 0, 0, 0);
	widget->setLayout(layout);
	radio->setChecked(_checked);
	radio->setObjectName("RadioButton");
	connect(radio, &QRadioButton::toggled, this, [=] { RadioButtonStateChanged(_row, _col, radio); });
	setCellWidget(_row, _col, widget);
	return radio;
}

QRadioButton* CQtTable::GetRadioButton(int _row, int _col) const
{
	return cellWidget(_row, _col)->findChild<QRadioButton*>("RadioButton");
}

void CQtTable::SetRadioButtonChecked(int _row, int _col, bool _checked) const
{
	auto* radio = GetRadioButton(_row, _col);
	if (!radio) return;
	QSignalBlocker blocker(radio);
	radio->setChecked(_checked);
}

bool CQtTable::GetRadioButtonChecked(int _row, int _col) const
{
	auto* radio = GetRadioButton(_row, _col);
	if (!radio) return false;
	return radio->isChecked();
}

QComboBox* CQtTable::SetComboBox(const int _row, const int _col, const std::vector<QString>& _vNames, const std::vector<QVariant>& _vData, int _iSelected)
{
	delete item(_row, _col);
	auto *pComboBox = new QComboBox(this);
	for (size_t i = 0; i < _vNames.size(); ++i)
		pComboBox->insertItem(pComboBox->count(), _vNames[i], i < _vData.size() ? _vData[i] : QVariant());
	pComboBox->setCurrentIndex(_iSelected);
	connect(pComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=] { ComboBoxIndexChanged(_row, _col, pComboBox); });
	setCellWidget(_row, _col, pComboBox);
	return pComboBox;
}

QComboBox* CQtTable::SetComboBox(int _row, int _col, const std::vector<std::string>& _names, const std::vector<std::string>& _data, const std::string& _dataSelected)
{
	if (_names.size() != _data.size()) return nullptr;
	std::vector<QString> names;
	std::vector<QVariant> data;
	int iSelected = -1;
	for (int i = 0; i < static_cast<int>(_names.size()); ++i)
	{
		names.emplace_back(QString::fromStdString(_names[i]));
		data.emplace_back(QString::fromStdString(_data[i]));
		if (_data[i] == _dataSelected)	// this one is selected
			iSelected = i;
	}
	return SetComboBox(_row, _col, names, data, iSelected);
}

QComboBox* CQtTable::SetComboBox(int _row, int _col, const std::vector<std::string>& _names, const std::vector<size_t>& _data, size_t _dataSelected)
{
	if (_names.size() != _data.size()) return nullptr;
	std::vector<QString> names;
	std::vector<QVariant> data;
	int iSelected = -1;
	for (int i = 0; i < static_cast<int>(_names.size()); ++i)
	{
		names.emplace_back(QString::fromStdString(_names[i]));
		data.emplace_back(QVariant::fromValue(_data[i]));
		if (_data[i] == _dataSelected)	// this one is selected
			iSelected = i;
	}
	return SetComboBox(_row, _col, names, data, iSelected);
}

QComboBox* CQtTable::GetComboBox(int _row, int _col) const
{
	return dynamic_cast<QComboBox*>(cellWidget(_row, _col));
}

QPushButton* CQtTable::SetPushButton(int _row, int _col, const QString& _text)
{
	delete item(_row, _col);
	auto* widget = new QWidget(this);
	auto* button = new QPushButton(widget);
	auto* layout = new QHBoxLayout(widget);
	layout->addWidget(button);
	layout->setAlignment(Qt::AlignCenter);
	layout->setContentsMargins(0, 0, 0, 0);
	widget->setLayout(layout);
	button->setText(_text);
	button->setAutoDefault(false);
	button->setObjectName("PushButton");
	connect(button, &QPushButton::clicked, this, [=] { PushButtonClicked(_row, _col, button); });
	setCellWidget(_row, _col, widget);
	return button;
}

QPushButton* CQtTable::GetPushButton(int _row, int _col) const
{
	return cellWidget(_row, _col)->findChild<QPushButton*>("PushButton");
}

QToolButton* CQtTable::SetToolButton(int _row, int _col, const QString& _text)
{
	delete item(_row, _col);
	auto *pWidget = new QWidget(this);
	auto *pToolButton = new QToolButton(pWidget);
	auto *pLayout = new QHBoxLayout(pWidget);
	pLayout->addWidget(pToolButton);
	pLayout->setAlignment(Qt::AlignCenter);
	pLayout->setContentsMargins(0, 0, 0, 0);
	pWidget->setLayout(pLayout);
	pToolButton->setText(_text);
	pToolButton->setObjectName("ToolButton");
	connect(pToolButton, &QToolButton::clicked, this, [=] { ToolButtonClicked(_row, _col, pToolButton); });
	setCellWidget(_row, _col, pWidget);
	return pToolButton;
}

QToolButton* CQtTable::GetToolButton(int _row, int _col) const
{
	return cellWidget(_row, _col)->findChild<QToolButton*>("ToolButton");
}

QLabel* CQtTable::SetLabel(const int _row, const int _col, const QString& _text)
{
	delete item(_row, _col);
	auto *pWidget = new QWidget(this);
	auto *pLabel = new QLabel(pWidget);
	pLabel->setText(_text);
	auto *pLayout = new QHBoxLayout(pWidget);
	pLayout->addWidget(pLabel);
	pLayout->setContentsMargins(2, 2, 2, 2);
	pWidget->setLayout(pLayout);
	setCellWidget(_row, _col, pWidget);
	return pLabel;
}

void CQtTable::SetItemBackgroundColor(const int _row, const int _col, const QColor& _color) const
{
	if (_row < 0 || _row >= rowCount()) return;
	if (_col < 0 || _col >= columnCount()) return;
	if (item(_row, _col))
		item(_row, _col)->setBackground(_color);
}

void CQtTable::SetColBackgroundColor(int _col, const QColor& _color) const
{
	for (int i = 0; i < rowCount(); ++i)
		SetItemBackgroundColor(i, _col, _color);
}

void CQtTable::SetRowBackgroundColor(int _row, const QColor& _color) const
{
	for (int i = 0; i < columnCount(); ++i)
		SetItemBackgroundColor(_row, i, _color);
}

void CQtTable::SetBackgroundColor(const QColor& _color) const
{
	for (int i = 0; i < rowCount(); ++i)
		for (int j = 0; j < columnCount(); ++j)
			SetItemBackgroundColor(i, j, _color);
}

void CQtTable::SetItemFontItalic(int _row, int _col) const
{
	if (_row < 0 || _row >= rowCount()) return;

	QFont font = this->font();
	font.setItalic(true);
	if (_col != -1)
		item(_row, _col)->setFont(font);
	else
		for (int i = 0; i < columnCount(); ++i)
			item(_row, i)->setFont(font);
}

void CQtTable::SetEditable(bool _flag)
{
	if (_flag)
		setEditTriggers(DoubleClicked | EditKeyPressed | AnyKeyPressed);
	else
		setEditTriggers(NoEditTriggers);
}

std::pair<int, int> CQtTable::GetCurrentCellPos() const
{
	return { currentRow(), currentColumn() };
}

void CQtTable::SetCurrentCellPos(const std::pair<int, int>& _cellPos)
{
	SetCurrentCellPos(_cellPos.first, _cellPos.second);
}

void CQtTable::SetCurrentCellPos(int _row, int _col)
{
	if (rowCount() <= 0 || columnCount() <= 0) return;
	setCurrentCell(std::clamp(_row, 0, rowCount() - 1), std::clamp(_col, 0, columnCount() - 1), QItemSelectionModel::SelectCurrent);
}

QString CQtTable::GetCurrentItemUserData() const
{
	if (!currentItem()) return {};
	return currentItem()->data(Qt::UserRole).toString();
}

std::string CQtTable::GetCurrentItemUserDataStr() const
{
	return GetCurrentItemUserData().toStdString();
}

QString CQtTable::GetItemUserData(int _row /*= -1*/, int _col /*= -1*/) const
{
	const int row = _row == -1 ? currentRow() : _row;
	const int col = _col == -1 ? currentColumn() : _col;
	if(const QTableWidgetItem* pItem = item(row, col))
		return pItem->data(Qt::UserRole).toString();
	return {};
}

std::vector<double> CQtTable::GetRowValues(const int _row) const
{
	if (_row >= rowCount()) return std::vector<double>{};
	std::vector<double> res;
	for (int i = 0; i < columnCount(); ++i)
		res.push_back(item(_row, i)->text().toDouble());
	return res;
}

std::vector<double> CQtTable::GetColumnValues(const int _col) const
{
	if (_col >= columnCount()) return std::vector<double>{};
	std::vector<double> res;
	for (int i = 0; i < rowCount(); ++i)
		res.push_back(item(i, _col)->text().toDouble());
	return res;
}

void CQtTable::ShowRow(int _row, bool _show)
{
	if (_show)	showRow(_row);
	else		hideRow(_row);
}

void CQtTable::ShowCol(int _col, bool _show)
{
	if (_show)	showColumn(_col);
	else		hideColumn(_col);
}

bool CQtTable::blockSignals(bool _flag)
{
	QList<QCheckBox*> listCheckBox = findChildren<QCheckBox*>(QString(), Qt::FindChildrenRecursively);
	for (auto& cb : listCheckBox)
		cb->blockSignals(_flag);
	QList<QComboBox*> listComboBox = findChildren<QComboBox*>(QString(), Qt::FindChildrenRecursively);
	for (auto& cb : listComboBox)
		cb->blockSignals(_flag);
	return QTableWidget::blockSignals(_flag);
}

void CQtTable::keyPressEvent(QKeyEvent *event)
{
	if (event == QKeySequence::Delete)
		Clear();
	else if (event->matches(QKeySequence::Copy))
		Copy();
	else if (event->matches(QKeySequence::Paste))
		Paste();
	else
		QTableWidget::keyPressEvent(event);
}

void CQtTable::Clear()
{
	const bool bOldBlock = blockSignals(true);
	QModelIndexList indexes = selectionModel()->selection().indexes();
	for (int i = indexes.front().row(); i <= indexes.back().row(); ++i)
		for (int j = indexes.front().column(); j <= indexes.back().column(); ++j)
			if(item(i, j) && item(i, j)->flags().testFlag(Qt::ItemIsEditable))
				item(i, j)->setText("");
	blockSignals(bOldBlock);
	emit cellChanged(indexes.back().row(), indexes.back().column());
}

void CQtTable::Copy()
{
	QModelIndexList indexes = this->selectionModel()->selection().indexes();
	QString str;

	for (int i = indexes.front().row(); i <= indexes.back().row(); ++i)
	{
		for (int j = indexes.front().column(); j <= indexes.back().column(); ++j)
		{
			if (this->item(i, j))
			{
				str += this->item(i, j)->text();
				if (j != indexes.back().column())
					str += m_numberSeparator;
			}
		}
		if (i != indexes.back().row())
			str += "\n";
	}
	QApplication::clipboard()->setText(str);
}

void CQtTable::Paste()
{
	const QModelIndexList indexes = selectionModel()->selection().indexes();
	const int iFirstRow = indexes.count() ? indexes.at(0).row()    : 0;
	const int iFirstCol = indexes.count() ? indexes.at(0).column() : 0;

	emit PasteInitiated(iFirstRow, iFirstCol);
	if (!m_pasteEnabled) return;

	const bool oldBlock = blockSignals(true);

	const auto data = ParseClipboardAsDoubles();
	int rowMax = static_cast<int>(data.size());
	if (rowMax > rowCount() - iFirstRow)
		rowMax = rowCount() - iFirstRow;
	for (int i = 0; i < rowMax; ++i)
	{
		int colMax = static_cast<int>(data[i].size());
		if (colMax > columnCount() - iFirstCol)
			colMax = columnCount() - iFirstCol;
		for (int j = 0; j < colMax; ++j)
			if (item(i, j + iFirstCol) && item(i, j + iFirstCol)->flags().testFlag(Qt::ItemIsEditable))
				item(i + iFirstRow, j + iFirstCol)->setText(QString::number(data[i][j]));
	}

	blockSignals(oldBlock);
	emit DataPasted();
	emit cellChanged(rowMax - 1 + iFirstRow, static_cast<int>(data[rowMax - 1].size()) - 1 + iFirstCol);
}
