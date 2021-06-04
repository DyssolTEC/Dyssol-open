/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <QTableWidget>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QRadioButton>
#include <QToolButton>

class CQtTable : public QTableWidget
{
	Q_OBJECT
public:
	CQtTable(QWidget* parent = nullptr);
	CQtTable(int rows, int columns, QWidget* parent = nullptr);

	std::pair<int, int> GetGeometry() const;
	void SetGeometry(int _rows, int _cols);

	QString GetColHeaderItem(int _col) const;
	QString GetRowHeaderItem(int _row) const;
	std::vector<QString> GetColHeaderItems(int _startcol) const;
	std::vector<QString> GetRowHeaderItems(int _startrow) const;

	void SetColHeaderItem(int _col, const std::string& _text);
	void SetRowHeaderItem(int _row, const std::string& _text);
	void SetColHeaderItems(int _startcol, const std::vector<std::string>& _text);
	void SetRowHeaderItems(int _startrow, const std::vector<std::string>& _text);

	QString GetItem(int _row, int _col) const;
	std::vector<QString> GetItemsCol(int _startrow, int _col) const;
	std::vector<QString> GetItemsRow(int _row, int _startcol) const;

	void SetItemEditable(int _row, int _col, const QString& _text, const QVariant& _userData = -1);
	void SetItemEditable(int _row, int _col, const std::string& _text, const QVariant& _userData = -1);
	void SetItemEditable(int _row, int _col, double _value, const QVariant& _userData = -1);
	void SetItemEditablePrecise(int _row, int _col, double _value, int _precision, const QVariant& _userData = -1);
	void SetItemEditable(int _row, int _col);

	void SetItemsColEditable(int _startrow, int _col, const std::vector<double>& _val);
	void SetItemsColEditable(int _startrow, int _col, const std::vector<int64_t>& _val);
	void SetItemsColEditable(int _startrow, int _col, const std::vector<uint64_t>& _val);
	void SetItemsColEditable(int _startrow, int _col, const std::vector<std::string>& _val);
	void SetItemsRowEditable(int _row, int _startcol, const std::vector<double>& _val);
	void SetItemsRowEditable(int _row, int _startcol, const std::vector<std::string>& _val);

	void SetItemNotEditable(int _row, int _col, const QString& _text, const QVariant& _userData = -1);
	void SetItemNotEditable(int _row, int _col, const std::string& _text, const QVariant& _userData = -1);
	void SetItemNotEditable(int _row, int _col, const std::wstring& _text, const QVariant& _userData = -1);
	void SetItemNotEditable(int _row, int _col, double _value, const QVariant& _userData = -1);
	void SetItemNotEditable(int _row, int _col);

	void SetItemsColNotEditable(int _startrow, int _col, const std::vector<double>& _val);
	void SetItemsColNotEditable(int _startrow, int _col, const std::string& _val);
	void SetItemsRowNotEditable(int _row, int _startcol, const std::vector<double>& _val);

	QCheckBox* SetCheckBox(int _row, int _col, bool _checked = true);
	QCheckBox* GetCheckBox(int _row, int _col) const;
	void SetCheckBoxChecked(int _row, int _col, bool _checked) const;
	bool GetCheckBoxChecked(int _row, int _col) const;
	QRadioButton* SetRadioButton(int _row, int _col, bool _checked = false);
	QRadioButton* GetRadioButton(int _row, int _col) const;
	void SetRadioButtonChecked(int _row, int _col, bool _checked) const;
	bool GetRadioButtonChecked(int _row, int _col) const;
	QComboBox* SetComboBox(int _row, int _col, const std::vector<QString>& _vNames, const std::vector<QVariant>& _vData, int _iSelected);
	QComboBox* SetComboBox(int _row, int _col, const std::vector<std::string>& _names, const std::vector<std::string>& _data, const std::string& _dataSelected);
	QComboBox* SetComboBox(int _row, int _col, const std::vector<std::string>& _names, const std::vector<size_t>& _data, size_t _dataSelected);
	QComboBox* GetComboBox(int _row, int _col) const;
	QPushButton* SetPushButton(int _row, int _col, const QString& _text);
	QPushButton* GetPushButton(int _row, int _col) const;
	QToolButton* SetToolButton(int _row, int _col, const QString& _text);
	QToolButton* GetToolButton(int _row, int _col) const;
	QLabel* SetLabel(int _row, int _col, const QString& _text);

	void SetRowBackgroundColor(int _row, const QColor& _color) const;
	void SetItemBackgroundColor(int _row, int _col, const QColor& _color) const;
	void SetColBackgroundColor(int _col, const QColor& _color) const;
	void SetBackgroundColor(const QColor& _color) const;

	void SetItemFontItalic(int _row, int _col) const;
	void SetEditable(bool _flag);

	std::pair<int, int> CurrentCellPos() const;
	void RestoreSelectedCell(const std::pair<int, int>& _cellPos);
	void RestoreSelectedCell(int _row, int _col);

	QString GetItemUserData(int _row = -1, int _col = -1) const;

	std::vector<double> GetRowValues(int _row) const;
	std::vector<double> GetColumnValues(int _col) const;

	void ShowRow(int _row, bool _show);
	void ShowCol(int _col, bool _show);

	bool blockSignals(bool _flag);

public slots:
	void keyPressEvent(QKeyEvent *event) override;

private:
	void Clear();
	void Copy();
	void Paste();

signals:
	void CheckBoxStateChanged(int _row, int _col, QCheckBox* _pCheckBox);
	void RadioButtonStateChanged(int _row, int _col, QRadioButton* _radioButton);
	void ComboBoxIndexChanged(int _row, int _col, QComboBox* _pComboBox);
	void PushButtonClicked(int _row, int _col, QPushButton* _pushButton);
	void ToolButtonClicked(int _row, int _col, QToolButton* _pToolButton);
};
