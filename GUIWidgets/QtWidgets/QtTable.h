/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

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

private:
	bool m_pasteAllowed{ true };	///< Allow pasting from the clipboard.
	bool m_addRowsOnPaste{ false }; ///< Allow adding new rows to the table during pasting from the clipboard.
	bool m_blockOnPaste{ true };	///< Block all signals from the table during pasting from the clipboard.
	QString m_numberSeparator { " " };
	QString m_decimalSeparator{ "." };

public:
	CQtTable(QWidget* parent = nullptr);
	CQtTable(int rows, int columns, QWidget* parent = nullptr);

	std::pair<int, int> GetGeometry() const;
	void SetGeometry(int _rows, int _cols);

	void EnablePasting(bool _flag);
	void EnableAddRowsOnPaste(bool _flag);
	/**
	 * \brief Block all signals from the table during pasting from the clipboard.
	 * \param _flag Flag.
	 * \return Previous state of the flag.
	 */
	bool EnableBlockOnPaste(bool _flag);

	QString GetColHeaderItem(int _col) const;
	QString GetRowHeaderItem(int _row) const;
	std::vector<QString> GetColHeaderItems(int _startcol) const;
	std::vector<QString> GetRowHeaderItems(int _startrow) const;

	QTableWidgetItem* SetColHeaderItem(int _col);
	QTableWidgetItem* SetColHeaderItem(int _col, const std::string& _text);
	QTableWidgetItem* SetColHeaderItem(int _col, const std::wstring& _text);
	QTableWidgetItem* SetRowHeaderItem(int _row);
	QTableWidgetItem* SetRowHeaderItem(int _row, const std::string& _text);
	QTableWidgetItem* SetRowHeaderItem(int _row, const std::wstring& _text);
	void SetColHeaderItems(int _startcol, const std::vector<std::string>& _text);
	void SetRowHeaderItems(int _startrow, const std::vector<std::string>& _text);

	QString GetItemText(int _row, int _col) const;
	std::vector<QString> GetItemsTextCol(int _startrow, int _col) const;
	std::vector<QString> GetItemsTextRow(int _row, int _startcol) const;

	/**
	 * \brief Inserts a new cell widget item to the table and makes it editable.
	 * \details Sets the given text and user data to the item.
	 * If the item already exists, it is used to set the new data.
	 * \param _row Row of the cell.
	 * \param _col Column of the cell.
	 * \param _text Text to set.
	 * \param _userData User data to set.
	 * \return Pointer to the cell widget item.
	 */
	QTableWidgetItem* SetItemEditable(int _row, int _col, const QString& _text, const QVariant& _userData = -1);
	QTableWidgetItem* SetItemEditable(int _row, int _col, const std::string& _text, const QVariant& _userData = -1);
	QTableWidgetItem* SetItemEditable(int _row, int _col, double _value, const QVariant& _userData = -1);
	QTableWidgetItem* SetItemEditablePrecise(int _row, int _col, double _value, int _precision, const QVariant& _userData = -1);
	QTableWidgetItem* SetItemEditable(int _row, int _col);

	void SetItemsColEditable(int _startrow, int _col, const std::vector<double>& _val);
	void SetItemsColEditable(int _startrow, int _col, const std::vector<int64_t>& _val);
	void SetItemsColEditable(int _startrow, int _col, const std::vector<uint64_t>& _val);
	void SetItemsColEditable(int _startrow, int _col, const std::vector<std::string>& _val);
	void SetItemsRowEditable(int _row, int _startcol, const std::vector<double>& _val);
	void SetItemsRowEditable(int _row, int _startcol, const std::vector<std::string>& _val);

	void SetItemNotEditable(int _row, int _col, const QString& _text, const QVariant& _userData = -1);
	void SetItemNotEditable(int _row, int _col, const std::string& _text, const QVariant& _userData = -1);
	void SetItemNotEditable(int _row, int _col, const std::string& _text, const std::string& _userData);
	void SetItemNotEditable(int _row, int _col, const std::wstring& _text, const QVariant& _userData = -1);
	void SetItemNotEditable(int _row, int _col, double _value, const QVariant& _userData = -1);
	void SetItemNotEditable(int _row, int _col);

	void SetItemsColNotEditable(int _startrow, int _col, const std::vector<double>& _val);
	void SetItemsColNotEditable(int _startrow, int _col, const std::string& _val);
	void SetItemsColNotEditable(int _startrow, int _col, const std::vector<std::string>& _val);
	void SetItemsRowNotEditable(int _row, int _startcol, const std::vector<double>& _val);

	/**
	 * \brief Find an item by its user data.
	 * \details If multiple items with the same user data exist, the first one is returned.
	 * If nothing fount, nullptr is returned.
	 * \param _userData User data.
	 * \return Pointer to the found item.
	 */
	QTableWidgetItem* FindItem(const std::string& _userData) const;

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
	QComboBox* SetComboBox(int _row, int _col, const std::vector<std::string>& _names, const std::vector<uint32_t>& _data, uint32_t _dataSelected);
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
	void SetItemFontColor(int _row, int _col, const QColor& _color) const;
	void SetEditable(bool _flag);

	[[nodiscard]] std::pair<int, int> GetCurrentCellPos() const;
	void SetCurrentCellPos(const std::pair<int, int>& _cellPos);
	void SetCurrentCellPos(int _row, int _col);

	[[nodiscard]] QVariant GetCurrentItemUserData() const;
	[[nodiscard]] QString GetCurrentItemUserDataQStr() const;
	[[nodiscard]] std::string GetCurrentItemUserDataStr() const;
	[[nodiscard]] QVariant GetItemUserData(int _row, int _col = -1) const;
	[[nodiscard]] QString GetItemUserDataQStr(int _row, int _col = -1) const;
	[[nodiscard]] std::string GetItemUserDataStr(int _row, int _col = -1) const;

	std::vector<double> GetRowValues(int _row) const;
	std::vector<double> GetColumnValues(int _col) const;

	void ShowRow(int _row, bool _show);
	void ShowCol(int _col, bool _show);

	bool blockSignals(bool _flag);

public slots:
	void keyPressEvent(QKeyEvent *event) override;

private:
	void Clear();
	void Copy() const;
	void Paste();

signals:
	/**
	 * \brief Is emitted when pasting from the clipboard starts.
	 * \details Is emitted before any actual action is taken.
	 * \param _row Row where the pasting starts.
	 * \param _col Column where the pasting starts.
	 */
	void PasteStarted(int _row, int _col);
	/**
	 * \brief Is emitted after pasting from the clipboard is done.
	 * \details Is emitted after all actions are taken.
	 * \param _rowBeg Row where the pasting started.
	 * \param _colBeg Column where the pasting started.
	 * \param _rowEnd Row of the last pasted item.
	 * \param _colEnd Column of the last pasted item.
	 */
	void PasteFinished(int _rowBeg, int _colBeg, int _rowEnd, int _colEnd);
	void DataPasted();
	void CheckBoxStateChanged(int _row, int _col, QCheckBox* _pCheckBox);
	void RadioButtonStateChanged(int _row, int _col, QRadioButton* _radioButton);
	void ComboBoxIndexChanged(int _row, int _col, QComboBox* _pComboBox);
	void PushButtonClicked(int _row, int _col, QPushButton* _pushButton);
	void ToolButtonClicked(int _row, int _col, QToolButton* _pToolButton);
};
