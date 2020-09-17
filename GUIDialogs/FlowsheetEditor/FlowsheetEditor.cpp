/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define NOMINMAX
#include "FlowsheetEditor.h"
#include "Stream.h"
#include "DyssolStringConstants.h"
#include <sstream>
#include <QMessageBox>

CFlowsheetEditor::CFlowsheetEditor(CFlowsheet *_pFlowsheet, QWidget *parent /*= 0 */)
	: QWidget(parent),
	m_pFlowsheet(_pFlowsheet),
	m_pSelectedModel(nullptr),
	m_pSelectedStream(nullptr)
{
	ui.setupUi(this);
}

void CFlowsheetEditor::InitializeConnections()
{
	connect(ui.buttonAddModel,		&QPushButton::clicked,					this, &CFlowsheetEditor::AddModel);
	connect(ui.buttonDeleteModel,	&QPushButton::clicked,					this, &CFlowsheetEditor::DeleteModel);
	connect(ui.buttonUpModel,		&QPushButton::clicked,					this, &CFlowsheetEditor::UpModel);
	connect(ui.buttonDownModel,		&QPushButton::clicked,					this, &CFlowsheetEditor::DownModel);
	connect(ui.listModels,			&QTableWidget::cellChanged,				this, &CFlowsheetEditor::ChangeModelName);
	connect(ui.listModels,			&QTableWidget::itemSelectionChanged,	this, &CFlowsheetEditor::ChangeSelectedModel);

	connect(ui.buttonAddStream,		&QPushButton::clicked,					this, &CFlowsheetEditor::AddStream);
	connect(ui.buttonDeleteStream,	&QPushButton::clicked,					this, &CFlowsheetEditor::DeleteStream);
	connect(ui.buttonUpStream,		&QPushButton::clicked,					this, &CFlowsheetEditor::UpStream);
	connect(ui.buttonDownStream,	&QPushButton::clicked,					this, &CFlowsheetEditor::DownStream);
	connect(ui.listStreams,			&QTableWidget::cellChanged,				this, &CFlowsheetEditor::ChangeStreamName);
	connect(ui.listStreams,			&QTableWidget::itemSelectionChanged,	this, &CFlowsheetEditor::ChangeSelectedStream);

	connect(ui.comboUnits,			QOverload<int>::of(&QComboBox::currentIndexChanged),	this, &CFlowsheetEditor::ChangeUnitInModel);

	connect(ui.tablePorts,          &CQtTable::ComboBoxIndexChanged,        this, &CFlowsheetEditor::NewPortStreamSelected);

	connect(ui.buttonAddTDValue,	&QPushButton::clicked,					this, &CFlowsheetEditor::AddUnitParamTimePoint);
	connect(ui.buttonRemoveTDValue,	&QPushButton::clicked,					this, &CFlowsheetEditor::DeleteUnitParamTimePoint);
	connect(ui.tableTDValues,		&QTableWidget::itemChanged,				this, &CFlowsheetEditor::TDValueChanged);

	connect(ui.tableUnitParams,		&QTableWidget::itemSelectionChanged,	this, &CFlowsheetEditor::NewUnitParameterSelected);
	connect(ui.tableUnitParams,		&QTableWidget::cellChanged,				this, &CFlowsheetEditor::UnitParamValueChanged);
	connect(ui.tableUnitParams,		&CQtTable::ComboBoxIndexChanged,		this, &CFlowsheetEditor::UnitParamValueChanged);
	connect(ui.tableUnitParams,		&CQtTable::CheckBoxStateChanged,		this, &CFlowsheetEditor::UnitParamValueChanged);
}

void CFlowsheetEditor::setVisible(bool _bVisible)
{
	QWidget::setVisible(_bVisible);
	if (_bVisible)
		UpdateWholeView();
}

void CFlowsheetEditor::UpdateWholeView()
{
	UpdateModelsView();
	UpdateStreamsView();
	UpdateAvailableUnits();
}

void CFlowsheetEditor::UpdateAvailableUnits() const
{
	QSignalBlocker blocker(ui.comboUnits);

	ui.comboUnits->clear();

	const std::vector<SUnitDescriptor> units = m_pFlowsheet->GetModelsManager()->GetAvailableUnits();
	for (int i = 0; i < static_cast<int>(units.size()); ++i)
		ui.comboUnits->insertItem(i, QString::fromStdString(units[i].name), QString::fromStdString(units[i].uniqueID));

	// sort units in combobox alphabetically
	ui.comboUnits->model()->sort(0);

	UpdateUnitCombo();
}

void CFlowsheetEditor::UpdateAvailableSolvers() const
{
	UpdateUnitParamTable();
}

void CFlowsheetEditor::AddModel()
{
	CBaseModel* model = m_pFlowsheet->AddModel();
	model->SetModelName(StrConst::FE_UnitDefaultName + std::to_string(m_pFlowsheet->GetModelsCount()));

	UpdateModelsView();
	QSignalBlocker blocker(ui.listModels);
	ui.listModels->setCurrentItem(ui.listModels->item(ui.listModels->rowCount() - 1, 0));
	ChangeSelectedModel();

	emit ModelsChanged();
	emit DataChanged();
}

void CFlowsheetEditor::DeleteModel()
{
	if (!m_pSelectedModel) return;

	m_pFlowsheet->DeleteModel(m_pSelectedModel->GetModelKey());

	UpdateModelsView();
	UpdateUnitCombo();
	UpdatePortsView();
	UpdateUnitParamTable();
	UpdateUnitParamDescr();
	UpdateTDValuesTable();

	emit ModelsChanged();
	emit DataChanged();
}

void CFlowsheetEditor::UpModel()
{
	if (!m_pSelectedModel || ui.listModels->currentRow() <= 0) return;
	QSignalBlocker blocker(ui.listModels);
	m_pFlowsheet->ShiftModelUp(m_pSelectedModel->GetModelKey());
	ui.listModels->setCurrentItem(ui.listModels->item(ui.listModels->currentRow() - 1, 0));
	UpdateModelsView();
	emit DataChanged();
}

void CFlowsheetEditor::DownModel()
{
	if (!m_pSelectedModel || ui.listModels->currentRow() >= ui.listModels->rowCount() - 1) return;
	QSignalBlocker blocker(ui.listModels);
	m_pFlowsheet->ShiftModelDown(m_pSelectedModel->GetModelKey());
	ui.listModels->setCurrentItem(ui.listModels->item(ui.listModels->currentRow() + 1, 0));
	UpdateModelsView();
	emit DataChanged();
}

void CFlowsheetEditor::AddStream()
{
	CStream*stream = m_pFlowsheet->AddStream();
	stream->SetName(StrConst::FE_StreamDefaultName + std::to_string(m_pFlowsheet->GetStreamsCount()));

	UpdateStreamsView();
	QSignalBlocker blocker(ui.listStreams);
	ui.listStreams->setCurrentItem(ui.listStreams->item(ui.listStreams->rowCount() - 1, 0));
	ChangeSelectedStream();
	UpdatePortsView();

	emit StreamsChanged();
	emit DataChanged();
}

void CFlowsheetEditor::DeleteStream()
{
	if (!m_pSelectedStream) return;

	m_pFlowsheet->DeleteStream(m_pSelectedStream->GetKey());

	UpdateStreamsView();
	UpdatePortsView();

	emit StreamsChanged();
	emit DataChanged();
}

void CFlowsheetEditor::UpStream()
{
	if (!m_pSelectedStream || ui.listStreams->currentRow() <= 0) return;
	m_pFlowsheet->ShiftStreamUp(m_pSelectedStream->GetKey());
	UpdateStreamsView();
	UpdatePortsView();
	ui.listStreams->setCurrentItem(ui.listStreams->item(ui.listStreams->currentRow() - 1, 0));
	emit DataChanged();
}

void CFlowsheetEditor::DownStream()
{
	if (!m_pSelectedStream || ui.listStreams->currentRow() >= ui.listStreams->rowCount() - 1) return;
	m_pFlowsheet->ShiftStreamDown(m_pSelectedStream->GetKey());
	UpdateStreamsView();
	UpdatePortsView();
	ui.listStreams->setCurrentItem(ui.listStreams->item(ui.listStreams->currentRow() + 1, 0));
	emit DataChanged();
}

void CFlowsheetEditor::ChangeSelectedStream()
{
	const QTableWidgetItem *pItem = ui.listStreams->currentItem();
	m_pSelectedStream = pItem ? m_pFlowsheet->GetStream(pItem->data(Qt::UserRole).toString().toStdString()) : nullptr;
}

void CFlowsheetEditor::ChangeSelectedModel()
{
	const QTableWidgetItem *pItem = ui.listModels->currentItem();
	m_pSelectedModel = pItem ? m_pFlowsheet->GetModel(pItem->data(Qt::UserRole).toString().toStdString()) : nullptr;
	m_pModelParams = m_pSelectedModel ? m_pSelectedModel->GetUnitParametersManager() : nullptr;

	EnableGUIElements();
	UpdateUnitCombo();
	UpdatePortsView();
	UpdateUnitParamDescr();
	UpdateTDValuesTable();
	UpdateUnitParamTable();
}

void CFlowsheetEditor::ChangeUnitInModel(int _index)
{
	if (!m_pSelectedModel) return;

	// set new unit
	m_pSelectedModel->SetUnit(ui.comboUnits->itemData(_index).toString().toStdString());
	m_pFlowsheet->InitializeModel(m_pSelectedModel->GetModelKey());
	m_pFlowsheet->SetTopologyModified(true);
	m_pModelParams = m_pSelectedModel->GetUnitParametersManager();

	UpdatePortsView();
	UpdateUnitParamDescr();
	UpdateTDValuesTable();
	UpdateUnitParamTable();

	emit UnitChanged();
	emit DataChanged();
}

void CFlowsheetEditor::ChangeModelName(int _iRow, int _iCol)
{
	if (!m_pSelectedModel) return;

	m_pSelectedModel->SetModelName(ui.listModels->item(_iRow, _iCol)->text().simplified().toStdString());

	UpdateModelsView();

	emit ModelsChanged();
	emit DataChanged();
}

void CFlowsheetEditor::ChangeStreamName(int _iRow, int _iCol)
{
	if (!m_pSelectedStream) return;

	m_pSelectedStream->SetName(ui.listStreams->item(_iRow, _iCol)->text().simplified().toStdString());

	UpdateStreamsView();
	UpdatePortsView();

	emit StreamsChanged();
	emit DataChanged();
}

void CFlowsheetEditor::NewPortStreamSelected(int _iRow, int _iCol, QComboBox* _comboBox)
{
	if (!m_pSelectedModel) return;
	m_pSelectedModel->SetPortStreamKey(_iRow, _comboBox->currentData(Qt::UserRole).toString().toStdString());
	m_pFlowsheet->SetTopologyModified(true);
	emit DataChanged();
}

void CFlowsheetEditor::NewUnitParameterSelected() const
{
	UpdateUnitParamDescr();
	UpdateTDValuesTable();
}

void CFlowsheetEditor::AddUnitParamTimePoint()
{
	if (!m_pModelParams) return;

	// get selected parameter
	auto* param = dynamic_cast<CTDUnitParameter*>(m_pModelParams->GetParameter(ui.tableUnitParams->GetItemUserData(ui.tableUnitParams->currentRow(), 0).toInt()));
	if (!param) return;

	// add new time point
	if (param->IsEmpty()) // add the first time point
		param->SetValue(0, 0);
	else                  // add next as (lastTime + 1, lastValue)
		param->SetValue(param->GetTimes().back() + 1, param->GetValues().back());

	UpdateUnitParamTable();
	UpdateTDValuesTable();

	emit DataChanged();
}

void CFlowsheetEditor::DeleteUnitParamTimePoint()
{
	if (!m_pModelParams) return;

	// get selected parameter
	auto* param = dynamic_cast<CTDUnitParameter*>(m_pModelParams->GetParameter(ui.tableUnitParams->GetItemUserData(ui.tableUnitParams->currentRow(), 0).toInt()));
	if (!param) return;

	if (param->Size() < 2)
	{
		QMessageBox::information(this, StrConst::Dyssol_ApplicationName, StrConst::FE_TDRemoveLast);
		return;
	}

	// get selected time point
	const int iTP = ui.tableTDValues->currentRow();

	if (iTP != -1) // remove selected
		param->RemoveValue(param->GetTimes()[iTP]);
	else           // remove last
		param->RemoveValue(param->GetTimes().back());

	UpdateUnitParamTable();
	UpdateTDValuesTable();

	emit DataChanged();
}

void CFlowsheetEditor::TDValueChanged()
{
	if (!m_pModelParams) return;

	// get selected parameter
	auto* param = dynamic_cast<CTDUnitParameter*>(m_pModelParams->GetParameter(ui.tableUnitParams->GetItemUserData(ui.tableUnitParams->currentRow(), 0).toInt()));
	if (!param) return;

	const int iRow = ui.tableTDValues->currentRow();
	if (ui.tableTDValues->currentColumn() == 0) // change time
		param->RemoveValue(param->GetTimes()[iRow]);
	param->SetValue(ui.tableTDValues->GetItem(iRow, 0).toDouble(), ui.tableTDValues->GetItem(iRow, 2).toDouble());

	UpdateUnitParamTable();
	UpdateTDValuesTable();

	emit DataChanged();
}

void CFlowsheetEditor::UnitParamValueChanged(int _row, int _col)
{
	if (!m_pModelParams) return;

	// get selected parameter
	auto* param = m_pModelParams->GetParameter(ui.tableUnitParams->GetItemUserData(_row, 0).toInt());
	if (!param) return;

	switch (param->GetType())
	{
	case EUnitParameter::CONSTANT:
		dynamic_cast<CConstRealUnitParameter*>(param)->SetValue(ui.tableUnitParams->GetItem(_row, _col).toDouble());
		break;
	case EUnitParameter::TIME_DEPENDENT:
		dynamic_cast<CTDUnitParameter*>(param)->SetValue(dynamic_cast<CTDUnitParameter*>(param)->GetTimes().front(), ui.tableUnitParams->GetItem(_row, _col).toDouble());
		break;
	case EUnitParameter::STRING:
		dynamic_cast<CStringUnitParameter*>(param)->SetValue(ui.tableUnitParams->GetItem(_row, _col).toStdString());
		break;
	case EUnitParameter::CHECKBOX:
	{
		const QCheckBox* checkbox = ui.tableUnitParams->GetCheckBox(_row, _col);
		dynamic_cast<CCheckBoxUnitParameter*>(param)->SetChecked(checkbox->isChecked());
		break;
	}
	case EUnitParameter::SOLVER:
	{
		const QComboBox* combo = ui.tableUnitParams->GetComboBox(_row, _col);
		const QString solverKey = combo->itemData(combo->currentIndex()).toString();
		dynamic_cast<CSolverUnitParameter*>(param)->SetKey(solverKey.toStdString());
		break;
	}
	case EUnitParameter::COMBO:
	{
		const QComboBox* combo = ui.tableUnitParams->GetComboBox(_row, _col);
		const size_t value = combo->itemData(combo->currentIndex()).toUInt();
		dynamic_cast<CComboUnitParameter*>(param)->SetValue(value);
		break;
	}
	case EUnitParameter::GROUP:
	{
		const QComboBox* combo = ui.tableUnitParams->GetComboBox(_row, _col);
		const size_t value = combo->itemData(combo->currentIndex()).toUInt();
		dynamic_cast<CComboUnitParameter*>(param)->SetValue(value);
		break;
	}
	case EUnitParameter::COMPOUND:
	{
		const QComboBox* combo = ui.tableUnitParams->GetComboBox(_row, _col);
		const QString key = combo->itemData(combo->currentIndex()).toString();
		dynamic_cast<CCompoundUnitParameter*>(param)->SetCompound(key.toStdString());
		break;
	}
	case EUnitParameter::UNKNOWN:
		break;
	}

	UpdateUnitParamTable();
	UpdateTDValuesTable();

	emit DataChanged();
}

void CFlowsheetEditor::UpdateModelsView()
{
	QSignalBlocker blocker(ui.listModels);

	// save last selected model
	const auto oldPos = ui.listModels->CurrentCellPos();

	// update list of models
	ui.listModels->setRowCount(static_cast<int>(m_pFlowsheet->GetModelsCount()));
	for (int i = 0; i < static_cast<int>(m_pFlowsheet->GetModelsCount()); ++i)
		ui.listModels->SetItemEditable(i, 0, m_pFlowsheet->GetModel(i)->GetModelName(), QString::fromStdString(m_pFlowsheet->GetModel(i)->GetModelKey()));

	// restore selection
	ui.listModels->RestoreSelectedCell(oldPos);
	ChangeSelectedModel();
}

void CFlowsheetEditor::UpdateStreamsView()
{
	QSignalBlocker blocker(ui.listStreams);
	const auto oldPos = ui.listStreams->CurrentCellPos();

	ui.listStreams->setRowCount(static_cast<int>(m_pFlowsheet->GetStreamsCount()));
	for (int i = 0; i < static_cast<int>(m_pFlowsheet->GetStreamsCount()); ++i)
		ui.listStreams->SetItemEditable(i, 0, m_pFlowsheet->GetStream(i)->GetName(), QString::fromStdString(m_pFlowsheet->GetStream(i)->GetKey()));

	// restore selection
	ui.listStreams->RestoreSelectedCell(oldPos);
	ChangeSelectedStream();
}

void CFlowsheetEditor::UpdateUnitCombo() const
{
	QSignalBlocker blocker(ui.comboUnits);
	ui.comboUnits->setCurrentIndex(-1);

	if (!m_pSelectedModel) return;

	// set selected unit
	const QString unitKey = QString::fromStdString(m_pFlowsheet->GetModel(m_pSelectedModel->GetModelKey())->GetUnitKey());
	for (int i = 0; i < ui.comboUnits->count(); ++i)
		if (ui.comboUnits->itemData(i).toString() == unitKey)
		{
			ui.comboUnits->setCurrentIndex(i);
			break;
		}
}

void CFlowsheetEditor::UpdatePortsView() const
{
	QSignalBlocker blocker(ui.tablePorts);

	// clear elements
	ui.tablePorts->setRowCount(0);
	ui.labelPorts->setText("Ports");

	if (!m_pSelectedModel) return;

	// update label
	ui.labelPorts->setText("Ports of " + QString::fromStdString(m_pSelectedModel->GetModelName()));

	// get list of ports for current unit
	std::vector<sPortStruct> ports = m_pFlowsheet->GetModel(m_pSelectedModel->GetModelKey())->GetUnitPorts();

	// resize the table
	ui.tablePorts->setRowCount(static_cast<int>(ports.size()));

	// prepare data for combo boxes
	std::vector<QString> streamNames;
	std::vector<QVariant> streamKeys;
	for (size_t i = 0; i < m_pFlowsheet->GetStreamsCount(); ++i)
	{
		streamNames.push_back(QString::fromStdString(m_pFlowsheet->GetStream(i)->GetName()));
		streamKeys.emplace_back(QString::fromStdString(m_pFlowsheet->GetStream(i)->GetKey()));
	}

	// create table with total ports of the unit
	for (int i = 0; i < static_cast<int>(ports.size()); ++i)
	{
		ui.tablePorts->SetItemNotEditable(i, 0, ports[i].sName);
		ui.tablePorts->SetItemNotEditable(i, 1, ports[i].nType == INPUT_PORT ? QString(StrConst::FE_PortTypeInput) : QString(StrConst::FE_PortTypeOutput));
		ui.tablePorts->SetRowBackgroundColor(i, ports[i].nType == INPUT_PORT ? QColor(0, 255, 0, 50) : QColor(255, 216, 0, 50));
		QComboBox* combo = ui.tablePorts->SetComboBox(i, 2, streamNames, streamKeys, static_cast<int>(m_pFlowsheet->GetStreamIndex(ports[i].sStreamKey)));
		combo->setMaxVisibleItems(20);
	}
}

void CFlowsheetEditor::UpdateUnitParamTable() const
{
	QSignalBlocker blocker(ui.tableUnitParams);
	const auto oldPos = ui.tableUnitParams->CurrentCellPos();

	// clear some GUI elements
	ui.tableUnitParams->setRowCount(0);

	if (!m_pModelParams) return;

	int iRow = 0;
	for (int i = 0; i < static_cast<int>(m_pModelParams->ParametersNumber()); ++i)
	{
		if (!m_pModelParams->IsParameterActive(i)) continue; // do not show this parameter

		const CBaseUnitParameter* param = m_pModelParams->GetParameter(i);
		ui.tableUnitParams->insertRow(iRow);
		ui.tableUnitParams->SetItemNotEditable(iRow, 0, param->GetName(), i);

		switch (param->GetType())
		{
		case EUnitParameter::CONSTANT:
		{
			const auto* p = dynamic_cast<const CConstRealUnitParameter*>(param);
			ui.tableUnitParams->SetItemEditable(iRow, 2, p->GetValue());
			if (!p->IsInBounds())
				ui.tableUnitParams->SetItemBackgroundColor(iRow, 2, Qt::red);
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, !p->GetUnits().empty() ? "[" + p->GetUnits() + "]" : std::string{});
			break;
		}
		case EUnitParameter::TIME_DEPENDENT:
		{
			const auto* p = dynamic_cast<const CTDUnitParameter*>(param);
			if (p->Size() == 1)
			{
				ui.tableUnitParams->SetItemEditable(iRow, 2, p->GetValue(p->GetTimes().front()));
				if (!p->IsInBounds())
					ui.tableUnitParams->SetItemBackgroundColor(iRow, 2, Qt::red);
			}
			else
			{
				ui.tableUnitParams->SetItemNotEditable(iRow, 2, QString(StrConst::FE_TDParamMessage));
				ui.tableUnitParams->SetItemBackgroundColor(iRow, 2, Qt::gray);
				ui.tableUnitParams->SetItemFontItalic(iRow, 2);
			}
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, !p->GetUnits().empty() ? "[" + p->GetUnits() + "]" : std::string{});
			break;
		}
		case EUnitParameter::STRING:
		{
			const auto* p = dynamic_cast<const CStringUnitParameter*>(param);
			ui.tableUnitParams->SetItemEditable(iRow, 2, p->GetValue());
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, QString{});
			break;
		}
		case EUnitParameter::CHECKBOX:
		{
			const auto* p = dynamic_cast<const CCheckBoxUnitParameter*>(param);
			ui.tableUnitParams->SetCheckBox(iRow, 2, p->IsChecked());
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, QString{});
			break;
		}
		case EUnitParameter::SOLVER:
		{
			const auto* p = dynamic_cast<const CSolverUnitParameter*>(param);
			// create combo with total list of possible solvers with the specified type
			const std::vector<SSolverDescriptor> solvers = m_pFlowsheet->GetModelsManager()->GetAvailableSolvers();
			std::vector<std::string> names;
			std::vector<std::string> keys;
			for (const auto& s : solvers)
				if (s.solverType == p->GetSolverType()) // selected solver type
				{
					names.push_back(s.name);
					keys.push_back(s.uniqueID);
				}
			ui.tableUnitParams->SetComboBox(iRow, 2, names, keys, p->GetKey());
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, QString{});
			break;
		}
		case EUnitParameter::COMBO:
		case EUnitParameter::GROUP:
		{
			const auto* p = dynamic_cast<const CComboUnitParameter*>(param);
			// create combo with the list of possible values
			ui.tableUnitParams->SetComboBox(iRow, 2, p->GetNames(), p->GetItems(), p->GetValue());
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, QString{});
			break;
		}
		case EUnitParameter::COMPOUND:
		{
			const auto* p = dynamic_cast<const CCompoundUnitParameter*>(param);
			// create combo with total list of possible compounds
			ui.tableUnitParams->SetComboBox(iRow, 2, m_pFlowsheet->GetCompoundsNames(), m_pFlowsheet->GetCompounds(), p->GetCompound());
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, QString{});
			break;
		}
		case EUnitParameter::UNKNOWN:
			break;
		}

		++iRow;
	}

	ui.tableUnitParams->RestoreSelectedCell(oldPos);
}

void CFlowsheetEditor::UpdateTDValuesTable() const
{
	QSignalBlocker blocker(ui.tableTDValues);

	// switch off and clear some GUI elements
	ui.tableTDValues->setRowCount(0);
	ui.frameTDValues->setEnabled(false);

	if (!m_pModelParams) return;
	if (ui.tableUnitParams->currentRow() == -1) return;

	// get selected parameter
	const auto* param = m_pModelParams->GetTDParameter(ui.tableUnitParams->GetItemUserData(ui.tableUnitParams->currentRow(), 0).toInt());
	if (!param) return;

	// enable GUI elements
	ui.frameTDValues->setEnabled(true);

	// create table with parameter values
	ui.tableTDValues->setRowCount(static_cast<int>(param->Size()));
	ui.tableTDValues->SetItemsColEditable(0, 0, param->GetTimes());
	ui.tableTDValues->SetItemsColEditable(0, 2, param->GetValues());
	ui.tableTDValues->SetItemsColNotEditable(0, 1, "[" + param->GetUnits() + "]");

	// check if values are in boundaries
	if (!param->IsInBounds())
		for (int i = 0; i < static_cast<int>(param->Size()); ++i)
			if (param->GetValue(param->GetTimes()[i]) < param->GetMin() || param->GetValue(param->GetTimes()[i]) > param->GetMax())
				ui.tableTDValues->item(i, 2)->setBackground(Qt::red);
}

void CFlowsheetEditor::UpdateUnitParamDescr() const
{
	ui.textParamDescr->clear();

	if (!m_pModelParams) return;
	if (ui.tableUnitParams->currentRow() == -1) return;

	auto* param = m_pModelParams->GetParameter(ui.tableUnitParams->GetItemUserData(ui.tableUnitParams->currentRow(), 0).toInt());
	if (!param) return;

	const EUnitParameter type = param->GetType();

	// compose description text
	QString text = QString::fromStdString(param->GetDescription());
	int extraLines = 0;
	if (type == EUnitParameter::CONSTANT || type == EUnitParameter::TIME_DEPENDENT)
	{
		text += "\n" + QString(StrConst::FE_UnitParamMinVal) + QString::number(type == EUnitParameter::CONSTANT
			? dynamic_cast<CConstRealUnitParameter*>(param)->GetMin()
			: dynamic_cast<CTDUnitParameter*>(param)->GetMin());
		text += "\n" + QString(StrConst::FE_UnitParamMaxVal) + QString::number(type == EUnitParameter::CONSTANT
			? dynamic_cast<CConstRealUnitParameter*>(param)->GetMax()
			: dynamic_cast<CTDUnitParameter*>(param)->GetMax());
		extraLines = 2;
	}

	// resize widget according to text length
	const QString tempText = QString::fromStdString(param->GetDescription());
	extraLines += tempText.split("\n", Qt::SkipEmptyParts).count();
	const QFontMetrics fontMetrics(ui.textParamDescr->font());
	const int fontHeight = fontMetrics.height();
	const int textWidth = fontMetrics.boundingRect(tempText).width();
	const int linesNumber = static_cast<int>(std::ceil(static_cast<double>(textWidth) / ui.textParamDescr->width())) + extraLines;
	const int textHeight = std::max(50, std::min(fontHeight * linesNumber, 400));
	ui.textParamDescr->setMinimumHeight(textHeight);
	ui.textParamDescr->setMaximumHeight(textHeight);

	// set text
	ui.textParamDescr->setText(text);
}

void CFlowsheetEditor::EnableGUIElements() const
{
	const bool bNoUnit = ui.listModels->currentRow() == -1;
	ui.frameUnitParams->setEnabled(!bNoUnit);
	ui.framePorts->setEnabled(!bNoUnit);
	if (bNoUnit)
	{
		ui.tablePorts->clearContents();
		ui.comboUnits->setCurrentIndex(-1);
		ui.textParamDescr->clear();
		ui.tableUnitParams->clearContents();
		ui.tableTDValues->clearContents();
	}
}
