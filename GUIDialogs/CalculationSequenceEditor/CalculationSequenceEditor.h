/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_CalculationSequenceEditor.h"

class CCalculationSequence;
class CFlowsheet;

class CCalculationSequenceEditor: public QDialog
{
	Q_OBJECT

	// Describes a selection in the tree table.
	struct SSelection
	{
		enum class EType { PARTITION, MODEL, STREAM, UNKNOWN };
		size_t partition; // Index of a partition.
		EType type;       // PARTITION, MODEL or STREAM.
		size_t index;     // Index of a model or stream within a partition or a partition itself.
	};

	Ui::CCalculationSequenceEditorClass ui;

	CFlowsheet* m_pFlowsheet;          // Pointer to the flowsheet.
	CCalculationSequence* m_pSequence; // Pointer to the calculation sequence.

public:
	CCalculationSequenceEditor(CFlowsheet* _pFlowsheet, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

	void InitializeConnections() const;

public slots:
	void setVisible(bool _bVisible) override;
	void UpdateWholeView() const;

private slots:
	void AddItem();    // Add new item depending on the current selection.
	void RemoveItem(); // Remove an item depending on the current selection.
	void UpItem();     // Move an item upwards depending on the current selection.
	void DownItem();   // Move an item downwards depending on the current selection.

	void ChangeItem(const QComboBox* _combo, QTreeWidgetItem* _item); // Change unique key of an item.

	void CalculateSequence(); // Recalculate calculation sequence.

private:
	void UpdatePartitionsList() const; // Update the list of partitions.

	void AddPartition();                                    // Add new partition.
	void RemovePartition(size_t _iPartition);               // Remove partition.
	void UpPartition(size_t _iPartition);                   // Move selected partition upwards.
	void DownPartition(size_t _iPartition);                 // Move selected partition downwards.

	void AddModel(size_t _iPartition);                      // Add new model to the specified partition.
	void RemoveModel(size_t _iPartition, size_t _iModel);   // Remove model from the specified partition.
	void UpModel(size_t _iPartition, size_t _iModel);       // Move selected model upwards.
	void DownModel(size_t _iPartition, size_t _iModel);     // Move selected model downwards.

	void AddStream(size_t _iPartition);                     // Add new tear stream to the specified partition.
	void RemoveStream(size_t _iPartition, size_t _iStream); // Remove tear stream from the specified partition.
	void UpStream(size_t _iPartition, size_t _iStream);     // Move selected tear stream upwards.
	void DownStream(size_t _iPartition, size_t _iStream);   // Move selected tear stream downwards.

	void SetNewModel(size_t _iPartition, size_t _iModel, const std::string& _key);   // Set new model key to the selected model in the specified partition.
	void SetNewStream(size_t _iPartition, size_t _iStream, const std::string& _key); // Set new stream key to the selected tear stream in the specified partition.

	SSelection ParseSelection(QTreeWidgetItem* _item) const;
	size_t SelectedPartition(QTreeWidgetItem* _item) const;       // Returns index of a partition for a given item.
	SSelection::EType SelectedType(QTreeWidgetItem* _item) const; // Returns type of a given item (PARTITION/STREAM/MODEL).
	size_t SelectedIndex(QTreeWidgetItem* _item) const;           // Returns index of a model or stream within a partition.

signals:
	void DataChanged();
};
