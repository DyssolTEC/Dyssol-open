/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <QCheckBox>
#include <QTreeWidget>
#include <QComboBox>

namespace QtTreeUtils
{
	enum class EFlag : uint8_t
	{
		DEFAULT = 1 << 0,
		EDIT = 1 << 1,
		NO_EDIT = 1 << 2,
		SELECT = 1 << 3,
		NO_SELECT = 1 << 4,
		ENABLED = 1 << 5,
		DISABLED = 1 << 6,
	};

	constexpr EFlag operator|(EFlag _lhs, EFlag _rhs)
	{
		using type = std::underlying_type_t<EFlag>;
		return static_cast<EFlag>(static_cast<type>(_lhs) | static_cast<type>(_rhs));
	}

	constexpr EFlag operator&(EFlag _lhs, EFlag _rhs)
	{
		using type = std::underlying_type_t<EFlag>;
		return static_cast<EFlag>(static_cast<type>(_lhs) & static_cast<type>(_rhs));
	}

	/**
	 * \brief Checks if the group of flags contains the target one.
	 * \param _group Group of flags to search in.
	 * \param _flag Target flag to search for.
	 * \return True if \p _group contains \p _flag.
	 */
	constexpr bool Contains(EFlag _group, EFlag _flag)
	{
		return (_group & _flag) == _flag;
	}
}

class CQtTree : public QTreeWidget
{
	Q_OBJECT

public:
	using EFlags = QtTreeUtils::EFlag;
	inline static constexpr auto ItemSelectableEditable = (EFlags::EDIT | EFlags::SELECT);
	inline static constexpr auto ItemSelectableNotEditable = (EFlags::NO_EDIT | EFlags::SELECT);
	inline static constexpr auto ItemSelectableEnabledNotEditable = (EFlags::NO_EDIT | EFlags::SELECT | EFlags::ENABLED);
	inline static constexpr auto ItemNotSelectabledNotEditable = (EFlags::NO_EDIT | EFlags::NO_SELECT);

	CQtTree(QWidget* _parent = nullptr);

	// Creates a new item as a child of the tree.
	QTreeWidgetItem* CreateItem(EFlags _flags = EFlags::DEFAULT);
	// Creates a new item as a child of the widget.
	static QTreeWidgetItem* CreateItem(QTreeWidgetItem* _parent, EFlags _flags = EFlags::DEFAULT);
	// Creates a new item as a child of the tree and sets the given text and data to the specified column.
	QTreeWidgetItem* CreateItem(int _col, const std::string& _text, EFlags _flags = EFlags::DEFAULT, const QVariant& _data = -1);
	// Creates a new item as a child of the tree and sets the given text and data to the specified column.
	static QTreeWidgetItem* CreateItem(QTreeWidget* _parent, int _col, const std::string& _text, EFlags _flags = EFlags::DEFAULT, const QVariant& _data = -1);
	// Creates a new item as a child of the widget and sets the given text and data to the specified column.
	static QTreeWidgetItem* CreateItem(QTreeWidgetItem* _parent, int _col, const std::string& _text, EFlags _flags = EFlags::DEFAULT, const QVariant& _data = -1);
	// Creates a new item as a child of the widget and sets the given text and data to the specified column.
	static QTreeWidgetItem* CreateItem(QTreeWidgetItem* _parent, int _col, const std::string& _text, EFlags _flags = EFlags::DEFAULT, const std::string& _data = {});

	// Sets new text and data to the selected column of the given existing item.
	static void SetText(QTreeWidgetItem* _item, int _col, const std::string& _text, const QVariant& _data = -1);

	// Adds a combo box to the selected column of the given existing item.
	QComboBox* SetComboBox(QTreeWidgetItem* _item, int _col, const std::vector<QString>& _names, const std::vector<QVariant>& _data, const QVariant& _selected);
	// Configures a combo box widget at the selected column of the given existing item.
	void SetupComboBox(QTreeWidgetItem* _item, int _col, const std::vector<QString>& _names, const std::vector<QVariant>& _data, const QVariant& _selected) const;
	// Configures a combo box widget at the selected column of the given existing item.
	void SetupComboBox(QTreeWidgetItem* _item, int _col, const std::vector<std::string>& _names, const std::vector<std::string>& _data, const std::string& _selected) const;
	// Sets a value of the combo box widget at the selected column of the given existing item.
	void SetComboBoxValue(QTreeWidgetItem* _item, int _col, const QVariant& _value) const;
	// Returns a value of the combo box widget at the selected column of the given existing item.
	QVariant GetComboBoxValue(QTreeWidgetItem* _item, int _col) const;

	// Adds a check box widget to the selected column of the given existing item.
	QCheckBox* SetCheckBox(QTreeWidgetItem* _item, int _col, const QString& _text, bool _value);
	// Sets a value of the check box widget at the selected column of the given existing item.
	void SetCheckBoxValue(QTreeWidgetItem* _item, int _col, bool _value) const;
	// Returns a value of the check box widget at the selected column of the given existing item.
	bool GetCheckBoxValue(QTreeWidgetItem* _item, int _col) const;

	// Sets flags to the given item.
	static void SetItemFlags(QTreeWidgetItem* _item, EFlags _flags);

	// Sets an item with the specified index as current.
	void SetCurrentItem(const std::vector<size_t>& _indices);
	// Sets an item with the specified user data as current.
	void SetCurrentItem(const QVariant& _data);
	// Returns an item with the specified user data.
	QTreeWidgetItem* GetItem(const QVariant& _data) const;

	// Returns user data of the given column in the specified item.
	static QVariant GetData(const QTreeWidgetItem* _item, int _col = 0);
	// Returns user data of the given column in the specified item.
	static QString GetDataQStr(const QTreeWidgetItem* _item, int _col = 0);
	// Returns user data of the given column in the specified item.
	static std::string GetDataStr(const QTreeWidgetItem* _item, int _col = 0);
	// Returns user data of the given column in current item.
	QString GetCurrentDataQStr(int _col = 0) const;

	/**
	 * \brief Check if one item is a successor of another.
	 * \param _parent Parent item.
	 * \param _child Testing child item.
	 * \return Whether child is a successor of the parent.
	 */
	static bool IsSuccessor(const QTreeWidgetItem* _parent, const QTreeWidgetItem* _child);

	bool blockSignals(bool _flag);
	bool eventFilter(QObject* _object, QEvent* _event) override;

private:
	// Creates a new item as a child of the widget.
	template<typename T>
	static QTreeWidgetItem* CreateItem(T* _parent, int _col, const std::string& _text, EFlags _flags = EFlags::DEFAULT, const QVariant& _data = -1);

signals:
	void ComboBoxIndexChanged(QComboBox* _combo, QTreeWidgetItem* _item);
	void CheckBoxStateChanged(QCheckBox* _check, QTreeWidgetItem* _item);
};
