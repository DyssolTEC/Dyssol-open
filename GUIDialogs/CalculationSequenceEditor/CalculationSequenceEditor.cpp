/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "CalculationSequenceEditor.h"
#include "Flowsheet.h"
#include "Stream.h"
#include <QComboBox>
#include <QMessageBox>
#include "DyssolStringConstants.h"

CCalculationSequenceEditor::CCalculationSequenceEditor(CFlowsheet* _pFlowsheet, QWidget* parent, Qt::WindowFlags flags)
	: QDialog(parent, flags),
	m_pFlowsheet(_pFlowsheet),
	m_pSequence(_pFlowsheet->GetCalculationSequence())
{
	ui.setupUi(this);
	ui.treeWidget->setColumnWidth(1, 20);
}

void CCalculationSequenceEditor::InitializeConnections() const
{
	connect(ui.buttonCalculate, &QPushButton::clicked,          this, &CCalculationSequenceEditor::CalculateSequence);

	connect(ui.buttonAdd,       &QPushButton::clicked,          this, &CCalculationSequenceEditor::AddItem);
	connect(ui.buttonRemove,    &QPushButton::clicked,          this, &CCalculationSequenceEditor::RemoveItem);
	connect(ui.buttonUp,        &QPushButton::clicked,          this, &CCalculationSequenceEditor::UpItem);
	connect(ui.buttonDown,      &QPushButton::clicked,          this, &CCalculationSequenceEditor::DownItem);

	connect(ui.treeWidget,      &CQtTree::ComboBoxIndexChanged, this, &CCalculationSequenceEditor::ChangeItem);
}

void CCalculationSequenceEditor::setVisible(bool _bVisible)
{
	QDialog::setVisible(_bVisible);
	if (_bVisible)
		UpdateWholeView();
}

void CCalculationSequenceEditor::UpdateWholeView() const
{
	UpdatePartitionsList();
}

void CCalculationSequenceEditor::UpdatePartitionsList() const
{
	QSignalBlocker blocker(ui.treeWidget);
	ui.treeWidget->clear();

	// prepare data
	std::vector<QString> modelsNames;  // names of all models for combo boxes
	std::vector<QVariant> modelsKeys;  // user data for each model - unique key
	std::vector<QString> streamsNames; // names of all streams for combo boxes
	std::vector<QVariant> streamsKeys; // user data for each stream - unique key
	modelsNames.reserve(m_pFlowsheet->GetUnitsNumber());
	modelsKeys.reserve(m_pFlowsheet->GetUnitsNumber());
	streamsNames.reserve(m_pFlowsheet->GetStreamsNumber());
	streamsKeys.reserve(m_pFlowsheet->GetStreamsNumber());
	for (const auto& unit : m_pFlowsheet->GetAllUnits())
	{
		modelsNames.push_back(QString::fromStdString(unit->GetName()));
		modelsKeys.push_back(QString::fromStdString(unit->GetKey()));
	}
	for (const auto& stream : m_pFlowsheet->GetAllStreams())
	{
		streamsNames.push_back(QString::fromStdString(stream->GetName()));
		streamsKeys.push_back(QString::fromStdString(stream->GetKey()));
	}

	// create tree table
	for (size_t i = 0; i < m_pSequence->PartitionsNumber(); ++i)
	{
		// partition
		QTreeWidgetItem* partitionItem = ui.treeWidget->AddItem(0, StrConst::CSE_Partition + std::to_string(i + 1), i);

		// models
		QTreeWidgetItem* modelsItem = ui.treeWidget->AddChildItem(partitionItem, 0, StrConst::CSE_Models, i);
		for (const auto& model : m_pSequence->PartitionModels(i))
			ui.treeWidget->AddChildItemComboBox(modelsItem, 0, modelsNames, modelsKeys, model ? QString::fromStdString(model->GetKey()) : "");

		// tear streams
		QTreeWidgetItem* streamsItem = ui.treeWidget->AddChildItem(partitionItem, 0, StrConst::CSE_Streams, i);
		for (const auto& stream : m_pSequence->PartitionTearStreams(i))
			ui.treeWidget->AddChildItemComboBox(streamsItem, 0, streamsNames, streamsKeys, stream ? QString::fromStdString(stream->GetKey()) : "");
	}

	ui.treeWidget->expandAll();
	ui.treeWidget->resizeColumnToContents(0);
}

void CCalculationSequenceEditor::CalculateSequence()
{
	m_pFlowsheet->SetStreamsToPorts();
	const std::string err = m_pFlowsheet->CheckPortsConnections();
	if (!err.empty())
	{
		QMessageBox::warning(this, "Error", QString::fromStdString(err));
		return;
	}
	m_pFlowsheet->DetermineCalculationSequence();
	UpdateWholeView();
	emit DataChanged();
}

void CCalculationSequenceEditor::AddItem()
{
	const SSelection selection = ParseSelection(ui.treeWidget->currentItem());
	switch (selection.type)
	{
	case SSelection::EType::PARTITION: AddPartition();                             break;
	case SSelection::EType::MODEL:	   AddModel(selection.partition);              break;
	case SSelection::EType::STREAM:    AddStream(selection.partition);             break;
	case SSelection::EType::UNKNOWN:   if (m_pSequence->IsEmpty()) AddPartition(); break;
	}
}

void CCalculationSequenceEditor::RemoveItem()
{
	const SSelection selection = ParseSelection(ui.treeWidget->currentItem());
	switch (selection.type)
	{
	case SSelection::EType::PARTITION: RemovePartition(selection.partition);               break;
	case SSelection::EType::MODEL:	   RemoveModel(selection.partition, selection.index);  break;
	case SSelection::EType::STREAM:    RemoveStream(selection.partition, selection.index); break;
	case SSelection::EType::UNKNOWN:                                                       break;
	}
}

void CCalculationSequenceEditor::UpItem()
{
	const SSelection selection = ParseSelection(ui.treeWidget->currentItem());
	switch (selection.type)
	{
	case SSelection::EType::PARTITION: UpPartition(selection.partition);               break;
	case SSelection::EType::MODEL:	   UpModel(selection.partition, selection.index);  break;
	case SSelection::EType::STREAM:    UpStream(selection.partition, selection.index); break;
	case SSelection::EType::UNKNOWN:                                                   break;
	}
}

void CCalculationSequenceEditor::DownItem()
{
	const SSelection selection = ParseSelection(ui.treeWidget->currentItem());
	switch (selection.type)
	{
	case SSelection::EType::PARTITION: DownPartition(selection.partition);               break;
	case SSelection::EType::MODEL:	   DownModel(selection.partition, selection.index);  break;
	case SSelection::EType::STREAM:    DownStream(selection.partition, selection.index); break;
	case SSelection::EType::UNKNOWN:                                                     break;
	}
}

void CCalculationSequenceEditor::ChangeItem(const QComboBox* _combo, QTreeWidgetItem* _item)
{
	const SSelection selection = ParseSelection(_item);
	switch (selection.type)
	{
	case SSelection::EType::MODEL:	   SetNewModel(selection.partition, selection.index, _combo->currentData(Qt::UserRole).toString().toStdString());  break;
	case SSelection::EType::STREAM:    SetNewStream(selection.partition, selection.index, _combo->currentData(Qt::UserRole).toString().toStdString()); break;
	case SSelection::EType::PARTITION:                                                                                                                 break;
	case SSelection::EType::UNKNOWN:                                                                                                                   break;
	}
}

void CCalculationSequenceEditor::AddPartition()
{
	m_pSequence->AddPartition({}, {});
	UpdatePartitionsList();
	ui.treeWidget->SetCurrentItem({ m_pSequence->PartitionsNumber() - 1 });
	emit DataChanged();
}

void CCalculationSequenceEditor::RemovePartition(size_t _iPartition)
{
	m_pSequence->DeletePartition(_iPartition);
	UpdatePartitionsList();
	ui.treeWidget->SetCurrentItem({ _iPartition });
	emit DataChanged();
}

void CCalculationSequenceEditor::UpPartition(size_t _iPartition)
{
	m_pSequence->ShiftPartitionUp(_iPartition);
	UpdatePartitionsList();
	ui.treeWidget->SetCurrentItem({ _iPartition > 0 ? _iPartition - 1 : _iPartition });
	emit DataChanged();
}

void CCalculationSequenceEditor::DownPartition(size_t _iPartition)
{
	m_pSequence->ShiftPartitionDown(_iPartition);
	UpdatePartitionsList();
	ui.treeWidget->SetCurrentItem({ _iPartition < m_pSequence->PartitionsNumber() - 1 ? _iPartition + 1 : _iPartition });
	emit DataChanged();
}

void CCalculationSequenceEditor::AddModel(size_t _iPartition)
{
	m_pSequence->AddModel(_iPartition, {});
	UpdatePartitionsList();
	ui.treeWidget->SetCurrentItem({ _iPartition, 0, m_pSequence->ModelsNumber(_iPartition) - 1 });
	emit DataChanged();
}

void CCalculationSequenceEditor::RemoveModel(size_t _iPartition, size_t _iModel)
{
	if (_iModel == static_cast<size_t>(-1)) return;
	m_pSequence->DeleteModel(_iPartition, _iModel);
	UpdatePartitionsList();
	ui.treeWidget->SetCurrentItem({ _iPartition, 0, _iModel });
	emit DataChanged();
}

void CCalculationSequenceEditor::UpModel(size_t _iPartition, size_t _iModel)
{
	if (_iModel == static_cast<size_t>(-1)) return;
	m_pSequence->ShiftModelUp(_iPartition, _iModel);
	UpdatePartitionsList();
	ui.treeWidget->SetCurrentItem({ _iPartition, 0, _iModel > 0 ? _iModel - 1 : _iModel });
	emit DataChanged();
}

void CCalculationSequenceEditor::DownModel(size_t _iPartition, size_t _iModel)
{
	if (_iModel == static_cast<size_t>(-1)) return;
	m_pSequence->ShiftModelDown(_iPartition, _iModel);
	UpdatePartitionsList();
	ui.treeWidget->SetCurrentItem({ _iPartition, 0, _iModel < m_pSequence->ModelsNumber(_iPartition) - 1 ? _iModel + 1 : _iModel });
	emit DataChanged();
}

void CCalculationSequenceEditor::AddStream(size_t _iPartition)
{
	m_pSequence->AddStream(_iPartition, {});
	UpdatePartitionsList();
	ui.treeWidget->SetCurrentItem({ _iPartition, 1, m_pSequence->TearStreamsNumber(_iPartition) - 1 });
	emit DataChanged();
}

void CCalculationSequenceEditor::RemoveStream(size_t _iPartition, size_t _iStream)
{
	if (_iStream == static_cast<size_t>(-1)) return;
	m_pSequence->DeleteStream(_iPartition, _iStream);
	UpdatePartitionsList();
	ui.treeWidget->SetCurrentItem({ _iPartition, 1, _iStream });
	emit DataChanged();
}

void CCalculationSequenceEditor::UpStream(size_t _iPartition, size_t _iStream)
{
	if (_iStream == static_cast<size_t>(-1)) return;
	m_pSequence->ShiftStreamUp(_iPartition, _iStream);
	UpdatePartitionsList();
	ui.treeWidget->SetCurrentItem({ _iPartition, 1, _iStream > 0 ? _iStream - 1 : _iStream });
	emit DataChanged();
}

void CCalculationSequenceEditor::DownStream(size_t _iPartition, size_t _iStream)
{
	if (_iStream == static_cast<size_t>(-1)) return;
	m_pSequence->ShiftStreamDown(_iPartition, _iStream);
	UpdatePartitionsList();
	ui.treeWidget->SetCurrentItem({ _iPartition, 1, _iStream < m_pSequence->TearStreamsNumber(_iPartition) - 1 ? _iStream + 1 : _iStream });
	emit DataChanged();
}

void CCalculationSequenceEditor::SetNewModel(size_t _iPartition, size_t _iModel, const std::string& _key)
{
	m_pSequence->SetModel(_iPartition, _iModel, _key);
	UpdatePartitionsList();
	ui.treeWidget->SetCurrentItem({ _iPartition , 0, _iModel });
	emit DataChanged();
}

void CCalculationSequenceEditor::SetNewStream(size_t _iPartition, size_t _iStream, const std::string& _key)
{
	m_pSequence->SetStream(_iPartition, _iStream, _key);
	UpdatePartitionsList();
	ui.treeWidget->SetCurrentItem({ _iPartition , 1, _iStream });
	emit DataChanged();
}

CCalculationSequenceEditor::SSelection CCalculationSequenceEditor::ParseSelection(QTreeWidgetItem* _item) const
{
	if (!_item) return SSelection{ static_cast<size_t>(-1), SSelection::EType::UNKNOWN, static_cast<size_t>(-1) };
	return SSelection{ SelectedPartition(_item), SelectedType(_item), SelectedIndex(_item) };
}

size_t CCalculationSequenceEditor::SelectedPartition(QTreeWidgetItem* _item) const
{
	QTreeWidgetItem* item = _item;
	while (item)
	{
		const int iPartition = ui.treeWidget->indexOfTopLevelItem(item);
		if (iPartition != -1)
			return iPartition;
		item = item->parent();
	}
	return -1;
}

CCalculationSequenceEditor::SSelection::EType CCalculationSequenceEditor::SelectedType(QTreeWidgetItem* _item) const
{
	const int iPartition = ui.treeWidget->indexOfTopLevelItem(_item);
	if (iPartition != -1)
		return SSelection::EType::PARTITION;

	const QString currentText = _item->text(0);
	if (currentText == StrConst::CSE_Models)
		return SSelection::EType::MODEL;
	if (currentText == StrConst::CSE_Streams)
		return SSelection::EType::STREAM;

	const QString parentText = _item->parent()->text(0);
	if (parentText == StrConst::CSE_Models)
		return SSelection::EType::MODEL;
	if (parentText == StrConst::CSE_Streams)
		return SSelection::EType::STREAM;

	return SSelection::EType::UNKNOWN;
}

size_t CCalculationSequenceEditor::SelectedIndex(QTreeWidgetItem* _item) const
{
	if (ui.treeWidget->indexOfTopLevelItem(_item) != -1 || _item->text(0) == StrConst::CSE_Models || _item->text(0) == StrConst::CSE_Streams)
		return -1;
	return ui.treeWidget->currentIndex().row();
}
